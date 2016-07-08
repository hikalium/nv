#include "nv.h"

int NV_isDebugMode;
int main(int argc, char *argv[])
{
	int i;
	char line[MAX_LINE_LEN];
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0) NV_isDebugMode = 1;
	}

	NV_Env *env = NV_allocEnv();
	env->varSet = NV_allocVariableSet();
	env->langDef = NV_getDefaultLang();
	
	while(fgets(line, sizeof(line), stdin) != NULL){
		NV_tokenize(env, line);
		NV_Evaluate(env);
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
// Varibale
//
NV_Variable *NV_allocVariable(NV_VariableSet *vs)
{
	NV_Variable *t;
	//
	if(vs->varUsed >= MAX_VARS){
		NV_printError("No more variables.", stderr);
		exit(EXIT_FAILURE);
	}
	t = &vs->varList[vs->varUsed++];
	//
	t->name[0] = 0;
	t->type = None;
	t->byteSize = 0;
	t->revision = 0;
	t->data = NULL;
	//
	return t;
}

void NV_resetVariable(NV_Variable *v)
{
	// excludes namestr.
	if(v->type == None) return;
	v->type = None;
	v->byteSize = 0;
	if(v->data) free(v->data);
	v->data = NULL;
}

void NV_assignVariable_Integer(NV_Variable *v, int32_t newVal)
{
	NV_resetVariable(v);
	v->type = Integer;
	v->byteSize = sizeof(int32_t);	// int32s only now.
	v->data = NV_malloc(v->byteSize);
	v->revision++;
	*((int32_t *)v->data) = newVal;
}

void NV_tryConvertTermFromUnknownToImm(NV_VariableSet *vs, NV_Term **term)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(vs, (*term)->data);
	if(var){
		if(var->type == Integer && var->byteSize == sizeof(int32_t)){
			new = NV_createTerm_Imm32(*((int32_t *)var->data));
			NV_overwriteTerm((*term), new);
			*term = new;
			return;
		}
	}
}

void NV_tryConvertTermFromUnknownToVariable(NV_VariableSet *vs, NV_Term **term)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(vs, (*term)->data);
	if(var) new = NV_createTerm_Variable(vs, var->name);
	else new = NV_createTerm_Variable(vs, (*term)->data);
	if(new){
		NV_overwriteTerm((*term), new);
		*term = new;
		return;
	}
}

NV_Variable *NV_getVariableByName(NV_VariableSet *vs, const char *name)
{
	NV_Variable *var;
	int i;
	for(i = 0; i < vs->varUsed; i++){
		var = &vs->varList[i];
		if(strncmp(var->name, name, MAX_TOKEN_LEN) == 0){
			return var;
		}
	}
	if(NV_isDebugMode) printf("NV_getVariableByName: Variable '%s' not found.\n", name);
	return NULL;
}

void NV_printVarsInVarList(NV_VariableSet *vs)
{
	NV_Variable *var;
	int32_t *tmpint32;
	int i;

	printf("Variable Table (%p): %d\n", vs, vs->varUsed);
	for(i = 0; i < vs->varUsed; i++){
		var = &vs->varList[i];
		printf("%s", var->name);
		printf("\t rev: %d", var->revision);
		if(var->type == Integer){
			printf("\t Integer(%d)", var->byteSize);
			if(var->byteSize == sizeof(int32_t)){
				tmpint32 = var->data;
				printf("\t = %d", *tmpint32);
			}
		}
		putchar('\n');
	}
}

//
// Variable Set
//
NV_VariableSet *NV_allocVariableSet()
{
	NV_VariableSet *t;

	t = NV_malloc(sizeof(NV_VariableSet));
	//
	t->varUsed = 0;

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

void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm))
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
// Environment
//

NV_Env *NV_allocEnv()
{
	NV_Env *t;

	t = NV_malloc(sizeof(NV_Env));
	//
	t->langDef = NULL;
	NV_initRootTerm(&t->termRoot);

	return t;
}

//
// Tokenize
//

int NV_getCharType(NV_LangDef *lang, char c)
{
	if(c == '\0'){
		return -1;
	} else if(strchr(lang->char0List, c)){
		// type 0 chars divide tokens but can't be a part of a token.
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
		if(cType != lastCType || cType == 2 || lastCType == 2){
			if(lastCType != 0){
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

int NV_tokenize(NV_Env *env, const char *s)
{
	int i;
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len = 0;
	//
	NV_tokenize0(env->langDef, token0, MAX_TOKENS, &token0Len, s);
	for(i = 0; i < token0Len; i++){
		NV_appendTerm(env->langDef, &env->termRoot, token0[i]);
	}
	if(NV_isDebugMode) NV_printTerms(&env->termRoot);
	//
	return 0;
}

//
// Evaluate
//
void NV_Evaluate(NV_Env *env)
{
	env->autoPrintValue = 1;
	if(NV_EvaluateSentence(env, &env->termRoot)){
		// Ended with error
		printf("Bad Syntax\n");
	} else{
		// Ended with Success
		if(env->autoPrintValue){
			printf("= ");
			NV_printLastTermValue(&env->termRoot);
			printf("\n");
		}
	}
	NV_removeTermTree(&env->termRoot);
	if(NV_isDebugMode) NV_printVarsInVarList(env->varSet);
}

int NV_EvaluateSentence(NV_Env *env, NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op, *currentOp;
	int minOpIndex, opIndex;

	if(!root || !root->next) return 1;

	env->changeFlag = 1;
	while(env->changeFlag){
		env->changeFlag = 0;
		minOpIndex = -1;
		currentOp = NULL;
		for(t = root->next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				opIndex = NV_getOperatorIndex(env->langDef, op);
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
			for(; t != root; t = t->before){
				// rewind
				t = NV_TryExecOp(env, currentOp, t, root);
				if(!t){
					if(NV_isDebugMode) printf("Evaluate end (Operator Mismatched)\n");
					return 1;
				}
			}
		}
	}
	if(NV_isDebugMode) printf("Evaluate end (changeFlas == 0)\n");
	return 0;
}

NV_Term *NV_TryExecOp(NV_Env *env, NV_Operator *currentOp, NV_Term *t, NV_Term *root)
{
	NV_Operator *fallbackOp;
	NV_Term *orgTerm = t;
	if(t->type == Operator && t->data == currentOp){
		t = currentOp->nativeFunc(env, t);
		if(!t){
			// try fallback
			fallbackOp = NV_getFallbackOperator(env->langDef, currentOp);
			if(!fallbackOp){
				NV_printError("Operator mismatched: %s\n", currentOp->name);
				NV_printTerms(root);
				return NULL;
			}
			orgTerm->data = fallbackOp;
			env->changeFlag = 1;
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

