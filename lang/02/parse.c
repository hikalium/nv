#include "../../nv.h"

void NV_parseToCodeGraph_infixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName)
{
	NV_ID funcNode = NV_Node_createWithString("infixOp");
	NV_ID op = NV_Node_createWithString(opName);
	NV_ID opL = NV_Array_getByIndex(tokenList, opIndex - 1);
	NV_ID opR = NV_Array_getByIndex(tokenList, opIndex + 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, opIndex - 1);
	NV_Array_removeIndex(tokenList, opIndex - 1);
	NV_Array_writeToIndex(tokenList, opIndex - 1, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
}

void NV_parseToCodeGraph_prefixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName)
{
	NV_ID funcNode = NV_Node_createWithString("prefixOp");
	NV_ID op = NV_Node_createWithString(opName);
	NV_ID opR = NV_Array_getByIndex(tokenList, opIndex + 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, opIndex);
	NV_Array_writeToIndex(tokenList, opIndex, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
}

void NV_parseToCodeGraph_postfixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName)
{
	NV_ID funcNode = NV_Node_createWithString("postfixOp");
	NV_ID op = NV_Node_createWithString(opName);
	NV_ID opR = NV_Array_getByIndex(tokenList, opIndex - 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, opIndex - 1);
	NV_Array_writeToIndex(tokenList, opIndex - 1, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
}

void NV_parseToCodeGraph_if
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	int i, count;
	NV_ID t;
	NV_ID condT = NODEID_NOT_FOUND, doT;
	NV_ID terminateNode = NV_Node_createWithString("endif");

	for(i = 1; ; i++){
		t = NV_Array_getByIndex(tokenList, opIndex + i);
		if(i & 1){
			// 奇数: 条件節、もしくはelse節
			if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
				// 終了
				break;
			}
			condT = t;
		} else{
			// 偶数：実行部分
			if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
				// 直前の節(condT)はelseだった。
				if(!NV_NodeID_isEqual(&condT, &NODEID_NOT_FOUND)){
					NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &condT);
					*lastNode = condT;
				}
				break;
			}
			doT = t;
			//
			condT = NV_parseToCodeGraph(&condT, opDict);
			doT = NV_parseToCodeGraph(&doT, opDict);
			//
			NV_ID func = NV_Node_createWithString("cond");
			NV_ID doFunc = NV_Node_createWithString("do");
			//
			NV_Dict_addUniqueEqKeyByCStr(&func, "flag", &condT);
			NV_Dict_addUniqueEqKeyByCStr(&func, "truePath", &doFunc);
			NV_Dict_addUniqueEqKeyByCStr(&doFunc, "call", &doT);
			//
			NV_Dict_addUniqueEqKeyByCStr(&doFunc, "next", &terminateNode);
			//
			NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &func);
			*lastNode = func;
		}
	}
	count = i;
	for(i = 0; i < count; i++){
		NV_Array_removeIndex(tokenList, opIndex);
	}
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &terminateNode);
	*lastNode = terminateNode;
}

