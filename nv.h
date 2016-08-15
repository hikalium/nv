#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG	0

#define MAX_INPUT_LEN	1024
#define MAX_TOKEN_LEN	64
#define MAX_TOKENS		128
#define MAX_VARS		32

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define NV_Error(fmt, ...)	NV_printError("%s: %d: " fmt, __FUNCTION__, __LINE__, __VA_ARGS__)

typedef enum	NV_TERM_TYPE	NV_TermType;
typedef enum	NV_VAR_TYPE		NV_VarType;
typedef struct	NV_TERM 		NV_Term;
typedef struct	NV_OPERATOR 	NV_Operator;
typedef struct	NV_LANGDEF		NV_LangDef;
typedef struct	NV_VARIABLE		NV_Variable;
typedef struct	NV_VARSET		NV_VariableSet;
typedef struct	NV_ENV			NV_Env;

enum NV_TERM_TYPE {
	Root,
	// data -> null
	Unknown,
	// data -> termStr (static)
	Sentence,
	// data -> NV_Term (malloced)
	Variable,
	// data -> NV_Variable (static)
	Operator,
	// data -> NV_Operator (static)
	Imm32s,
	// data -> int (malloced)
	String,
};

enum NV_VAR_TYPE {
	VNone,
	VAlias,
	VInteger,
	//Real,
	VString,
	VStructure,
	VStructureItem,
	// data -> NV_term (do not free this pointer) 
};

//
//
//

enum NV_ELEMENT_TYPE {
	ENone,
	ERawData,
	EEnv
};

typedef enum NV_ELEMENT_TYPE NV_ElementType;
typedef struct NV_ELEMENT NV_Element;
typedef struct NV_POINTER NV_Pointer;

struct NV_POINTER {
	int token;
	NV_Element *data;	// Always exists
};

//
//
//

struct NV_TERM {
	NV_TermType type;
	void *data;
	NV_Term *prev, *next;
};

struct NV_OPERATOR {
	char name[MAX_TOKEN_LEN];
	int precedence;		// do not change after adding.
	NV_Operator *next;	// sorted in a descending order of precedence.
	NV_Term *(*nativeFunc)(NV_Pointer env, NV_Term *thisTerm);
	// retv: Last of Result Term
};

struct NV_LANGDEF {
	// interpreter params
	int char0Len;
	const char *char0List;	// should be terminated with 0
	int char1Len;
	const char *char1List;	// should be terminated with 0
	int char2Len;
	const char *char2List;	// should be terminated with 0
	NV_Operator *opRoot;
};

struct NV_VARIABLE {
	char name[MAX_TOKEN_LEN];
	NV_VarType type;
	int byteSize;
	int revision;
	void *data;	// malloced addr
};

struct NV_VARSET {
	NV_Variable varList[MAX_VARS];
	int varUsed;
};

struct NV_ENV {
	// interpreter params
	NV_LangDef *langDef;
	// interpreter env
	NV_Term termRoot;
	int autoPrintValue;
	int endFlag;
	NV_VariableSet *varSet;
};

extern int NV_isDebugMode;

// @nv.c
NV_LangDef *NV_allocLangDef();
//
NV_Operator *NV_allocOperator();
void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Pointer env, NV_Term *thisTerm));
NV_Operator *NV_isOperator(NV_LangDef *lang, const char *termStr);
NV_Operator *NV_getFallbackOperator(NV_LangDef *lang, NV_Operator *baseOp);
int NV_getOperatorIndex(NV_LangDef *lang, NV_Operator *op);
//
int NV_getCharType(NV_LangDef *lang, char c);
void NV_tokenize0(NV_LangDef *langDef, char (*token0)[MAX_TOKEN_LEN], int token0Len, int *token0Used,  const char *s);
int NV_tokenize(NV_Pointer env, const char *s);
//
void NV_Evaluate(NV_Pointer env);
int NV_EvaluateSentence(NV_Pointer env, NV_Term *root);
NV_Term *NV_TryExecOp(NV_Pointer env, NV_Operator *currentOp, NV_Term *t, NV_Term *root);
//
void NV_printError(const char *format, ...);

// @nv_element.c
NV_Pointer NV_E_malloc_type(NV_ElementType type);
NV_Pointer NV_E_malloc_size(int size);
void NV_E_free(NV_Pointer p);
int NV_E_isValidPointer(NV_Pointer p);
int NV_E_isType(NV_Pointer p, NV_ElementType et);
void *NV_E_getRawPointer(NV_Pointer p, NV_ElementType et);

