#include "nv.h"
// パスは、経路となるエッジ(Relation)IDもしくは値を要素にもつリストである。
// 始点ノードとパスを指定することで、パスの到達点となるノードを取得できる。
// 値がエッジとして指定されている場合には、その値と等価なエッジが選択される。

NV_ID NV_Path_createWithOrigin(const NV_ID *origin)
{
	NV_ID newArray, path;
	//
	newArray = NV_Array_create();
	path = NV_Node_create();
	NV_NodeID_createRelation(&path, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_PATH);
	NV_Dict_addKeyByCStr(&path, "route", &newArray);
	NV_Dict_addKeyByCStr(&path, "origin", origin);
	//
	return path;
}

NV_ID NV_Path_createAbsoluteWithCodeBlock(NV_ID *code)
{
	// 単に元の配列からパス区切りとなる記号を除去した配列を生成する。
	NV_ID newArray, t, path;
	int i;
	//
	path = NV_Path_createWithOrigin(&NODEID_NULL);
	newArray = NV_Dict_getByStringKey(&path, "route");
	//
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(code, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		if(NV_Node_String_compareWithCStr(&t, "/") == 0) continue;
		NV_Array_push(&newArray, &t);
	}
	return path;
}

NV_ID NV_Path_createAbsoluteWithCStr(const char *pathStr)
{
	NV_ID cTypeList, tokenList, path;
	//
	cTypeList = NV_createCharTypeList();
	tokenList = NV_tokenize(&cTypeList, pathStr);
	path = NV_Path_createAbsoluteWithCodeBlock(&tokenList);
	return path;
}

void NV_Path_appendRoute(const NV_ID *path, const NV_ID *r)
{
	NV_ID route;
	route = NV_Dict_getByStringKey(path, "route");
	NV_Array_push(&route, r);
}

NV_ID NV_Path_getTarget(const NV_ID *path)
{
	NV_ID t, route, p;
	int i;
	//
	route = NV_Dict_getByStringKey(path, "route");
	p = NV_Dict_getByStringKey(path, "origin");
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(&route, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		p = NV_NodeID_getEqRelatedNodeFrom(&p, &t);
		if(NV_NodeID_isEqual(&p, &NODEID_NOT_FOUND)){
			printf("key not found:");
			NV_Term_print(&t);
		}
	}
	return p;
}

int NV_Path_statTarget(const NV_ID *path)
{
	// 0: not existed, 1: existed
	NV_ID t, route, p;
	int i;
	//
	route = NV_Dict_getByStringKey(path, "route");
	p = NV_Dict_getByStringKey(path, "origin");
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(&route, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		p = NV_NodeID_getEqRelatedNodeFrom(&p, &t);
		if(NV_NodeID_isEqual(&p, &NODEID_NOT_FOUND)){
			return 0;
		}
	}
	return 1;
}

void NV_Path_assign(const NV_ID *path, const NV_ID *data)
{
	NV_ID rel, route, p, q, relNode;
	int i;
	//
	relNode = NODEID_NOT_FOUND;
	route = NV_Dict_getByStringKey(path, "route");
	q = NV_Dict_getByStringKey(path, "origin");
	for(i = 0; ; i++){
		rel = NV_Array_getByIndex(&route, i);
		if(NV_NodeID_isEqual(&rel, &NODEID_NOT_FOUND)) break;
		p = q;
		relNode = NV_NodeID_getEqRelationFrom(&p, &rel);
		q = NV_NodeID_Relation_getIDLinkTo(&relNode);
		if(NV_NodeID_isEqual(&q, &NODEID_NOT_FOUND)){
			// 途中の経路で生成されていない部分があれば作る
			q = NV_Node_create();
			relNode = NV_NodeID_createRelation(&p, &rel, &q);
		}
	}
	if(!NV_NodeID_isEqual(&relNode, &NODEID_NOT_FOUND)){
		NV_NodeID_updateRelationTo(&relNode, data);
	}
}

NV_ID NV_Path_print(const NV_ID *path)
{
	NV_ID t, route, p;
	int i;
	//
	printf("(Path ");
	route = NV_Dict_getByStringKey(path, "route");
	p = NV_Dict_getByStringKey(path, "origin");
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(&route, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		p = NV_NodeID_getEqRelatedNodeFrom(&p, &t);
		NV_Term_print(&t);
		printf(" "); 
	}
	printf(")");
	return p;
}

