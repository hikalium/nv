#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>

#define MAX_INPUT_LEN		1024
#define MAX_TOKEN_LEN		256
#define MAX_SAVE_DATA_ENTRY_SIZE	4096

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
#define ESC_ANSI_YELLOW(s)	"\033[33m"s"\033[39m"
#define ESC_ANSI_CYAN(s)	"\033[36m"s"\033[39m"

#define NV_Error(fmt, ...)	printf(ESC_ANSI_RED("\nError: %s: %d: ")fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#define IS_DEBUG_MODE()	(NV_globalExecFlag & NV_EXEC_FLAG_VERBOSE)

#if DEBUG
	#define NV_DbgInfo(fmt, ...) \
		printf("\nInfo : %3d:%s: " fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__)
	#define NV_DbgInfo0(s) \
		printf("\nInfo : %3d:%s: %s\n", __LINE__, __FUNCTION__, s)
	#define NV_DbgInfo_mem(n, s) \
		printf("Info : Mem: %08X: %2d %s %s\n", n->id.d[0], n->type, NV_NodeTypeList[n->type], s);
#else
	#define NV_DbgInfo(fmt, ...)
	#define NV_DbgInfo0(s)
	#define NV_DbgInfo_mem(n, s)
#endif

typedef struct NV_NODE NV_Node;
typedef enum NV_NODE_TYPE NV_NodeType;
typedef struct NV_ELEMENT_ID NV_ID;

typedef struct NV_RELATION NV_Relation;

enum NV_NODE_TYPE {
	kNone,
	kRelation,
	kBLOB,
	kString,
	kInteger,
	//
	kNodeTypeCount,
};

struct NV_ELEMENT_ID {
	uint32_t d[4];
};

struct NV_NODE {
	NV_ID id;
	void *data;
	NV_Node *prev;
	NV_Node *next;
	NV_NodeType type;
	int size;	// size of data, bytes.
	int refCount;
	//
	const NV_Node *relCache; // link from this node. recently referenced.
};

struct NV_RELATION {
	NV_ID from;
	NV_ID rel;
	NV_ID to;
};

// @nv.c
#define NV_EXEC_FLAG_VERBOSE		0x01
#define NV_EXEC_FLAG_INTERRUPT		0x02
#define NV_EXEC_FLAG_INTERACTIVE	0x04
extern volatile sig_atomic_t NV_globalExecFlag;
//
NV_ID NV_tokenize(const NV_ID *cTypeList, const char *input);
int NV_interactiveInput(const NV_ID *cTypeList, const NV_ID *ctx);
void NV_evalLoop(const NV_ID *opList, const NV_ID *ctx);

// @nv_array.c
NV_ID NV_Array_create();
NV_ID NV_Array_clone(const NV_ID *base);
NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data);
NV_ID NV_Array_pop(const NV_ID *array);
NV_ID NV_Array_last(const NV_ID *array);
int32_t NV_Array_count(const NV_ID *array);
NV_ID NV_Array_getByIndex(const NV_ID *array, int index);
void NV_Array_removeIndex(const NV_ID *array, int index);
void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data);
NV_ID NV_Array_joinWithCStr(const NV_ID *array, const char *sep);
NV_ID NV_Array_getSorted(const NV_ID *array, int (*f)(const void *n1, const void *n2));
void NV_Array_print(const NV_ID *array);

// @nv_context.c
NV_ID NV_getContextList();
NV_ID NV_Context_create();
NV_ID NV_Context_getEvalStack(const NV_ID *ctx);
NV_ID NV_Context_createChildScopeWithArgs(const NV_ID *ctx, const NV_ID *argsBlock);
void NV_Context_pushToEvalStack(const NV_ID *ctx, const NV_ID *code, const NV_ID *newScope);
NV_ID NV_Context_getCurrentCode(const NV_ID *ctx);
NV_ID NV_Context_getCurrentScope(const NV_ID *ctx);
NV_ID NV_Context_getLastResult(const NV_ID *ctx);
void NV_Context_setOpDict(const NV_ID *ctx, const NV_ID *opDict);
NV_ID NV_Context_getOpDict(const NV_ID *ctx);

