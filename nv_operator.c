#include "nv.h"

NV_Operator *NV_E_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	//
	t->name = NV_NullPointer;
	t->precedence = NV_NullPointer;
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
		printf("(%s/%d: ", 
			NV_String_getCStr(op->name),
			NV_Integer_getImm32(op->precedence));
		
		if(NV_E_isType(op->body, EBlob)){
			nf = NV_Blob_getDataAsCPointer(op->body);
			if(nf){
				printf("native@%p)", nf);
			}
		} else if(NV_E_isType(op->body, EList)){
			NV_printElement(op->body); printf(")");
		}
	}
}

NV_Pointer NV_Operator_allocNative(int precedence, const char *name, NV_OpFunc nativeFunc)
{
	NV_Pointer opData;
	NV_Operator *opRawData;
	//
	opData = NV_E_malloc_type(EOperator);
	opRawData = NV_E_getRawPointer(opData, EOperator);
	//
	opRawData->name = NV_String_alloc(name);
	opRawData->precedence = NV_Integer_alloc(precedence);
	opRawData->body = NV_Blob_allocForCPointer(nativeFunc);
	//
	return opData;
}

NV_Pointer NV_Operator_allocNativeStruct(int precedence, const char *name, NV_OpFunc nativeFunc)
{
	NV_Pointer opData;
	//
	opData = NV_E_malloc_type(EList);
	//
	NV_List_push(opData, NV_E_autorelease(NV_String_alloc(name)));
	NV_List_push(opData, NV_E_autorelease(NV_Integer_alloc(precedence)));
	NV_List_push(opData, NV_E_autorelease(NV_Blob_allocForCPointer(nativeFunc)));
	//
	return opData;
}

NV_Pointer NV_Operator_alloc(NV_Pointer prec, NV_Pointer name, NV_Pointer body)
{
	NV_Pointer opData;
	NV_Operator *opRawData;
	//
	if(	!NV_E_isType(prec, EInteger) || 
		!NV_E_isType(name, EString) ||
		!NV_E_isType(body, EList)){
		NV_Error("%s", "Type check failed.");
		return NV_NullPointer;
	}
	//
	opData = NV_E_malloc_type(EOperator);
	opRawData = NV_E_getRawPointer(opData, EOperator);
	opRawData->name			= NV_E_clone(name);
	opRawData->precedence	= NV_E_clone(prec);
	opRawData->body			= NV_E_clone(body);
	//
	return opData;
}

int NV_getOperatorPrecedence(NV_Pointer op)
{
	NV_Operator *opData;
	opData = NV_E_getRawPointer(op, EOperator);
	if(!opData) return -1;
	return NV_Integer_getImm32(opData->precedence);
}

void
NV_Operator_exec
(NV_Pointer op, int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisTerm)
{
	NV_Operator *opData;
	NV_OpFunc nf;
	opData = NV_E_getRawPointer(op, EOperator);
	if(opData){
		if(NV_E_isType(opData->body, EBlob)){
			nf = NV_Blob_getDataAsCPointer(opData->body);
			if(nf){
				nf(excFlag, lang, vDict, thisTerm);
			} else{
				NV_Error("%s", "naitive func is null!");
			}
		} else if(NV_E_isType(opData->body, EList)){
			NV_Pointer subScope, tmp;
			NV_Pointer cRoot = NV_E_clone(opData->body);
			subScope = NV_Variable_allocNewScope(vDict);
			//
			tmp = NV_Variable_allocByCStr(subScope, "thisItem");
			NV_Variable_assignData(tmp, thisTerm);
			NV_E_free(&tmp);
			//
			NV_evaluateSentence(excFlag, lang, subScope, cRoot);
			//
			NV_E_free(&cRoot);
			NV_E_free(&subScope);
		} else{
			NV_Error("%s", "Unknown op body type!!");
		}
	} else{
		NV_Error("%s", "opData is NULL!");
	}
}
