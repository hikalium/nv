#include "nv.h"
// パスは、経路となるエッジ(Relation)IDもしくは値を要素にもつリストである。
// 始点ノードとパスを指定することで、パスの到達点となるノードを取得できる。
// 値がエッジとして指定されている場合には、その値と等価なエッジが選択される。

NV_ID NV_Path_createWithCodeBlock(NV_ID *code)
{
	// 単に元の配列からパス区切りとなる記号を除去した配列を生成する。
	NV_ID newArray, t;
	int i;
	//
	newArray = NV_Array_create();
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(code, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		if(NV_NodeID_String_compareWithCStr(&t, "/") == 0) continue;
		NV_Array_push(&newArray, &t);
	}
	return newArray;
}

NV_ID NV_Path_createWithCStr(const char *pathStr)
{
	NV_ID cTypeList, tokenList, path;
	//
	cTypeList = NV_createCharTypeList();
	tokenList = NV_tokenize(&cTypeList, pathStr);
	path = NV_Path_createWithCodeBlock(&tokenList);
	return path;
}
