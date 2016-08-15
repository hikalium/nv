#include "nv.h"

struct NV_ELEMENT {
    NV_ElementType type;
    int token;
    void *data;
};

NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data);

NV_Element *freeRoot = NULL;

NV_Pointer NV_E_malloc_type(NV_ElementType type)
{
	NV_Pointer p;
	switch(type){
		case EEnv:
			return NV_E_malloc_internal(EEnv, NV_allocEnv());
		default:
			NV_Error("Unknown element type %d\n", type);
			p.data = NULL;
			p.token = rand();
	}
	return p;
}

void NV_E_free(NV_Pointer p)
{
	NV_Element *e = p.data;
	if(!NV_E_isValidPointer(p)) return;
	e->token = rand();
	e->type = ENone;
	NV_free(e->data);
	//
	e->data = freeRoot;
	freeRoot = e;
}

int NV_E_isValidPointer(NV_Pointer p)
{
	NV_Element *e = p.data;
	if(!e){
		NV_Error("NULL pointer passed: token %d\n", p.token);
		return 0;
	}
	if(e->token != p.token){
		NV_Error("Attemp to free memory already freed: token %d\n", p.token);
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

