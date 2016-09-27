#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct NV_NODE NV_Node;
typedef struct NV_EDGE NV_Edge;
typedef enum NV_NODE_TYPE NV_NodeType;
typedef struct NV_ELEMENT_ID NV_ElementID;

enum NV_NODE_TYPE {
	kNone,
	kString,
};

struct NV_ELEMENT_ID {
	int32_t d[4];
};

struct NV_NODE {
	NV_ElementID id;
	NV_NodeType type;
	void *data;
	//
	NV_Node *next;
};

struct NV_EDGE {
	NV_ElementID id;
	NV_ElementID from;
	NV_ElementID to;
	NV_ElementID rel;
	//
	NV_Edge *next;
};

extern NV_Node *nodeRoot;
extern NV_Edge *edgeRoot;

const NV_ElementID NODEID_NULL
	= {{0x00000000, 0, 0, 0}};
const NV_ElementID NODEID_TREE_TYPE_ARRAY
	= {{0xBA7C82D7, 0, 0, 0}};
const NV_ElementID NODEID_TREE_TYPE_VARIABLE
	= {{0x67DEB167, 0, 0, 0}};
const NV_ElementID RELID_ARRAY_NEXT
	= {{0xA71CE915, 0, 0, 0}};
const NV_ElementID RELID_VARIABLE_DATA
	= {{0x40776C61, 0, 0, 0}};
const NV_ElementID RELID_POINTER_TARGET
	= {{0xCD64EF96, 0, 0, 0}};
const NV_ElementID RELID_TREE_TYPE
	= {{0xE804DE81, 0, 0, 0}};

//
// NV_ElementID
//

NV_ElementID NV_ElementID_generateRandom()
{
	NV_ElementID id;
	id.d[0] = rand();
	id.d[1] = rand();
	id.d[2] = rand();
	id.d[3] = rand();
	return id;
}

int NV_ElementID_isEqual(NV_ElementID a, NV_ElementID b)
{
	int i;
	for(i = 0; i < 4; i++){
		if(a.d[i] != b.d[i]) return 0;
	}
	return 1;
}

//
// Node
//

NV_ElementID NV_Node_add()
{
	NV_Node *n = malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = NV_ElementID_generateRandom();
	n->type = kNone;
	n->data = NULL;
	//
	n->next = nodeRoot;
	nodeRoot = n;
	//
	return n->id;
}

NV_ElementID NV_Node_addWithID(NV_ElementID id)
{
	NV_Node *n = malloc(sizeof(NV_Node));
	if(!n) exit(EXIT_FAILURE);
	//
	n->id = id;
	n->type = kNone;
	n->data = NULL;
	//
	n->next = nodeRoot;
	nodeRoot = n;
	//
	return n->id;
}

NV_Node *NV_Node_getByID(NV_ElementID id)
{
	NV_Node *n;
	//
	for(n = nodeRoot; n; n = n->next){
		if(NV_ElementID_isEqual(n->id, id)) return n;
	}
	return NULL;
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
		}
	}
}

