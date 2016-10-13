#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define malloc(s)			DO_NOT_USE_THIS_FUNC(s)
#define free(p)				DO_NOT_USE_THIS_FUNC(p)
// #define strncpy(p, q, r)		DO_NOT_USE_THIS_FUNC(p)
// #define strcpy(p, q)			DO_NOT_USE_THIS_FUNC(p)

#define PARAM_UNUSED(x)	((void)x)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SET_FLAG(f, d)	f |= (d)
#define CLR_FLAG(f, d) f &= ~(d)

#define ESC_ANSI_RED(s)		"\033[31m"s"\033[39m"
#define ESC_ANSI_GREEN(s)	"\033[32m"s"\033[39m"
#define ESC_ANSI_YERROW(s)	"\033[33m"s"\033[39m"
#define ESC_ANSI_CYAN(s)	"\033[36m"s"\033[39m"

#define NV_Error(fmt, ...)	printf(ESC_ANSI_RED("\nError: %s: %d: ")fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define NV_DbgInfo(fmt, ...) \
	printf("\nInfo : %s: %d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

typedef struct NV_NODE NV_Node;
typedef enum NV_NODE_TYPE NV_NodeType;
typedef struct NV_ELEMENT_ID NV_ElementID;

typedef struct NV_RELATION NV_Relation;

enum NV_NODE_TYPE {
	kNone,
	kRelation,
	kWeakRelation,
	kString,
	kInteger,
	//
	kNodeTypeCount,
};

struct NV_ELEMENT_ID {
	int32_t d[4];
};

struct NV_NODE {
	NV_ElementID id;
	void *data;
	NV_Node *prev;
	NV_Node *next;
	NV_NodeType type;
	int size;	// size of data, bytes.
	int refCount;
};

struct NV_RELATION {
	NV_ElementID from;
	NV_ElementID rel;
	NV_ElementID to;
};

// @nv.c
extern NV_Node nodeRoot;
void NV_Graph_init();
void NV_Graph_dump();
int NV_isTreeType(const NV_ElementID *node, const NV_ElementID *tType);

// @nv_array.c
NV_ElementID NV_Array_create();
NV_ElementID NV_Array_push(const NV_ElementID *array, const NV_ElementID *data);
void NV_Array_print(const NV_ElementID *array);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_id.c
NV_ElementID NV_ElementID_generateRandom();
int NV_ElementID_isEqual(const NV_ElementID *a, const NV_ElementID *b);

// @nv_node.c
int NV_Node_existsID(const NV_ElementID *id);
NV_Node *NV_Node_getByID(const NV_ElementID *id);
NV_ElementID NV_Node_createWithID(const NV_ElementID *id);
NV_ElementID NV_Node_create();
NV_ElementID NV_Node_createRelation(const NV_ElementID *from, const NV_ElementID *rel,  const NV_ElementID *to);
NV_ElementID NV_Node_clone(const NV_ElementID *baseID);
void NV_Node_retain(const NV_ElementID *id);
void NV_Node_release(const NV_ElementID *id);
void NV_Node_cleanup();
void NV_Node_resetDataOfID(const NV_ElementID *id);
void NV_Node_setStrToID(const NV_ElementID *id, const char *s);
void NV_Node_setInt32ToID(const NV_ElementID *id, int32_t v);
void NV_Node_setRelation
	(const NV_ElementID *relnid, const NV_ElementID *from, const NV_ElementID *rel, const NV_ElementID *to);
void NV_Node_updateRelationTo(const NV_ElementID *relnid, const NV_ElementID *to);
NV_ElementID NV_Node_getRelationFrom(const NV_ElementID *from, const NV_ElementID *rel);
NV_ElementID NV_Node_getRelatedNodeFrom(const NV_ElementID *from, const NV_ElementID *rel);
void NV_Node_dump(const NV_Node *n);
void NV_Node_printPrimVal(const NV_Node *n);

// @nv_static.c
extern const NV_ElementID NODEID_NULL;
extern const NV_ElementID NODEID_REL_MASTERLINK;
extern const NV_ElementID NODEID_TREE_TYPE_ARRAY;
extern const NV_ElementID NODEID_TREE_TYPE_VARIABLE;
extern const NV_ElementID RELID_ARRAY_NEXT;
extern const NV_ElementID RELID_VARIABLE_DATA;
extern const NV_ElementID RELID_POINTER_TARGET;
extern const NV_ElementID RELID_TREE_TYPE;
extern const NV_ElementID NODEID_NV_STATIC_ROOT;
const char *NV_NodeTypeList[kNodeTypeCount];

// @nv_variable.c
NV_ElementID NV_Variable_create();
void NV_Variable_assign(const NV_ElementID *vid, const NV_ElementID *data);
void NV_Variable_print(const NV_ElementID *vid);
void NV_Variable_printiPrimVal(const NV_ElementID *vid);