NV_ID NV_parseToCodeGraph_for
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict)
{
	// for {init block}{conditional block}{update block}[{statement}]
	NV_ID initT, condT, updtT, doT;
	NV_ID initF, condF, updtF, doF;
	NV_ID terminateF, t;
	int i;
	for(i = 1; i <= 3; i++){
		t = NV_Array_getByIndex(tokenList, opIndex + i);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
			fprintf(stderr, "too few codeblocks for for.\n");
			return NODEID_NULL;
		}
	}
	initT = NV_Array_getByIndex(tokenList, opIndex + 1);
	condT = NV_Array_getByIndex(tokenList, opIndex + 2);
	updtT = NV_Array_getByIndex(tokenList, opIndex + 3);
	doT   = NV_Array_getByIndex(tokenList, opIndex + 4);
	for(i = 0; i < 4; i++){
		NV_Array_removeIndex(tokenList, opIndex);
	}
	if(!NV_isTermType(&doT, &NODEID_TERM_TYPE_ARRAY)){
		doT = NV_Array_create();
	} else{
		NV_Array_removeIndex(tokenList, opIndex);	
	}
	//
	initT = NV_parseToCodeGraph(&initT, opDict);
	if(NV_NodeID_isEqual(&initT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	condT = NV_parseToCodeGraph(&condT, opDict);
	if(NV_NodeID_isEqual(&condT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	updtT = NV_parseToCodeGraph(&updtT, opDict);
	if(NV_NodeID_isEqual(&updtT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	doT   = NV_parseToCodeGraph(&doT  , opDict);
	if(NV_NodeID_isEqual(&doT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	//
	initF = NV_Node_createWithString("init");
	NV_Dict_addUniqueEqKeyByCStr(&initF, "call", &initT);
	condF = NV_Node_createWithString("cond");
	NV_Dict_addUniqueEqKeyByCStr(&condF, "flag", &condT);
	updtF = NV_Node_createWithString("updt");
	NV_Dict_addUniqueEqKeyByCStr(&updtF, "call", &updtT);
	doF   = NV_Node_createWithString("do");
	NV_Dict_addUniqueEqKeyByCStr(&doF  , "call", &doT);
	terminateF = NV_Node_createWithString("endfor");
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &initF);
	NV_Dict_addUniqueEqKeyByCStr(&initF, "next", &condF);
	NV_Dict_addUniqueEqKeyByCStr(&condF, "next", &terminateF);
	NV_Dict_addUniqueEqKeyByCStr(&condF, "truePath", &doF);
	NV_Dict_addUniqueEqKeyByCStr(&doF, "next", &updtF);
	NV_Dict_addUniqueEqKeyByCStr(&updtF, "next", &condF);
	//
	*lastNode = terminateF;
	return *lastNode;
}

NV_ID NV_parseToCodeGraph(const NV_ID *tokenList, const NV_ID *opDict)
{
	// retv: codeGraphRoot
	NV_ID codeGraphRoot = NV_Node_createWithString("eval");
	NV_ID lastNode = codeGraphRoot, e;
	int opIndex;

	for(;;){
		opIndex = NV_getNextOpIndex(tokenList, opDict);
		if(opIndex == -1) break;
		NV_ID n = NV_Array_getByIndex(tokenList, opIndex);
		if(NV_NodeID_isEqual(&n, &NODEID_NOT_FOUND)) break;
		// vvvv Nothing
		if(NV_Node_String_compareWithCStr(&n, " ") == 0){
			NV_Array_removeIndex(tokenList, opIndex);
			continue;
		} else if(NV_Node_String_compareWithCStr(&n, ";") == 0){
			NV_Array_removeIndex(tokenList, opIndex);
			continue;
		// infix arithmetic
		} else if(NV_Node_String_compareWithCStr(&n, "+") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "add");
		} else if(NV_Node_String_compareWithCStr(&n, "-") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "sub");
		} else if(NV_Node_String_compareWithCStr(&n, "*") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "mul");
		} else if(NV_Node_String_compareWithCStr(&n, "/") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "div");
		} else if(NV_Node_String_compareWithCStr(&n, "%") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "mod");
		// infix compare
		} else if(NV_Node_String_compareWithCStr(&n, "<") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_lt");
		} else if(NV_Node_String_compareWithCStr(&n, ">=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_gte");
		} else if(NV_Node_String_compareWithCStr(&n, "<=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_lte");
		} else if(NV_Node_String_compareWithCStr(&n, ">") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_gt");
		} else if(NV_Node_String_compareWithCStr(&n, "==") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_eq");
		} else if(NV_Node_String_compareWithCStr(&n, "!=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_neq");
		// infix other
		} else if(NV_Node_String_compareWithCStr(&n, "=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "assign");
		// postfix
		} else if(NV_Node_String_compareWithCStr(&n, "++") == 0){
			NV_parseToCodeGraph_postfixOp(tokenList, &lastNode, opIndex, "inc");
		} else if(NV_Node_String_compareWithCStr(&n, "--") == 0){
			NV_parseToCodeGraph_postfixOp(tokenList, &lastNode, opIndex, "dec");
		// prefix other
		} else if(NV_Node_String_compareWithCStr(&n, "print") == 0){
			NV_parseToCodeGraph_prefixOp(tokenList, &lastNode, opIndex, "print");
		// syntax structure
		} else if(NV_Node_String_compareWithCStr(&n, "{") == 0){
			NV_Op_codeBlock(tokenList, opIndex, "{", "}");
		} else if(NV_Node_String_compareWithCStr(&n, "if") == 0){
			NV_parseToCodeGraph_if(tokenList, &lastNode, opIndex, opDict);
		} else if(NV_Node_String_compareWithCStr(&n, "for") == 0){
			e = NV_parseToCodeGraph_for(tokenList, &lastNode, opIndex, opDict);
			if(NV_NodeID_isEqual(&e, &NODEID_NULL)){
				return NODEID_NULL;
			}
		} else{
			fprintf(stderr, "NV_parseToCodeGraph: op not implemented for");
			NV_NodeID_printForDebug(&n);
			putchar('\n');
			return NODEID_NULL;
		}
	}
	//NV_Dict_print(&codeGraphRoot); putchar('\n');
	//NV_Array_print(tokenList); putchar('\n');
	//NV_saveCodeGraphForVisualization(&codeGraphRoot, "note/code.dot");
	return codeGraphRoot;
}
