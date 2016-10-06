#include "nv.h"
//
// Variable
//
NV_ElementID NV_Variable_create()
{
	NV_ElementID v, rel;
	//
	v = NV_Node_add();
	//
	rel = NV_Node_add();
	NV_Node_setRelation(rel, v, RELID_TREE_TYPE, NODEID_TREE_TYPE_VARIABLE);
	//
	rel = NV_Node_add();
	NV_Node_setRelation(rel, v, RELID_VARIABLE_DATA, NODEID_NULL);
	return v;
}

void NV_Variable_assign(NV_ElementID vid, NV_ElementID data)
{
	NV_ElementID eid;
	//
	eid = NV_Node_getRelationFrom(vid, RELID_VARIABLE_DATA);
	NV_Node_updateRelationTo(eid, data);
}

void NV_Variable_print(NV_ElementID vid)
{
	NV_ElementID targetID;
	if(!NV_isTreeType(vid, NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid.d[0]);
		return;
	}
	targetID = NV_Node_getRelatedNodeFrom(vid, RELID_VARIABLE_DATA);
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
	targetID = NV_Node_getRelatedNodeFrom(vid, RELID_VARIABLE_DATA);
	NV_Node_printPrimVal(NV_Node_getByID(targetID));
}

