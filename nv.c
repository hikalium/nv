#include "nv.h"

int NV_isDebugMode;
int main(int argc, char *argv[])
{
	int i;
	char line[MAX_INPUT_LEN];
	NV_Pointer env;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0) NV_isDebugMode = 1;
	}

	env = NV_E_malloc_type(EEnv);
	NV_Env_setVarSet(env, NV_allocVariableSet());
	NV_Env_setLangDef(env, NV_getDefaultLang());
	
	while(NV_gets(line, sizeof(line)) != NULL){
		NV_tokenize(env, line);
		NV_Evaluate(env);
		if(NV_Env_getEndFlag(env)) break;
	}
	return 0;
}

//
// LangDef
//

NV_LangDef *NV_allocLangDef()
{
	NV_LangDef *t;

	t = NV_malloc(sizeof(NV_LangDef));
	//
	t->char0Len = 0;
	t->char0List = "";
	t->char1Len = 0;
	t->char1List = "";
	t->char2Len = 0;
	t->char2List = "";
	//
	t->opRoot = NULL;

	return t;
	
}

//
// Operator
//

NV_Operator *NV_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	//
	t->name[0] = 0;
	t->precedence = 0;
	t->next = NULL;
	t->nativeFunc = NULL;

	return t;
}

void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Pointer env, NV_Term *thisTerm))
{
	NV_Operator *t, **p;

	t = NV_allocOperator();
	NV_strncpy(t->name, name, sizeof(t->name), strlen(name));
	t->name[sizeof(t->name) - 1] = 0;
	t->precedence = precedence;
	t->nativeFunc = nativeFunc;
	// op list is sorted in a descending order of precedence.
	for(p = &lang->opRoot; *p; p = &(*p)->next){
		if((*p)->precedence < t->precedence) break;
	}
	t->next = *p;
	*p = t;
}

NV_Operator *NV_isOperator(NV_LangDef *lang, const char *termStr)
{
	NV_Operator *op;
	for(op = lang->opRoot; op != NULL; op = op->next){
		if(strcmp(op->name, termStr) == 0){
			return op;
		}
	}
	return NULL;
}

NV_Operator *NV_getFallbackOperator(NV_LangDef *lang, NV_Operator *baseOp)
{
	NV_Operator *op;
	for(op = lang->opRoot; op != NULL; op = op->next){
		if(op == baseOp){
			op = baseOp->next;
			break;
		}
	}
	for(; op != NULL; op = op->next){
		if(strcmp(op->name, baseOp->name) == 0){
			return op;
		}
	}
	return NULL;
}

int NV_getOperatorIndex(NV_LangDef *lang, NV_Operator *op)
{
	NV_Operator *t;
	int i = 0;
	for(t = lang->opRoot; t; t = t->next){
		if(t == op) return i;
		i++;
	}
	return -1;
}

//
// Tokenize
//

int NV_getCharType(NV_LangDef *lang, char c)
{
	if(c == '\0'){
		return -1;
	} else if(strchr(lang->char0List, c)){
		// type 0 chars divide tokens but can't be a part of a token. (only to disappear)
		return 0;
	} else if(strchr(lang->char1List, c)){
		// sequences of type 1 chars make tokens.
		return 1;
	} else if(strchr(lang->char2List, c)){
		// type 2 chars make token separately.
		return 2;
	}
	// sequences of type 2 chars make tokens.
	return 3;
}

void NV_tokenize0(NV_LangDef *langDef, char (*token0)[MAX_TOKEN_LEN], int token0Len, int *token0Used,  const char *s)
{
	const char *p;
	int i, lastCType, cType;
	lastCType = 0;
	p = s;
	*token0Used = 0;
	for(i = 0; ; i++){
		cType = NV_getCharType(langDef, s[i]);
		if(cType != lastCType || cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(s + i - p != 0){
				if((s + i - p) > MAX_TOKEN_LEN){
					NV_printError("Too long token.\n", stderr);
					exit(EXIT_FAILURE);
				}
				NV_strncpy(token0[*token0Used], p, MAX_TOKEN_LEN, s + i - p);
				(*token0Used)++;
			}
			p = s + i;
		}
		lastCType = cType;
		if(!s[i]) break;
	}
}

