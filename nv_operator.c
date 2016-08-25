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

NV_Pointer NV_Operator_clone(NV_Pointer p)
{
	// do nothing
	return p;
}


void NV_Operator_print(NV_Pointer t)
{
	NV_Operator *op;
	op = NV_E_getRawPointer(t, EOperator);
	if(op){
		//printf("Op %s : %d @ %p", op->name, op->precedence, op->nativeFunc);
		printf("(%s/%d)", op->name, op->precedence);
	}
}

NV_Pointer NV_Operator_alloc(int precedence, const char *name, NV_OpFunc nativeFunc)
{
	NV_Pointer opData;
	NV_Operator *opRawData;
	//
	opData = NV_E_malloc_type(EOperator);
	opRawData = NV_E_getRawPointer(opData, EOperator);
	//
	NV_strncpy(opRawData->name, name, sizeof(opRawData->name), strlen(name));
	opRawData->precedence = precedence;
	opRawData->nativeFunc = nativeFunc;
	//
	return opData;
}

int NV_getOperatorPrecedence(NV_Pointer op)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	if(!opData) return -1;
	return opData->precedence;
}

NV_Pointer NV_Operator_exec(NV_Pointer op, NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	if(opData) return opData->nativeFunc(env, thisTerm);
	return NV_NullPointer;
}
