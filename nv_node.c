#include "nv.h"

//
// Node Internal
//

void NV_Node_Internal_resetData(NV_Node *n);
void NV_Node_Internal_remove(NV_Node *n);
void NV_Node_Internal_removeAllRelationFrom(const NV_ID *from);
void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s);
void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v);

void NV_Node_Internal_resetData(NV_Node *n)
{
	if(n){
		if(n->data){
			if(n->type == kRelation){
				NV_Relation *reld = n->data;
				NV_Node_release(&reld->to);
			}
			NV_DbgInfo("Free Data type: %s", NV_NodeTypeList[n->type]);
			NV_free(n->data);
			n->data = NULL;
			n->size = 0;
		}
		n->type = kNone;
	}
}
void NV_Node_Internal_remove(NV_Node *n)
{
	if(n){
		NV_DbgInfo_mem(n, "free");
		NV_DbgInfo("Free Node type: %s", NV_NodeTypeList[n->type]);
		if(n->type != kNone) NV_Node_Internal_resetData(n);
		if(n->prev) n->prev->next = n->next;
		if(n->next) n->next->prev = n->prev;
		NV_free(n);
	}
	NV_Node_Internal_removeAllRelationFrom(&n->id);
}
void NV_Node_Internal_removeAllRelationFrom(const NV_ID *from)
{
	NV_Node *n;
	const NV_Relation *reld;
	for(n = &nodeRoot; n->next; n = n->next){
		if(n->next->type == kRelation){
			reld = n->next->data;
			//NV_DbgInfo_mem(n->next, "check");
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
	// すでに存在するIDについては，新たに確保せず，既存の内容をリセットする．
	NV_Node *n;
	n = NV_Node_getByID(id);
	if(n){
		NV_Node_Internal_resetData(n);
		return n->id;
	}
	// 新規作成
	n = NV_malloc(sizeof(NV_Node));
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

NV_ID NV_Node_restoreFromString(const char *s)
{
	const char *p;
	int n, i;
	NV_ID id;
	NV_ID from, rel, to;
	char *buf;
	//
	if(NV_ID_setFromString(&id, s)){
		printf("Invalid id format.\n");
		return NODEID_NULL;
	}
	// NV_ID_dumpIDToFile(&id, stdout);
	// putchar(' ');
	NV_Node_createWithID(&id);
	//
	p = &s[32];
	n = NV_strtolSeq(&p, 10);
	switch(n){
		case kString:
			n = NV_strtolSeq(&p, 10);
			p++;
			buf = NV_malloc(n);
			for(i = 0; i < n - 1; i++){
				buf[i] = c2hexTable[(int)*(p++)];
				buf[i] <<= 4;
				buf[i] |= c2hexTable[(int)*(p++)];
			}
			buf[i] = 0;
			//printf("str(%d) = %s\n", n, buf);
			NV_Node_Internal_setStrToID(&id, buf);
			NV_free(buf);
			break;
		case kInteger:
			n = NV_strtolSeq(&p, 10);
			// printf("int(%d) = ", n);
			//
			n = NV_strtolSeq(&p, 16);
			// printf("%d\n", n);
			NV_Node_Internal_setInt32ToID(&id, n);
			break;
		case kRelation:
			NV_ID_setFromString(&from, &p[1]);
			NV_ID_setFromString(&rel, &p[ 1 + 32 + 1]);
			NV_ID_setFromString(&to, &p[1 + 32 + 1 + 32 + 1]);
/*
			printf("rel ");
			printf(" ");
			NV_ID_dumpIDToFile(&from, stdout);
			printf(" ");
			NV_ID_dumpIDToFile(&rel, stdout);
			printf(" ");
			NV_ID_dumpIDToFile(&to, stdout);
			printf("\n");
*/
			NV_Node_setRelation(&id, &from, &rel, &to);
			break;
	}
	return id;
}

void NV_Node_retain(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		n->refCount++;
		//NV_DbgInfo_mem(n, "retain");
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
		//NV_DbgInfo_mem(n, "release");
/*
		NV_DbgInfo("release type: %s, refCount becomes: %d",
			NV_NodeTypeList[n->type],
			n->refCount);
*/
	}
}

void NV_Node_cleanup()
{
	// メモリ解放戦略：
	// そのNodeがRelationで、
	//		どちらか片方でもNodeが見つからなかった場合、解放する。
	//	それ以外の場合で、
	//		参照カウントが0になった場合は、解放する。
	NV_Node **n, *p, *q, *r;
	//
	for(n = &nodeRoot.next; *n; n = &(*n)->next){
		(*n)->refCount = ((*n)->type == kRelation);
	}
	for(n = &nodeRoot.next; *n; n = &(*n)->next){
		if((*n)->type == kRelation){
			p = NV_Node_Relation_getLinkFrom(&(*n)->id);
			if(!p){
				(*n)->refCount = 0;
				continue;
			}
			//
			q = NV_Node_Relation_getLinkTo(&(*n)->id);
			if(!q){
				(*n)->refCount = 0;
				continue;
			}
			//
			r = NV_Node_Relation_getLinkRel(&(*n)->id);
			//
			p->refCount = 1;
			q->refCount = 1;
			r->refCount = 1;
		}
	}
	for(n = &nodeRoot.next; *n; n = &(*n)->next){
		if((*n)->refCount) continue;
		NV_Node_printForDebug(*n); putchar('\n');
		NV_Node_Internal_remove(*n);
	}
}

void NV_Node_fdump(FILE *fp, const NV_Node *n)
{
	if(!n){
		fprintf(fp, "(null)");
		return;
	}
	// n->id
	NV_ID_dumpIDToFile(&n->id, fp);
	// n->type
	fprintf(fp, " %d ", n->type);
	// n->data
	if(n->type == kString){
		int i;
		fprintf(fp, "%d ", n->size);
		for(i = 0; ((const char *)n->data)[i]; i++){
			fprintf(fp, "%02X", ((const char *)n->data)[i]);
		}
	} else if(n->type == kInteger){
		if(n->size == sizeof(int32_t)){
			fprintf(fp, "%d %08X", n->size, *((int32_t *)n->data));
		}
	} else if(n->type == kRelation){
		const NV_Relation *e = n->data;
		NV_ID_dumpIDToFile(&e->from, fp);
		fputc(' ', fp);
		NV_ID_dumpIDToFile(&e->rel, fp);
		fputc(' ', fp);
		NV_ID_dumpIDToFile(&e->to, fp);
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

void NV_Node_printForDebug(const NV_Node *n)
{
	NV_ID_dumpIDToFile(&n->id, stdout);
	printf(": ");
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
	} else if(n->type == kRelation){
		printf("Rel: ");
		const NV_Relation *e = n->data;
		NV_ID_dumpIDToFile(&e->from, stdout);
		fputc(' ', stdout);
		NV_ID_dumpIDToFile(&e->rel, stdout);
		fputc(' ', stdout);
		NV_ID_dumpIDToFile(&e->to, stdout);
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
/*
	if(!NV_Node_existsID(from) || !NV_Node_existsID(rel) || !NV_Node_existsID(to)){
		return;
	}
*/
	n = NV_Node_getByID(relnid);
	if(n){
		if(n->type != kNone) NV_Node_Internal_resetData(n);
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

NV_Node *NV_Node_Relation_getLinkFrom(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(!n || n->type != kRelation) return NULL;
	reld = n->data;
	return NV_Node_getByID(&reld->from);
}

NV_Node *NV_Node_Relation_getLinkTo(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(!n || n->type != kRelation) return NULL;
	reld = n->data;
	return NV_Node_getByID(&reld->to);
}

NV_Node *NV_Node_Relation_getLinkRel(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_Node_getByID(relnid);
	if(!n || n->type != kRelation) return NULL;
	reld = n->data;
	return NV_Node_getByID(&reld->rel);
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
	return NODEID_NOT_FOUND;
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
	return NODEID_NOT_FOUND;
}

NV_ID NV_Node_getEqRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type == kRelation){
			reld = n->data;
			if(	NV_ID_isEqual(&reld->from, from) &&
				NV_ID_isEqualInValue(&reld->rel, rel)){
				return reld->to;
			}
		}
	}
	return NODEID_NOT_FOUND;
}

//
// String
//

int NV_Node_isString(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
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

void NV_Node_createWithIDAndString(const NV_ID *id, const char *s)
{
	NV_Node_createWithID(id);
	NV_Node_Internal_setStrToID(id, s);
}

const char *NV_Node_getCStr(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(!n || n->type != kString) return NULL; 
	return n->data;
}

void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_Internal_resetData(n);
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

int NV_Node_String_compareWithCStr(const NV_Node *na, const char *s)
{
	// compatible with strcmp
	// but if node->data is null, returns -1.
	// "" == "" -> true
	if(!na || !s || na->type != kString) return -1;
	return strcmp(na->data, s);
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

size_t NV_Node_String_strlen(const NV_Node *ns)
{
	if(!ns || ns->type != kString) return 0;
	return strlen(ns->data);
}

//
// Integer
//

int NV_Node_isInteger(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(!n || n->type != kInteger) return 0;
	return 1;
}

void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_Internal_resetData(n);
		n->type = kInteger;
		n->size = sizeof(int32_t);
		n->data = NV_malloc(n->size);
		*((int32_t *)n->data) = v;
	}
}

NV_ID NV_Node_createWithInt32(int32_t v)
{
	NV_ID id;
	id = NV_Node_create();
	NV_Node_Internal_setInt32ToID(&id, v);
	return id;
}


int32_t NV_Node_getInt32FromID(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_Node_getByID(id);
	if(!n || n->type != kInteger) return -1;
	return *((int32_t *)n->data);
}

