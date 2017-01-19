#include "nv.h"
//
// Variable
//
NV_ID NV_Variable_create()
{
	NV_ID v;
	//
	v = NV_Node_create();
	//
	NV_Node_createRelation(&v, &RELID_TREE_TYPE, &NODEID_TREE_TYPE_VARIABLE);
	NV_Node_createRelation(&v, &RELID_VARIABLE_DATA, &NODEID_NULL);
	return v;
}

NV_ID NV_Variable_createWithName(const NV_ID *parentNode, const NV_ID *nameNode)
{
	NV_ID v = NV_Variable_create();
	NV_Node_createRelation(parentNode, nameNode, &v);
	return v;
}

void NV_Variable_assign(const NV_ID *vid, const NV_ID *data)
{
	NV_ID eid;
	//
	eid = NV_Node_getRelationFrom(vid, &RELID_VARIABLE_DATA);
	NV_Node_updateRelationTo(&eid, data);
}

NV_ID NV_Variable_getData(const NV_ID *vid)
{
	return NV_Node_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
}

void NV_Variable_print(const NV_ID *vid)
{
	NV_ID targetID;
	if(!NV_isTreeType(vid, &NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid->d[0]);
		return;
	}
	targetID = NV_Node_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
	printf("Variable ");
	NV_Node_dump(NV_Node_getByID(vid));
	printf(" = ");
	NV_Node_dump(NV_Node_getByID(&targetID));
	putchar('\n');
}

void NV_Variable_printPrimVal(const NV_ID *vid)
{
	NV_ID targetID;
	if(!NV_isTreeType(vid, &NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid->d[0]);
		return;
	}
	targetID = NV_Node_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
	NV_printNode(NV_Node_getByID(&targetID));
}

