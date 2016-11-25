#include "nv.h"

#define NV_LANG_CHAR_LIST_LEN 3
int NV_Lang_getCharType(const NV_ID *cTypeList, char c)
{
	NV_ID t;
	int i;
	if(c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		t = NV_Array_getByIndex(cTypeList, i);
		if(NV_Node_String_strchr(NV_Node_getByID(&t), c)) break;
	}
	return i;
}


NV_ID NV_createCharTypeList()
{
	NV_ID ns;
	NV_ID cList = NV_Array_create();
	//
	ns = NV_Node_createWithString(" \t\r\n");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("#!%&-=^~|+*:.<>/");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("(){}[],;\"`\\");
	NV_Array_push(&cList, &ns);
	//
	return cList;
}

void NV_addOp(const NV_ID *opList, const char *token, int32_t prec, const NV_ID *func)
{
	NV_ID opEntry;
	NV_ID ePrec;
	opEntry = NV_Node_create();
	NV_Node_createRelation(
		&opEntry, &RELID_TREE_TYPE, &NODEID_TREE_TYPE_OP);
	ePrec = NV_Node_createWithInt32(prec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_PRECEDENCE, &ePrec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_FUNC, func);
	
	//
	NV_Dict_addByStringKey(opList, token, &opEntry);
}

NV_ID NV_createOpList()
{
	NV_ID nv;
	NV_ID opList = NV_Node_createWithString("NV_OpList");
	//
	nv = NV_Node_createWithString("NV_Op_add");
	NV_addOp(&opList, "+", 100, &nv);
	//
	nv = NV_Node_createWithString("NV_Op_sub");
	NV_addOp(&opList, "-", 100, &nv);
	//
	nv = NV_Node_createWithString("NV_Op_mul");
	NV_addOp(&opList, "*", 200, &nv);
	//
	nv = NV_Node_createWithString("NV_Op_div");
	NV_addOp(&opList, "/", 200, &nv);
	//
	nv = NV_Node_createWithString("NV_Op_mod");
	NV_addOp(&opList, "%", 200, &nv);
	//
	nv = NV_Node_createWithString("NV_Op_nothing");
	NV_addOp(&opList, " ", 300, &nv);
	//
	NV_Dict_print(&opList);
	return opList;
}

void NV_Op_print(const NV_ID *op)
{
	NV_ID eFunc;
	NV_ID ePrec;
	eFunc = NV_Node_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	ePrec = NV_Node_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	printf("(op ");
	NV_printNodeByID(&eFunc);
	printf("/");
	NV_printNodeByID(&ePrec);
	printf(")");
}
