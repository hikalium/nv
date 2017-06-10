#include "nv.h"

//
// Internal
//
/*
//void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(n){
		if(n->type != kNone){
			//NV_Node_Internal_resetData(n)
			printf("Try to modify data existed. abort.");
			exit(EXIT_FAILURE);
		};
		n->type = kString;
		n->size = strlen(s) + 1;
		n->data = NV_malloc(n->size);
		NV_strncpy(n->data, s, n->size, strlen(s));
		((char *)n->data)[n->size - 1] = 0;
	}
}
*/
long NV_Node_String_Internal_strtol(const NV_Node *ns, int *endptrindex, int base)
{
	long v;
	char *ep;
	const char *str = NV_Node_getDataAsType(ns, kString);
	if(!str){
		if(endptrindex) *endptrindex = 0; 
		return 0;
	}
	v = strtol(str, &ep, base);
	if(endptrindex) *endptrindex = ep - str;
	return v;
}


//
// String
//

int NV_NodeID_isString(const NV_ID *id)
{
	return (NV_Node_getType(id) == kString);
}

NV_ID NV_Node_createWithString(const char *s)
{
	// string will be copied
	NV_ID id = NV_ID_generateRandom();
	return NV_Node_createWith_ID_String(&id, s);
}

NV_ID NV_Node_createWith_ID_String(const NV_ID *id, const char *s)
{
	// string will be copied
	size_t len = strlen(s);
	size_t size = len + 1;
	char *buf = NV_malloc(size);
	NV_strncpy(buf, s, size, len);
	return NV_Node_createWith_ID_Type_Data_Size(id, kString, buf, size);
}

NV_ID NV_Node_createWithStringFormat(const char *fmt, ...)
{
	char *s;
	int sLen;
	va_list ap;
	NV_ID newID;
	//
	sLen = strlen(fmt) * 2;
	s = NV_malloc(sLen);
	//
	va_start(ap, fmt);
	vsnprintf(s, sLen, fmt, ap);
	va_end(ap);
	//
	newID = NV_Node_createWithString(s);
	//
	NV_free(s);
	//
	return newID;
}

const char *NV_NodeID_getCStr(const NV_ID *id)
{
	return NV_NodeID_getDataAsType(id, kString);
}

int NV_Node_String_compare(const NV_ID *ida, const NV_ID *idb)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	const char *strA = NV_NodeID_getDataAsType(ida, kString);
	const char *strB = NV_NodeID_getDataAsType(idb, kString);
	if(!strA || !strB) return -1;
	return strcmp(strA, strB);
}

int NV_Node_String_compareWithCStr(const NV_ID *ida, const char *s)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	const char *str = NV_NodeID_getDataAsType(ida, kString);
	if(!str) return -1;
	return strcmp(str, s);
}

char *NV_Node_String_strchr(const NV_ID *id, char c)
{
	const char *str = NV_NodeID_getDataAsType(id, kString);
	if(!str) return NULL;
	return strchr(str, c);
}

long NV_Node_String_strtol(const NV_ID *ns, int *endptrindex, int base)
{
	return NV_Node_String_Internal_strtol(NV_NodeID_getNode(ns), endptrindex, base);

}

size_t NV_Node_String_strlen(const NV_ID *id)
{
	const char *str = NV_NodeID_getDataAsType(id, kString);
	if(!str) return 0;
	return strlen(str);
}

