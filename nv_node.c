#include "nv.h"

//
// Node Internal
//

void NV_Node_resetData(NV_Node *n);
void NV_Node_remove(NV_Node *n);
void NV_Node_removeAllRelationFrom(const NV_ID *from);

void NV_Node_resetData(NV_Node *n)
{
	if(n){
		if(n->data){
			if(n->type == kRelation){
				NV_Relation *reld = n->data;
				NV_Node_release(&reld->to);
			}
			//NV_DbgInfo("Free Data type: %s", NV_NodeTypeList[n->type]);
			NV_free(n->data);
			n->data = NULL;
			n->size = 0;
		}
		n->type = kNone;
	}
}

void NV_Node_remove(NV_Node *n)
{
	if(n){
		NV_DbgInfo_mem(n, "free");
		//NV_DbgInfo("Free Node type: %s", NV_NodeTypeList[n->type]);
		if(n->type != kNone) NV_Node_resetData(n);
		if(n->prev) n->prev->next = n->next;
		if(n->next) n->next->prev = n->prev;
		NV_free(n);
	}
	NV_Node_removeAllRelationFrom(&n->id);
}

void NV_Node_removeAllRelationFrom(const NV_ID *from)
{
	NV_Node *n;
	const NV_Relation *reld;
	for(n = &nodeRoot; n->next; n = n->next){
		if(n->next->type == kRelation){
			reld = n->next->data;
			NV_DbgInfo_mem(n->next, "check");
			if(	NV_ID_isEqual(&reld->from, from)){
				// 削除とマーク
				n->next->refCount = 0;
			}
		}
	}
}

//
// Node
//

int NV_Node_existsID(const NV_ID *id)
{
	return NV_Node_getByID(id) != NULL;
}

NV_Node *NV_Node_getByID(const NV_ID *id)
{
	NV_Node *n;
	//
	if(!id) return NULL;
	for(n = nodeRoot.next; n; n = n->next){
		if(NV_ID_isEqual(&n->id, id)) return n;
	}
	return NULL;
}

int NV_Node_isEqualInValue(const NV_Node *na, const NV_Node *nb)
{
	if(!na || !nb) return 0;
	if(na->type != nb->type) return 0;
	if(na->type == kString){
		return (NV_Node_String_compare(na, nb) == 0);
	}
	return 0;
}
/*
int NV_Node_isLiveNode(NV_Node *n)
{
	if(!n) return 0;
	if(n->refCount == 0) return 0;
	return 1;
}
*/
NV_ID NV_Node_createWithID(const NV_ID *id)
{
	NV_Node *n = NV_malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = *id;
	n->type = kNone;
	n->data = NULL;
	n->size = 0;
	n->refCount = 0;
	//
	n->next = nodeRoot.next;
	if(n->next) n->next->prev = n;
	n->prev = &nodeRoot;
	if(n->prev) n->prev->next = n;
	//
	return n->id;
}

NV_ID NV_Node_create()
{
	NV_ID id = NV_ID_generateRandom();
	return NV_Node_createWithID(&id);
}

NV_ID NV_Node_clone(const NV_ID *baseID)
{
	NV_Node *base, *new;
	NV_ID newID = NV_Node_create();
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

void NV_Node_retain(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		n->refCount++;
		NV_DbgInfo_mem(n, "retain");
/*
		NV_DbgInfo("retain type: %s, refCount becomes: %d",
			NV_NodeTypeList[n->type],
			n->refCount);
*/
	}
}

void NV_Node_release(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		n->refCount--;
		NV_DbgInfo_mem(n, "release");
/*
		NV_DbgInfo("release type: %s, refCount becomes: %d",
			NV_NodeTypeList[n->type],
			n->refCount);
*/
	}
}

void NV_Node_cleanup()
{
	/*
	メモリ解放戦略：
		そのNodeがRelationで、
			どちらか片方でもNodeが見つからなかった場合、解放する。
		それ以外の場合で、
			参照カウントが0になった場合は、解放する。
	*/
	NV_Node **n;
	//
	for(n = &nodeRoot.next; *n;){
		if((*n)->refCount == 0){
			NV_Node_remove(*n);
			continue;
		}
		n = &(*n)->next;
	}
}

void NV_Node_resetDataOfID(const NV_ID *id)
{
	NV_Node_resetData(NV_Node_getByID(id));
}

void NV_Node_fdump(FILE *fp, const NV_Node *n)
{
	if(!n){
		fprintf(fp, "(null)");
		return;
	}
	fprintf(fp, "%08X%08X%08X%08X %d %d ", n->id.d[0], n->id.d[1], n->id.d[2], n->id.d[3], n->type, n->refCount);
	if(n->type == kString){
		fprintf(fp, "%s", n->data);
	} else if(n->type == kInteger){
		if(n->size == sizeof(int32_t)){
			fprintf(fp, "%d", *((int32_t *)n->data));
		}
	} else if(n->type == kRelation){
		const NV_Relation *e = n->data;
		fprintf(fp, "%08X%08X%08X%08X -- %08X%08X%08X%08X -> %08X%08X%08X%08X",
			e->from.d[0],e->from.d[1],e->from.d[2],e->from.d[3], 
			e->rel.d[0],e->rel.d[1],e->rel.d[2],e->rel.d[3],
			e->to.d[0],e->to.d[1],e->to.d[2],e->to.d[3]
		);
	}
}

void NV_Node_dump(const NV_Node *n)
{
	NV_Node_fdump(stdout, n);
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



//
// Relation
//
/*
int NV_Node_isLiveRelation(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(n){
		reld = n->data;
	}
	
}
*/
NV_ID NV_Node_createRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	NV_ID r = NV_Node_create();
	NV_Node_setRelation(&r, from, rel, to);
	return r;
}

void NV_Node_setRelation
(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	// retains to.
	// Relationはtoを保持し、
	// fromはRelationを保持する。
	NV_Node *n;
	NV_Relation *reld;
	//
	if(!NV_Node_existsID(from) || !NV_Node_existsID(rel) || !NV_Node_existsID(to)){
		return;
	}
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
		//
		NV_Node_retain(to);
		//
		n->refCount++;
	}
}

void NV_Node_updateRelationTo(const NV_ID *relnid, const NV_ID *to)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(n){
		if(n->type != kRelation) return;
		reld = n->data;
		//
		NV_Node_release(&reld->to);
		//
		reld->to = *to;
		NV_Node_retain(&reld->to);
	}
}

NV_ID NV_Node_getRelationFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ID_isEqual(&reld->from, from) && 
				NV_ID_isEqual(&reld->rel, rel)){
				return n->id;
			}
		}
	}
	return NODEID_NULL;
}

NV_ID NV_Node_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ID_isEqual(&reld->from, from) &&
				NV_ID_isEqual(&reld->rel, rel)){
				return reld->to;
			}
		}
	}
	return NODEID_NULL;
}


//
// String
//

NV_ID NV_Node_createWithString(const char *s)
{
	NV_ID id;
	id = NV_Node_create();
	NV_Node_setStrToID(&id, s);
	return id;
}

void NV_Node_setStrToID(const NV_ID *id, const char *s)
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

int NV_Node_String_compare(const NV_Node *na, const NV_Node *nb)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	if(!na || !nb || na->type != kString || nb->type != kString) return -1;
	return strcmp(na->data, nb->data);
}

char *NV_Node_String_strchr(const NV_Node *ns, char c)
{
	if(!ns || ns->type != kString) return NULL;;
	return strchr(ns->data, c);
}

//
// Integer
//

void NV_Node_setInt32ToID(const NV_ID *id, int32_t v)
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

