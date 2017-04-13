#include "nv.h"
//
// Variable
//
NV_ID NV_Variable_create()
{
	NV_ID v;
	//
	v = NV_Node_createWithString("Var");
	//
	NV_NodeID_createRelation(&v, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_VARIABLE);
	NV_NodeID_createRelation(&v, &RELID_VARIABLE_DATA, &NODEID_NULL);
	return v;
}

NV_ID NV_Variable_createWithName(const NV_ID *parentNode, const NV_ID *nameNode)
{
	NV_ID v = NV_Variable_create();
	NV_NodeID_createRelation(parentNode, nameNode, &v);
	return v;
}

NV_ID NV_Variable_getNamed(const NV_ID *parentNode, const NV_ID *nameNode)
{
	return NV_NodeID_getEqRelatedNodeFrom(parentNode, nameNode);
}

void NV_Variable_assign(const NV_ID *vid, const NV_ID *data)
{
	NV_ID eid;
	//
	eid = NV_NodeID_getRelationFrom(vid, &RELID_VARIABLE_DATA);
	NV_NodeID_updateRelationTo(&eid, data);
}

NV_ID NV_Variable_getData(const NV_ID *vid)
{
	return NV_NodeID_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
}

void NV_Variable_print(const NV_ID *vid)
{
	NV_ID targetID;
	if(!NV_isTermType(vid, &NODEID_TERM_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid->d[0]);
		return;
	}
	targetID = NV_NodeID_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
	printf("(Var ");
	NV_ID_dumpIDToFile(vid, stdout);
	printf(" = ");
	NV_Term_print(&targetID);
	printf(")");
}

void NV_Variable_printPrimVal(const NV_ID *vid)
{
	NV_ID targetID;
	if(!NV_isTermType(vid, &NODEID_TERM_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid->d[0]);
		return;
	}
	targetID = NV_NodeID_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
	NV_Term_print(&targetID);
}

