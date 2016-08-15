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

void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Pointer (*nativeFunc)(NV_Pointer env, NV_Pointer thisTerm))
{
	NV_Pointer t;
	NV_Operator *rawData;

	t = NV_E_malloc_type(EOperator);
	rawData = NV_E_getRawPointer(t, EOperator);
	//
	NV_strncpy(rawData->name, name, sizeof(rawData->name), strlen(name));
	rawData->precedence = precedence;
	rawData->nativeFunc = nativeFunc;
	// op list is sorted in a descending order of precedence.
/*
	for(p = &lang->opRoot; *p; p = &(*p)->next){
		if((*p)->precedence < t->precedence) break;
	}
*/
	//t->next = *p;
	//*p = t;
}
/*
NV_Pointer NV_getOperatorFromString(NV_LangDef *lang, const char *termStr)
{
	NV_Pointer p;
	NV_Operator *op;
	for(p = lang->opRoot; !NV_E_isNullPointer(p); p = NV_List_getNextItem(p)){
		op = NV_E_getRawPointer(p, EOperator);
		if(strcmp(op->name, termStr) == 0){
			return p;
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
	NV_Pointer p;
	int i = 0;
	if(NV_E_isValidPointer(op)){
		for(p = lang->opRoot; !NV_E_isNullPointer(p); p = NV_List_getNextItem(p)){
			if(NV_E_isValidPointer(p) && op.data == p.data) return i;
			i++;
		}
	}
	return -1;
}
*/
