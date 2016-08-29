#include "nv.h"

struct NV_STRING {
	char *s;
};

//
// NV_Element
//

NV_String *NV_E_allocString()
{
	NV_String *t;
	t = NV_malloc(sizeof(NV_String));
	t->s = NV_malloc(8);
	t->s[0] = 0;
	return t;
}

void NV_E_free_internal_String(NV_Pointer p, NV_Pointer pool)
{
	NV_String *v;
	v = NV_E_getRawPointer(p, EString);
NV_DbgInfo("%p", v->s);
	if(v) NV_free(v->s);
}

//
// NV_String
//

NV_Pointer NV_String_clone(NV_Pointer p)
{
	NV_String *v;
	NV_Pointer c;
	if(!NV_E_isType(p, EString)) return NV_NullPointer;
	v = NV_E_getRawPointer(p, EString);
	if(!v || !v->s) return NV_NullPointer;
	c = NV_E_malloc_type(EString);
	NV_String_setString(c, v->s);
	return c;
}

void NV_String_setString(NV_Pointer strItem, const char *s)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	int len = strlen(s);
	if(!v) return;
NV_DbgInfo("%p", v->s);
	NV_free(v->s);
	v->s = NV_malloc(len + 1);
	NV_strncpy(v->s, s, len + 1, len + 1);
}

int NV_String_isEqualToCStr(NV_Pointer strItem, const char *cstr)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	if(!v || !v->s) return 0;
	return (strcmp(v->s, cstr) == 0);
}

int NV_String_isEqual(NV_Pointer str0, NV_Pointer str1)
{
	NV_String *v0 = NV_E_getRawPointer(str0, EString);
	NV_String *v1 = NV_E_getRawPointer(str1, EString);
	if(!v0 || !v0->s || !v1 || !v1->s) return 0;
	return (strcmp(v0->s, v1->s) == 0);
}

void NV_String_print(NV_Pointer strItem)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	if(!v) return;
	printf("\"%s\"", v->s);
}
