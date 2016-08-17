#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG	0

#define MAX_INPUT_LEN	1024
#define MAX_TOKEN_LEN	2048
#define MAX_VARS		32

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define NV_Error(fmt, ...)	printf("Error: %s: %d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define NV_DbgInfo(fmt, ...) \
	if(NV_isDebugMode) printf("Info : %s: %d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

typedef enum	NV_ELEMENT_TYPE NV_ElementType;
typedef enum	NV_BIN_OP_TYPE	NV_BinOpType;

typedef struct	NV_ELEMENT		NV_Element;
typedef struct	NV_POINTER		NV_Pointer;

typedef struct	NV_LANGDEF		NV_LangDef;
typedef struct	NV_VARSET		NV_VariableSet;

typedef struct	NV_LIST_ITEM	NV_ListItem;
typedef struct	NV_ENV			NV_Env;
typedef struct	NV_OPERATOR 	NV_Operator;
typedef struct	NV_INTEGER		NV_Integer;
typedef struct	NV_STRING		NV_String;
typedef struct	NV_VARIABLE		NV_Variable;

enum NV_ELEMENT_TYPE {
	ENone,
	EList,
	EListItem,
	EEnv,
	EOperator,
	EInteger,
	EString,
	EVariable,
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
	NV_Pointer (*nativeFunc)(NV_Pointer env, NV_Pointer thisTerm);
	// retv: Last of Result Term
	// thisTerm.type == EList
};

struct NV_LANGDEF {
	// interpreter params
	int char0Len;
	const char *char0List;	// should be terminated with 0
	int char1Len;
	const char *char1List;	// should be terminated with 0
	int char2Len;
	const char *char2List;	// should be terminated with 0
	NV_Pointer opRoot;
};

struct NV_VARIABLE {
	char name[MAX_TOKEN_LEN];
	int revision;
	NV_Pointer data;
};

struct NV_VARSET {
	NV_Pointer varList[MAX_VARS];
	int varUsed;
};

extern int NV_isDebugMode;


// @nv.c
NV_LangDef *NV_allocLangDef();
//
//
int NV_getCharType(NV_LangDef *lang, char c);
void NV_tokenize(NV_LangDef *langDef, NV_Pointer termRoot, const char *input);
void NV_tokenizeItem(NV_LangDef *langDef, NV_Pointer termRoot, const char *termStr);
//
void NV_Evaluate(NV_Pointer env);
int NV_EvaluateSentence(NV_Pointer env, NV_Pointer root);
NV_Pointer NV_TryExecOp(NV_Pointer env, NV_Pointer currentOp, NV_Pointer t, NV_Pointer root);

// @nv_element.c
extern const NV_Pointer NV_NullPointer;
NV_Pointer NV_E_malloc_type(NV_ElementType type);
int NV_E_isNullPointer(NV_Pointer p);
void NV_E_free(NV_Pointer *p);
int NV_E_isValidPointer(NV_Pointer p);
int NV_E_isType(NV_Pointer p, NV_ElementType et);
void *NV_E_getRawPointer(NV_Pointer p, NV_ElementType et);
void NV_printElement(NV_Pointer p);

// @nv_env.c
NV_Env *NV_allocEnv();
int NV_Env_setVarSet(NV_Pointer env, NV_VariableSet *vs);
NV_VariableSet *NV_Env_getVarSet(NV_Pointer env);
int NV_Env_setLangDef(NV_Pointer env, NV_LangDef *ld);
NV_LangDef *NV_Env_getLangDef(NV_Pointer env);
int NV_Env_setAutoPrintValueEnabled(NV_Pointer env, int b);
int NV_Env_getAutoPrintValueEnabled(NV_Pointer env);
int NV_Env_setEndFlag(NV_Pointer env, int b);
int NV_Env_getEndFlag(NV_Pointer env);
NV_Pointer NV_Env_getTermRoot(NV_Pointer env);

// @nv_envdep.c
char *NV_gets(char *str, int size);
void NV_printf(const char *format, ...);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_integer.c
NV_Integer *NV_allocInteger();
void NV_Integer_setImm32(NV_Pointer t, int32_t data);
int32_t NV_Integer_getImm32(NV_Pointer t);
void NV_Integer_print(NV_Pointer t);
//
NV_Pointer NV_Integer_evalBinOp(NV_Pointer vL, NV_Pointer vR, NV_BinOpType type);

// @nv_lang.c
NV_LangDef *NV_getDefaultLang();

// @nv_list.c
NV_ListItem *NV_allocListItem();
NV_Pointer NV_List_allocRoot();
NV_Pointer NV_List_getNextItem(NV_Pointer item);
NV_Pointer NV_List_getPrevItem(NV_Pointer item);
NV_Pointer NV_List_lastItem(NV_Pointer root);
NV_Pointer NV_List_removeItem(NV_Pointer item);
NV_Pointer NV_List_removeItemByIndex(NV_Pointer rootItem, int i);
void NV_List_insertItemAfter(NV_Pointer prevItem, NV_Pointer newItem);
void NV_List_insertAllAfter(NV_Pointer prevItem, NV_Pointer rootItem);
void NV_List_insertAllAfterIndex(NV_Pointer dstRoot, int index, NV_Pointer rootItem);
NV_Pointer NV_List_getItemByIndex(NV_Pointer rootItem, int i);
int NV_List_getItemIndex(NV_Pointer item);
//
void NV_List_push(NV_Pointer rootItem, NV_Pointer newData);
NV_Pointer NV_List_pop(NV_Pointer pRoot);
NV_Pointer NV_List_shift(NV_Pointer rootItem);
void NV_List_unshift(NV_Pointer rootItem, NV_Pointer newData);
void NV_List_insertDataAfterItem(NV_Pointer itemInList, NV_Pointer newData);
void NV_List_insertDataBeforeItem(NV_Pointer itemInList, NV_Pointer newData);
void NV_List_insertDataAfterIndex(NV_Pointer root, int index, NV_Pointer newData);
NV_Pointer NV_List_getItemData(NV_Pointer item);
NV_Pointer NV_List_getDataByIndex(NV_Pointer rootItem, int i);
void *NV_List_getItemRawData(NV_Pointer item, NV_ElementType et);
NV_Pointer NV_List_setItemData(NV_Pointer item, NV_Pointer newData);
int NV_List_isItemType(NV_Pointer item, NV_ElementType et);
int NV_List_indexOfData(NV_Pointer root, NV_Pointer data);
//
void NV_List_printAll(NV_Pointer root, const char *delimiter);

// @nv_operator.c
NV_Operator *NV_allocOperator();
void NV_Operator_print(NV_Pointer t);
void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Pointer(*nativeFunc)(NV_Pointer env, NV_Pointer thisTerm));
NV_Pointer NV_getOperatorFromString(NV_LangDef *lang, const char *termStr);
NV_Pointer NV_getFallbackOperator(NV_LangDef *lang, NV_Pointer baseP);
int NV_getOperatorIndex(NV_LangDef *lang, NV_Pointer op);
int NV_Operator_isLeftAssociative(NV_Pointer op);
NV_Pointer NV_Operator_exec(NV_Pointer op, NV_Pointer env, NV_Pointer thisTerm);

