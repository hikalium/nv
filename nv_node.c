#include "nv.h"
//
// Node
//
NV_Node *NV_Node_getByID(const NV_ElementID *id)
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		if(NV_ElementID_isEqual(&n->id, id)) return n;
	}
	return NULL;
}

NV_ElementID NV_Node_createWithID(const NV_ElementID *id)
{
	NV_Node *n = NV_malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = *id;
	n->type = kNone;
	n->data = NULL;
	n->size = 0;
	//
	n->next = nodeRoot.next;
	if(n->next) n->next->prev = n;
	n->prev = &nodeRoot;
	if(n->prev) n->prev->next = n;
	//
	return n->id;
}

NV_ElementID NV_Node_create()
{
	NV_ElementID id = NV_ElementID_generateRandom();
	return NV_Node_createWithID(&id);
}

NV_ElementID NV_Node_createRelation
(const NV_ElementID *from, const NV_ElementID *rel, const NV_ElementID *to)
{
	NV_ElementID r = NV_Node_create();
	NV_Node_setRelation(&r, from, rel, to);
	return r;
}

NV_ElementID NV_Node_clone(const NV_ElementID *baseID)
{
	NV_Node *base, *new;
	NV_ElementID newID = NV_Node_create();
	new = NV_Node_getByID(&newID);
	base = NV_Node_getByID(baseID);
	new->type = base->type;
	if(base->data){
		new->data = NV_malloc(base->size);
		new->size = base->size;
		memcpy(new->data, base->data, new->size);
	}
	return newID;
}

void NV_Node_remove(const NV_ElementID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(id);
		if(n->prev) n->prev->next = n->next;
		if(n->next) n->next->prev = n->prev;
		NV_free(n);
	}

}

void NV_Node_resetDataOfID(const NV_ElementID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		n->type = kNone;
		if(n->data){
			NV_free(n->data);
			n->data = NULL;
			n->size = 0;
		}
	}
}

void NV_Node_setStrToID(const NV_ElementID *id, const char *s)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(id);
		n->type = kString;
		n->size = strlen(s) + 1;
		n->data = NV_malloc(n->size);
		NV_strncpy(n->data, s, n->size, strlen(s));
		((char *)n->data)[n->size - 1] = 0;
	}
}

void NV_Node_setInt32ToID(const NV_ElementID *id, int32_t v)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(id);
		n->type = kInteger;
		n->size = sizeof(int32_t);
		n->data = NV_malloc(n->size);
		*((int32_t *)n->data) = v;
	}
}

void NV_Node_setRelation
(const NV_ElementID *relnid, const NV_ElementID *from, const NV_ElementID *rel, const NV_ElementID *to)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(relnid);
		n->type = kRelation;
		n->size = sizeof(NV_Relation);
		n->data = NV_malloc(n->size);
		//:
		reld = n->data;
		reld->from = *from;
		reld->rel = *rel;
		reld->to = *to;
	}
}

void NV_Node_updateRelationTo(const NV_ElementID *relnid, const NV_ElementID *to)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(n){
		if(n->type != kRelation) return;
		reld = n->data;
		reld->to = *to;
	}
}

NV_ElementID NV_Node_getRelationFrom(const NV_ElementID *from, const NV_ElementID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ElementID_isEqual(&reld->from, from) && 
				NV_ElementID_isEqual(&reld->rel, rel)){
				return n->id;
			}
		}
	}
	return NODEID_NULL;
}

NV_ElementID NV_Node_getRelatedNodeFrom(const NV_ElementID *from, const NV_ElementID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ElementID_isEqual(&reld->from, from) &&
				NV_ElementID_isEqual(&reld->rel, rel)){
				return reld->to;
			}
		}
	}
	return NODEID_NULL;
}

void NV_Node_dump(const NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	printf("%08X %d ", n->id.d[0], n->type);
	if(n->type == kString){
		printf("%s", n->data);
	} else if(n->type == kInteger){
		if(n->size == sizeof(int32_t)){
			printf("%d", *((int32_t *)n->data));
		}
	} else if(n->type == kRelation){
		const NV_Relation *e = n->data;
		printf("%08X %08X -- %08X -> %08X",
			n->id.d[0], e->from.d[0], e->rel.d[0], e->to.d[0]);
	}
}

void NV_Node_printPrimVal(const NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	if(n->type == kString){
		printf("\"%s\"", n->data);
	} else if(n->type == kInteger){
		if(n->size == sizeof(int32_t)){
			printf("%d", *((int32_t *)n->data));
		}
	}
}

