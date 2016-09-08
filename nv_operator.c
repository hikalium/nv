#include "nv.h"

struct NV_OPERATOR {
	NV_Pointer target;	// [name, prec, body]
};

NV_Operator *NV_E_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	t->target = NV_NullPointer;
	return t;
}

NV_Pointer NV_Operator_clone(NV_Pointer p)
{
	// do nothing
	return NV_E_retain(p);
}


void NV_Operator_print(NV_Pointer op)
{
	NV_Operator *t;
	t = NV_E_getRawPointer(op, EOperator);
	if(t){
		printf("(%s/%d: ", 
			NV_String_getCStr(NV_Operator_getName(op)),
			NV_Operator_getPrecedence(op));
			NV_printElement(t->target); printf(")");
	}
}

NV_Pointer NV_Operator_allocNative(int precedence, const char *name, NV_OpFunc nativeFunc)
{
	NV_Pointer op;
	NV_Operator *t;
	//
	op = NV_E_malloc_type(EOperator);
	t = NV_E_getRawPointer(op, EOperator);
	//
	t->target = NV_Operator_allocNativeStruct(precedence, name, nativeFunc);
	//
	return op;
}

NV_Pointer NV_Operator_allocNativeStruct(int precedence, const char *name, NV_OpFunc nativeFunc)
{
	NV_Pointer opStruct;
	//
	opStruct = NV_E_malloc_type(EList);
	//
	NV_List_push(opStruct, NV_E_autorelease(NV_String_alloc(name)));
	NV_List_push(opStruct, NV_E_autorelease(NV_Integer_alloc(precedence)));
	NV_List_push(opStruct, NV_E_autorelease(NV_Blob_allocForCPointer(nativeFunc)));
	//
	return opStruct;
}

NV_Pointer NV_Operator_alloc(NV_Pointer prec, NV_Pointer name, NV_Pointer body)
{
	NV_Pointer op;
	NV_Operator *t;
	//
	op = NV_E_malloc_type(EOperator);
	t = NV_E_getRawPointer(op, EOperator);
	//
	t->target = NV_Operator_allocStruct(prec, name, body);
	//
	return op;
}

NV_Pointer NV_Operator_allocStruct(NV_Pointer prec, NV_Pointer name, NV_Pointer body)
{
	NV_Pointer opStruct;
	//
	opStruct = NV_E_malloc_type(EList);
	//
	NV_List_push(opStruct, NV_E_autorelease(NV_E_clone(prec)));
	NV_List_push(opStruct, NV_E_autorelease(NV_E_clone(name)));
	NV_List_push(opStruct, NV_E_autorelease(NV_E_clone(body)));
	//
	return opStruct;
}

int NV_Operator_getPrecedence(NV_Pointer op)
{
	NV_Operator *t;
	t = NV_E_getRawPointer(op, EOperator);
	if(!t) return -1;
	return NV_Integer_getImm32(NV_List_getDataByIndex(t->target, 1));
}

NV_Pointer NV_Operator_getName(NV_Pointer op)
{
	NV_Operator *t;
	t = NV_E_getRawPointer(op, EOperator);
	if(!t) return NV_NullPointer;
	return NV_List_getDataByIndex(t->target, 0);
}

void
NV_Operator_exec
(NV_Pointer op, int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisTerm)
{
	NV_Operator *t;
	NV_OpFunc nf;
	NV_Pointer body;
	t = NV_E_getRawPointer(op, EOperator);
	if(t){
		body = NV_List_getDataByIndex(t->target, 2);
		if(NV_E_isType(body, EBlob)){
			nf = NV_Blob_getDataAsCPointer(body);
			if(nf){
				nf(excFlag, lang, vDict, thisTerm);
			} else{
				NV_Error("%s", "naitive func is null!");
			}
		} else if(NV_E_isType(body, EList)){
			NV_Pointer subScope, tmp;
			NV_Pointer cRoot = NV_E_clone(body);
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