// @nv_string.c
NV_String *NV_allocString();
void NV_String_setString(NV_Pointer t, const char *s);
void NV_String_print(NV_Pointer t);

// @nv_term.c
NV_Pointer NV_allocTerm();
NV_Pointer NV_cloneTerm(const NV_Pointer src);
NV_Pointer NV_createTerm_Operator(NV_LangDef *langDef, const char *opName);
NV_Pointer NV_createTerm_Imm32(int imm32);
NV_Pointer NV_createTerm_String(const char *s);
NV_Pointer NV_createTerm_Variable(NV_VariableSet *vs, const char *name);
NV_Pointer NV_createTerm_Sentence(NV_Pointer baseTree);
void NV_printRealTermValue(NV_Pointer t, NV_VariableSet *vs);
int NV_canReadTermAsInt(NV_Pointer t);
int NV_getValueOfTermAsInt(NV_Pointer t);
int NV_canReadTermAsSentence(NV_Pointer t);
void NV_getValueOfTermAsSentence(NV_Pointer t, NV_Pointer dstRoot);


// @nv_termlist.c
/*
int NV_initRootTerm(NV_Pointer t);
void NV_changeRootTerm(NV_PointeroldRoot, NV_PointernewRoot);
void NV_cloneTermTree(NV_PointerdstRoot, const NV_PointersrcRoot);
void NV_insertTermAfter(NV_Pointerbase, NV_Pointernew);
void NV_insertAllTermAfter(NV_Pointerbase, NV_PointersrcRoot);
NV_PointerNV_overwriteTerm(NV_Pointertarget, NV_Pointernew);
void NV_divideTerm(NV_PointersubRoot, NV_PointersubBegin);
void NV_appendAll(NV_PointerdstRoot, NV_PointersrcRoot);
//
void NV_removeTerm(NV_Pointert);
void NV_removeTermTree(NV_Pointerroot);
void NV_printTerms(NV_Pointerroot);
NV_PointerNV_getTermByIndex(NV_Pointerroot, int index);
NV_PointerNV_getLastTerm(NV_Pointerroot);
void NV_printLastTermValue(NV_Pointerroot, NV_VariableSet *vs);
*/

// @nv_var.c
NV_Variable *NV_allocVariable();
int NV_resetVariable(NV_Pointer v);
int NV_Variable_assignTermValue(NV_Variable *v, NV_Pointer src);
void NV_Variable_assignVariable(NV_Pointer dst, NV_Pointer src);
void NV_Variable_assignInteger(NV_Variable *v, int32_t newVal);
void NV_Variable_assignString(NV_Variable *v, const char *src);
void NV_Variable_assignStructure(NV_Variable *dst, const NV_Pointer srcRoot);
void NV_Variable_assignStructureItem(NV_Variable *dst, NV_Pointer term);
void NV_tryConvertTermFromUnknownToVariable(NV_VariableSet *vs, NV_Pointer *term, int allowCreateNewVar); 
void NV_tryConvertTermFromUnknownToImm(NV_VariableSet *vs, NV_Pointer *term);
NV_Pointer NV_getVariableByName(NV_VariableSet *vs, const char *name);
NV_Pointer NV_getItemFromStructureByIndex(NV_Variable *v, int index);
void NV_printVariable(NV_Variable *var, int verbose);

// @nv_varset.c
NV_VariableSet *NV_allocVariableSet();
void NV_printVarsInVarSet(NV_VariableSet *vs);

