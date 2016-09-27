#include "nv.h"
//
// Variable
//
NV_ElementID NV_Variable_create()
{
	NV_ElementID v;
	v = NV_Node_add();
	NV_Edge_add(v, RELID_TREE_TYPE, NODEID_TREE_TYPE_VARIABLE);
	NV_Edge_add(v, RELID_VARIABLE_DATA, NODEID_NULL);
	return v;
}

void NV_Variable_assign(NV_ElementID vid, NV_ElementID data)
{
	NV_ElementID eid;
	NV_Edge *e;
	eid = NV_Edge_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	e = NV_Edge_getByID(eid);
	if(e){
		e->to = data;
	}
}

void NV_Variable_print(NV_ElementID vid)
{
	NV_ElementID targetID;
	if(!NV_isTreeType(vid, NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid.d[0]);
		return;
	}
	targetID = NV_Node_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	printf("Variable ");
	NV_Node_dump(NV_Node_getByID(vid));
	printf(" = ");
	NV_Node_dump(NV_Node_getByID(targetID));
	putchar('\n');
}

void NV_Variable_printiPrimVal(NV_ElementID vid)
{
	NV_ElementID targetID;
	if(!NV_isTreeType(vid, NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid.d[0]);
		return;
	}
	targetID = NV_Node_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	NV_Node_printPrimVal(NV_Node_getByID(targetID));
}

