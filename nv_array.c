#include "nv.h"
//
// Internal
//

void NV_Array_Internal_setCount(const NV_ID *array, int32_t count) {
  NV_ID cn;
  cn = NV_Node_createWithInt32(count);

  NV_Dict_addUniqueIDKey(array, &RELID_ARRAY_COUNT, &cn);
}

int32_t NV_Array_Internal_getCount(const NV_ID *array) {
  NV_ID cn;
  cn = NV_Dict_getEqID(array, &RELID_ARRAY_COUNT);
  return NV_NodeID_getInt32(&cn);
}

void NV_Array_Internal_updateCountRel(const NV_ID *array, int32_t diff) {
  int32_t count;
  count = NV_Array_Internal_getCount(array);
  NV_Array_Internal_setCount(array, count + diff);
}

//
//
//

int NV_Array_isArray(const NV_ID *id) {
  return NV_isTermType(id, &NODEID_TERM_TYPE_ARRAY);
}

NV_ID NV_Array_create() {
  NV_ID array;
  array = NV_Node_create();
  NV_NodeID_createRelation(&array, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_ARRAY);
  //
  NV_Array_Internal_setCount(&array, 0);
  //
  return array;
}

NV_ID NV_Array_clone(const NV_ID *base) {
  NV_ID newArray, t;
  int i;
  //
  newArray = NV_Array_create();
  for (i = 0;; i++) {
    t = NV_Array_getByIndex(base, i);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
    NV_Array_push(&newArray, &t);
  }
  return newArray;
}

NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data) {
  NV_ID v, t, next;
  //
  v = NV_Node_create();
  NV_NodeID_createUniqueIDRelation(&v, &RELID_ARRAY_DATA, data);
  //
  t = *array;
  for (;;) {
    next = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
    if (NV_NodeID_isEqual(&next, &NODEID_NOT_FOUND)) break;
    t = next;
  }
  NV_NodeID_createRelation(&t, &RELID_ARRAY_NEXT, &v);
  //
  NV_Array_Internal_updateCountRel(array, +1);
  //
  return v;
}

NV_ID NV_Array_pop(const NV_ID *array) {
  NV_ID prev, t, next;
  prev = *array;
  t = NV_NodeID_getRelatedNodeFrom(&prev, &RELID_ARRAY_NEXT);
  for (;;) {
    next = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
    if (NV_NodeID_isEqual(&next, &NODEID_NOT_FOUND)) break;
    prev = t;
    t = next;
  }
  // t is retv.
  NV_ID relnid;
  relnid = NV_NodeID_getRelationFrom(&prev, &RELID_ARRAY_NEXT);
  // NV_NodeID_remove(&relnid);
  //
  if (!NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) {
    NV_Array_Internal_updateCountRel(array, -1);
  }
  //
  return NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_DATA);
}

NV_ID NV_Array_last(const NV_ID *array) {
  NV_ID t;
  int32_t count, i;
  //
  count = NV_Array_count(array);
  //
  t = NV_NodeID_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
  //
  for (i = 0; i < count - 1; i++) {
    t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
  }
  // t is retv.
  return NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_DATA);
}

int32_t NV_Array_count(const NV_ID *array) {
  if (!array) return 0;
  if (!NV_isTermType(array, &NODEID_TERM_TYPE_ARRAY)) return 0;
  return NV_Array_Internal_getCount(array);
}

NV_ID NV_Array_getAssignableByIndex(const NV_ID *array, int index) {
  // indexが範囲外ならばNOT_FOUND
  NV_ID t;
  if (index < 0) {
    return NODEID_NOT_FOUND;
  }
  t = NV_NodeID_getRelatedNodeFrom(array, &RELID_ARRAY_NEXT);
  for (; index > 0; index--) {
    t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) return NODEID_NOT_FOUND;
  }
  return NV_Variable_createWithName(&t, &RELID_ARRAY_DATA);
}

NV_ID NV_Array_getByIndex(const NV_ID *array, int index) {
  NV_ID v;
  v = NV_Array_getAssignableByIndex(array, index);
  if (NV_Term_isNotFound(&v)) return NODEID_NOT_FOUND;
  return NV_Variable_getData(&v);
}

void NV_Array_removeIndex(const NV_ID *array, int index) {
  NV_ID t, tn, tnn, r;
  if (index < 0) return;
  t = *array;
  for (; index; index--) {
    if (index == 0) break;
    t = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
  }
  // tのnextが削除対象。これをtnとおく。
  if (!NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) {
    tn = NV_NodeID_getRelatedNodeFrom(&t, &RELID_ARRAY_NEXT);
    tnn = NV_NodeID_getRelatedNodeFrom(&tn, &RELID_ARRAY_NEXT);
    r = NV_NodeID_getRelationFrom(&t, &RELID_ARRAY_NEXT);
    NV_NodeID_updateRelationTo(&r, &tnn);
    //
    NV_Array_Internal_updateCountRel(array, -1);
    //
  }
}

void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data) {
  NV_ID v;
  v = NV_Array_getAssignableByIndex(array, index);
  NV_Variable_assign(&v, data);
}

NV_ID NV_Array_joinWithCStr(const NV_ID *array, const char *sep) {
  size_t sumLen = 1;
  NV_ID t;
  char *buf, *p;
  const char *s;
  int i;
  //
  for (i = 0;; i++) {
    t = NV_Array_getByIndex(array, i);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
    sumLen += NV_Node_String_strlen(&t);
  }
  sumLen += i * strlen(sep);
  buf = NV_malloc(sumLen);
  p = buf;
  for (i = 0;; i++) {
    t = NV_Array_getByIndex(array, i);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
    s = NV_NodeID_getCStr(&t);
    if (s) {
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

NV_ID NV_Array_getSorted(const NV_ID *array,
                         int (*f)(const void *n1, const void *n2)) {
  size_t nel = NV_Array_count(array);
  NV_ID *buf = NV_malloc(sizeof(NV_ID) * nel);
  size_t i;
  NV_ID new;
  //
  for (i = 0; i < nel; i++) {
    buf[i] = NV_Array_getByIndex(array, i);
  }
  //
  qsort(buf, nel, sizeof(NV_ID), f);
  //
  new = NV_Array_create();
  for (i = 0; i < nel; i++) {
    NV_Array_push(&new, &buf[i]);
  }
  //
  NV_free(buf);
  return new;
}

int32_t NV_Array_calcHash(const NV_ID *array) {
  int32_t hash = 0;
  NV_ID t;
  int i;
  for (i = 0;; i++) {
    t = NV_Array_getByIndex(array, i);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
    hash ^= NV_Term_calcHash(&t);
  }
  return hash;
}

void NV_Array_print(const NV_ID *array) {
  NV_ID t;
  int i;
  printf("[");
  for (i = 0;; i++) {
    t = NV_Array_getByIndex(array, i);
    if (NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
    if (i != 0) printf(",");
    NV_Term_print(&t);
  }
  printf("]");
}
