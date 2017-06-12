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

int NV_Dict_Internal_merge(void *d, const NV_ID *rel, const NV_ID *to)
{
	NV_Dict_addUniqueIDKey((const NV_ID *)d, rel, to);
	return 1;
}

NV_ID NV_Dict_createMergedNode(const NV_ID *a, const NV_ID *b)
{
	NV_ID m = NV_Node_create();
	NV_Dict_foreach(a, &m, NV_Dict_Internal_merge);
	NV_Dict_foreach(b, &m, NV_Dict_Internal_merge);
	return m;
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

int NV_Dict_foreach
(const NV_ID *dict, void *d, int (*f)(void *d, const NV_ID *rel, const NV_ID *to))
{
	// ノードdictを起点とするすべての関係をたどって。そのrelationとtoをfに引き渡す。
	// fの戻り値がfalseの場合はそこでループを中止する。
	// 戻り値は、fを呼んだ回数である。
	const NV_Node *n;
	const NV_Relation *reld;
	int count = 0;
	for(n = NV_Node_getNextNode(&nodeRoot); n; n = NV_Node_getNextNode(n)){
		reld = NV_Node_getDataAsType(n, kRelation);
		if(reld && NV_NodeID_isEqual(&reld->from, dict)){
			count++;
			if(!f(d, &reld->rel, &reld->to)) break;
		}
	}
	return count;
}

int NV_Dict_foreachWithRelFilter
(const NV_ID *dict, void *d, int (*f)(void *d, const NV_ID *rel, const NV_ID *to), int (*filter)(const NV_ID *rel))
{
	// ノードdictを起点とするすべての関係をたどって。そのrelationとtoをfに引き渡す。
	// filterがtrueを返す関係のノードのみたどる。
	// fの戻り値がfalseの場合はそこでループを中止する。
	// 戻り値は、fを呼んだ回数である。
	const NV_Node *n;
	const NV_Relation *reld;
	int count = 0;
	NV_ID id;
	for(n = NV_Node_getNextNode(&nodeRoot); n; n = NV_Node_getNextNode(n)){
		reld = NV_Node_getDataAsType(n, kRelation);
		id = NV_Node_getID(n);
		if(reld && NV_NodeID_isEqual(&reld->from, dict) && filter(&id)){
			count++;
			if(!f(d, &reld->rel, &reld->to)) break;
		}
	}
	return count;
}

NV_ID NV_Dict_getByStringKey
(const NV_ID *root, const char *key)
{
	NV_ID strid = NV_Node_createWithString(key);
	return NV_Dict_get(root, &strid);
}

int NV_Dict_Internal_printSub(void *d, const NV_ID *rel, const NV_ID *to)
{
	printf("├─── ");
	NV_Node_printPrimVal(rel);
	printf(": ");
	NV_Node_printPrimVal(to);
	printf("\n");
	(*(int *)d)++;
	return 1;
}

void NV_Dict_print(const NV_ID *root)
{
	int cnt = 0;
	NV_Node_printPrimVal(root);
	printf(":\n");
	NV_Dict_foreach(root, &cnt, NV_Dict_Internal_printSub);
	printf("(%d items)\n", cnt);
}

typedef struct {
	int depth;
	int current;
} NV_Dict_DepthInfo;

int NV_Dict_Internal_printWithDepthSub(void *d, const NV_ID *rel, const NV_ID *to)
{
	NV_Dict_DepthInfo *info = d;
	int i;
	for(i = 0; i < info->current; i++){
		printf("│   ");
	}
	printf("├── ");
	NV_Node_printPrimVal(rel);
	printf(": ");
	NV_Dict_printWithDepth(to, info->depth, info->current + 1);
	
	return 1;
}

void NV_Dict_printWithDepth(const NV_ID *root, int depth, int current)
{

	NV_Node_printPrimVal(root); printf("\n");
	if(depth <= current) return;
	NV_Dict_DepthInfo info;
	info.depth = depth;
	info.current = current;
	NV_Dict_foreach(root, &info, NV_Dict_Internal_printWithDepthSub);
}

