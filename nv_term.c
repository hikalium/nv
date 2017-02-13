#include "nv.h"

//
//	Type checks
//

int NV_Term_isInteger(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
		id = &vid;
	} else if(NV_Node_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		vid = NV_Variable_getData(&vid);
		id = &vid;
	}
	return NV_Node_isInteger(id);
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

//
// Read term data
//
int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
		id = &vid;
	} else if(NV_Node_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		vid = NV_Variable_getData(&vid);
		id = &vid;
	}
	if(!NV_Node_isInteger(id)) return 0;
	return NV_Node_getInt32FromID(id);
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
//
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
		NV_Op_print(&n->id);
	} else{
		printf("prim: ");
		NV_Node_printPrimVal(n);
	}
}
