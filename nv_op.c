#include "nv.h"

//
// public
//
#define NV_LANG_CHAR_LIST_LEN 3
int NV_Lang_getCharType(const NV_ID *cTypeList, char c)
{
	NV_ID t;
	int i;
	if(c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		t = NV_Array_getByIndex(cTypeList, i);
		if(NV_Node_String_strchr(&t, c)) break;
	}
	return i;
}

void NV_addOp(const NV_ID *opDict, const char *token, int32_t prec, const NV_ID *func)
{
	NV_ID opDir;
	NV_ID opEntry;
	NV_ID ePrec;
	// まずtokenごとに分けたDirがある
	opDir = NV_Dict_getByStringKey(opDict, token);
	if(NV_NodeID_isEqual(&opDir, &NODEID_NOT_FOUND)){
		// このtokenは初出なので新規追加
		opDir = NV_Array_create();
		NV_Dict_addKeyByCStr(opDict, token, &opDir);
	}
	// opEntry(ひとつのOpを表現)を作成
	opEntry = NV_Node_create();
	NV_NodeID_createRelation(
		&opEntry, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_OP);
	ePrec = NV_Node_createWithInt32(prec);
	NV_NodeID_createRelation(
		&opEntry, &RELID_OP_PRECEDENCE, &ePrec);
	NV_NodeID_createRelation(
		&opEntry, &RELID_OP_FUNC, func);
	// opEntryをopDirに追加
	NV_Array_push(&opDir, &opEntry);
}

void NV_addBuiltinOp(const NV_ID *opDict, const char *token, int32_t prec, const char *funcStr)
{
	NV_ID funcStrID;
	funcStrID = NV_Node_createWithString(funcStr);
	NV_addOp(opDict, token, prec, &funcStrID);
}


int NV_isBuiltinOp(const NV_ID *term, const char *ident)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(term, &RELID_OP_FUNC);
	return NV_Node_String_compareWithCStr(&func, ident) == 0;
}

const char *NV_Op_getOpFuncNameCStr(const NV_ID *op)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	return NV_NodeID_getCStr(&func);
}

int32_t NV_getOpPrec(const NV_ID *op)
{
	NV_ID ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	return NV_NodeID_getInt32(&ePrec);
}

void NV_getOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, NV_ID *idBuf, int count)
{
	int i;
	for(i = 0; i < count; i++){
		idBuf[i] = NV_Array_getByIndex(tList,  baseIndex + relIndexList[i]);
	}
}

void NV_removeOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, int count)
{
	// relIndexListが昇順にソートされていると仮定している．
	int i;
	for(i = count - 1; i >= 0; i--){
		NV_Array_removeIndex(tList, relIndexList[i] + baseIndex);
	}
}

NV_ID NV_Op_codeBlock
(const NV_ID *tList, int index, const char *openTerm, const char *closeTerm)
{
	NV_ID v;
	//
	NV_ID root;
	int nc = 1;	// nest count
	//
	root = NV_Array_create();
	for(;;){
		v = NV_Array_getByIndex(tList, index + 1);
		if(NV_NodeID_isEqual(&v, &NODEID_NOT_FOUND)){
			// おかしい
			return NV_Node_createWithString(
				"Error: Expected closeTerm but not found.");
		}
		NV_Array_removeIndex(tList, index + 1);
		if(NV_Node_String_compareWithCStr(&v, openTerm) == 0){
			// 開きかっこ
			nc++;
		}
		if(NV_Node_String_compareWithCStr(&v, closeTerm) == 0){
			// 終了
			nc--;
			if(nc == 0) break;
		}
		NV_Array_push(&root, &v);
	}
	NV_Array_writeToIndex(tList, index, &root);
	return NODEID_NULL;
}


NV_ID NV_Op_strLiteral(const NV_ID *tList, int index)
{
	NV_ID v, s;
	//
	NV_ID root;
	int esc = 0;	// escape flag
	//
	root = NV_Array_create();
	for(;;){
		v = NV_Array_getByIndex(tList, index + 1);
		if(NV_NodeID_isEqual(&v, &NODEID_NOT_FOUND)){
			// おかしい
			return NV_Node_createWithString(
				"Error: Expected \" but not found.");
		}
		NV_Array_removeIndex(tList, index + 1);
		if(esc){
			esc = 0;
			NV_Array_push(&root, &v);
		} else{
			if(NV_Node_String_compareWithCStr(&v, "\\") == 0){
				esc = 1;
				continue;
			}
			if(NV_Node_String_compareWithCStr(&v, "\"") == 0){
				// 終了
				break;
			}
			NV_Array_push(&root, &v);
		}
	}
	s = NV_Array_joinWithCStr(&root, "");
	NV_Array_writeToIndex(tList, index, &s);
	return NODEID_NULL;
}

void NV_printOp(const NV_ID *op)
{
	NV_ID eFunc;
	NV_ID ePrec;
	eFunc = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	printf("(op ");
	NV_Term_print(&eFunc);
	printf("/");
	NV_Term_print(&ePrec);
	printf(")");
}
