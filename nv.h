#define MAX_LINE_LEN	1024
#define MAX_OPERATOR_NAME_LEN	16
#define MAX_TOKEN_LEN	64
#define MAX_TOKENS		128

typedef enum	NV_TERM_TYPE	NV_TermType;
typedef struct	NV_TERM 		NV_Term;
typedef struct	NV_OPERATOR 	NV_Operator;
typedef struct	NV_ENV			NV_Env;

enum NV_TERM_TYPE {
	Root,
	Unknown,
	Variable,
	Operator,
	Imm32s,
};

struct NV_TERM {
	NV_TermType type;
	void *data;
	NV_Term *before, *next;
};

struct NV_OPERATOR {
	char name[MAX_OPERATOR_NAME_LEN];
	int precedence;
	NV_Operator *next;
	NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm);
};

struct NV_ENV {
	// interpreter params
	int char0Len;
	const char *char0List;	// should be terminated with 0
	int char1Len;
	const char *char1List;	// should be terminated with 0
	NV_Operator *opRoot;
	// parser env
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len;
	// interpreter env
	NV_Term termRoot;
	int changeFlag;
};

NV_Term *NV_LANG00_Op_binaryOperator(NV_Env *env, NV_Term *thisTerm);
NV_Term *NV_LANG00_Op_nothingButDisappear(NV_Env *env, NV_Term *thisTerm);
void NV_setDefaultEnv(NV_Env *env);
//
NV_Term *NV_allocTerm();
void NV_initRootTerm(NV_Term *t);
void NV_appendTermRaw(NV_Env *env, NV_Term *new);
void NV_removeTerm(NV_Term *t);
NV_Term *NV_createTerm_Imm32(int imm32);
void NV_appendTerm(NV_Env *env, const char *termStr);
void NV_printTerms(NV_Term *root);
//
NV_Operator *NV_allocOperator();
void NV_addOperator(NV_Env *env, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm));
NV_Operator *NV_isOperator(NV_Env *env, const char *termStr);
//
NV_Env *NV_allocEnv();
//
int NV_getCharType(NV_Env *env, char c);
void NV_tokenize0(NV_Env *env, const char *s);
int NV_tokenize(NV_Env *env, const char *s);
//
void NV_Evaluate(NV_Env *env);

