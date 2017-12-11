#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT_LEN 1024
#define MAX_TOKEN_LEN 256
#define MAX_SAVE_DATA_ENTRY_SIZE 4096

#define malloc(s) DO_NOT_USE_THIS_FUNC(s)
#define free(p) DO_NOT_USE_THIS_FUNC(p)
// #define strncpy(p, q, r)		DO_NOT_USE_THIS_FUNC(p)
// #define strcpy(p, q)			DO_NOT_USE_THIS_FUNC(p)

#define PARAM_UNUSED(x) ((void)x)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SET_FLAG(f, d) f |= (d)
#define CLR_FLAG(f, d) f &= ~(d)

#define ESC_ANSI_RED(s) \
  "\033[31m"s           \
  "\033[39m"
#define ESC_ANSI_GREEN(s) \
  "\033[32m"s             \
  "\033[39m"
#define ESC_ANSI_YELLOW(s) \
  "\033[33m"s              \
  "\033[39m"
#define ESC_ANSI_CYAN(s) \
  "\033[36m"s            \
  "\033[39m"

#define NV_Error(fmt, ...)                                                   \
  printf(ESC_ANSI_RED("\nError: %s: %d: ") fmt "\n", __FUNCTION__, __LINE__, \
         __VA_ARGS__)

#define IS_DEBUG_MODE() (NV_globalExecFlag & NV_EXEC_FLAG_VERBOSE)

#if DEBUG
#define NV_DbgInfo(fmt, ...) \
  printf("\nInfo : %3d:%s: " fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__)
#define NV_DbgInfo0(s) \
  printf("\nInfo : %3d:%s: %s\n", __LINE__, __FUNCTION__, s)
#define NV_DbgInfo_mem(n, s)                                   \
  printf("Info : Mem: %08X: %2d %s %s\n", n->id.d[0], n->type, \
         NV_NodeTypeList[n->type], s);
#else
#define NV_DbgInfo(fmt, ...)
#define NV_DbgInfo0(s)
#define NV_DbgInfo_mem(n, s)
#endif

typedef struct NV_NODE NV_Node;
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
typedef enum NV_NODE_TYPE NV_NodeType;

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
  int index, prec;
  NV_ID dict;
};
typedef struct NV_LANG_OP_TAG {
  const char *token;
  int prec;
  const char *funcStr;
} NV_Lang_OpTag;

// @main.c
extern NV_ID rootScope;

// @nv.c
#define NV_EXEC_FLAG_VERBOSE 0x01
#define NV_EXEC_FLAG_INTERRUPT 0x02
#define NV_EXEC_FLAG_INTERACTIVE 0x04
#define NV_EXEC_FLAG_SAVECODEGRAPH 0x08
#define NV_EXEC_FLAG_SUPRESS_AUTOPRINT 0x10

extern volatile sig_atomic_t NV_globalExecFlag;

// @nv_node.c
extern NV_Node nodeRoot;

// @nv_lang.c
#define NV_LANG_CHAR_TYPE_LIST_LEN 3
typedef struct {
  const char *name;
  NV_ID(*parser)
  (const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident);
} NV_Lang_ParseTag;

typedef struct {
  const char *name;
  NV_ID(*evaluator)
  (NV_ID *const p, NV_ID *const lastEvalVal, const NV_ID *scope);
} NV_Lang_EvalTag;

// @lang/02/parse.c
extern const char *NV_Lang02_charTypeList[NV_LANG_CHAR_TYPE_LIST_LEN];
extern NV_Lang_OpTag NV_Lang02_opList[];
extern NV_Lang_ParseTag NV_Lang02_funcList[];
extern NV_Lang_EvalTag NV_Lang02_evalList[];
// @nv_static.c
#include "nv_static.h"
//
const char *NV_NodeTypeList[kNodeTypeCount];
const char c2hexTable[0x100];
//

#include "nv_func.h"
