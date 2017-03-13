#include "nv.h"

/*
	このシステム上では、あらゆるオブジェクトがDictとみなせる。
	Relationのrelをキーとし、与えられたキーと同値なrelをもつRelationの
	先にあるオブジェクトを返す。
	同値とは、プリミティブなノードであれば、そのノードの値が一致すること。
	複雑なノードに関しては、現状では定義しない。(None != Noneが常に成立)
	該当するノードが複数ある場合、
	複数返すことが前提になっている関数は、可能な限り全てを返し、
	ひとつしか返せない関数は、そのうちのどれかを返す。
	どのNodeひとつを返すかは実装依存である。
	
*/

NV_ID NV_Dict_addKey
(const NV_ID *root, const NV_ID *key, const NV_ID *value)
{
	return NV_NodeID_createRelation(root, key, value);	
}

NV_ID NV_Dict_addKeyByCStr
(const NV_ID *root, const char *key, const NV_ID *value)
{
	NV_ID strid = NV_Node_createWithString(key);
	return NV_Dict_addKey(root, &strid, value);
}

NV_ID NV_Dict_addUniqueIDKey
(const NV_ID *root, const NV_ID *key, const NV_ID *value)
{
	return NV_NodeID_createUniqueIDRelation(root, key, value);	
}

NV_ID NV_Dict_addUniqueEqKeyByCStr
(const NV_ID *root, const char *key, const NV_ID *value)
{
	NV_ID strid = NV_Node_createWithString(key);
	return NV_NodeID_createUniqueEqRelation(root, &strid, value);
}

NV_ID NV_Dict_removeUniqueIDKey(const NV_ID *root, const NV_ID *key)
{
	// removeと言っているが、実際はリンク先をNOT_FOUNDに向けるだけ
	return NV_NodeID_createUniqueIDRelation(root, key, &NODEID_NOT_FOUND);
}

NV_ID NV_Dict_removeUniqueEqKeyByCStr(const NV_ID *root, const char *key)
{
	// removeと言っているが、実際はリンク先をNOT_FOUNDに向けるだけ
	NV_ID strid = NV_Node_createWithString(key);
	return NV_NodeID_createUniqueEqRelation(root, &strid, &NODEID_NOT_FOUND);
}

NV_ID NV_Dict_get(const NV_ID *root, const NV_ID *key)
{
	// keyが同じ値を持つ(IDが等しいとは限らない)オブジェクトを返す。
	return NV_NodeID_getEqRelatedNodeFrom(root, key);
}

NV_ID NV_Dict_getEqID(const NV_ID *root, const NV_ID *key)
{
	return NV_NodeID_getRelatedNodeFrom(root, key);
}

/*
NV_ID NV_Dict_getAll(const NV_ID *root, const NV_ID *key)
{
	// VARY SLOW!!!!!
	// keyが同じ値を持つ(IDが等しいとは限らない)オブジェクトを
	// すべて含むリストを返す。
	const NV_Node *n;
	const NV_Relation *reld;
	NV_ID list;
	//
	list = NV_Array_create();
	//
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_NodeID_isEqual(&reld->from, root) && 
				NV_NodeID_isEqualInValue(&reld->rel, key)){
				NV_Array_push(&list, &reld->to);
			}
		}
	}
	return list;
}
*/

NV_ID NV_Dict_getByStringKey
(const NV_ID *root, const char *key)
{
	NV_ID strid = NV_Node_createWithString(key);
	return NV_Dict_get(root, &strid);
}

void NV_Dict_print(const NV_ID *root)
{
	const NV_Node *n;
	const NV_Relation *reld;
	int cnt = 0;
	NV_NodeID_printPrimVal(root);
	printf(":\n");
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(NV_NodeID_isEqual(&reld->from, root)){
				printf("  |--- ");
				NV_NodeID_printPrimVal(&reld->rel);
				printf(": ");
				NV_printNodeByID(&reld->to);
				printf("\n");
				cnt++;
			}
		}
	}
	printf("(%d items)\n", cnt);
}
