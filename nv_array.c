#include "nv.h"
//
// Array
//

NV_ID NV_Array_create()
{
	NV_ID arrayRoot;
	arrayRoot = NV_Node_create();
	NV_Node_createRelation(&arrayRoot, &RELID_TREE_TYPE, &NODEID_TREE_TYPE_ARRAY);
	NV_Node_createRelation(&arrayRoot, &RELID_ARRAY_NEXT, &NODEID_NULL);
	return arrayRoot;
}

NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data)
{
	// Always copy data element.
	NV_ID v, t, next, d;
	//
	v = NV_Variable_create();
	d = NV_Node_clone(data);
	NV_Variable_assign(&v, &d);
	//
	t = *array;
	for(;;){
		next = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&next, &NODEID_NULL)) break;
		t = next;
	}
	NV_Node_createRelation(&t, &RELID_ARRAY_NEXT, &v);
	return v;
}

NV_ID NV_Array_getByIndex(const NV_ID *array, int index)
{
	NV_ID t;
	if(index < 0) return NODEID_NULL;
	t = NV_Node_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(; index; index--){
		if(index == 0) break;
		t = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NULL)) break;
	}
	return NV_Variable_getData(&t);
}

void NV_Array_removeIndex(const NV_ID *array, int index)
{
	NV_ID t, tn, tnn, r;
	if(index < 0) return;
	t = *array;
	for(; index; index--){
		if(index == 0) break;
		t = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NULL)) break;
	}
	// tのnextが削除対象。これをtnとおく。
	if(!NV_ID_isEqual(&t, &NODEID_NULL)){
		tn = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		tnn = NV_Node_getRelatedNodeFrom(&tn, &RELID_ARRAY_NEXT);
		r = NV_Node_getRelationFrom(&t, &RELID_ARRAY_NEXT);
		NV_Node_updateRelationTo(&r, &tnn);
	}
}

void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data)
{
	NV_ID t;
	if(index < 0) return;
	t = NV_Node_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(; index; index--){
		if(index == 0) break;
		t = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NULL)) return;
	}
	NV_Variable_assign(&t, data);
}

void NV_Array_print(const NV_ID *array)
{
	NV_ID t;
	printf("[");
	t = NV_Node_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(;!NV_ID_isEqual(&t, &NODEID_NULL);){
		NV_printNodeByID(&t);
		t = NV_Node_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(!NV_ID_isEqual(&t, &NODEID_NULL)){
			printf(",");
		}
	}
	printf("]\n");
}

