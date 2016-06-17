#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	int (*nativeFunc)(NV_Env *env, NV_Term *thisTerm);
};

struct NV_ENV {
	int char0Len;
	const char *char0List;	// should be terminated with 0
	int char1Len;
	const char *char1List;	// should be terminated with 0
	//
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len;
	NV_Term termRoot;
	NV_Operator *opRoot;
};

void NV_setDefaultEnv(NV_Env *env);
NV_Term *NV_allocTerm();
void NV_initRootTerm(NV_Term *t);
NV_Operator *NV_allocOperator();
NV_Env *NV_allocEnv();
void NV_addOperator(NV_Env *env, int precedence, const char *name, int (*nativeFunc)(NV_Env *env, NV_Term *thisTerm));
int NV_tokenize(NV_Env *env, const char *s);

int main(int argc, char *argv[])
{
	char line[MAX_LINE_LEN];
	NV_Env *env = NV_allocEnv();
	NV_setDefaultEnv(env);
	
	
	while(fgets(line, sizeof(line), stdin) != NULL){
		fprintf(stderr, "> %s", line);
		if(NV_tokenize(env, line)){
			fputs("Bad syntax.\n", stderr);
		} else{
			fputs("OK.\n", stdout);
		}
	}

	return 0;
}

void NV_setDefaultEnv(NV_Env *env)
{
	char *char0 = " ;\n";
	char *char1 = "+=*/";
	env->char0Len = strlen(char0);
	env->char0List = char0;
	env->char1Len = strlen(char1);
	env->char1List = char1;

	
}

int NV_LANG00_Op_add(NV_Env *env, NV_Term *thisTerm)
{
	return 0;
}

NV_Term *NV_allocTerm()
{
	NV_Term *t;

	t = malloc(sizeof(NV_Term));
	if(!t){
		fputs("malloc error", stderr);
		exit(EXIT_FAILURE);
	}

	t->type = Unknown;
	t->data = NULL;
	t->before = NULL;
	t->next = NULL;

	return t;
}

void NV_initRootTerm(NV_Term *t)
{
	t->type = Root;
	t->data = NULL;
	t->before = NULL;	
	t->next = NULL;
}

NV_Operator *NV_allocOperator()
{
	NV_Operator *t;

	t = malloc(sizeof(NV_Operator));
	if(!t){
		fputs("malloc error", stderr);
		exit(EXIT_FAILURE);
	}

	t->name[0] = 0;
	t->precedence = 0;
	t->next = NULL;
	t->nativeFunc = NULL;

	return t;
}

NV_Env *NV_allocEnv()
{
	NV_Env *t;

	t = malloc(sizeof(NV_Env));
	if(!t){
		fputs("malloc error", stderr);
		exit(EXIT_FAILURE);
	}

	t->char0Len = 0;
	t->char0List = "";
	t->char1Len = 0;
	t->char1List = "";
	//
	t->token0Len = 0;
	NV_initRootTerm(&t->termRoot);
	t->opRoot = NULL;

	return t;
}

void NV_addOperator(NV_Env *env, int precedence, const char *name, int (*nativeFunc)(NV_Env *env, NV_Term *thisTerm))
{
	NV_Operator *t;

	t = NV_allocOperator();
	strncpy(t->name, name, sizeof(t->name));
	t->precedence = precedence;
	t->next = env->opRoot;
	env->opRoot = t;
	t->nativeFunc = nativeFunc;
}

NV_Operator *NV_isOperator(NV_Env *env, const char *termStr)
{
	NV_Operator *op;
	for(op = env->opRoot; op != NULL; op = op->next){
		if(strcmp(op->name, termStr) == 0){
			return op;
		}
	}
	return NULL;
}

void NV_appendTermRaw(NV_Env *env, NV_Term *new)
{
	NV_Term *t;
	for(t = &env->termRoot; t->next; t = t->next);
	new->next = NULL;
	new->before = t;
	t->next = new;
	return;
}

int NV_appendTerm(NV_Env *env, const char *termStr)
{
	// retv: isValid
	NV_Term *new;
	NV_Operator *op;
	int tmpNum;
	char *p;
	//
	new = NV_allocTerm();
	//
	op = NV_isOperator(env, termStr);
	if(op){
		new->type = Operator;
		new->data = op;
		return 1;
	}
	tmpNum = strtol(termStr, &p, 0);
	//if(p != termStr && )
	return 0;
}

int NV_getCharType(NV_Env *env, char c)
{
	if(strchr(env->char0List, c)){
		return 0;
	}
	if(strchr(env->char1List, c)){
		return 1;
	}
	return 2;
}

void NV_tokenize0(NV_Env *env, const char *s)
{
	const char *p;
	int i, lastCType, cType;
	lastCType = NV_getCharType(env, s[0]);
	p = s;
	env->token0Len = 0;
	for(i = 0; ; i++){
		cType = NV_getCharType(env, s[i]);
		if(cType != lastCType){
			// division between tokens
			if((p - s + i) > MAX_TOKEN_LEN){
				fputs("Too long token.\n", stderr);
				exit(EXIT_FAILURE);
			}
			strncpy(&env->token0[env->token0Len++][0], p, s + i - p);
			printf("[%s]", &env->token0[env->token0Len - 1][0]);
			p = s + i;
			lastCType = cType;
		}
		if(!s[i]) break;
	}
	putchar('\n');
}

int NV_tokenize(NV_Env *env, const char *s)
{
	NV_tokenize0(env, s);
	return 0;
}
