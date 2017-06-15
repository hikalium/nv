#include "nv.h"

//
// public
//

NV_ID NV_Lang_createCharTypeList(const char *cTypeList[])
{
	NV_ID ns;
	NV_ID cList = NV_Array_create();
	//
	ns = NV_Node_createWithString(cTypeList[0]);
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString(cTypeList[1]);
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString(cTypeList[2]);
	NV_Array_push(&cList, &ns);
	//
	return cList;
}

NV_ID NV_Lang_createOpDict(NV_Lang_OpTag *opList)
{
	// opList should be terminated with 
	// {"", -1, ""}
	NV_ID opDict = NV_Node_createWithString("NV_OpList");
	//
	int i;
	for(i = 0; opList[i].prec >= 0; i++){
		NV_Lang_addOpWithFuncStr(&opDict,
			opList[i].token, opList[i].prec, opList[i].funcStr);
	}
	//
	if(IS_DEBUG_MODE()){
		NV_Dict_print(&opDict);
	}
	return opDict;
}

int NV_Lang_getCharType(const NV_ID *cTypeList, char c)
{
	NV_ID t;
	int i;
	if(c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_TYPE_LIST_LEN; i++){
		t = NV_Array_getByIndex(cTypeList, i);
		if(NV_Node_String_strchr(&t, c)) break;
	}
	return i;
}

void NV_Lang_addOp
(const NV_ID *opDict, const char *token, int32_t prec, const NV_ID *func)
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

void NV_Lang_addOpWithFuncStr
(const NV_ID *opDict, const char *token, int32_t prec, const char *funcStr)
{
	NV_ID funcStrID;
	funcStrID = NV_Node_createWithString(funcStr);
	NV_Lang_addOp(opDict, token, prec, &funcStrID);
}


int NV_Lang_isOp(const NV_ID *term, const char *ident)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(term, &RELID_OP_FUNC);
	return NV_Node_String_compareWithCStr(&func, ident) == 0;
}

const char *NV_Lang_getOpFuncNameCStr(const NV_ID *op)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	return NV_NodeID_getCStr(&func);
}

int32_t NV_Lang_getOpPrec(const NV_ID *op)
{
	NV_ID ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	return NV_NodeID_getInt32(&ePrec);
}

void NV_Lang_getOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, NV_ID *idBuf, int count)
{
	int i;
	for(i = 0; i < count; i++){
		idBuf[i] = NV_Array_getByIndex(tList,  baseIndex + relIndexList[i]);
	}
}

void NV_Lang_removeOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, int count)
{
	// relIndexListが昇順にソートされていると仮定している．
	int i;
	for(i = count - 1; i >= 0; i--){
		NV_Array_removeIndex(tList, relIndexList[i] + baseIndex);
	}
}

NV_ID NV_Lang_parseCodeBlock
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


NV_ID NV_Lang_parseStrLiteral(const NV_ID *tList, int index)
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

void NV_Lang_printOp(const NV_ID *op)
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

NV_ID NV_Lang_parseToCodeGraph
(const NV_ID *baseTokenList, const NV_ID *opDict, const NV_Lang_FuncTag funcList[])
{
	// retv: codeGraphRoot
	NV_ID tokenList = NV_Array_clone(baseTokenList);
	/*
	printf("parsing tokens: ");
	NV_Array_print(&tokenList); putchar('\n');
	*/
	//printf("tokenList hash = %08X\n", NV_Term_calcHash(baseTokenList));
	NV_ID codeGraphRoot = NV_Node_createWithString("eval");
	NV_ID lastNode = codeGraphRoot;
	NV_OpPointer p;
	const char *reqFuncName = NULL;
	int i;
	NV_ID retv;

	//NV_Dict_print(opDict);

	for(;;){
		p = NV_getNextOp(&tokenList, opDict);
		if(p.index == -1) break;
		NV_ID n = NV_Array_getByIndex(&tokenList, p.index);
		if(NV_NodeID_isEqual(&n, &NODEID_NOT_FOUND)) break;
		reqFuncName = NV_Lang_getOpFuncNameCStr(&p.op);
		for(i = 0; funcList[i].name; i++){
			if(strcmp(reqFuncName, funcList[i].name) == 0) break;
		}
		if(funcList[i].name){
			if(IS_DEBUG_MODE()){
				printf("parse: %s\n", funcList[i].name);
			}
			retv = funcList[i].parser(
					&tokenList, &lastNode, &p, NV_NodeID_getCStr(&n));
			if(!NV_Term_isNull(&retv)){
				NV_ID triedPrec = NV_Node_createWithInt32(p.prec);
				NV_Dict_addUniqueEqKeyByCStr(&n, "triedPrec", &triedPrec);
			}
		} else{
			fprintf(stderr, "NV_parseToCodeGraph: op not implemented for");
			NV_NodeID_printForDebug(&n);
			putchar('\n');
			return NODEID_NULL;
		}
	}
	if(NV_globalExecFlag & NV_EXEC_FLAG_SAVECODEGRAPH){
		NV_saveCodeGraphForVisualization(&codeGraphRoot, "note/code");
	}
	return codeGraphRoot;
}
