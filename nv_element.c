#include "nv.h"

struct NV_ELEMENT {
    NV_ElementType type;
    int token;
    void *data;
};

NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data);

NV_Element *freeRoot = NULL;
const NV_Pointer NV_NullPointer = {
	0,
	NULL
};

NV_Pointer NV_E_malloc_type(NV_ElementType type)
{
	NV_Pointer p;
	switch(type){
		case EList:
			return NV_E_malloc_internal(EList,		NV_allocList());
		case EEnv:
			return NV_E_malloc_internal(EEnv,		NV_allocEnv());
		case EOperator:
			return NV_E_malloc_internal(EOperator,	NV_allocOperator());
		case EInteger:
			return NV_E_malloc_internal(EInteger, 	NV_allocInteger());
		//case EString:
			//return NV_E_malloc_internal(EString, 	NV_allocString());
		case EVariable:
			return NV_E_malloc_internal(EVariable, 	NV_allocVariable());
		default:
			NV_Error("Unknown element type %d\n", type);
			p = NV_NullPointer;
	}
	return p;
}

int NV_E_isNullPointer(NV_Pointer p)
{
	if(p.data == NULL) return 1;
	return 0;
}

void NV_E_free(NV_Pointer *p)
{
	if(!p) return;
	NV_Element *e = p->data;
	if(!NV_E_isValidPointer(*p)) return;
	e->token = rand();
	e->type = ENone;
	NV_free(e->data);
	//
	e->data = freeRoot;
	freeRoot = e;
	//
	*p = NV_NullPointer;
}

int NV_E_isValidPointer(NV_Pointer p)
{
	NV_Element *e = p.data;
	if(!e){
		//NV_Error("NULL pointer passed: token %d\n", p.token);
		return 0;
	}
	if(e->token != p.token){
		return 0;
	}
	return 1;
}

int NV_E_isType(NV_Pointer p, NV_ElementType et)
{
	if(!NV_E_isValidPointer(p)) return 0;
	if(p.data->type != et) return 0;
	return 1;
}

void *NV_E_getRawPointer(NV_Pointer p, NV_ElementType et)
{
	if(!NV_E_isType(p, et)) return NULL;
	return p.data->data;
}

void NV_printElement(NV_Pointer p)
{
	if(NV_E_isValidPointer(p)){
		if(NV_E_isType(p, EInteger)){
			NV_Integer_print(p);		
		}
	} else{
		printf("(Invalid) %p ", p.data);
	}
}

//
// internal function
//

NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data)
{
	NV_Element *e;
	NV_Pointer p;
	//
	if(freeRoot){
		e = freeRoot;
		freeRoot = e->data;
	} else{
		e = NV_malloc(sizeof(NV_Element));
	}
	e->type = type;
	e->token = rand();
	e->data = data;
	//
	p.token = e->token;
	p.data = e;
	return p;
}