void NV_Node_setStrToID(NV_ElementID id, const char *s)
{
	NV_Node *n;
	int buflen;
	//
	n = NV_Node_getByID(id);
	if(n){
		if(n->type != kNone) NV_Node_resetDataOfID(id);
		n->type = kString;
		buflen = strlen(s) + 1;
		n->data = malloc(buflen);
		strncpy(n->data, s, buflen - 1);
		((char *)n->data)[buflen - 1] = 0;
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

//
// Graph
//
NV_Node *nodeRoot;
NV_Edge *edgeRoot;

void NV_Graph_init()
{
	nodeRoot = NULL;
	edgeRoot = NULL;
}

void NV_Graph_dumpNode(NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	printf("%08X %d ", n->id.d[0], n->type);
	if(n->type == kString){
		printf("%s", n->data);
	}
}

void NV_Graph_printNodePrimVal(NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	if(n->type == kString){
		printf("\"%s\"", n->data);
	}
}

void NV_Graph_dumpEdge(NV_Edge *e)
{
	if(!e){
		printf("(null)");
		return;
	}
	printf("%08X %08X -- %08X -> %08X", e->id.d[0], e->from.d[0], e->rel.d[0], e->to.d[0]);
}

void NV_Graph_dump()
{
	NV_Node *n;
	NV_Edge *e;
	//
	puts("Node:");
	for(n = nodeRoot; n; n = n->next){
		NV_Graph_dumpNode(n); putchar('\n');
	}
	puts("Edge:");
	for(e = edgeRoot; e; e = e->next){
		NV_Graph_dumpEdge(e); putchar('\n');
	}
}

//
// Edge
//

NV_Edge *NV_Edge_getByID(NV_ElementID id)
{
	NV_Edge *e;
	for(e = edgeRoot; e; e = e->next){
		if(NV_ElementID_isEqual(e->id, id)) return e;
	}
	return NULL;
}

NV_ElementID NV_Edge_add(NV_ElementID from, NV_ElementID rel, NV_ElementID to)
{
	NV_Edge *e = malloc(sizeof(NV_Edge));
	if(!e) exit(EXIT_FAILURE);
	//
	e->id = NV_ElementID_generateRandom();
	e->from = from;
	e->rel = rel;
	e->to = to;
	//
	e->next = edgeRoot;
	edgeRoot = e;
	//
	return e->id;
}

void NV_Edge_update
(NV_ElementID eid, NV_ElementID from, NV_ElementID rel, NV_ElementID to)
{
	NV_Edge *e = NV_Edge_getByID(eid);
	if(e){
		e->from = from;
		e->rel = rel;
		e->to = to;
	}
}

NV_ElementID NV_Edge_getConnectedFrom(NV_ElementID from, NV_ElementID rel)
{
	NV_Edge *e;
	for(e = edgeRoot; e; e = e->next){
		if(	NV_ElementID_isEqual(e->from, from) && 
			NV_ElementID_isEqual(e->rel, rel))
			return e->id;
	}
	return NODEID_NULL;
}

//
// common
//
int NV_isTreeType(NV_ElementID node, NV_ElementID tType)
{
	return NV_ElementID_isEqual(
		NV_Node_getConnectedFrom(node, RELID_TREE_TYPE), tType);
}
/*
NV_ElementID NV_clone(NV_ElementID base)
{
	
}
*/
//
// Variable
//
NV_ElementID NV_Variable_create()
{
	NV_ElementID v;
	v = NV_Node_add();
	NV_Edge_add(v, RELID_TREE_TYPE, NODEID_TREE_TYPE_VARIABLE);
	NV_Edge_add(v, RELID_VARIABLE_DATA, NODEID_NULL);
	return v;
}

void NV_Variable_assign(NV_ElementID vid, NV_ElementID data)
{
	NV_ElementID eid;
	NV_Edge *e;
	eid = NV_Edge_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	e = NV_Edge_getByID(eid);
	if(e){
		e->to = data;
	}
}

void NV_Variable_print(NV_ElementID vid)
{
	NV_ElementID targetID;
	if(!NV_isTreeType(vid, NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid.d[0]);
		return;
	}
	targetID = NV_Node_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	printf("Variable ");
	NV_Graph_dumpNode(NV_Node_getByID(vid));
	printf(" = ");
	NV_Graph_dumpNode(NV_Node_getByID(targetID));
	putchar('\n');
}

void NV_Variable_printiPrimVal(NV_ElementID vid)
{
	NV_ElementID targetID;
	if(!NV_isTreeType(vid, NODEID_TREE_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid.d[0]);
		return;
	}
	targetID = NV_Node_getConnectedFrom(vid, RELID_VARIABLE_DATA);
	NV_Graph_printNodePrimVal(NV_Node_getByID(targetID));
}

//
// Array
//

NV_ElementID NV_Array_create()
{
	NV_ElementID arrayRoot;
	arrayRoot = NV_Node_add();
	NV_Edge_add(arrayRoot, RELID_TREE_TYPE, NODEID_TREE_TYPE_ARRAY);
	NV_Edge_add(arrayRoot, RELID_ARRAY_NEXT, NODEID_NULL);
	return arrayRoot;
}

NV_ElementID NV_Array_push(NV_ElementID array, NV_ElementID data)
{
	NV_ElementID v, t, next;
	//
	v = NV_Variable_create();
	NV_Variable_assign(v, data);
	//
	t = array;
	for(;;){
		next = NV_Node_getConnectedFrom(t, RELID_ARRAY_NEXT);
		if(NV_ElementID_isEqual(next, NODEID_NULL)) break;
		t = next;
	}
	NV_Edge_add(t, RELID_ARRAY_NEXT, v);
	return v;
}

void NV_Array_print(NV_ElementID array)
{
	NV_ElementID t;
	printf("[");
	t = NV_Node_getConnectedFrom(array, RELID_ARRAY_NEXT);
	for(;!NV_ElementID_isEqual(t, NODEID_NULL);){
		NV_Variable_printiPrimVal(t);
		t = NV_Node_getConnectedFrom(t, RELID_ARRAY_NEXT);
		if(!NV_ElementID_isEqual(t, NODEID_NULL)){
			printf(",");
		}
	}
	printf("]\n");
}

//
// main
//


int main(int argc, char *argv[])
{
	NV_ElementID id, id2;
	//
	srand(time(NULL));
	NV_Graph_init();
	//
	id = NV_Node_addWithID(NODEID_NULL);
	NV_Node_setStrToID(id, "NullElement");
	//
	id = NV_Node_addWithID(NODEID_TREE_TYPE_ARRAY);
	NV_Node_setStrToID(id, "TreeType(Array)");
	//
	id = NV_Node_addWithID(NODEID_TREE_TYPE_VARIABLE);
	NV_Node_setStrToID(id, "TreeType(Variable)");
	//
	id = NV_Node_addWithID(RELID_TREE_TYPE);
	NV_Node_setStrToID(id, "relTreeType");
	//
	id = NV_Node_addWithID(RELID_ARRAY_NEXT);
	NV_Node_setStrToID(id, "relArrayNext");
	//
	id = NV_Node_addWithID(RELID_VARIABLE_DATA);
	NV_Node_setStrToID(id, "relVariableData");
	//
	id = NV_Node_addWithID(RELID_POINTER_TARGET);
	NV_Node_setStrToID(id, "relPointerTarget");
	//
	id2 = NV_Node_add();
	NV_Node_setStrToID(id2, "Hello");
	//
	id = NV_Node_add();
	NV_Node_setStrToID(id, "World");
	//
	id = NV_Array_create();
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	//
	id = NV_Variable_create();
NV_Variable_print(id);
	NV_Node_setStrToID(id, "var1");
NV_Variable_print(id);
	NV_Variable_assign(id, id2);
NV_Variable_print(id);
	//
	NV_Graph_dump();

	return 0;
}


