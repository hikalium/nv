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
	if(v) NV_free(v->s);
}

//
// NV_String
//

NV_Pointer NV_String_alloc(const char *cs)
{
	NV_Pointer s;
	s = NV_E_malloc_type(EString);
	NV_String_setString(s, cs);
	return s;
}

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
	NV_free(v->s);
	v->s = NV_malloc(len + 1);
	NV_strncpy(v->s, s, len + 1, len + 1);
}

void NV_String_concatenateCStr(NV_Pointer str1, const char *s2)
{
	NV_Pointer s;
	NV_String *v = NV_E_getRawPointer(str1, EString);
	int len;
	char *buf;
	const char *s1 = NV_String_getCStr(str1);
	if(!s1) s1 = "";
	if(!s2) s2 = "";
	len = strlen(s1) + strlen(s2);
	s = NV_E_malloc_type(EString);
	v = NV_E_getRawPointer(s, EString);
	buf = NV_malloc(len + 1);
	NV_strncpy(buf, s1, strlen(s1) + 1, strlen(s1) + 1);
	NV_strncpy(buf + strlen(s1), s2, strlen(s2) + 1, strlen(s2) + 1);
	NV_String_setString(str1, buf);
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

void NV_String_convertFromEscaped(NV_Pointer s)
{
	NV_String *v = NV_E_getRawPointer(s, EString);
	const char *pr;
	char *pw;
	if(!v) return;
	pr = v->s;
	pw = v->s;
	while(*pr){
		if(*pr == '\\'){
			pr++;
			if(*pr == '\\'){
				*pw = '\\';
			} else if(*pr == '"'){
				*pw = '"';
			} else if(*pr == 'b'){
				*pw = 0x08;
			} else if(*pr == 't'){
				*pw = 0x09;
			} else if(*pr == 'n'){
				*pw = 0x0a;
			} else if(*pr == 'r'){
				*pw = 0x0d;
			} else{
				NV_Error("Invalid esc seq in [%s]", v->s);
				break;
			}
		} else{
			*pw = *pr;
		}
		pw++;
		pr++;
	}
	*pw = 0;
	return; 
}

const char *NV_String_getCStr(NV_Pointer s)
{
	NV_String *v = NV_E_getRawPointer(s, EString);
	if(!v) return NULL;
	return v->s; 
}

char NV_String_charAt(NV_Pointer strItem, int index)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	if(!v || !v->s || index >= strlen(v->s)) return 0;
	return v->s[index];
}

char *NV_String_strchr(NV_Pointer strItem, char c)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	if(!v || !v->s) return NULL;
	return strchr(v->s, c);
}

void NV_String_print(NV_Pointer strItem)
{
	NV_String *v = NV_E_getRawPointer(strItem, EString);
	if(!v) return;
	printf("\"%s\"", v->s);
}

