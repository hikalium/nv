#include "../../nv.h"

NV_ID NV_LangOSECPU_OpFunc_infixOp
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID op = NV_Dict_getByStringKey(p, "op");
	NV_ID opL = NV_Dict_getByStringKey(p, "opL");
	NV_ID opR = NV_Dict_getByStringKey(p, "opR");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	if(NV_Variable_isVariable(&opL)){
		// 元々変換無しでVariableだった場合は、単に戻り値を格納しているVariable
		// なので中身をとりだす。
		opL = NV_Variable_getData(&opL);
	}
	if(NV_Variable_isVariable(&opR)){
		// 元々変換無しでVariableだった場合は、単に戻り値を格納しているVariable
		// なので中身をとりだす。
		opR = NV_Variable_getData(&opR);
	}
	//
	const char *opStr = NV_NodeID_getCStr(&op);
	int32_t opLVal = NV_Term_getInt32(&opL, scope);
	int32_t opRVal = NV_Term_getInt32(&opR, scope);
	int32_t ans;
	int isAnsNotInteger = 0;
	//
	//printf("op: %s, opLv: %d, opRv: %d\n", opStr, opLVal, opRVal);
	if(strcmp(opStr, "+") == 0) 		ans = opLVal + opRVal;
	else if(strcmp(opStr, "-") == 0)	ans = opLVal - opRVal;
	else if(strcmp(opStr, "*") == 0)	ans = opLVal * opRVal;
	else if(strcmp(opStr, "/") == 0)	ans = opLVal / opRVal;
	else if(strcmp(opStr, "%") == 0)	ans = opLVal % opRVal;
	//
	else if(strcmp(opStr, "<") == 0)	ans = opLVal < opRVal;
	else if(strcmp(opStr, ">=") == 0)	ans = opLVal >= opRVal;
	else if(strcmp(opStr, "<=") == 0)	ans = opLVal <= opRVal;
	else if(strcmp(opStr, ">") == 0)	ans = opLVal > opRVal;
	else if(strcmp(opStr, "==") == 0)	ans = opLVal == opRVal;
	else if(strcmp(opStr, "!=") == 0)	ans = opLVal != opRVal;
	//
	else if(strcmp(opStr, "=") == 0){
		// 
		isAnsNotInteger = 1;
		NV_ID v;
		v = opL;
		opR = NV_Term_getPrimNodeID(&opR, scope);
		if(NV_Term_isAssignable(&v, scope)){
			v = NV_Term_getAssignableNode(&v, scope);
			if(IS_DEBUG_MODE()){
				printf("Assign to ");
				NV_Variable_print(&v); putchar('\n');
			}
		} else{
			if(IS_DEBUG_MODE()){
				printf("Variable created by assign op\n");
			}
			v = NV_Variable_createWithName(scope, &v);
		}
		NV_Term_assign(&v, &opR);
		*lastEvalVal = v;
	} else if(strcmp(opStr, ".") == 0){
		// 変数, 名前 -> 名前withスコープ
		// 左オペランドの変数の値に相当するノードをスコープとし、
		// 名前をキーとする変数を作成して返す。
		isAnsNotInteger = 1;
		if(IS_DEBUG_MODE()){
			printf("opL: ");
			NV_NodeID_printForDebug(&opL); putchar('\n');
		}
		if(!NV_Term_isAssignable(&opL, scope)){
			return NV_Node_createWithString("Origin node was NOT_FOUND");
		}
		opL = NV_Term_getAssignableNode(&opL, scope);
		NV_ID v;
		if(IS_DEBUG_MODE()){
			printf("Assignable!!\n");
			NV_Variable_print(&v); putchar('\n');
		}
		NV_ID baseScope = NV_Variable_getData(&opL);
		if(NV_Term_isNotFound(&baseScope)){
			return NV_Node_createWithString("parent scope not found.");
		}
		v = NV_Variable_createWithName(&baseScope, &opR);
		if(IS_DEBUG_MODE()){
			printf("Created scoped var:");
			NV_Variable_print(&v); putchar('\n');
		}
		*lastEvalVal = v;
		NV_Variable_assign(&result, &v);
	} else if(strcmp(opStr, ":") == 0){
		// 左辺をキー、右辺を値とするオブジェクトを生成する
		// key : val -> obj
		isAnsNotInteger = 1;
		opR = NV_Term_getPrimNodeID(&opR, scope);
		NV_ID obj = NV_Node_create();
		NV_Dict_addKey(&obj, &opL, &opR);
		//
		*lastEvalVal = obj;
		NV_Variable_assign(&result, &obj);
	} else if(strcmp(opStr, ",") == 0){
		// 両辺のオブジェクトを統合した新しいオブジェクトを返す。
		// つまり、両辺のオブジェクトを起点とするようなエッジをすべてもつ
		// 新しい起点ノードを生成し返す。
		// 衝突するキーが存在する場合は、右辺のものが採用される。
		isAnsNotInteger = 1;
		opL = NV_Term_getPrimNodeID(&opL, scope);
		opR = NV_Term_getPrimNodeID(&opR, scope);
		NV_ID merged = NV_Dict_createMergedNode(&opL, &opR);
		//
		*lastEvalVal = merged;
		NV_Variable_assign(&result, &merged);
	} else{
		*lastEvalVal = NV_Node_createWithStringFormat("infix: No op for %s", opStr);
		return *lastEvalVal;
	}
	if(!isAnsNotInteger){
		//printf("Ans integer = %d\n", ans);
		NV_ID ansNode = NV_Node_createWithInt32(ans);
		NV_Variable_assign(&result, &ansNode);
		//NV_Variable_print(&result);
		*lastEvalVal = ansNode;
	}
	*p = NV_Dict_getByStringKey(p, "next");
	return NODEID_NULL;
}
/*
NV_ID NV_Lang02_OpFunc_prefixOp
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID op = NV_Dict_getByStringKey(p, "op");
	NV_ID opR = NV_Dict_getByStringKey(p, "opR");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	const char *opStr = NV_NodeID_getCStr(&op);
	//int32_t opRVal = NV_Term_getInt32(&opR, &NODEID_NULL);
	int32_t ans = 0;
	int isAnsNotInteger = 0;
	//
	if(NV_Variable_isVariable(&opR)){
		// 元々変換無しでVariableだった場合は、単に戻り値を格納しているVariable
		// なので中身をとりだす。
		opR = NV_Variable_getData(&opR);
	}
	//
	//printf("op: %s\n", opStr);
	//
	if(strcmp(opStr, "print") == 0){
		isAnsNotInteger = 1;
		opR = NV_Term_getPrimNodeID(&opR, scope);
		NV_Term_print(&opR); putchar('\n');
		*lastEvalVal = opR;
		NV_globalExecFlag |= NV_EXEC_FLAG_SUPRESS_AUTOPRINT;
	} else if(strcmp(opStr, "ls") == 0){
		isAnsNotInteger = 1;
		NV_Dict_print(scope);
		*lastEvalVal = NODEID_NULL;
	} else if(strcmp(opStr, "lsdep") == 0){
		isAnsNotInteger = 1;
		NV_Node_printDependencyTree(&rootScope, 0);
		*lastEvalVal = NODEID_NULL;
	} else if(strcmp(opStr, "clean") == 0){
		isAnsNotInteger = 1;
		NV_Node_cleanup();
		*lastEvalVal = NODEID_NULL;
	} else if(strcmp(opStr, "dump") == 0){
		isAnsNotInteger = 1;
		FILE *fp = fopen("dump.txt", "w");
		if(fp){
			NV_Node_dumpAllToFile(fp);
			fclose(fp);
		}
		*lastEvalVal = NODEID_NULL;
	} else if(strcmp(opStr, "+") == 0){
		opR = NV_Term_getPrimNodeID(&opR, scope);
		ans = NV_Term_getInt32(&opR ,scope);
	} else if(strcmp(opStr, "-") == 0){
		opR = NV_Term_getPrimNodeID(&opR, scope);
		ans = -NV_Term_getInt32(&opR, scope);
	} else{
		*lastEvalVal = NV_Node_createWithStringFormat("prefix: No op for %s", opStr);
		return *lastEvalVal;
	}
	if(!isAnsNotInteger){
		NV_ID ansNode = NV_Node_createWithInt32(ans);
		NV_Variable_assign(&result, &ansNode);
		*lastEvalVal = ansNode;
	}
	*p = NV_Dict_getByStringKey(p, "next");
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_postfixOp
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID op = NV_Dict_getByStringKey(p, "op");
	NV_ID opL = NV_Dict_getByStringKey(p, "opL");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	const char *opStr = NV_NodeID_getCStr(&op);
	//int32_t opRVal = NV_Term_getInt32(&opR, &NODEID_NULL);
	int32_t ans = 0;
	int isAnsNotInteger = 0;
	//
	//printf("op: %s\n", opStr);
	//
	if(strcmp(opStr, "++") == 0){
		NV_ID v, newVal;
		v = NV_Term_getAssignableNode(&opL, scope);
		opL = NV_Term_getPrimNodeID(&opL, scope);
		ans = NV_Term_getInt32(&opL, scope);
		newVal = NV_Node_createWithInt32(ans + 1);
		NV_Variable_assign(&v, &newVal);
	} else if(strcmp(opStr, "--") == 0){
		NV_ID v, newVal;
		v = NV_Term_getAssignableNode(&opL, scope);
		opL = NV_Term_getPrimNodeID(&opL, scope);
		ans = NV_Term_getInt32(&opL, scope);
		newVal = NV_Node_createWithInt32(ans - 1);
		NV_Variable_assign(&v, &newVal);
	} else{
		*lastEvalVal = NV_Node_createWithStringFormat("postfix: No op for %s", opStr);
		return *lastEvalVal;
	}
	if(!isAnsNotInteger){
		NV_ID ansNode = NV_Node_createWithInt32(ans);
		NV_Variable_assign(&result, &ansNode);
		*lastEvalVal = ansNode;
	}
	*p = NV_Dict_getByStringKey(p, "next");
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_cond
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID callBlock = NV_Dict_getByStringKey(p, "flag");
	NV_ID result = NV_evalGraph(&callBlock, scope, NV_Lang02_evalList);
	int32_t resultVal = NV_Term_getInt32(&result, scope);
	if(IS_DEBUG_MODE()){
		printf("cond: resultVal = %d\n", resultVal);
	}
	if(resultVal){
		*p = NV_Dict_getByStringKey(p, "truePath");
	} else{
		*p = NV_Dict_getByStringKey(p, "next");
	}
	*lastEvalVal = result;
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_do
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID callBlock = NV_Dict_getByStringKey(p, "call");
	//NV_ID subScope = NV_Variable_createSubScopeOf(scope);
	NV_ID result = NV_evalGraph(&callBlock, scope, NV_Lang02_evalList);
	*p = NV_Dict_getByStringKey(p, "next");
	*lastEvalVal = result;
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_parentheses
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	NV_ID inner = NV_Dict_getByStringKey(p, "inner");
	NV_ID opL = NV_Dict_getByStringKey(p, "opL");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	NV_ID innerResult = NV_evalGraph(&inner, scope, NV_Lang02_evalList);
	// same scope
	if(!NV_Term_isNotFound(&opL)){
		// exec code block of opL
		// 演算子は、実行時のコンテキストで評価する

		NV_ID opDict = NV_Term_getPrimNodeIDByCStr("opDict", scope);
		//NV_NodeID_printForDebug(&opL);
		NV_ID codeBlock = NV_Term_getPrimNodeID(&opL, scope);
		//NV_Array_print(&codeBlock); putchar('\n');
		NV_ID parsedBlock = NV_Lang_parseToCodeGraph(&codeBlock, &opDict, NV_Lang02_funcList);
		//
		NV_ID ansNode;
		{
			NV_ID subScope = NV_Variable_createSubScopeOf(scope);
			NV_Dict_addUniqueEqKeyByCStr(&subScope, "args", &innerResult);
			NV_Dict_addUniqueEqKeyByCStr(&subScope, "codeGraph", &parsedBlock);
			ansNode = NV_evalGraph(&parsedBlock, &subScope, NV_Lang02_evalList);
			// new scope
		}
		NV_Variable_assign(&result, &ansNode); 
	} else{
		// simply set result of inner
		NV_Variable_assign(&result, &innerResult);
	}
	//
	*p = NV_Dict_getByStringKey(p, "next");
	*lastEvalVal = result;
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_doNothing
(NV_ID * const p, NV_ID * const lastEvalVal, const NV_ID *scope)
{
	PARAM_UNUSED(lastEvalVal);
	PARAM_UNUSED(scope);
	//
	*p = NV_Dict_getByStringKey(p, "next");
	return NODEID_NULL;
}


*/
