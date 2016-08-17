#include "nv.h"

NV_Operator *NV_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	//
	t->name[0] = 0;
	t->precedence = 0;
	t->nativeFunc = NULL;

	return t;
}

void NV_Operator_print(NV_Pointer t)
{
	NV_Operator *op;
	op = NV_E_getRawPointer(t, EOperator);
	if(op){
		printf("Op %s : %d @ %p", op->name, op->precedence, op->nativeFunc);
	}
}

void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Pointer (*nativeFunc)(NV_Pointer env, NV_Pointer thisTerm))
{
	NV_Pointer opData, tOpItem;
	NV_Operator *opRawData, *tOpRawData;
	

	opData = NV_E_malloc_type(EOperator);
	opRawData = NV_E_getRawPointer(opData, EOperator);
	//
	NV_strncpy(opRawData->name, name, sizeof(opRawData->name), strlen(name));
	opRawData->precedence = precedence;
	opRawData->nativeFunc = nativeFunc;
	// op list is sorted in a descending order of precedence.
	tOpItem = NV_List_getNextItem(lang->opRoot);
	for(; !NV_E_isNullPointer(tOpItem); tOpItem = NV_List_getNextItem(tOpItem)){
		tOpRawData = NV_List_getItemRawData(tOpItem, EOperator);
		if(tOpRawData->precedence < opRawData->precedence) break;
	}
	if(NV_E_isNullPointer(tOpItem)){
		NV_List_push(lang->opRoot, opData);
	} else{
		NV_List_insertDataBeforeItem(tOpItem, opData);
	}
}

NV_Pointer NV_getOperatorFromString(NV_LangDef *lang, const char *termStr)
{
	NV_Pointer p;
	NV_Operator *op;
	p = NV_List_getNextItem(lang->opRoot);
	for(; !NV_E_isNullPointer(p); p = NV_List_getNextItem(p)){
		op = NV_List_getItemRawData(p, EOperator);
		if(strcmp(op->name, termStr) == 0){
			return NV_List_getItemData(p);
		}
	}
	return NV_NullPointer;
}

NV_Pointer NV_getFallbackOperator(NV_LangDef *lang, NV_Pointer baseP)
{
	NV_Pointer p;
	NV_Operator *op = NULL;
	NV_Operator *baseOp = NV_E_getRawPointer(baseP, EOperator);
	//
	for(p = lang->opRoot; !NV_E_isNullPointer(p); p = NV_List_getNextItem(p)){
		op = NV_E_getRawPointer(p, EOperator);
		if(op == baseOp){
			p = NV_List_getNextItem(p);
			break;
		}
	}
	for(; !NV_E_isNullPointer(p); p = NV_List_getNextItem(p)){
		op = NV_E_getRawPointer(p, EOperator);
		if(strcmp(op->name, baseOp->name) == 0){
			return p;
		}
	}
	return NV_NullPointer;
}

int NV_getOperatorIndex(NV_LangDef *lang, NV_Pointer op)
{
	return NV_List_indexOfData(lang->opRoot, op);
}

int NV_Operator_isLeftAssociative(NV_Pointer op)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	return !(opData && (opData->precedence & 1));
}

NV_Pointer NV_Operator_exec(NV_Pointer op, NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	if(opData) return opData->nativeFunc(env, thisTerm);
	return NV_NullPointer;
}
