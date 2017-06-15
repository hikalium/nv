#include "nv.h"

int NV_Op_isOperator(const NV_ID *id)
{
	return NV_isTermType(id, &NODEID_TERM_TYPE_OP);
}

NV_ID NV_Op_create(const char *ident, int32_t prec, const NV_ID *func)
{
	NV_ID op = NV_Node_create();
	NV_NodeID_createRelation(&op, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_OP);
	//
	NV_ID precNode = NV_Node_createWithInt32(prec);
	NV_ID identNode = NV_Node_createWithString(ident);
	//
	NV_NodeID_createRelation(&op, &RELID_OP_IDENT, &identNode);
	NV_NodeID_createRelation(&op, &RELID_OP_PRECEDENCE, &precNode);
	NV_NodeID_createRelation(&op, &RELID_OP_FUNC, func);
	//
	return op;
}
/*
// NV_ID NV_Op_getFunc(const NV_ID *op)
{
	return NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
}
*/

const char *NV_Op_getFuncAsCStr(const NV_ID *op)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	return NV_NodeID_getCStr(&func);
}

int32_t NV_Op_getPrec(const NV_ID *op)
{
	NV_ID prec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	return NV_NodeID_getInt32(&prec);
}

void NV_Lang_printOp(const NV_ID *op)
{
	NV_ID eFunc;
	NV_ID ePrec;
	eFunc = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	printf("(op ");
	NV_Term_print(&eFunc);
	printf("/");
	NV_Term_print(&ePrec);
	printf(")");
}

