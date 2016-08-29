#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG	0
#define NV_DEBUG_MEMORY

#define MAX_INPUT_LEN	256
#define MAX_TOKEN_LEN	256

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define NV_Error(fmt, ...)	printf("\nError: %s: %d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define NV_DbgInfo(fmt, ...) \
	if(NV_isDebugMode) printf("\nInfo : %s: %d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef NV_DEBUG_MEMORY
	#define NV_E_retain(p)		NV_E_retainWithInfo(p, __FUNCTION__)
	#define NV_E_free(p)		NV_E_freeWithInfo(p, __FUNCTION__)
	#define NV_E_autorelease(p)	NV_E_autoreleaseWithInfo(p, __FUNCTION__)
#else
	#define NV_E_retain(p)	NV_E_retain_raw(p)
	#define NV_E_free(p)	NV_E_free_raw(p)
	#define NV_E_autorelease(p)	NV_E_autorelease_raw(p)
#endif

#define ESC_ANSI_RED(s)		"\033[31m"s"\033[39m"
#define ESC_ANSI_GREEN(s)	"\033[32m"s"\033[39m"
#define ESC_ANSI_YERROW(s)	"\033[33m"s"\033[39m"
#define ESC_ANSI_CYAN(s)	"\033[36m"s"\033[39m"

#define malloc(s)	DO_NOT_USE_MALLOC(s)
#define free(p)		DO_NOT_USE_FREE(p)

typedef enum	NV_ELEMENT_TYPE NV_ElementType;
typedef enum	NV_ELEMENT_FLAG NV_ElementFlag;
typedef enum	NV_BIN_OP_TYPE	NV_BinOpType;

typedef struct	NV_ELEMENT		NV_Element;
typedef struct	NV_POINTER		NV_Pointer;

typedef struct	NV_LANG			NV_Lang;

typedef struct	NV_LIST_ITEM	NV_ListItem;
typedef struct	NV_DICT_ITEM	NV_DictItem;
typedef struct	NV_ENV			NV_Env;
typedef struct	NV_OPERATOR 	NV_Operator;
typedef struct	NV_INTEGER		NV_Integer;
typedef struct	NV_STRING		NV_String;
typedef struct	NV_VARIABLE		NV_Variable;

typedef NV_Pointer(*NV_OpFunc)(NV_Pointer env, NV_Pointer vDict, NV_Pointer thisTerm);

enum NV_ELEMENT_TYPE {
	ENone,
	EList,		// complex
	EListItem,	// complex (can't be a pool)
	EDict,		// complex
	EDictItem,	// complex (can't be a pool)
	EVariable,	// complex
	EEnv,
	ELang,
	EOperator,	// primitive
	EInteger,	// primitive
	EString,	// primitive
};

enum NV_ELEMENT_FLAG {
	EFUnknownToken = 1,
};

enum NV_BIN_OP_TYPE {
	BOpNone,
	BOpAdd,
	BOpSub,
	BOpMul,
	BOpDiv,
	BOpMod,
	BOpLogicOR,
	BOpLogicAND,
	BOpBitOR,
	BOpBitAND,
	BOpCmpEq,
	BOpCmpNEq,
	BOpCmpLt,
	BOpCmpGt,
	BOpCmpLtE,
	BOpCmpGtE,
};

struct NV_POINTER {
	int token;
	NV_Element *data;	// Always exists
};

struct NV_OPERATOR {
	char name[MAX_TOKEN_LEN];
	int precedence;		// do not change after adding.
	NV_OpFunc nativeFunc;
	// retv: Last of Result Term
	// thisTerm.type == EList
};

extern int NV_isDebugMode;

// @nv.c
//
//
void NV_tokenize(NV_Pointer langDef, NV_Pointer termRoot, const char *input);
void NV_tokenizeItem(NV_Pointer langDef, NV_Pointer termRoot, const char *termStr);
//
int NV_EvaluateSentence(NV_Pointer env, NV_Pointer root);
NV_Pointer NV_TryExecOp(NV_Pointer env, int currentOpPrec, NV_Pointer t, NV_Pointer root);

// @nv_dict.c
NV_Pointer NV_Dict_allocRoot();
NV_Pointer NV_Dict_clone(NV_Pointer p);
//
int NV_Dict_add(NV_Pointer dict, NV_Pointer key, NV_Pointer val);
NV_Pointer NV_Dict_getItemByKey(NV_Pointer dict, NV_Pointer key);
NV_Pointer NV_Dict_getValByKey(NV_Pointer dict, NV_Pointer key);
//NV_Pointer NV_Dict_getKeys(NV_Pointer dict, NV_Pointer val);
//
NV_Pointer NV_DictItem_getKey(NV_Pointer item);
NV_Pointer NV_DictItem_getVal(NV_Pointer item);
int32_t NV_DictItem_getRevision(NV_Pointer item);
void NV_DictItem_updateRevision(NV_Pointer item);
void NV_DictItem_setVal(NV_Pointer item, NV_Pointer val);
//
void NV_DictItem_print(NV_Pointer item);
void NV_Dict_printAll(NV_Pointer dict, const char *prefix, const char *delimiter, const char *suffix);

// @nv_element.c
extern const NV_Pointer NV_NullPointer;
NV_Pointer NV_E_malloc_type(NV_ElementType type);
int NV_E_isNullPointer(NV_Pointer p);
NV_Pointer NV_E_retain_raw(NV_Pointer p);
void NV_E_free_raw(NV_Pointer *p);
NV_Pointer NV_E_autorelease_raw(NV_Pointer p);
#ifdef NV_E_retain
NV_Pointer NV_E_retainWithInfo(NV_Pointer p, const char *fname);
void NV_E_freeWithInfo(NV_Pointer *p, const char *fname);
NV_Pointer NV_E_autoreleaseWithInfo(NV_Pointer p, const char *fname);
#endif
int NV_E_isValidPointer(NV_Pointer p);
int NV_E_isType(NV_Pointer p, NV_ElementType et);
int NV_E_isSamePointer(NV_Pointer p, NV_Pointer q);
int NV_E_isEqual(NV_Pointer p, NV_Pointer q);
void *NV_E_getRawPointer(NV_Pointer p, NV_ElementType et);
void NV_E_unbox(NV_Pointer * maybeBoxedItem);
void NV_E_convertUnknownToKnown(NV_Pointer vDict, NV_Pointer *mayStr);
void NV_E_convertToContents(NV_Pointer vDict, NV_Pointer *item);
void NV_E_setFlag(NV_Pointer p, int32_t flag);
void NV_E_clearFlag(NV_Pointer p, int32_t flag);
int NV_E_checkFlag(NV_Pointer p, int32_t pattern);
NV_Pointer NV_E_clone(NV_Pointer p);
//
void NV_printElement(NV_Pointer p);
void NV_E_printMemStat();

// @nv_env.c
NV_Pointer NV_Env_getVarRoot(NV_Pointer env);
int NV_Env_setLang(NV_Pointer env, NV_Pointer lang);
NV_Pointer NV_Env_getLang(NV_Pointer env);
int NV_Env_setAutoPrintValueEnabled(NV_Pointer env, int b);
int NV_Env_getAutoPrintValueEnabled(NV_Pointer env);
int NV_Env_setEndFlag(NV_Pointer env, int b);
int NV_Env_getEndFlag(NV_Pointer env);

// @nv_envdep.c
char *NV_gets(char *str, int size);
void NV_prinitf(const char *format, ...);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_integer.c
NV_Pointer NV_Integer_alloc(int32_t data);
NV_Pointer NV_Integer_clone(NV_Pointer p);
void NV_Integer_setImm32(NV_Pointer t, int32_t data);
int32_t NV_Integer_getImm32(NV_Pointer t);
void NV_Integer_print(NV_Pointer t);
//
NV_Pointer NV_Integer_evalBinOp(NV_Pointer vL, NV_Pointer vR, NV_BinOpType type);

// @nv_lang.c
NV_Pointer NV_allocDefaultLang();
void NV_Lang_setCharList(NV_Pointer lang, int type, const char *s);
int NV_Lang_getCharType(NV_Pointer lang, char c);
NV_Pointer NV_Lang_getOpList(NV_Pointer lang);
NV_Pointer NV_Lang_getOperatorFromString(NV_Pointer lang, const char *termStr);
NV_Pointer NV_Lang_getFallbackOperator(NV_Pointer lang, NV_Pointer baseP);
void NV_Lang_registerOperator(NV_Pointer lang, NV_Pointer op);
void NV_Lang_addOp(NV_Pointer lang, int pr, const char *name, NV_OpFunc f);

// @nv_lang00.c
NV_Pointer NV_allocLang00();

// @nv_list.c
// ---- Item ----
NV_Pointer NV_ListItem_getNext(NV_Pointer item);
NV_Pointer NV_ListItem_getPrev(NV_Pointer item);
NV_Pointer NV_ListItem_getData(NV_Pointer item);
void NV_ListItem_setData(NV_Pointer item, NV_Pointer newData);
void *NV_ListItem_getRawData(NV_Pointer item, NV_ElementType et);
int NV_ListItem_isDataType(NV_Pointer item, NV_ElementType et);
void NV_ListItem_convertUnknownToKnown(NV_Pointer vDict, NV_Pointer item);
void NV_ListItem_print(NV_Pointer t);
// ---- List ----
NV_Pointer NV_List_allocRoot();
NV_Pointer NV_List_clone(NV_Pointer p);
NV_Pointer NV_List_getItemByIndex(NV_Pointer rootItem, int i);
NV_Pointer NV_List_getLastItem(NV_Pointer root);
/*
void NV_List_unlinkItem(NV_Pointer item);
void NV_List_unlinkItemByIndex(NV_Pointer rootItem, int i);
*/
void NV_List_insertItemAfter(NV_Pointer prevItem, NV_Pointer newItem);
void NV_List_insertAllAfter(NV_Pointer prevItem, NV_Pointer rootItem);
void NV_List_insertAllAfterIndex(NV_Pointer dstRoot, int index, NV_Pointer rootItem);
NV_Pointer NV_List_divideBefore(NV_Pointer dividerItem);
void NV_List_push(NV_Pointer rootItem, NV_Pointer newData);
NV_Pointer NV_List_pop(NV_Pointer pRoot);
NV_Pointer NV_List_shift(NV_Pointer rootItem);
void NV_List_unshift(NV_Pointer rootItem, NV_Pointer newData);
void NV_List_insertDataAfterItem(NV_Pointer itemInList, NV_Pointer newData);
void NV_List_insertDataBeforeItem(NV_Pointer itemInList, NV_Pointer newData);
void NV_List_insertDataAfterIndex(NV_Pointer root, int index, NV_Pointer newData);
NV_Pointer NV_List_getDataByIndex(NV_Pointer rootItem, int i);
int NV_List_indexOfData(NV_Pointer root, NV_Pointer data);
void NV_List_printAll(NV_Pointer root, const char *prefix, const char *delimiter, const char *suffix);

// @nv_operator.c
NV_Pointer NV_Operator_alloc(int precedence, const char *name, NV_OpFunc nativeFunc);
NV_Pointer NV_Operator_clone(NV_Pointer p);
void NV_Operator_print(NV_Pointer t);
int NV_getOperatorPrecedence(NV_Pointer op);
NV_Pointer NV_Operator_exec(NV_Pointer op, NV_Pointer env, NV_Pointer thisTerm);

// @nv_string.c
NV_Pointer NV_String_clone(NV_Pointer p);
void NV_String_setString(NV_Pointer t, const char *s);
int NV_String_isEqualToCStr(NV_Pointer str, const char *cstr);
int NV_String_isEqual(NV_Pointer str0, NV_Pointer str1);
void NV_String_print(NV_Pointer t);

// @nv_var.c
NV_Pointer NV_Variable_clone(NV_Pointer p);
NV_Pointer NV_Variable_allocByStr(NV_Pointer vDict, NV_Pointer str);
NV_Pointer NV_Variable_allocByCStr(NV_Pointer vDict, const char *s);
void NV_Variable_setTarget(NV_Pointer var, NV_Pointer target);
void NV_Variable_assignData(NV_Pointer var, NV_Pointer data);
NV_Pointer NV_Variable_getData(NV_Pointer var);
void NV_Variable_print(NV_Pointer p);

