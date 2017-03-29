#include "nv.h"
#include "nv_node.h"

//
// Node Internal
//
NV_ID NV_NodeID_createNew(const NV_ID *id)
{
	NV_Node *n;
	// 新規作成
	n = NV_malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = *id;
	n->type = kNone;
	n->data = NULL;
	n->size = 0;
	n->refCount = 0;
	n->relCache = NULL;
	//
	n->next = nodeRoot.next;
	if(n->next) n->next->prev = n;
	n->prev = &nodeRoot;
	if(n->prev) n->prev->next = n;
	//
	return n->id;
}

void NV_Node_Internal_resetData(NV_Node *n)
{
	if(n){
		if(n->data){
			if(n->type == kRelation){
				NV_Relation *reld = n->data;
				NV_NodeID_release(&reld->to);
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
			if(	NV_NodeID_isEqual(&reld->from, from)){
				// 削除とマーク
				n->next->refCount = 0;
			}
		}
	}
}

int NV_Node_Internal_isEqualInValue(const NV_Node *na, const NV_Node *nb)
{
	// 2つのNodeが値として等しいか否かを返す。
	if(!na || !nb) return 0;
	if(na->type != nb->type) return 0;
	if(na->type == kString){
		return (NV_Node_String_compare(&na->id, &nb->id) == 0);
	}
	return 0;
}

//
// Node
//
NV_Node nodeRoot;

void NV_Node_initRoot()
{
	srand(time(NULL));
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
}

int NV_Node_getNodeCount()
{
	NV_Node *n;
	int i;
	//
	i = 0;
	for(n = nodeRoot.next; n; n = n->next){
		i++;
	}
	return i;
}

NV_ID NV_NodeID_create(const NV_ID *id)
{
	// すでに存在するIDについては，新たに確保せず，既存の内容をリセットする．
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	if(n){
		NV_Node_Internal_resetData(n);
		return n->id;
	}
	// 新規作成
	return NV_NodeID_createNew(id);
}

NV_ID NV_Node_create()
{
	NV_ID id = NV_ID_generateRandom();
	return NV_NodeID_createNew(&id);
}


int NV_NodeID_isEqual(const NV_ID *a, const NV_ID *b)
{
	int i;
	for(i = 0; i < 4; i++){
		if(a->d[i] != b->d[i]) return 0;
	}
	return 1;
}

int NV_NodeID_isEqualInValue(const NV_ID *a, const NV_ID *b)
{
	NV_Node *na, *nb;
	na = NV_NodeID_getNode(a);
	nb = NV_NodeID_getNode(b);
	return NV_Node_Internal_isEqualInValue(na, nb);
}

int NV_NodeID_exists(const NV_ID *id)
{
	return NV_NodeID_getNode(id) != NULL;
}

#define NODE_CACHE_MASK	0xFFFF

NV_Node *nodeIDCache[NODE_CACHE_MASK + 1];

NV_Node *NV_NodeID_getNode(const NV_ID *id)
{
	NV_Node *n;
	//
	if(!id) return NULL;
	// check cache
	if(nodeIDCache[id->d[0] & NODE_CACHE_MASK]){
		if(NV_NodeID_isEqual(&nodeIDCache[id->d[0] & NODE_CACHE_MASK]->id, id)){
			// hit!
			return nodeIDCache[id->d[0] & NODE_CACHE_MASK];
		}
	}
	//
	for(n = nodeRoot.next; n; n = n->next){
		if(NV_NodeID_isEqual(&n->id, id)){
			// found
			// add to cache
			nodeIDCache[id->d[0] & NODE_CACHE_MASK] = n;
			//
			return n;
		}
	}
	return NULL;
}

NV_NodeType NV_Node_getType(const NV_ID *id)
{
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	if(n) return n->type;
	return -1;
}

void *NV_Node_getDataAsType(const NV_ID *id, NV_NodeType type)
{
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	if(!n || n->type != type) return NULL;
	return n->data;
}

void NV_Node_dumpAll()
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(&n->id); putchar('\n');
	}
}

void NV_Node_dumpAllToFile(FILE *fp)
{
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_fdump(fp, &n->id); fputc('\n', fp);
	}
}

void NV_Node_restoreFromFile(FILE *fp)
{
	char s[MAX_SAVE_DATA_ENTRY_SIZE];

	while(fgets(s, sizeof(s), fp)){
		NV_Node_restoreFromString(s);
	}
}


void NV_NodeID_remove(const NV_ID *baseID)
{
	NV_Node *n;
	n = NV_NodeID_getNode(baseID);
	if(n) NV_Node_Internal_remove(n);
}

NV_ID NV_NodeID_clone(const NV_ID *baseID)
{
	NV_Node *base, *new;
	NV_ID newID = NV_Node_create();
	new = NV_NodeID_getNode(&newID);
	base = NV_NodeID_getNode(baseID);
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
	NV_NodeID_create(&id);
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

void NV_NodeID_retain(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
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

void NV_NodeID_release(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
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
			p = NV_NodeID_Relation_getLinkFrom(&(*n)->id);
			if(!p){
				(*n)->refCount = 0;
				continue;
			}
			//
			q = NV_NodeID_Relation_getLinkTo(&(*n)->id);
			if(!q){
				(*n)->refCount = 0;
				continue;
			}
			//
			r = NV_NodeID_Relation_getLinkRel(&(*n)->id);
			//
			p->refCount = 1;
			q->refCount = 1;
			r->refCount = 1;
		}
	}
	for(n = &nodeRoot.next; *n; n = &(*n)->next){
		if((*n)->refCount) continue;
		NV_NodeID_printForDebug(&(*n)->id); putchar('\n');
		NV_Node_Internal_remove(*n);
	}
}

void NV_Node_fdump(FILE *fp, const NV_ID *id)
{
	const NV_Node *n = NV_NodeID_getNode(id);
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

void NV_Node_dump(const NV_ID *id)
{
	NV_Node_fdump(stdout, id);
}

void NV_Node_printPrimVal(const NV_ID *id)
{
	const NV_Node *n = NV_NodeID_getNode(id);
	if(!n){
		printf("(null)");
		return;
	}
	if(n->type == kString){
		printf("%s", n->data);
	} else if(n->type == kInteger){
		if(n->size == sizeof(int32_t)){
			printf("%d", *((int32_t *)n->data));
		}
	}
}

void NV_NodeID_printForDebug(const NV_ID *id)
{
	const NV_Node *n = NV_NodeID_getNode(id);

	if(!n){
		printf("(NV_Node is NULL)");
		return;
	}
	NV_ID_dumpIDToFile(&n->id, stdout);
	printf(": ");
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

