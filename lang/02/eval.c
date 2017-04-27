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
	int32_t opLVal = NV_Term_getInt32(&opL, &NODEID_NULL);
	int32_t opRVal = NV_Term_getInt32(&opR, &NODEID_NULL);
	int32_t ans;
	int isAnsNotInteger = 0;
	//
	//printf("op: %s\n", opStr);
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

NV_ID NV_evalGraph(const NV_ID *codeGraphRoot)
{
	NV_ID lastEvalVal = NODEID_NULL;
	NV_ID p = *codeGraphRoot;
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
		} else{
			lastEvalVal = NV_Node_createWithStringFormat(
					"NV_evalGraph: No func for %s", s);
			return lastEvalVal;
		}
	}
	return lastEvalVal;
}
