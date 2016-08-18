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
	NV_Env_setLangDef(env, NV_getDefaultLang());

	while(NV_gets(line, sizeof(line)) != NULL){
		NV_tokenize(NV_Env_getLangDef(env), NV_Env_getTermRoot(env), line);
		NV_Evaluate(env);
		if(NV_Env_getEndFlag(env)) break;
	}
	return 0;

/*
	int i;
	NV_Pointer k, v;
	NV_Pointer dict = NV_Dict_allocRoot();
	char s[32];
	for(i = 0; i < 5; i++){
		NV_printElement(dict); printf("\n");	
		v = NV_E_malloc_type(EInteger);
		NV_Integer_setImm32(v, i);
		//
		k = NV_E_malloc_type(EString);
		snprintf(s, sizeof(s), "abc%dxx", i);
		NV_String_setString(k, s);
		//
		NV_Dict_add(dict, k, v);
	}
	NV_printElement(dict); printf("\n");
	//
	k = NV_E_malloc_type(EString);
	//k = NV_E_malloc_type(EInteger);
	for(i = 0; i < 6; i++){
		snprintf(s, sizeof(s), "abc%dxy", i);
		NV_String_setString(k, s);
		//
		v = NV_Dict_getValByKey(dict, k);
		printf("%d -> ", i); NV_printElement(v); printf("\n");
	}
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
					NV_Error("%s", "Too long token.");
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
	if(NV_isDebugMode) NV_List_printAll(termRoot, NULL, NULL, "]\n");
}

void NV_tokenizeItem(NV_LangDef *langDef, NV_Pointer termRoot, const char *termStr)
{
	NV_Pointer t;
	int32_t tmpNum;
	char *p;
	
	t = NV_getOperatorFromString(langDef, termStr);
	if(!NV_E_isNullPointer(t)){
		NV_List_push(termRoot, t);
		return;
	}
	// check Integer
	tmpNum = strtol(termStr, &p, 0);
	if(termStr != p && *p == 0){
		t = NV_E_malloc_type(EInteger);
		NV_Integer_setImm32(t, tmpNum);
		NV_List_push(termRoot, t);
		return;
	}
	// unknown -> string
	t = NV_E_malloc_type(EString);
	NV_String_setString(t, termStr);
	NV_List_push(termRoot, t);
}
//
// Evaluate
//

void NV_resetEvalTree(NV_Pointer root)
{
	NV_Pointer item, data;
	for(;;){
		item = NV_List_getItemByIndex(root, 0);
		if(NV_E_isNullPointer(item)) break;
		data = NV_List_removeItem(item);
		if(NV_E_isType(data, EString) || NV_E_isType(data, EInteger)){
			NV_E_free(&data);
		}
	}
}

void NV_Evaluate(NV_Pointer env)
{
	NV_Pointer termRoot = NV_Env_getTermRoot(env);
	NV_Pointer lastTerm;
	//
	NV_Env_setAutoPrintValueEnabled(env, 1);
	if(NV_EvaluateSentence(env, termRoot)){
		// Ended with error
		NV_Error("%s\n", "Bad Syntax");
	} else{
		// Ended with Success
		if(NV_Env_getAutoPrintValueEnabled(env)){
			lastTerm = NV_List_getItemData(NV_List_lastItem(termRoot));
			if(!NV_E_isNullPointer(lastTerm)){
				printf("= ");
				NV_printElement(lastTerm);
				printf("\n");
			}
		}
	}
	NV_resetEvalTree(termRoot);
	//if(NV_isDebugMode) NV_printVarsInVarSet(NV_Env_getVarSet(env));
}

int NV_EvaluateSentence(NV_Pointer env, NV_Pointer root)
{
	NV_Pointer t;
	NV_Pointer op;
	int targetOpPrec, opPrec;

	if(!NV_E_isType(root, EList)) return 1;

	NV_Env_setEndFlag(env, 0);
	while(!NV_Env_getEndFlag(env)){
		// find op
		targetOpPrec = -1;
		t = NV_List_getNextItem(root);
		for(; !NV_E_isNullPointer(t); t = NV_List_getNextItem(t)){
			if(NV_List_isItemType(t, EOperator)){
				op = NV_List_getItemData(t);
				opPrec = NV_getOperatorPrecedence(op);
				if(opPrec == -1){
					NV_Error("%s", "Internal error: Op not found");
					return 1;
				}
				if(targetOpPrec == -1 || opPrec > targetOpPrec){
					// select max precedence of operator in eval tree
					targetOpPrec = opPrec;
				}
			}
		}
		if(targetOpPrec == -1){
			NV_DbgInfo("%s", "Evaluate end (no more op)");
			return 0;
		}
		if((targetOpPrec & 1) == 0){
			// left-associative
			t = NV_List_getNextItem(root);
			for(; !NV_E_isNullPointer(t); t = NV_List_getNextItem(t)){
				t = NV_TryExecOp(env, targetOpPrec, t, root);
				if(NV_E_isNullPointer(t)){
					NV_DbgInfo("%s", "Evaluate end (Op Mismatched)");
					return 1;
				}
			}
		} else{
			// right-associative
			t = NV_List_lastItem(root);
			for(; !NV_E_isNullPointer(t); t = NV_List_getPrevItem(t)){
				// rewind
				t = NV_TryExecOp(env, targetOpPrec, t, root);
				if(NV_E_isNullPointer(t)){
					NV_DbgInfo("%s", "Evaluate end (Op Mismatched)");
					return 1;
				}
			}
		}
		if(NV_Env_getEndFlag(env)){
			NV_DbgInfo("%s", "Evaluate end (End flag)");
			return 0;
		}
	}
	return 0;
}

NV_Pointer NV_TryExecOp(NV_Pointer env, int currentOpPrec, NV_Pointer thisTerm, NV_Pointer root)
{
	NV_Pointer fallbackOp, op;
	NV_Pointer orgTerm = thisTerm;
	//
	op = NV_List_getItemData(thisTerm);
	if(NV_E_isType(op, EOperator) && 
		NV_getOperatorPrecedence(op) == currentOpPrec){
		if(NV_isDebugMode){
			NV_DbgInfo("%s", "Begin native op: ");
			NV_Operator_print(op); putchar('\n');
		}
		thisTerm = NV_Operator_exec(op, env, thisTerm);
		if(NV_isDebugMode){
			NV_DbgInfo("%s", "End native op:");
			NV_Operator_print(op); putchar('\n');
		}
		if(NV_E_isNullPointer(thisTerm)){
			// try fallback
			fallbackOp = 
				NV_getFallbackOperator(NV_Env_getLangDef(env), op);
			if(NV_E_isNullPointer(fallbackOp)){
				NV_Error("%s", "Operator mismatched: ");
				NV_Operator_print(op); putchar('\n');
				NV_List_printAll(root, NULL, NULL, "]\n");
				return NV_NullPointer;
			}
			NV_List_setItemData(orgTerm, fallbackOp);
			thisTerm = orgTerm;
		}
		if(NV_isDebugMode) NV_List_printAll(root, NULL, NULL, "]\n");
	}
	return thisTerm;
}

