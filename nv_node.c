#include "nv.h"

//
// Node Internal
//
struct NV_NODE {
	NV_ID id;
	const void *data;
	NV_Node *prev;
	NV_Node *next;
	NV_NodeType type;
	int size;	// size of data, bytes.
	//
	const NV_Node *relCache; // link from this node. recently referenced.
	NV_ID owner;
};

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
			/*
			if(n->type == kRelation){
				NV_Relation *reld = n->data;
				//NV_NodeID_release(&reld->to);
			}
			*/
			NV_DbgInfo("Free Data type: %s", NV_NodeTypeList[n->type]);
			NV_free((void *)n->data);
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
	//NV_Node_Internal_removeAllRelationFrom(&n->id);
}

int NV_Node_Internal_isEqualInValue(const NV_Node *na, const NV_Node *nb)
{
	// 2つのNodeが値として等しいか否かを返す。
	// nodei
	if(!na || !nb) return 0;
	if(na->type != nb->type) return 0;
	if(NV_NodeID_isEqual(&na->id, &nb->id)) return 1;
	if(na->type == kString){
		return (NV_Node_String_compare(&na->id, &nb->id) == 0);
	}
	return 0;
}

void NV_Node_Internal_set_Type_Data_Size(const NV_ID *id, NV_NodeType type, const void *data, int32_t size)
{
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	n->type = type;
	n->size = size;
	n->data = data;
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

NV_Node *NV_Node_getNextNode(const NV_Node *n)
{
	if(!n) return NULL;
	return n->next;
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
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	if(n){
		printf("NodeID collision detected. abort.\n");
		exit(1);
	}
	// 新規作成
	return NV_NodeID_createNew(id);
}

NV_ID NV_Node_create()
{
	NV_ID id = NV_ID_generateRandom();
	return NV_NodeID_createNew(&id);
}

NV_ID NV_Node_createWithTypeAndData
(NV_NodeType type, const void *data, int32_t size)
{
	NV_ID id = NV_Node_create();
	NV_Node_Internal_set_Type_Data_Size(&id, type, data, size);
	return id;
}

NV_ID NV_Node_createWith_ID_Type_Data_Size
(const NV_ID *id, NV_NodeType type, const void *data, int32_t size)
{
	NV_NodeID_create(id);
	NV_Node_Internal_set_Type_Data_Size(id, type, data, size);
	return *id;
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
	if(NV_NodeID_isEqual(a, b)) return 1;	// 等しいidのNode同士の値は常に等しい
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

const NV_Node *NV_Node_getRelCache(const NV_Node *n)
{
	if(!n) return NULL;
	return n->relCache;
}

void NV_Node_setRelCache(NV_Node *n, const NV_Node *rel)
{
	if(!n) return;
	n->relCache = rel;
}

int32_t NV_Node_calcHash(const NV_Node *n)
{
	if(!n || !n->data) return 0;
	return fnv_1_hash_32(n->data, n->size);
}

int32_t NV_NodeID_calcHash(const NV_ID *id)
{
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	return NV_Node_calcHash(n);
}
/*
// void NV_Node_setDataAsType(const NV_Node *n, NV_NodeType type, void *data)
{
	// If .data has been set already, this func will fail.
	if(!n || n->type != type) return;
	if(n->data){
		printf("Try to overwrite existed data. abort.\n");
		return;
	}
	n->data = data;
}
*/
const void *NV_Node_getDataAsType(const NV_Node *n, NV_NodeType type)
{
	if(!n || n->type != type) return NULL;
	return n->data;
}

const void *NV_NodeID_getDataAsType(const NV_ID *id, NV_NodeType type)
{
	NV_Node *n;
	n = NV_NodeID_getNode(id);
	return NV_Node_getDataAsType(n, type);
}

NV_ID NV_Node_getID(const NV_Node *n)
{
	if(!n) return NODEID_NOT_FOUND;
	return n->id;
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
/*
void NV_Node_restoreFromFile(FILE *fp)
{
	char s[MAX_SAVE_DATA_ENTRY_SIZE];

	while(fgets(s, sizeof(s), fp)){
		NV_Node_restoreFromString(s);
	}
}
*/
void NV_NodeID_remove(const NV_ID *baseID)
{
	NV_Node *n;
	n = NV_NodeID_getNode(baseID);
	if(n) NV_Node_Internal_remove(n);
}

NV_ID NV_NodeID_clone(const NV_ID *baseID)
{
	NV_Node *base;
	base = NV_NodeID_getNode(baseID);
	void *data = NULL;
	if(base->data){
		data = NV_malloc(base->size);
		memcpy(data, base->data, base->size);
	}
	return NV_Node_createWithTypeAndData(base->type, data, base->size);
}


//void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v);
//void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s);
/*
//NV_ID NV_Node_restoreFromString(const char *s)
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
			NV_Node_setRelation(&id, &from, &rel, &to);
			break;
	}
	return id;
}
*/
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
		printf("(null)#%08X", id->d[0]);
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

typedef struct {
	int currentDepth;
	NV_ID owner;
} NV_Node_DepthInfo;

int NV_Node_printDependencyTreeSub(void *d, const NV_ID *reln, const NV_ID *rel, const NV_ID *to)
{
	NV_Node_DepthInfo *info = d;
	int i;
	for(i = 0; i < info->currentDepth; i++){
		printf("│   ");
	}
	printf("├── ");
	NV_Node_printPrimVal(rel);
	printf(": ");
	//
	if(!NV_Node_hasOwner(reln)) NV_Node_setOwner(reln, &info->owner);
	if(!NV_Node_hasOwner(rel)) NV_Node_setOwner(rel, &info->owner);
	if(!NV_Node_hasOwner(to)) NV_Node_setOwner(to, &info->owner);
	//
	NV_Node_printDependencyTree(to, info->currentDepth + 1);
	return 1;
}

void NV_Node_setOwner(const NV_ID *id, const NV_ID *owner)
{
	NV_Node *n = NV_NodeID_getNode(id);
	if(n){
		n->owner = *owner;
	}
}

int NV_Node_hasOwner(const NV_ID *id)
{
	NV_Node *n = NV_NodeID_getNode(id);
	if(n){
		return !NV_NodeID_isEqual(&n->owner, &NODEID_NOT_FOUND);
	}
	return 1;
}

void NV_Node_printDependencyTree(const NV_ID *root, int currentDepth)
{
	if(currentDepth == 0){
		NV_Node *n;
		for(n = nodeRoot.next; n; n = n->next){
			n->owner = NODEID_NOT_FOUND;
		}
		NV_Node_setOwner(root, root);
	}
	NV_Node_printPrimVal(root);printf("\n");
	//NV_NodeID_printForDebug(root);printf("\n");
	//if(currentDepth && NV_Node_hasOwner(root)) return;
	NV_Node_DepthInfo info;
	info.currentDepth = currentDepth;
	info.owner = *root;
	NV_Dict_foreach(
			root, &info,
			NV_Node_printDependencyTreeSub);

	if(currentDepth == 0){
		int count = 0;
		NV_Node *n;
		//printf("nodes to be removed:\n");
		for(n = nodeRoot.next; n; n = n->next){
			if(!NV_Node_hasOwner(&n->id)){
				//NV_NodeID_printForDebug(&n->id); NV_NodeID_printForDebug(&n->owner); putchar('\n');
				//NV_NodeID_remove(&n->id);
				count++;
			}
		}
		printf("%d nodes have no owner.\n", count);
	}
}

void NV_Node_cleanup()
{
	NV_Node_cleanDepTree(&rootScope, 0);
}

int NV_Node_cleanDepTreeSub(void *d, const NV_ID *reln, const NV_ID *rel, const NV_ID *to)
{
	NV_Node_DepthInfo *info = d;
	//
	if(!NV_Node_hasOwner(reln)) NV_Node_setOwner(reln, &info->owner);
	if(!NV_Node_hasOwner(rel)) NV_Node_setOwner(rel, &info->owner);
	if(!NV_Node_hasOwner(to)) NV_Node_setOwner(to, &info->owner);
	//
	NV_Node_cleanDepTree(to, info->currentDepth + 1);
	return 1;
}

void NV_Node_cleanDepTree(const NV_ID *root, int currentDepth)
{
	if(currentDepth == 0){
		NV_Node *n;
		for(n = nodeRoot.next; n; n = n->next){
			n->owner = NODEID_NOT_FOUND;
		}
		NV_Node_setOwner(root, root);
	}
	NV_Node_DepthInfo info;
	info.currentDepth = currentDepth;
	info.owner = *root;
	NV_Dict_foreach(
			root, &info,
			NV_Node_cleanDepTreeSub);

	if(currentDepth == 0){
		int count = 0;
		NV_Node *n;
		for(n = nodeRoot.next; n; n = n->next){
			if(!NV_Node_hasOwner(&n->id)){
				NV_NodeID_remove(&n->id);
				count++;
			}
		}
		printf("Removed %d nodes.\n", count);
	}

}