int NV_tokenize(NV_Pointer env, const char *s)
{
	NV_Term *termRoot = NV_Env_getTermRoot(env);
	NV_LangDef *langDef = NV_Env_getLangDef(env);
	// 
	int i;
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len = 0;
	//
	NV_tokenize0(langDef, token0, MAX_TOKENS, &token0Len, s);
	for(i = 0; i < token0Len; i++){
		NV_appendTerm(langDef, termRoot, token0[i]);
	}
	if(NV_isDebugMode) NV_printTerms(termRoot);
	//
	return 0;
}

//
// Evaluate
//

void NV_Evaluate(NV_Pointer env)
{
	NV_Term *termRoot = NV_Env_getTermRoot(env);
	//
	NV_Env_setAutoPrintValueEnabled(env, 1);
	if(NV_EvaluateSentence(env, termRoot)){
		// Ended with error
		printf("Bad Syntax\n");
	} else{
		// Ended with Success
		if(NV_Env_getAutoPrintValueEnabled(env) && NV_getLastTerm(termRoot)){
			printf("= ");
			NV_printLastTermValue(termRoot, NV_Env_getVarSet(env));
			printf("\n");
		}
	}
	NV_removeTermTree(termRoot);
	if(NV_isDebugMode) NV_printVarsInVarSet(NV_Env_getVarSet(env));
}

int NV_EvaluateSentence(NV_Pointer env, NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op, *currentOp;
	int minOpIndex, opIndex;

	if(!root) return 1;
	if(!root->next) return 0;	// empty input

	NV_Env_setEndFlag(env, 0);
	while(!NV_Env_getEndFlag(env)){
		minOpIndex = -1;
		currentOp = NULL;
		for(t = root->next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				opIndex = NV_getOperatorIndex(NV_Env_getLangDef(env), op);
				if(opIndex == -1){
					NV_printError("Internal error: Op not found: %s\n", op->name);
					return 1;
				}
				if(minOpIndex == -1 || opIndex < minOpIndex){
					minOpIndex = opIndex;
					currentOp = op;
				}
			}
		}
		if(currentOp == NULL){
			if(NV_isDebugMode) printf("Evaluate end (currentOp is NULL)\n");
			return 0;
		}
		if(NV_isDebugMode) printf("current op: [%s]\n", currentOp->name);
		if((currentOp->precedence & 1) == 0){
			// left-associative
			for(t = root->next; t; t = t->next){
				t = NV_TryExecOp(env, currentOp, t, root);
				if(!t){
					if(NV_isDebugMode) printf("Evaluate end (Operator Mismatched)\n");
					return 1;
				}
			}
		} else{
			// right-associative
			for(t = root; t->next; t = t->next); // skip
			for(; t != root; t = t->prev){
				// rewind
				t = NV_TryExecOp(env, currentOp, t, root);
				if(!t){
					if(NV_isDebugMode) printf("Evaluate end (Operator Mismatched)\n");
					return 1;
				}
			}
		}
		if(NV_Env_getEndFlag(env)){
			if(NV_isDebugMode) printf("Evaluate end (End flag)\n");
			return 0;
		}
	}
	return 0;
}

NV_Term *NV_TryExecOp(NV_Pointer env, NV_Operator *currentOp, NV_Term *t, NV_Term *root)
{
	NV_Operator *fallbackOp;
	NV_Term *orgTerm = t;
	//
	if(t->type == Operator && t->data == currentOp){
		if(NV_isDebugMode) printf("Begin native op: [%s]\n", currentOp->name);
		t = currentOp->nativeFunc(env, t);
		if(NV_isDebugMode) printf("End native op: [%s]\n", currentOp->name);
		if(!t){
			// try fallback
			fallbackOp = NV_getFallbackOperator(NV_Env_getLangDef(env), currentOp);
			if(!fallbackOp){
				NV_printError("Operator mismatched: %s\n", currentOp->name);
				NV_printTerms(root);
				return NULL;
			}
			orgTerm->data = fallbackOp;
			t = orgTerm;
		}
		if(NV_isDebugMode) NV_printTerms(root);
	}
	return t;
}

void NV_printError(const char *format, ...)
{
	va_list args;
	printf("Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

