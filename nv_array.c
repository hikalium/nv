#include "nv.h"
//
// Array
//

NV_ElementID NV_Array_create()
{
	NV_ElementID arrayRoot;
	arrayRoot = NV_Node_add();
	NV_Node_addRelation(&arrayRoot, &RELID_TREE_TYPE, &NODEID_TREE_TYPE_ARRAY);
	NV_Node_addRelation(&arrayRoot, &RELID_ARRAY_NEXT, &NODEID_NULL);
	return arrayRoot;
}

NV_ElementID NV_Array_push(const NV_ElementID *array, const NV_ElementID *data)
{
	// Always copy data element.
	NV_ElementID v, t, next;
	//
	v = NV_Variable_create();
	NV_Variable_assign(&v, data);
	//
	t = *array;
	for(;;){
		next = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ElementID_isEqual(&next, &NODEID_NULL)) break;
		t = next;
	}
	NV_Node_addRelation(&t, &RELID_ARRAY_NEXT, &v);
	return v;
}

void NV_Array_print(const NV_ElementID *array)
{
	NV_ElementID t;
	printf("[");
	t = NV_Node_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(;!NV_ElementID_isEqual(&t, &NODEID_NULL);){
		NV_Variable_printiPrimVal(&t);
		t = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(!NV_ElementID_isEqual(&t, &NODEID_NULL)){
			printf(",");
		}
	}
	printf("]\n");
}
