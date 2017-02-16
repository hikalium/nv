#include "nv.h"

//
//	Type checks
//

NV_ID NV_Term_tryConvertToVariable(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	//
	vid = *id;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
	} else if(NV_Node_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		vid = NV_Variable_getData(&vid);
	}
	return vid;
}

int NV_Term_isInteger(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid = NV_Term_tryConvertToVariable(id, ctx);
	return NV_Node_isInteger(&vid);
}

int NV_Term_isAssignable(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_Node_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		id = &vid;
	}

	return NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE);
}

int NV_Term_isArray(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid = NV_Term_tryConvertToVariable(id, ctx);
	return NV_isTermType(&vid, &NODEID_TERM_TYPE_ARRAY);
}

//
// Read term data
//
int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid = NV_Term_tryConvertToVariable(id, ctx);
	if(!NV_Node_isInteger(&vid)) return 0;
	return NV_Node_getInt32FromID(&vid);
}

NV_ID NV_Term_getAssignableNode(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_Node_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		id = &vid;
	}
	if(!NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)) return NODEID_NOT_FOUND;
	return *id;
}

//
// print
//

void NV_printNodeByID(const NV_ID *id)
{
	NV_Node *n = NV_Node_getByID(id);
	NV_printNode(n);
}

void NV_printNode(const NV_Node *n)
{
	if(!n){
		printf("(null pointer node)");
		return;
	}
	if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_ARRAY)){
		NV_Array_print(&n->id);
	} else if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_VARIABLE)){
		NV_Variable_print(&n->id);
	} else if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_OP)){
		NV_printOp(&n->id);
	} else{
		NV_Node_printPrimVal(n);
	}
}
