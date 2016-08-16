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
		NV_tokenize(NV_Env_getLangDef(env), NV_Env_getTermRoot(env), line);
		//NV_Evaluate(env);
		if(NV_Env_getEndFlag(env)) break;
	}
	return 0;
/*

	int i;
	NV_Pointer t;
	NV_Pointer root1 = NV_List_allocRoot();
	char s[32];
	for(i = 0; i < 3; i++){
		t = NV_E_malloc_type(EString);
		snprintf(s, sizeof(s), "abc%dxx", i);
		NV_String_setString(t, s);
		NV_List_push(root1, t);
		NV_List_printAll(root1, ", ");
	}
	
	NV_Pointer root2 = NV_List_allocRoot();
	for(i = 100; i < 105; i++){
		t = NV_E_malloc_type(EInteger);
		NV_Integer_setImm32(t, i);
		NV_List_push(root2, t);
		NV_List_printAll(root2, ", ");
	}
	
	NV_List_insertAllAfterIndex(root1, 4, root2);
	NV_List_printAll(root1, ", ");
	NV_List_printAll(root2, ", ");
*/	
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
	t->opRoot = NV_List_allocRoot();

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
		// (only to disappear)
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

void NV_tokenize(NV_LangDef *langDef, NV_Pointer termRoot, const char *input)
{
	const char *p;
	int i, lastCType, cType;
	char buf[MAX_TOKEN_LEN];
	lastCType = 0;
	p = input;
	for(i = 0; ; i++){
		cType = NV_getCharType(langDef, input[i]);
		if(cType != lastCType ||
			cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(input + i - p != 0){
				if((input + i - p) > MAX_TOKEN_LEN){
					NV_printError("Too long token.\n", stderr);
					exit(EXIT_FAILURE);
				}
				NV_strncpy(buf, p, MAX_TOKEN_LEN, input + i - p);
				NV_tokenizeItem(langDef, termRoot, buf);
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
	}
	if(NV_isDebugMode) NV_List_printAll(termRoot, ", ");
}

void NV_tokenizeItem(NV_LangDef *langDef, NV_Pointer termRoot, const char *termStr)
{
	NV_Pointer t;
	/*
	// check operator
	new = NV_createTerm_Operator(langDef, termStr);
	if(new){
		NV_appendTermRaw(termRoot, new);
		return;
	}
	// check Integer
	tmpNum = strtol(termStr, &p, 0);
	if(termStr != p && *p == 0){
		new = NV_createTerm_Imm32(tmpNum);
		NV_appendTermRaw(termRoot, new);
		return;
	}
	*/
	// unknown -> string
	t = NV_E_malloc_type(EString);
	NV_String_setString(t, termStr);
	NV_List_push(termRoot, t);
}
//
// Evaluate
//
/*
void NV_Evaluate(NV_Pointer env)
{
	NV_Pointer termRoot = NV_Env_getTermRoot(env);
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

int NV_EvaluateSentence(NV_Pointer env, NV_Pointer root)
{
	NV_Pointer t;
	NV_Pointer op, currentOp;
	int minOpIndex, opIndex;

	if(!NV_E_isType(root, EList)) return 1;

	NV_Env_setEndFlag(env, 0);
	while(!NV_Env_getEndFlag(env)){
		minOpIndex = -1;
		currentOp = NV_NullPointer;
		t = NV_List_getNextItem(t);
		for(; !NV_E_isNullPointer(t); t = NV_List_getNextItem(t)){
			if(NV_E_isItemType(t, EOperator)){
				op = NV_List_getItemData(t);
				opIndex = NV_getOperatorIndex(NV_Env_getLangDef(env), op);
				if(opIndex == -1){
					NV_printError("Internal error: Op not found:\n", "");
					return 1;
				}
				if(minOpIndex == -1 || opIndex < minOpIndex){
					minOpIndex = opIndex;
					currentOp = op;
				}
			}
		}
		if(NV_E_isNullPointer(currentOp)){
			if(NV_isDebugMode) printf("Evaluate end (currentOp is NULL)\n");
			return 0;
		}
		//if(NV_isDebugMode) printf("current op: [%s]\n", currentOp->name);
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
*/
void NV_printError(const char *format, ...)
{
	va_list args;
	printf("Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

