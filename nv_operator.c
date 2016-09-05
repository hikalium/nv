#include "nv.h"

NV_Operator *NV_E_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	//
	t->name = NV_NullPointer;
	t->precedence = NV_NullPointer;
	t->nativeFunc = NV_NullPointer;
	t->body = NV_NullPointer;
	return t;
}

NV_Pointer NV_Operator_clone(NV_Pointer p)
{
	// do nothing
	return NV_E_retain(p);
}


void NV_Operator_print(NV_Pointer t)
{
	NV_Operator *op;
	NV_OpFunc nf;

	op = NV_E_getRawPointer(t, EOperator);
	if(op){
		nf = NV_Blob_getDataAsCPointer(op->nativeFunc);
		if(nf){
			printf("(");
			NV_printElement(op->name);
			printf("/%d: native@%p)", 
				NV_Integer_getImm32(op->precedence), nf);
		}
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
	opRawData->name = NV_String_alloc(name);
	opRawData->precedence = NV_Integer_alloc(precedence);
	opRawData->nativeFunc = NV_Blob_allocForCPointer(nativeFunc);
	//
	return opData;
}

//NV_Pointer NV_Operator_allocWithBody(int precedence, const cha *name, )

int NV_getOperatorPrecedence(NV_Pointer op)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	if(!opData) return -1;
	return NV_Integer_getImm32(opData->precedence);
}

NV_Pointer
NV_Operator_exec
(NV_Pointer op, int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisTerm)
{
	NV_Operator *opData;
	NV_OpFunc nf;
	opData = NV_E_getRawPointer(op, EOperator);
	if(opData){
		nf = NV_Blob_getDataAsCPointer(opData->nativeFunc);
		return nf(excFlag, lang, vDict, thisTerm);
	} else{
		NV_Error("%s", "dynamic op!");
	}
	return NV_NullPointer;
}