// @nv_dict.c
NV_ID NV_Dict_addKey(const NV_ID *root, const NV_ID *key, const NV_ID *value);
NV_ID NV_Dict_addKeyByCStr(const NV_ID *root, const char *key, const NV_ID *value);
NV_ID NV_Dict_addUniqueIDKey(const NV_ID *root, const NV_ID *key, const NV_ID *value);
NV_ID NV_Dict_addUniqueEqKeyByCStr(const NV_ID *root, const char *key, const NV_ID *value);
NV_ID NV_Dict_removeUniqueIDKey(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_removeUniqueEqKeyByCStr(const NV_ID *root, const char *key);
NV_ID NV_Dict_get(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getEqID(const NV_ID *root, const NV_ID *key);
//NV_ID NV_Dict_getAll(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getByStringKey(const NV_ID *root, const char *key);
void NV_Dict_print(const NV_ID *root);

// @nv_driver.c
char *NV_gets(char *str, int size);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
long NV_strtolSeq(const char **restrict p, int base);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_graph.c
extern NV_Node nodeRoot;
void NV_Graph_init();
void NV_Graph_insertInitialNode();
void NV_Graph_dump();
void NV_Graph_dumpToFile(FILE *fp);
void NV_Graph_restoreFromFile(FILE *fp);

// @nv_id.c
NV_ID NV_ID_generateRandom();
int NV_ID_setFromString(NV_ID *id, const char *s);
int NV_NodeID_isEqual(const NV_ID *a, const NV_ID *b);
void NV_ID_dumpIDToFile(const NV_ID *id, FILE *fp);
int NV_NodeID_isEqualInValue(const NV_ID *a, const NV_ID *b);
void NV_NodeID_printPrimVal(const NV_ID *id);

// @nv_node.c
int NV_NodeID_exists(const NV_ID *id);
NV_Node *NV_NodeID_getNode(const NV_ID *id);
int NV_Node_isEqualInValue(const NV_Node *na, const NV_Node *nb);
NV_ID NV_NodeID_create(const NV_ID *id);
NV_ID NV_Node_create();
void NV_NodeID_remove(const NV_ID *baseID);
NV_ID NV_NodeID_clone(const NV_ID *baseID);
NV_ID NV_Node_restoreFromString(const char *s);
void NV_NodeID_retain(const NV_ID *id);
void NV_NodeID_release(const NV_ID *id);
void NV_Node_cleanup();
void NV_Node_fdump(FILE *fp, const NV_Node *n);
void NV_Node_dump(const NV_Node *n);
void NV_Node_printPrimVal(const NV_Node *n);
void NV_NodeID_printForDebug(const NV_ID *id);

// @nv_relation.c
NV_ID NV_NodeID_createRelation(const NV_ID *from, const NV_ID *rel,  const NV_ID *to);
NV_ID NV_NodeID_createUniqueIDRelation(const NV_ID *from, const NV_ID *rel,  const NV_ID *to);
NV_ID NV_NodeID_createUniqueEqRelation(const NV_ID *from, const NV_ID *rel,  const NV_ID *to);
void NV_Node_setRelation
	(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to);
NV_Node *NV_NodeID_Relation_getLinkFrom(const NV_ID *relnid);
NV_ID NV_NodeID_Relation_getIDLinkTo(const NV_ID *relnid);
NV_Node *NV_NodeID_Relation_getLinkTo(const NV_ID *relnid);
NV_Node *NV_NodeID_Relation_getLinkRel(const NV_ID *relnid);
void NV_NodeID_updateRelationTo(const NV_ID *relnid, const NV_ID *to);
const NV_Node *NV_NodeID_getRelNodeFromWithCmp
	(const NV_ID *from, const NV_ID *rel, int (*cmp)(const NV_ID *p, const NV_ID *q));
NV_ID NV_NodeID_getRelationFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getEqRelationFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getEqRelatedNodeFrom(const NV_ID *from, const NV_ID *rel);

// @nv_string.c
int NV_NodeID_isString(const NV_ID *id);
NV_ID NV_Node_createWithString(const char *s);
NV_ID NV_Node_createWithStringFormat(const char *fmt, ...);
void NV_NodeID_createAndString(const NV_ID *id, const char *s);
const char *NV_NodeID_getCStr(const NV_ID *id);
int NV_Node_String_compare(const NV_Node *na, const NV_Node *nb);
int NV_Node_String_compareWithCStr(const NV_Node *na, const char *s);
int NV_NodeID_String_compareWithCStr(const NV_ID *na, const char *s);
char *NV_Node_String_strchr(const NV_Node *ns, char c);
long NV_Node_String_strtol(const NV_Node *ns, int *endptrindex, int base);
long NV_NodeID_String_strtol(const NV_ID *ns, int *endptrindex, int base);
size_t NV_Node_String_strlen(const NV_Node *ns);
size_t NV_NodeID_String_strlen(const NV_ID *ns);

// @nv_integer.c
int NV_NodeID_isInteger(const NV_ID *id);
NV_ID NV_Node_createWithInt32(int32_t v);
int32_t NV_NodeID_getInt32(const NV_ID *id);

// @nv_op.c
int NV_Lang_getCharType(const NV_ID *cTypeList, char c);
NV_ID NV_createCharTypeList();
void NV_addOp(const NV_ID *opList, const char *token, int32_t prec, const NV_ID *func);
NV_ID NV_createOpList();
int32_t NV_getOpPrec(const NV_ID *op);
void NV_getOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, NV_ID *idBuf, int count);
void NV_removeOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, int count);
void NV_tryExecOpAt(const NV_ID *tList, int index, const NV_ID *ctx);
void NV_printOp(const NV_ID *op);

