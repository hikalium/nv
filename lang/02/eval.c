#include "../../nv.h"

NV_ID NV_Lang02_OpFunc_infixOp(const NV_ID *p, NV_ID *lastEvalVal)
{
	const NV_ID scope = NODEID_NULL;
	//
	NV_ID op = NV_Dict_getByStringKey(p, "op");
	NV_ID opL = NV_Dict_getByStringKey(p, "opL");
	NV_ID opR = NV_Dict_getByStringKey(p, "opR");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	const char *opStr = NV_NodeID_getCStr(&op);
	int32_t opLVal = NV_Term_getInt32(&opL, &scope);
	int32_t opRVal = NV_Term_getInt32(&opR, &scope);
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
		opR = NV_Term_getPrimNodeID(&opR, &scope);
		if(NV_Term_isAssignable(&v, &scope)){
			v = NV_Term_getAssignableNode(&v, &scope);
		} else{
			if(IS_DEBUG_MODE()){
				printf("Variable created by assign op\n");
			}
			v = NV_Variable_createWithName(&scope, &v);
		}
		NV_Term_assign(&v, &opR);
		*lastEvalVal = v;
	} else if(strcmp(opStr, ".") == 0){
		// 変数, 名前 -> 名前withスコープ
		//NV_ID path;
		//int isLeftOpIrrelevant;
		if(NV_Term_isAssignable(&opL, &scope)){
			opL = NV_Term_getAssignableNode(&opL, &scope);
		} else{
			return NV_Node_createWithString("Origin node was NOT_FOUND");
		}
		
		//
		// Check left operands
		/*
		nL = NV_Term_tryReadAsOperator(&nL, &opDict);
		isLeftOpIrrelevant = NV_NodeID_isEqual(&nL, &NODEID_NOT_FOUND) || 
			NV_isTermType(&nL, &NODEID_TERM_TYPE_OP);
		*/
		/*
		isLeftOpIrrelevant = 0;
		if(isLeftOpIrrelevant){
			path = NV_Path_createWithOrigin(&NODEID_NULL);
		} else if(NV_isTermType(&opL, &NODEID_TERM_TYPE_PATH)){
			path = opL;
		} else if(NV_Term_isAssignable(&opL, &scope)){
			// nLは代入可能オブジェクトだったので、
			// 格納されているオブジェクトを起点にパスを作成
			opL = NV_Term_getPrimNodeID(&opL, &scope);
			if(!NV_NodeID_isEqual(&opL, &NODEID_NOT_FOUND)){
				path = NV_Path_createWithOrigin(&opL);
			} else{
				return NV_Node_createWithString("Origin node was NOT_FOUND");
			}
		} else{
			return NV_Node_createWithString("Origin node not found in this ctx");
		}
		// add right operand to path
		NV_Path_appendRoute(&path, &opR);
		//
		NV_Dict_print(&path);
		//
		isAnsNotInteger = 1;
		NV_Variable_assign(&result, &path);
		*lastEvalVal = result;
		*/
	} else{
		*lastEvalVal = NV_Node_createWithStringFormat("infix: No op for %s", opStr);
		return *lastEvalVal;
	}
	if(!isAnsNotInteger){
		NV_ID ansNode = NV_Node_createWithInt32(ans);
		NV_Variable_assign(&result, &ansNode);
		*lastEvalVal = ansNode;
	}
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_prefixOp(const NV_ID *p, NV_ID *lastEvalVal)
{
	const NV_ID scope = NODEID_NULL;
	//
	NV_ID op = NV_Dict_getByStringKey(p, "op");
	NV_ID opR = NV_Dict_getByStringKey(p, "opR");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	//
	const char *opStr = NV_NodeID_getCStr(&op);
	//int32_t opRVal = NV_Term_getInt32(&opR, &NODEID_NULL);
	int32_t ans = 0;
	int isAnsNotInteger = 0;
	//
	//printf("op: %s\n", opStr);
	//
	if(strcmp(opStr, "print") == 0){
		isAnsNotInteger = 1;
		opR = NV_Term_getPrimNodeID(&opR, &scope);
		NV_Term_print(&opR); putchar('\n');
		*lastEvalVal = opR;
		NV_globalExecFlag |= NV_EXEC_FLAG_SUPRESS_AUTOPRINT;
	} else if(strcmp(opStr, "ls") == 0){
		isAnsNotInteger = 1;
		NV_Dict_print(&scope);
		*lastEvalVal = NODEID_NULL;
	} else if(strcmp(opStr, "+") == 0){
		opR = NV_Term_getPrimNodeID(&opR, &scope);
		ans = NV_Term_getInt32(&opR ,&scope);
	} else if(strcmp(opStr, "-") == 0){
		opR = NV_Term_getPrimNodeID(&opR, &scope);
		ans = -NV_Term_getInt32(&opR, &scope);
	} else{
		*lastEvalVal = NV_Node_createWithStringFormat("prefix: No op for %s", opStr);
		return *lastEvalVal;
	}
	if(!isAnsNotInteger){
		NV_ID ansNode = NV_Node_createWithInt32(ans);
		NV_Variable_assign(&result, &ansNode);
		*lastEvalVal = ansNode;
	}
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_postfixOp(const NV_ID *p, NV_ID *lastEvalVal)
{
	const NV_ID scope = NODEID_NULL;
	//
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
		v = NV_Term_getAssignableNode(&opL, &scope);
		opL = NV_Term_getPrimNodeID(&opL, &scope);
		ans = NV_Term_getInt32(&opL, &scope);
		newVal = NV_Node_createWithInt32(ans + 1);
		NV_Variable_assign(&v, &newVal);
	} else if(strcmp(opStr, "--") == 0){
		NV_ID v, newVal;
		v = NV_Term_getAssignableNode(&opL, &scope);
		opL = NV_Term_getPrimNodeID(&opL, &scope);
		ans = NV_Term_getInt32(&opL, &scope);
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
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_cond(NV_ID *p, NV_ID *lastEvalVal)
{
	const NV_ID scope = NODEID_NULL;
	NV_ID callBlock = NV_Dict_getByStringKey(p, "flag");
	NV_ID result = NV_evalGraph(&callBlock);
	int32_t resultVal = NV_Term_getInt32(&result, &scope);
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

NV_ID NV_Lang02_OpFunc_do(NV_ID *p, NV_ID *lastEvalVal)
{
	//const NV_ID scope = NODEID_NULL;
	NV_ID callBlock = NV_Dict_getByStringKey(p, "call");
	NV_ID result = NV_evalGraph(&callBlock);
	*p = NV_Dict_getByStringKey(p, "next");
	*lastEvalVal = result;
	return NODEID_NULL;
}

NV_ID NV_Lang02_OpFunc_parentheses(NV_ID *p, NV_ID *lastEvalVal)
{
	//const NV_ID scope = NODEID_NULL;
	NV_ID inner = NV_Dict_getByStringKey(p, "inner");
	NV_ID opL = NV_Dict_getByStringKey(p, "opL");
	NV_ID result = NV_Dict_getByStringKey(p, "result");
	NV_ID ansNode = NV_evalGraph(&inner);
	//
	NV_Variable_assign(&result, &ansNode);
	//
	*p = NV_Dict_getByStringKey(p, "next");
	*lastEvalVal = result;
	return NODEID_NULL;
}

NV_ID NV_evalGraph(const NV_ID *codeGraphRoot)
{
	NV_ID lastEvalVal = NODEID_NULL;
	NV_ID p = *codeGraphRoot, r;
	const char *s;
	//printf("BEGIN eval: \n");
	p = NV_Dict_getByStringKey(&p, "next");
	for(;;){
		if(NV_NodeID_isEqual(&p, &NODEID_NOT_FOUND)) break;
		if(!NV_NodeID_isString(&p)) break;
		s = NV_NodeID_getCStr(&p);
		//printf("next: %s\n", s);
		if(strcmp(s, "infixOp") == 0){
			NV_Lang02_OpFunc_infixOp(&p, &lastEvalVal);
			p = NV_Dict_getByStringKey(&p, "next");
		} else if(strcmp(s, "prefixOp") == 0){
			NV_Lang02_OpFunc_prefixOp(&p, &lastEvalVal);
			p = NV_Dict_getByStringKey(&p, "next");
		} else if(strcmp(s, "postfixOp") == 0){
			NV_Lang02_OpFunc_postfixOp(&p, &lastEvalVal);
			p = NV_Dict_getByStringKey(&p, "next");
		} else if(strcmp(s, "cond") == 0){
			NV_Lang02_OpFunc_cond(&p, &lastEvalVal);
		} else if(strcmp(s, "do") == 0){
			NV_Lang02_OpFunc_do(&p, &lastEvalVal);
		} else if(strcmp(s, "updt") == 0){
			NV_Lang02_OpFunc_do(&p, &lastEvalVal);
		} else if(strcmp(s, "init") == 0){
			NV_Lang02_OpFunc_do(&p, &lastEvalVal);
		} else if(strcmp(s, "endfor") == 0){
			NV_Lang02_OpFunc_do(&p, &lastEvalVal);
		} else if(strcmp(s, "endif") == 0){
			p = NV_Dict_getByStringKey(&p, "next");
		} else if(strcmp(s, "()") == 0){
			r = NV_Lang02_OpFunc_parentheses(&p, &lastEvalVal);
			if(!NV_Term_isNull(&r)){
				return r;
			}
		} else{
			lastEvalVal = NV_Node_createWithStringFormat(
					"NV_evalGraph: No func for %s", s);
			return lastEvalVal;
		}
	}
	return lastEvalVal;
}
