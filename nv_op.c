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
	ns = NV_Node_createWithString("#!%&-=^~|+*:.<>/$");
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
		&opEntry, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_OP);
	ePrec = NV_Node_createWithInt32(prec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_PRECEDENCE, &ePrec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_FUNC, func);
	
	//
	NV_Dict_addByStringKey(opList, token, &opEntry);
}

void NV_addBuiltinOp(const NV_ID *opList, const char *token, int32_t prec, const char *funcStr)
{
	NV_ID funcStrID;
	funcStrID = NV_Node_createWithString(funcStr);
	NV_addOp(opList, token, prec, &funcStrID);
}

typedef struct NV_BUILTIN_OP_TAG {
	const char *token;
	int prec;
	const char *funcStr;
} NV_BuiltinOpTag;

NV_BuiltinOpTag builtinOpList[] = {
	{"ls",		0,		"NV_Op_ls"},
	{"save",	0,		"NV_Op_save"},
	{"restore",	0,		"NV_Op_restore"},
	{"show",	0,		"NV_Op_convToVal"},
	{"=",		0,		"NV_Op_assign"},
	{"+",		100,	"NV_Op_add"},
	{"-",		100,	"NV_Op_sub"},
	{"*",		200,	"NV_Op_mul"},
	{"/",		200,	"NV_Op_div"},
	{"%",		200,	"NV_Op_mod"},
	{" ",		300,	"NV_Op_nothing"},
	{"$",		300,	"NV_Op_getVarNamed"},
	//
	{"", -1, ""}	// terminate tag
};

NV_ID NV_createOpList()
{
	NV_ID opList = NV_Node_createWithString("NV_OpList");
	//
	int i;
	for(i = 0; builtinOpList[i].prec >= 0; i++){
		NV_addBuiltinOp(&opList,
			builtinOpList[i].token, builtinOpList[i].prec, builtinOpList[i].funcStr);
	}
	//
	NV_Dict_print(&opList);
	return opList;
}

int32_t NV_getOpPrecAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	NV_ID ePrec = NV_Node_getRelatedNodeFrom(&op, &RELID_OP_PRECEDENCE);
	return NV_Node_getInt32FromID(&ePrec);
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

void NV_Op_ExecBuiltinInfix(const NV_ID *tList, int index, int func)
{
	NV_ID nL, nR, ans;
	int vL, vR, v;
	nL = NV_Array_getByIndex(tList, index - 1);
	nR = NV_Array_getByIndex(tList, index + 1);
	if(!NV_Node_isInteger(&nL) || !NV_Node_isInteger(&nR)){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	vL = NV_Node_getInt32FromID(&nL);
	vR = NV_Node_getInt32FromID(&nR);
	//
	index--;
	NV_Array_removeIndex(tList, index);
	NV_Array_removeIndex(tList, index);
	//
	switch(func){
		case 0: v = vL + vR; break;
		case 1: v = vL - vR; break;
		case 2: v = vL * vR; break;
		case 3: v = vL / vR; break;
		case 4: v = vL % vR; break;
	}
	//
	ans = NV_Node_createWithInt32(v);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_save(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_Node_isString(&operand[0])){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	fname = NV_Node_getCStr(&operand[0]);
	if(!fname){
		NV_ID errObj = NV_Node_createWithString(
			"fname is null");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	FILE *fp = fopen(fname, "wb");
	if(!fp){
		NV_ID errObj = NV_Node_createWithString(
			"fopen failed");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	ans = NV_Node_createWithInt32(0);
	NV_Graph_dumpToFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_restore(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_Node_isString(&operand[0])){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	fname = NV_Node_getCStr(&operand[0]);
	if(!fname){
		NV_ID errObj = NV_Node_createWithString(
			"fname is null");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	FILE *fp = fopen(fname, "rb");
	if(!fp){
		NV_ID errObj = NV_Node_createWithString(
			"fopen failed");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	ans = NV_Node_createWithString("restore");
	NV_Graph_restoreFromFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_ls(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Dict_print(&NODEID_NULL);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_assign(const NV_ID *tList, int index)
{
	const int operandCount = 2;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {-1, 1};
	//
	NV_ID v = operand[0];
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_Term_isAssignable(&operand[0])){
		if(!NV_Node_isString(&operand[0])){
			NV_ID errObj = NV_Node_createWithString(
				"Error: Invalid Operand Type.");
			NV_Array_writeToIndex(tList, index, &errObj);
			return;
		}
		v = NV_Variable_createWithName(&NODEID_NULL, &operand[0]);
	}
	//
	NV_Variable_assign(&v, &operand[1]);	
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	NV_Array_writeToIndex(tList, index - 1, &v);
}

void NV_Op_convToVal(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	//
	NV_ID v;
	v = operand[0];
	if(NV_isTermType(&v, &NODEID_TERM_TYPE_VARIABLE)){
		// 右の項が変数だった
		v = NV_Variable_getData(&v);
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	NV_Array_writeToIndex(tList, index, &v);
}

void NV_Op_getVarNamed(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	//
	NV_ID v;
	v = operand[0];
	if(!NV_Node_isString(&operand[0])){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	v = NV_Variable_getNamed(&NODEID_NULL, &v);
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	NV_Array_writeToIndex(tList, index, &v);
}

int NV_isBuiltinOp(const NV_ID *term, const char *ident)
{
	return NV_Node_String_compareWithCStr(NV_Node_getByID(term), ident) == 0;
}

void NV_tryExecOpAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	NV_ID func = NV_Node_getRelatedNodeFrom(&op, &RELID_OP_FUNC);
	//
	printf("begin op ");
	NV_printNodeByID(&op);
	putchar('\n');
	//
	if(NV_isBuiltinOp(&func, "NV_Op_nothing")){
		NV_Array_removeIndex(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_add")){
		NV_Op_ExecBuiltinInfix(tList, index, 0);
	} else if(NV_isBuiltinOp(&func, "NV_Op_sub")){
		NV_Op_ExecBuiltinInfix(tList, index, 1);
	} else if(NV_isBuiltinOp(&func, "NV_Op_mul")){
		NV_Op_ExecBuiltinInfix(tList, index, 2);
	} else if(NV_isBuiltinOp(&func, "NV_Op_div")){
		NV_Op_ExecBuiltinInfix(tList, index, 3);
	} else if(NV_isBuiltinOp(&func, "NV_Op_mod")){
		NV_Op_ExecBuiltinInfix(tList, index, 4);
	} else if(NV_isBuiltinOp(&func, "NV_Op_save")){
		NV_Op_save(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_restore")){
		NV_Op_restore(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_ls")){
		NV_Op_ls(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_assign")){
		NV_Op_assign(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_convToVal")){
		NV_Op_convToVal(tList, index);
	} else if(NV_isBuiltinOp(&func, "NV_Op_getVarNamed")){
		NV_Op_getVarNamed(tList, index);
	} else{
		NV_ID errObj = NV_Node_createWithString(
			"Error: Op NOT found or NOT implemented.");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	printf("end op ");
	NV_printNodeByID(&op);
	putchar('\n');
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
