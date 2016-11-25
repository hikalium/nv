#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT_LEN		1024
#define MAX_TOKEN_LEN		256

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
	printf("\nInfo : %3d:%s: " fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__)
#define NV_DbgInfo0(s) \
	printf("\nInfo : %3d:%s: %s\n", __LINE__, __FUNCTION__, s)
#define NV_DbgInfo_mem(n, s) \
	printf("Info : Mem: %08X: %2d %s %s\n", n->id.d[0], n->type, NV_NodeTypeList[n->type], s);

typedef struct NV_NODE NV_Node;
typedef enum NV_NODE_TYPE NV_NodeType;
typedef struct NV_ELEMENT_ID NV_ID;

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
	NV_ID id;
	void *data;
	NV_Node *prev;
	NV_Node *next;
	NV_NodeType type;
	int size;	// size of data, bytes.
	int refCount;
};

struct NV_RELATION {
	NV_ID from;
	NV_ID rel;
	NV_ID to;
};

// @nv.c
extern NV_Node nodeRoot;
void NV_Graph_init();
void NV_Graph_dump();
int NV_isTreeType(const NV_ID *node, const NV_ID *tType);
NV_ID NV_tokenize(const NV_ID *cTypeList, const char *input);
int NV_runInteractive(const NV_ID *cTypeList, const NV_ID *opList);
int NV_convertLiteral(const NV_ID *tokenizedList, const NV_ID *opList);

// @nv_array.c
NV_ID NV_Array_create();
NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data);
NV_ID NV_Array_getByIndex(const NV_ID *array, int index);
void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data);
void NV_Array_print(const NV_ID *array);

// @nv_dict.c
NV_ID NV_Dict_add(const NV_ID *root, const NV_ID *key, const NV_ID *value);
NV_ID NV_Dict_addByStringKey(const NV_ID *root, const char *key, const NV_ID *value);
NV_ID NV_Dict_get(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getByStringKey(const NV_ID *root, const char *key);
void NV_Dict_print(const NV_ID *root);

// @nv_driver.c
char *NV_gets(char *str, int size);

// @nv_enode.c
void NV_printNodeByID(const NV_ID *id);
void NV_printNode(const NV_Node *n);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_id.c
NV_ID NV_ID_generateRandom();
int NV_ID_isEqual(const NV_ID *a, const NV_ID *b);
int NV_ID_isEqualInValue(const NV_ID *a, const NV_ID *b);
void NV_ID_printPrimVal(const NV_ID *id);

// @nv_node.c
int NV_Node_existsID(const NV_ID *id);
NV_Node *NV_Node_getByID(const NV_ID *id);
int NV_Node_isEqualInValue(const NV_Node *na, const NV_Node *nb);
NV_ID NV_Node_createWithID(const NV_ID *id);
NV_ID NV_Node_create();
NV_ID NV_Node_clone(const NV_ID *baseID);
void NV_Node_retain(const NV_ID *id);
void NV_Node_release(const NV_ID *id);
void NV_Node_cleanup();
void NV_Node_resetDataOfID(const NV_ID *id);
void NV_Node_fdump(FILE *fp, const NV_Node *n);
void NV_Node_dump(const NV_Node *n);
void NV_Node_printPrimVal(const NV_Node *n);
//
NV_ID NV_Node_createRelation(const NV_ID *from, const NV_ID *rel,  const NV_ID *to);
void NV_Node_setRelation
	(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to);
void NV_Node_updateRelationTo(const NV_ID *relnid, const NV_ID *to);
NV_ID NV_Node_getRelationFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_Node_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel);
//
NV_ID NV_Node_createWithString(const char *s);
void NV_Node_setStrToID(const NV_ID *id, const char *s);
int NV_Node_String_compare(const NV_Node *na, const NV_Node *nb);
char *NV_Node_String_strchr(const NV_Node *ns, char c);
long NV_Node_String_strtol(const NV_Node *ns, int *endptrindex, int base);
size_t NV_Node_String_strlen(const NV_Node *ns);
//
NV_ID NV_Node_createWithInt32(int32_t v);
void NV_Node_setInt32ToID(const NV_ID *id, int32_t v);
int32_t NV_Node_getInt32FromID(const NV_ID *id);

// nv_op.c
int NV_Lang_getCharType(const NV_ID *cTypeList, char c);
NV_ID NV_createCharTypeList();
void NV_addOp(const NV_ID *opList, const char *token, int32_t prec, const NV_ID *func);
NV_ID NV_createOpList();
void NV_Op_print(const NV_ID *op);

// @nv_static.c
extern const NV_ID NODEID_NULL;
extern const NV_ID NODEID_REL_MASTERLINK;
extern const NV_ID NODEID_TREE_TYPE_ARRAY;
extern const NV_ID NODEID_TREE_TYPE_VARIABLE;
extern const NV_ID NODEID_TREE_TYPE_OP;
extern const NV_ID RELID_ARRAY_NEXT;
extern const NV_ID RELID_VARIABLE_DATA;
extern const NV_ID RELID_POINTER_TARGET;
extern const NV_ID RELID_OP_PRECEDENCE;
extern const NV_ID RELID_OP_FUNC;
extern const NV_ID RELID_TREE_TYPE;
extern const NV_ID NODEID_NV_STATIC_ROOT;
const char *NV_NodeTypeList[kNodeTypeCount];

// @nv_test.c


// @nv_variable.c
NV_ID NV_Variable_create();
void NV_Variable_assign(const NV_ID *vid, const NV_ID *data);
NV_ID NV_Variable_getData(const NV_ID *vid);
void NV_Variable_print(const NV_ID *vid);
void NV_Variable_printPrimVal(const NV_ID *vid);

