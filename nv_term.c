#include "nv.h"

//
//	Type checks
//

int NV_Term_isInteger(NV_ID *id)
{
	NV_ID vid;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
		id = &vid;
	}
	return NV_Node_isInteger(id);
}

int NV_Term_isAssignable(NV_ID *id)
{
	return NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE);
}

//
// Read term data
//
int32_t NV_Term_getInt32(NV_ID *id)
{
	NV_ID vid;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
		id = &vid;
	}
	if(!NV_Node_isInteger(id)) return 0;
	return NV_Node_getInt32FromID(id);
}
