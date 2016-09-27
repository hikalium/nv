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
	int size;
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

// @nv.c
extern NV_Node *nodeRoot;
extern NV_Edge *edgeRoot;
void NV_Graph_init();
void NV_Graph_dump();
int NV_isTreeType(NV_ElementID node, NV_ElementID tType);

// @nv_array.c
NV_ElementID NV_Array_create();
NV_ElementID NV_Array_push(NV_ElementID array, NV_ElementID data);
void NV_Array_print(NV_ElementID array);

// @nv_edge.c
NV_Edge *NV_Edge_getByID(NV_ElementID id);
NV_ElementID NV_Edge_add(NV_ElementID from, NV_ElementID rel, NV_ElementID to);
void NV_Edge_update(NV_ElementID eid, NV_ElementID from, NV_ElementID rel, NV_ElementID to);
NV_ElementID NV_Edge_getConnectedFrom(NV_ElementID from, NV_ElementID rel);
void NV_Edge_dump(NV_Edge *e);

// @nv_id.c
NV_ElementID NV_ElementID_generateRandom();
int NV_ElementID_isEqual(NV_ElementID a, NV_ElementID b);

// @nv_node.c
NV_Node *NV_Node_getByID(NV_ElementID id);
NV_ElementID NV_Node_addWithID(NV_ElementID id);
NV_ElementID NV_Node_add();
NV_ElementID NV_Node_clone(NV_ElementID baseID);
void NV_Node_resetDataOfID(NV_ElementID id);
void NV_Node_setStrToID(NV_ElementID id, const char *s);
NV_ElementID NV_Node_getConnectedFrom(NV_ElementID from, NV_ElementID rel);
void NV_Node_dump(NV_Node *n);
void NV_Node_printPrimVal(NV_Node *n);

// @nv_static.c
extern const NV_ElementID NODEID_NULL;
extern const NV_ElementID NODEID_TREE_TYPE_ARRAY;
extern const NV_ElementID NODEID_TREE_TYPE_VARIABLE;
extern const NV_ElementID RELID_ARRAY_NEXT;
extern const NV_ElementID RELID_VARIABLE_DATA;
extern const NV_ElementID RELID_POINTER_TARGET;
extern const NV_ElementID RELID_TREE_TYPE;


// @nv_variable.c
NV_ElementID NV_Variable_create();
void NV_Variable_assign(NV_ElementID vid, NV_ElementID data);
void NV_Variable_print(NV_ElementID vid);
void NV_Variable_printiPrimVal(NV_ElementID vid);

