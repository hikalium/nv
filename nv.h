#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_LINE_LEN	1024
#define MAX_TOKEN_LEN	64
#define MAX_TOKENS		128
#define MAX_VARS		32

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum	NV_TERM_TYPE	NV_TermType;
typedef enum	NV_VAR_TYPE		NV_VarType;
typedef struct	NV_TERM 		NV_Term;
typedef struct	NV_OPERATOR 	NV_Operator;
typedef struct	NV_LANGDEF		NV_LangDef;
typedef struct	NV_VARIABLE		NV_Variable;
typedef struct	NV_ENV			NV_Env;

enum NV_TERM_TYPE {
	Root,
	Unknown,
	Sentence,
	Variable,
	Operator,
	Imm32s,
};

enum NV_VAR_TYPE {
	None,
	//Alias
	Integer,
	//Real,
	//String,
	//Structure
};

struct NV_TERM {
	NV_TermType type;
	void *data;
	NV_Term *before, *next;
};

struct NV_OPERATOR {
	char name[MAX_TOKEN_LEN];
	int precedence;
	NV_Operator *next;
	NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm);
	// retv: Last of Result Term
};

struct NV_LANGDEF {
	// interpreter params
	int char0Len;
	const char *char0List;	// should be terminated with 0
	int char1Len;
	const char *char1List;	// should be terminated with 0
	NV_Operator *opRoot;
};

struct NV_VARIABLE {
	char name[MAX_TOKEN_LEN];
	NV_VarType type;
	int byteSize;
	int revision;
	void *data;
};

struct NV_ENV {
	// interpreter params
	NV_LangDef *langDef;
	// interpreter env
	NV_Term termRoot;
	int changeFlag;
	NV_Variable varList[MAX_VARS];
	int varUsed;
};

//
NV_Term *NV_LANG00_Op_assign(NV_Env *env, NV_Term *thisTerm);
NV_Term *NV_LANG00_Op_binaryOperator(NV_Env *env, NV_Term *thisTerm);
NV_Term *NV_LANG00_Op_nothingButDisappear(NV_Env *env, NV_Term *thisTerm);
NV_Term *NV_LANG00_Op_sentenceSeparator(NV_Env *env, NV_Term *thisTerm);
//
NV_LangDef *NV_allocLangDef();
NV_LangDef *NV_getDefaultLang();
//
NV_Variable *NV_allocVariable(NV_Env *env);
void NV_resetVariable(NV_Variable *v);
void NV_assignVariable_Integer(NV_Variable *v, int32_t newVal);
void NV_tryConvertTermFromVariableToImm(NV_Variable *varList, int varUsed, NV_Term **term);
NV_Variable *NV_getVariableByName(NV_Variable *varList, int varUsed, const char *name);
void NV_printVarsInVarList(NV_Variable *varList, int varUsed);
// @nv_term.c
NV_Term *NV_allocTerm();
void NV_initRootTerm(NV_Term *t);
void NV_changeRootTerm(NV_Term *oldRoot, NV_Term *newRoot);
void NV_insertTermAfter(NV_Term *base, NV_Term *new);
void NV_insertAllTermAfter(NV_Term *base, NV_Term *srcRoot);
NV_Term *NV_overwriteTerm(NV_Term *target, NV_Term *new);
void NV_divideTerm(NV_Term *subRoot, NV_Term *subBegin);
void NV_appendAll(NV_Term *dstRoot, NV_Term *srcRoot);
void NV_appendTermRaw(NV_Term *root, NV_Term *new);
void NV_appendTerm(NV_LangDef *langDef, NV_Term *termRoot, const char *termStr);
void NV_removeTerm(NV_Term *t);
void NV_removeTermTree(NV_Term *root);
NV_Term *NV_createTerm_Operator(NV_LangDef *langDef, const char *opName);
NV_Term *NV_createTerm_Imm32(int imm32);
NV_Term *NV_createTerm_Variable(NV_Env *env, const char *name);
NV_Term *NV_createTerm_Sentence();
void NV_printTerms(NV_Term *root);
//
NV_Operator *NV_allocOperator();
void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm));
NV_Operator *NV_isOperator(NV_LangDef *lang, const char *termStr);
//
NV_Env *NV_allocEnv();
//
int NV_getCharType(NV_LangDef *lang, char c);
void NV_tokenize0(NV_LangDef *langDef, char (*token0)[MAX_TOKEN_LEN], int token0Len, int *token0Used,  const char *s);
int NV_tokenize(NV_Env *env, const char *s);
//
void NV_Evaluate(NV_Env *env);
void NV_EvaluateSentence(NV_Env *env, NV_Term *root);
// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
void *NV_malloc(size_t size);

