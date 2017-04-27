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
typedef struct NV_OP_POINTER NV_OpPointer;

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

struct NV_RELATION {
	NV_ID from;
	NV_ID rel;
	NV_ID to;
};

struct NV_OP_POINTER {
	NV_ID op;
	int index;
	NV_ID dict;
};

// @nv.c
#define NV_EXEC_FLAG_VERBOSE		0x01
#define NV_EXEC_FLAG_INTERRUPT		0x02
#define NV_EXEC_FLAG_INTERACTIVE	0x04
extern volatile sig_atomic_t NV_globalExecFlag;

// @nv_node.c
extern NV_Node nodeRoot;

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
//

#include "nv_func.h"
