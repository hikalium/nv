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

long NV_Node_String_Internal_strtol(const NV_Node *ns, int *endptrindex, int base)
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

int NV_Node_String_compare(const NV_ID *ida, const NV_ID *idb)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	NV_Node *na = NV_NodeID_getNode(ida);
	NV_Node *nb = NV_NodeID_getNode(idb);
	if(!na || !nb || na->type != kString || nb->type != kString) return -1;
	return strcmp(na->data, nb->data);
}

int NV_Node_String_compareWithCStr(const NV_ID *ida, const char *s)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	NV_Node *na = NV_NodeID_getNode(ida);
	if(!na || !s || na->type != kString) return -1;
	return strcmp(na->data, s);
}

char *NV_Node_String_strchr(const NV_ID *id, char c)
{
	NV_Node *ns = NV_NodeID_getNode(id);
	if(!ns || ns->type != kString) return NULL;
	return strchr(ns->data, c);
}

long NV_Node_String_strtol(const NV_ID *ns, int *endptrindex, int base)
{
	return NV_Node_String_Internal_strtol(NV_NodeID_getNode(ns), endptrindex, base);

}

size_t NV_Node_String_strlen(const NV_ID *id)
{
	NV_Node *ns = NV_NodeID_getNode(id);
	if(!ns || ns->type != kString) return 0;
	return strlen(ns->data);
}