// @nv_env.c
NV_Env *NV_allocEnv();
int NV_Env_setVarSet(NV_Pointer env, NV_VariableSet *vs);
NV_VariableSet *NV_Env_getVarSet(NV_Pointer env);
int NV_Env_setLangDef(NV_Pointer env, NV_LangDef *ld);
NV_LangDef *NV_Env_getLangDef(NV_Pointer env);
int NV_Env_setAutoPrintValueEnabled(NV_Pointer env, int b);
int NV_Env_setEndFlag(NV_Pointer env, int b);

// @nv_envdep.c
char *NV_gets(char *str, int size);
void NV_printf(const char *format, ...);

// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);

// @nv_lang.c
NV_LangDef *NV_getDefaultLang();

// @nv_term.c
NV_Term *NV_allocTerm();
NV_Term *NV_cloneTerm(const NV_Term *src);
NV_Term *NV_createTerm_Operator(NV_LangDef *langDef, const char *opName);
NV_Term *NV_createTerm_Imm32(int imm32);
NV_Term *NV_createTerm_String(const char *s);
NV_Term *NV_createTerm_Variable(NV_VariableSet *vs, const char *name);
NV_Term *NV_createTerm_Sentence(NV_Term *baseTree);
void NV_printRealTermValue(NV_Term *t, NV_VariableSet *vs);
int NV_canReadTermAsInt(NV_Term *t);
int NV_getValueOfTermAsInt(NV_Term *t);
int NV_canReadTermAsSentence(NV_Term *t);
void NV_getValueOfTermAsSentence(NV_Term *t, NV_Term *dstRoot);


// @nv_termlist.c
void NV_initRootTerm(NV_Term *t);
void NV_changeRootTerm(NV_Term *oldRoot, NV_Term *newRoot);
void NV_cloneTermTree(NV_Term *dstRoot, const NV_Term *srcRoot);
void NV_insertTermAfter(NV_Term *base, NV_Term *new);
void NV_insertAllTermAfter(NV_Term *base, NV_Term *srcRoot);
NV_Term *NV_overwriteTerm(NV_Term *target, NV_Term *new);
void NV_divideTerm(NV_Term *subRoot, NV_Term *subBegin);
void NV_appendAll(NV_Term *dstRoot, NV_Term *srcRoot);
void NV_appendTermRaw(NV_Term *root, NV_Term *new);
void NV_appendTerm(NV_LangDef *langDef, NV_Term *termRoot, const char *termStr);
void NV_removeTerm(NV_Term *t);
void NV_removeTermTree(NV_Term *root);
void NV_printTerms(NV_Term *root);
void NV_printTerms_noNewLine(NV_Term *root);
NV_Term *NV_getTermByIndex(NV_Term *root, int index);
NV_Term *NV_getLastTerm(NV_Term *root);
void NV_printLastTermValue(NV_Term *root, NV_VariableSet *vs);

// @nv_var.c
NV_Variable *NV_allocVariable(NV_VariableSet *vs);
void NV_resetVariable(NV_Variable *v);
int NV_Variable_assignTermValue(NV_Variable *v, NV_Term *src);
void NV_Variable_assignVariable(NV_Variable *dst, const NV_Variable *src);
void NV_Variable_assignInteger(NV_Variable *v, int32_t newVal);
void NV_Variable_assignString(NV_Variable *v, const char *src);
void NV_Variable_assignStructure(NV_Variable *dst, const NV_Term *srcRoot);
void NV_Variable_assignStructureItem(NV_Variable *dst, NV_Term *term);
void NV_tryConvertTermFromUnknownToVariable(NV_VariableSet *vs, NV_Term **term, int allowCreateNewVar); 
void NV_tryConvertTermFromUnknownToImm(NV_VariableSet *vs, NV_Term **term);
NV_Variable *NV_getVariableByName(NV_VariableSet *vs, const char *name);
NV_Term *NV_getItemFromStructureByIndex(NV_Variable *v, int index);
void NV_printVariable(NV_Variable *var, int verbose);

// @nv_varset.c
NV_VariableSet *NV_allocVariableSet();
void NV_printVarsInVarSet(NV_VariableSet *vs);

