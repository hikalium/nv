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

int32_t NV_getOpPrecAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	NV_ID ePrec = NV_Node_getRelatedNodeFrom(&op, &RELID_OP_PRECEDENCE);
	return NV_Node_getInt32FromID(&ePrec);
}

void NV_tryExecOpAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	NV_ID func = NV_Node_getRelatedNodeFrom(&op, &RELID_OP_FUNC);
	puts("begin op");
	NV_printNodeByID(&op);
	if(NV_Node_String_compareWithCStr(
		NV_Node_getByID(&func), "NV_Op_nothing") == 0){
		NV_Array_removeIndex(tList, index);
	} else if(NV_Node_String_compareWithCStr(
		NV_Node_getByID(&func), "NV_Op_add") == 0){
		NV_ID nL, nR, ans;
		int vL, vR;
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
		ans = NV_Node_createWithInt32(vL + vR);
		NV_Array_writeToIndex(tList, index, &ans);
	} else if(NV_Node_String_compareWithCStr(
		NV_Node_getByID(&func), "NV_Op_mul") == 0){
		NV_ID nL, nR, ans;
		int vL, vR;
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
		ans = NV_Node_createWithInt32(vL * vR);
		NV_Array_writeToIndex(tList, index, &ans);
	} else{
		NV_ID errObj = NV_Node_createWithString(
			"Error: Op NOT found or NOT implemented.");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
}

/*
void NV_tryExecOp(int32_t *excFlag, NV_Pointer lang, NV_Pointer thisTerm, NV_Pointer vDict, NV_Pointer root)
{
	NV_Pointer fallbackOp, op;
	NV_Pointer orgTerm = thisTerm;
	//
	op = NV_ListItem_getData(thisTerm);	
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_DbgInfo("%s", "Begin native op: ");
		NV_Operator_print(op); putchar('\n');
	}
#endif
	NV_Operator_exec(op, excFlag, lang, vDict, thisTerm);
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_DbgInfo("%s", "End native op:");
		NV_Operator_print(op); putchar('\n');
	}
#endif
	if(*excFlag & NV_EXC_FLAG_FAILED){
		// try fallback
		fallbackOp = NV_Lang_getFallbackOperator(lang, op);
		if(NV_E_isNullPointer(fallbackOp)){
			NV_Error("%s", "Operator mismatched: ");
			NV_Operator_print(op); putchar('\n');
			NV_List_printAll(root, NULL, NULL, "]\n");
			return;
		}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_DbgInfo("%s", "Fallback found:");
		NV_Operator_print(fallbackOp); putchar('\n');
	}
#endif
		CLR_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		NV_ListItem_setData(orgTerm, fallbackOp);
		thisTerm = orgTerm;
	}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_List_printAll(root, NULL, NULL, "]\n");
	}
#endif
}
*/

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
