#include "nv.h"
//
// Node
//
NV_Node *NV_Node_getByID(NV_ElementID id)
{
	NV_Node *n;
	//
	for(n = nodeRoot; n; n = n->next){
		if(NV_ElementID_isEqual(n->id, id)) return n;
	}
	return NULL;
}

NV_ElementID NV_Node_addWithID(NV_ElementID id)
{
	NV_Node *n = malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = id;
	n->type = kNone;
	n->data = NULL;
	n->size = 0;
	//
	n->next = nodeRoot;
	nodeRoot = n;
	//
	return n->id;
}

NV_ElementID NV_Node_add()
{
	return NV_Node_addWithID(NV_ElementID_generateRandom());
}

NV_ElementID NV_Node_clone(NV_ElementID baseID)
{
	NV_Node *base, *new;
	NV_ElementID newID = NV_Node_add();
	new = NV_Node_getByID(newID);
	base = NV_Node_getByID(baseID);
	new->type = base->type;
	if(base->data){
		new->data = malloc(base->size);
		new->size = base->size;
		memcpy(new->data, base->data, new->size);
	}
	return newID;
}

void NV_Node_resetDataOfID(NV_ElementID id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		n->type = kNone;
		if(n->data){
			free(n->data);
			n->data = NULL;
			n->size = 0;
		}
	}
}

void NV_Node_setStrToID(NV_ElementID id, const char *s)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(id);
		n->type = kString;
		n->size = strlen(s) + 1;
		n->data = malloc(n->size);
		strncpy(n->data, s, n->size - 1);
		((char *)n->data)[n->size - 1] = 0;
	}
}

NV_ElementID NV_Node_getConnectedFrom(NV_ElementID from, NV_ElementID rel)
{
	NV_Edge *e;
	for(e = edgeRoot; e; e = e->next){
		if(	NV_ElementID_isEqual(e->from, from) && 
			NV_ElementID_isEqual(e->rel , rel))
			return e->to;
	}
	return NODEID_NULL;
}
