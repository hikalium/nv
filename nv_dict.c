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

NV_ID NV_Dict_add
(const NV_ID *root, const NV_ID *key, const NV_ID *value)
{
	// 重複を許して追加する。
	return NV_NodeID_createRelation(root, key, value);	
}

NV_ID NV_Dict_addUniqueInKey
(const NV_ID *root, const NV_ID *key, const NV_ID *value)
{
	// 重複を許して追加する。
	return NV_NodeID_createRelation(root, key, value);	
}

NV_ID NV_Dict_addByStringKey
(const NV_ID *root, const char *key, const NV_ID *value)
{
	NV_ID strid = NV_Node_createWithString(key);
	return NV_Dict_add(root, &strid, value);
}


NV_ID NV_Dict_get(const NV_ID *root, const NV_ID *key)
{
	// keyが同じ値を持つ(IDが等しいとは限らない)オブジェクトを返す。
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ID_isEqual(&reld->from, root) && 
				NV_ID_isEqualInValue(&reld->rel, key)){
				return reld->to;
			}
		}
	}
	return NODEID_NOT_FOUND;
}

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
	printf("Dict ");
	NV_ID_printPrimVal(root);
	printf(":\n");
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(NV_ID_isEqual(&reld->from, root)){
				printf("|--- ");
				NV_ID_printPrimVal(&reld->rel);
				printf(": ");
				NV_printNodeByID(&reld->to);
				printf("\n");
			}
		}
	}
}
