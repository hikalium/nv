#include "nv.h"

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
	if(NV_isTreeType(&n->id, &NODEID_TERM_TYPE_ARRAY)){
		NV_Array_print(&n->id);
	} else if(NV_isTreeType(&n->id, &NODEID_TERM_TYPE_VARIABLE)){
		NV_Variable_printPrimVal(&n->id);
	} else if(NV_isTreeType(&n->id, &NODEID_TERM_TYPE_OP)){
		NV_Op_print(&n->id);
	} else{
		NV_Node_printPrimVal(n);
	}
}
