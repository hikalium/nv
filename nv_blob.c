#include "nv.h"

struct NV_BLOB {
	void *data;
	int size;
};

//
// NV_Element
//

NV_Blob *NV_E_allocBlob()
{
	NV_Blob *t;
	t = NV_malloc(sizeof(NV_Blob));
	t->data = NULL;
	t->size = 0;
	return t;
}

void NV_E_free_internal_Blob(NV_Pointer p)
{
	NV_Blob *t;
	t = NV_E_getRawPointer(p, EBlob);
	if(t && t->data){
		NV_free(t->data);
		t->size = 0;
	}
}

//
// NV_Blob
//
NV_Pointer NV_Blob_clone(NV_Pointer p)
{
	NV_Blob *t;
	NV_Pointer c;
	if(!NV_E_isType(p, EBlob)) return NV_NullPointer;
	t = NV_E_getRawPointer(p, EBlob);
	if(!t) return NV_NullPointer;
	c = NV_E_malloc_type(EBlob);
	NV_Blob_setDataSize(c, t->size);
	NV_Blob_copyRawData(c, t->data, t->size);
	return c;
}

NV_Pointer NV_Blob_allocForCPointer(void *p)
{
	NV_Pointer c;
	NV_Blob *t;
	c = NV_E_malloc_type(EBlob);
	NV_Blob_setDataSize(c, sizeof(void *));
	t = NV_E_getRawPointer(c, EBlob);
	*(void **)t->data = p;
	return c;
}

void NV_Blob_copyRawData(NV_Pointer dst, void *src, int copySize)
{
	NV_Blob *t;
	t = NV_E_getRawPointer(dst, EBlob);
	if(!t) return;
	memmove(t->data, src, MIN(copySize, t->size));
}

void *NV_Blob_getDataAsCPointer(NV_Pointer p)
{
	NV_Blob *t;
	t = NV_E_getRawPointer(p, EBlob);
	if(!t) return NULL;
	return *(void **)t->data; 
}

void NV_Blob_setDataSize(NV_Pointer blob, int size)
{
	NV_Blob *t = NV_E_getRawPointer(blob, EBlob);
	if(!t) return;
	if(t->data)NV_free(t->data);
	t->data = NV_malloc(size);
	t->size = size;
}

void NV_Blob_print(NV_Pointer blob)
{
	NV_Blob *t = NV_E_getRawPointer(blob, EBlob);
	if(!t) return;
	printf("(Blob)");
}

