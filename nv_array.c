#include "nv.h"
//
// TermType: Array
//

NV_ID NV_Array_create()
{
	NV_ID arrayRoot;
	arrayRoot = NV_Node_create();
	NV_NodeID_createRelation(&arrayRoot, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_ARRAY);
	//NV_NodeID_createRelation(&arrayRoot, &RELID_ARRAY_NEXT, &NODEID_NOT_FOUND);
	return arrayRoot;
}

NV_ID NV_Array_clone(const NV_ID *base)
{
	NV_ID newArray, t;
	int i;
	//
	newArray = NV_Node_create();
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(base, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		NV_Array_push(&newArray, &t);
	}
	return newArray;
}

NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data)
{
	NV_ID v, t, next;
	//
	v = NV_Variable_create();
	NV_Variable_assign(&v, data);
	//
	t = *array;
	for(;;){
		next = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&next, &NODEID_NOT_FOUND)) break;
		t = next;
	}
	NV_NodeID_createRelation(&t, &RELID_ARRAY_NEXT, &v);
	return v;
}

NV_ID NV_Array_pop(const NV_ID *array)
{
	NV_ID prev, t, next;
	prev = *array;
	t = NV_NodeID_getRelatedNodeFrom(&prev, &RELID_ARRAY_NEXT);
	for(;;){
		next = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&next, &NODEID_NOT_FOUND)) break;
		prev = t;
		t = next;
	}
	// t is retv.
	NV_ID relnid;
	relnid = NV_NodeID_getRelationFrom(&prev, &RELID_ARRAY_NEXT);
	NV_NodeID_remove(&relnid);
	return NV_Variable_getData(&t);
}

NV_ID NV_Array_last(const NV_ID *array)
{
	NV_ID prev, t, next;
	prev = *array;
	t = NV_NodeID_getRelatedNodeFrom(&prev, &RELID_ARRAY_NEXT);
	for(;;){
		next = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&next, &NODEID_NOT_FOUND)) break;
		prev = t;
		t = next;
	}
	// t is retv.
	return NV_Variable_getData(&t);
}

NV_ID NV_Array_getByIndex(const NV_ID *array, int index)
{
	NV_ID t;
	if(index < 0) return NODEID_NULL;
	t = NV_NodeID_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(; index; index--){
		if(index == 0) break;
		t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
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
		t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
	}
	// tのnextが削除対象。これをtnとおく。
	if(!NV_ID_isEqual(&t, &NODEID_NOT_FOUND)){
		tn = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		tnn = NV_NodeID_getRelatedNodeFrom(&tn, &RELID_ARRAY_NEXT);
		r = NV_NodeID_getRelationFrom(&t, &RELID_ARRAY_NEXT);
		NV_NodeID_updateRelationTo(&r, &tnn);
	}
}

void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data)
{
	NV_ID t;
	if(index < 0) return;
	t = NV_NodeID_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
	for(; index; index--){
		if(index == 0) break;
		t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) return;
	}
	NV_Variable_assign(&t, data);
}

NV_ID NV_Array_joinWithCStr(const NV_ID *array, const char *sep)
{
	size_t sumLen = 1;
	NV_ID t;
	char *buf, *p;
	const char *s;
	int i;
	//
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(array, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		sumLen += NV_NodeID_String_strlen(&t);
	}
	sumLen += i * strlen(sep);
	buf = NV_malloc(sumLen);
	p = buf;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(array, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		s = NV_NodeID_getCStr(&t);
		if(s){
			NV_strncpy(p, s, sumLen - (p - buf), strlen(s));
			p += strlen(s);
		}
		NV_strncpy(p, sep, sumLen - (p - buf), strlen(sep));
		p += strlen(sep);
	}
	*p = 0;
	t = NV_Node_createWithString(buf);
	NV_free(buf);
	return t;
}

void NV_Array_print(const NV_ID *array)
{
	NV_ID t;
	int i;
	printf("[");
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(array, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		if(i != 0) printf(",");
		NV_printNodeByID(&t);
	}
	printf("]");
}