// @nv_path.c
NV_ID NV_Path_createWithOrigin(const NV_ID *origin);
NV_ID NV_Path_createAbsoluteWithCodeBlock(NV_ID *code);
NV_ID NV_Path_createAbsoluteWithCStr(const char *pathStr);
void NV_Path_appendRoute(const NV_ID *path, const NV_ID *r);
NV_ID NV_Path_getTarget(const NV_ID *path);
int NV_Path_statTarget(const NV_ID *path);
NV_ID NV_Path_print(const NV_ID *path);
void NV_Path_assign(const NV_ID *path, const NV_ID *data);

// @nv_signal.c
void NV_signalHandler(int signum);

// @nv_static.c
extern const NV_ID NODEID_NV_STATIC_ROOT;
extern const NV_ID NODEID_NULL;
extern const NV_ID NODEID_NOT_FOUND;
extern const NV_ID NODEID_TERM_TYPE_ARRAY;
extern const NV_ID NODEID_TERM_TYPE_VARIABLE;
extern const NV_ID NODEID_TERM_TYPE_OP;
extern const NV_ID NODEID_TERM_TYPE_PATH;
//
extern const NV_ID RELID_ARRAY_NEXT;
extern const NV_ID RELID_ARRAY_COUNT;
extern const NV_ID RELID_VARIABLE_DATA;
extern const NV_ID RELID_POINTER_TARGET;
extern const NV_ID RELID_OP_PRECEDENCE;
extern const NV_ID RELID_OP_FUNC;
extern const NV_ID RELID_TERM_TYPE;
extern const NV_ID RELID_EVAL_STACK;
extern const NV_ID RELID_LAST_RESULT;
extern const NV_ID RELID_CURRENT_TERM_INDEX;
extern const NV_ID RELID_CURRENT_TERM_PHASE;
extern const NV_ID RELID_CURRENT_SCOPE;
extern const NV_ID RELID_PARENT_SCOPE;
extern const NV_ID RELID_CONTEXT_LIST;
extern const NV_ID RELID_NEXT_CONTEXT;
extern const NV_ID RELID_OP_DICT;
//
const char *NV_NodeTypeList[kNodeTypeCount];
const char c2hexTable[0x100];

// @nv_term.c
int NV_isTermType(const NV_ID *node, const NV_ID *tType);
NV_ID NV_Term_tryReadAsVariableData(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_tryReadAsVariable(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_getPrimNodeID(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_tryReadAsOperator(const NV_ID *id, const NV_ID *scope);
int NV_Term_isInteger(const NV_ID *id, const NV_ID *scope);
int NV_Term_isAssignable(const NV_ID *id, const NV_ID *scope);
int NV_Term_isArray(const NV_ID *id, const NV_ID *scope);
//
int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_getAssignableNode(const NV_ID *id, const NV_ID *scope);
//
void NV_printNodeByID(const NV_ID *id);
void NV_printNode(const NV_Node *n);

// @nv_test.c
//void NV_Test_Memory();

// @nv_variable.c
NV_ID NV_Variable_create();
NV_ID NV_Variable_createWithName(const NV_ID *parentNode, const NV_ID *nameNode);
NV_ID NV_Variable_getNamed(const NV_ID *parentNode, const NV_ID *nameNode);
void NV_Variable_assign(const NV_ID *vid, const NV_ID *data);
NV_ID NV_Variable_getData(const NV_ID *vid);
void NV_Variable_print(const NV_ID *vid);
void NV_Variable_printPrimVal(const NV_ID *vid);

