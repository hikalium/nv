#include "nv.h"

int NV_isDebugMode;
int main(int argc, char *argv[])
{
	int i;
	char line[MAX_INPUT_LEN];
	NV_Pointer env, root, lastData;
	// get interpreter args
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0) NV_isDebugMode = 1;
	}
	// init env
NV_E_printMemStat();
	env = NV_E_malloc_type(EEnv);
	NV_Env_setLang(env, NV_allocDefaultLang());
	// main loop
	while(NV_gets(line, sizeof(line)) != NULL){
		root = NV_E_malloc_type(EList);
		//
		NV_tokenize(NV_Env_getLang(env), root, line);
		//
		NV_Env_setAutoPrintValueEnabled(env, 1);
		if(NV_EvaluateSentence(env, root)){
			// Ended with error
			NV_Error("%s\n", "Bad Syntax");
		} else{
			// Ended with Success
			if(NV_Env_getAutoPrintValueEnabled(env)){
				lastData = NV_ListItem_getData(NV_List_getLastItem(root));
				if(!NV_E_isNullPointer(lastData)){
					printf("= ");
					NV_E_convertToContents(NV_Env_getVarRoot(env), &lastData);
					NV_printElement(lastData);
					printf("\n");
				}
			}
		}
		// cleanup current code
		NV_E_free(&root);
		if(NV_Env_getEndFlag(env)) break;
	}
	// cleanup
	NV_E_free(&env);
NV_E_printMemStat();
	return 0;
}

//
// Tokenize
//


void NV_tokenize(NV_Pointer lang, NV_Pointer termRoot, const char *input)
{
	const char *p;
	int i, lastCType, cType;
	char buf[MAX_TOKEN_LEN];
	lastCType = 0;
	p = input;
NV_DbgInfo("%s", "Tokenize begin");
	for(i = 0; ; i++){
		cType = NV_Lang_getCharType(lang, input[i]);
		if(cType != lastCType ||
			cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(input + i - p != 0){
				if((input + i - p) > MAX_TOKEN_LEN){
					NV_Error("%s", "Too long token.");
					exit(EXIT_FAILURE);
				}
				NV_strncpy(buf, p, MAX_TOKEN_LEN, input + i - p);
				NV_tokenizeItem(lang, termRoot, buf);
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
	}
	if(NV_isDebugMode) NV_List_printAll(termRoot, NULL, NULL, "]\n");
NV_DbgInfo("%s", "Tokenize end");
}

void NV_tokenizeItem(NV_Pointer lang, NV_Pointer termRoot, const char *termStr)
{
	NV_Pointer t;
	int32_t tmpNum;
	char *p;
	
	t = NV_Lang_getOperatorFromString(lang, termStr);
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
	NV_E_setFlag(t, EFUnknownToken);
	NV_List_push(termRoot, t);
}
//
// Evaluate
//

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
		t = NV_ListItem_getNext(root);
		for(; !NV_E_isNullPointer(t); t = NV_ListItem_getNext(t)){
			if(NV_ListItem_isDataType(t, EOperator)){
				op = NV_ListItem_getData(t);
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
			t = NV_ListItem_getNext(root);
			for(; !NV_E_isNullPointer(t); t = NV_ListItem_getNext(t)){
				t = NV_TryExecOp(env, targetOpPrec, t, root);
				if(NV_E_isNullPointer(t)){
					NV_DbgInfo("%s", "Evaluate end (Op Mismatched)");
					return 1;
				}
			}
		} else{
			// right-associative
			t = NV_List_getLastItem(root);
			for(; !NV_E_isNullPointer(t); t = NV_ListItem_getPrev(t)){
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
	op = NV_ListItem_getData(thisTerm);
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
				NV_Lang_getFallbackOperator(NV_Env_getLang(env), op);
			if(NV_E_isNullPointer(fallbackOp)){
				NV_Error("%s", "Operator mismatched: ");
				NV_Operator_print(op); putchar('\n');
				NV_List_printAll(root, NULL, NULL, "]\n");
				return NV_NullPointer;
			}
			NV_ListItem_setData(orgTerm, fallbackOp);
			thisTerm = orgTerm;
		}
		if(NV_isDebugMode) NV_List_printAll(root, NULL, NULL, "]\n");
	}
	return thisTerm;
}

