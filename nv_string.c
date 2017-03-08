#include "nv.h"
#include "nv_node.h"

//
// Internal
//

void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(n){
		if(n->type != kNone) NV_Node_Internal_resetData(n);
		n->type = kString;
		n->size = strlen(s) + 1;
		n->data = NV_malloc(n->size);
		NV_strncpy(n->data, s, n->size, strlen(s));
		((char *)n->data)[n->size - 1] = 0;
	}
}

//
// String
//

int NV_NodeID_isString(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(!n || n->type != kString) return 0;
	return 1;
}

NV_ID NV_Node_createWithString(const char *s)
{
	NV_ID id;
	id = NV_Node_create();
	NV_Node_Internal_setStrToID(&id, s);
	return id;
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

void NV_NodeID_createAndString(const NV_ID *id, const char *s)
{
	NV_NodeID_create(id);
	NV_Node_Internal_setStrToID(id, s);
}

const char *NV_NodeID_getCStr(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(!n || n->type != kString) return NULL; 
	return n->data;
}

int NV_Node_String_compare(const NV_Node *na, const NV_Node *nb)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	if(!na || !nb || na->type != kString || nb->type != kString) return -1;
	return strcmp(na->data, nb->data);
}

int NV_Node_String_compareWithCStr(const NV_Node *na, const char *s)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	if(!na || !s || na->type != kString) return -1;
	return strcmp(na->data, s);
}

int NV_NodeID_String_compareWithCStr(const NV_ID *na, const char *s)
{
	return NV_Node_String_compareWithCStr(NV_NodeID_getNode(na), s);
}

char *NV_Node_String_strchr(const NV_Node *ns, char c)
{
	if(!ns || ns->type != kString) return NULL;
	return strchr(ns->data, c);
}

long NV_Node_String_strtol(const NV_Node *ns, int *endptrindex, int base)
{
	long v;
	char *ep;
	if(!ns || ns->type != kString){
		if(endptrindex) *endptrindex = 0; 
		return 0;
	}
	v = strtol(ns->data, &ep, base);
	if(endptrindex) *endptrindex = ep - (char *)ns->data;
	return v;
}

long NV_NodeID_String_strtol(const NV_ID *ns, int *endptrindex, int base)
{
	return NV_Node_String_strtol(NV_NodeID_getNode(ns), endptrindex, base);

}

size_t NV_Node_String_strlen(const NV_Node *ns)
{
	if(!ns || ns->type != kString) return 0;
	return strlen(ns->data);
}

size_t NV_NodeID_String_strlen(const NV_ID *ns)
{
	return NV_Node_String_strlen(NV_NodeID_getNode(ns));
}

