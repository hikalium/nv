#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nv.h"

int main(int argc, char *argv[])
{
	char line[MAX_LINE_LEN];
	NV_Env *env = NV_allocEnv();
	env->langDef = NV_getDefaultLang();
	
	
	while(fgets(line, sizeof(line), stdin) != NULL){
		fprintf(stderr, "> %s", line);
		if(NV_tokenize(env, line)){
			fputs("Bad syntax.\n", stderr);
		} else{
			NV_Evaluate(env);
		}
	}

	return 0;
}

//
// Language set
//

NV_Term *NV_LANG00_Op_binaryOperator(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Term *result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	if(!before || !next){
		return NULL;
	}
	if(before->type == Imm32s && next->type == Imm32s){
		int resultVal;
		if(strcmp("+", op->name) == 0){
			resultVal = *((int *)before->data) + *((int *)next->data);
		} else if(strcmp("-", op->name) == 0){
			resultVal = *((int *)before->data) - *((int *)next->data);
		} else if(strcmp("*", op->name) == 0){
			resultVal = *((int *)before->data) * *((int *)next->data);
		} else if(strcmp("/", op->name) == 0){
			resultVal = *((int *)before->data) / *((int *)next->data);
		} else{
			return NULL;
		}
		result = NV_createTerm_Imm32(resultVal);
		result->before = before->before;
		result->next = next->next;
		//
		NV_removeTerm(thisTerm);		
		NV_removeTerm(before);
		NV_removeTerm(next);
		//
		if(result->before)	result->before->next = result;
		if(result->next)	result->next->before = result;
		//
		env->changeFlag = 1;
		return result;	
	}

	return NULL;
}

NV_Term *NV_LANG00_Op_nothingButDisappear(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	return before;
}

//
// LangDef
//

NV_LangDef *NV_allocLangDef()
{
	NV_LangDef *t;

	t = malloc(sizeof(NV_LangDef));
	if(!t){
		fputs("malloc error", stderr);
		exit(EXIT_FAILURE);
	}

	t->char0Len = 0;
	t->char0List = "";
	t->char1Len = 0;
	t->char1List = "";
	//
	t->opRoot = NULL;

	return t;
	
}

NV_LangDef *NV_getDefaultLang()
{
	NV_LangDef *lang = NV_allocLangDef();
	char *char0 = " ;\n";
	char *char1 = "+=*/";
	lang->char0Len = strlen(char0);
	lang->char0List = char0;
	lang->char1Len = strlen(char1);
	lang->char1List = char1;

	NV_addOperator(lang, 1024,	" ", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 1024,	"\n", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 50,		"+", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 50,		"-", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 100,	"*", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 100,	"/", NV_LANG00_Op_binaryOperator);
	return lang;
}

//
// Term
//

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

void NV_appendTermRaw(NV_Env *env, NV_Term *new)
{
	NV_Term *t;
	for(t = &env->termRoot; t->next; t = t->next);
	new->next = NULL;
	new->before = t;
	t->next = new;
	return;
}

void NV_removeTerm(NV_Term *t)
{
	// don't apply to the root Term.
	if(t->before)	t->before->next = t->next;
	if(t->next)		t->next->before = t->before;
	free(t);
}

void NV_removeTermTree(NV_Term *root)
{
	while(root->next != NULL){
		NV_removeTerm(root->next);
	}
}

NV_Term *NV_createTerm_Imm32(int imm32)
{
	NV_Term *new;
	//
	new = NV_allocTerm();
	new->type = Imm32s;
	new->data = malloc(sizeof(int));
	*((int *)new->data) = imm32;
	return new;
}

void NV_appendTerm(NV_Env *env, const char *termStr)
{
	// retv: isValid
	NV_Term *new;
	NV_Operator *op;
	int tmpNum;
	char *p;
	//
	op = NV_isOperator(env->langDef, termStr);
	if(op){
		new = NV_allocTerm();
		new->type = Operator;
		new->data = op;
		NV_appendTermRaw(env, new);
		return;
	}
	tmpNum = strtol(termStr, &p, 0);
	if(termStr != p && *p == 0){
		new = NV_createTerm_Imm32(tmpNum);
		NV_appendTermRaw(env, new);
		return;
	}
	//
	new = NV_allocTerm();
	new->type = Unknown;
	new->data = (void *)termStr;
	NV_appendTermRaw(env, new);
	return;
}

void NV_printTerms(NV_Term *root)
{
	NV_Term *t;
	for(t = root; ; t = t->next){
		printf("[%d]", t->type);
		if(!t->next) break;
	};
	putchar('\n');
	
}

//
// Operator
//

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

void NV_addOperator(NV_LangDef *lang, int precedence, const char *name, NV_Term *(*nativeFunc)(NV_Env *env, NV_Term *thisTerm))
{
	NV_Operator *t;

	t = NV_allocOperator();
	strncpy(t->name, name, sizeof(t->name));
	t->precedence = precedence;
	t->next = lang->opRoot;
	lang->opRoot = t;
	t->nativeFunc = nativeFunc;
}

NV_Operator *NV_isOperator(NV_LangDef *lang, const char *termStr)
{
	NV_Operator *op;
	for(op = lang->opRoot; op != NULL; op = op->next){
		if(strcmp(op->name, termStr) == 0){
			return op;
		}
	}
	return NULL;
}

//
// Environment
//

NV_Env *NV_allocEnv()
{
	NV_Env *t;

	t = malloc(sizeof(NV_Env));
	if(!t){
		fputs("malloc error", stderr);
		exit(EXIT_FAILURE);
	}

	//
	t->langDef = NULL;
	t->token0Len = 0;
	NV_initRootTerm(&t->termRoot);

	return t;
}

//
// Tokenize
//

int NV_getCharType(NV_LangDef *lang, char c)
{
	if(strchr(lang->char0List, c)){
		return 0;
	}
	if(strchr(lang->char1List, c)){
		return 1;
	}
	return 2;
}

void NV_tokenize0(NV_Env *env, const char *s)
{
	const char *p;
	int i, lastCType, cType;
	lastCType = NV_getCharType(env->langDef, s[0]);
	p = s;
	env->token0Len = 0;
	for(i = 0; ; i++){
		cType = NV_getCharType(env->langDef, s[i]);
		if(cType != lastCType){
			// division between tokens
			if((p - s + i) > MAX_TOKEN_LEN){
				fputs("Too long token.\n", stderr);
				exit(EXIT_FAILURE);
			}
			strncpy(env->token0[env->token0Len++], p, s + i - p);
			printf("[%s]", env->token0[env->token0Len - 1]);
			p = s + i;
			lastCType = cType;
		}
		if(!s[i]) break;
	}
	putchar('\n');
}

int NV_tokenize(NV_Env *env, const char *s)
{
	int i;
	NV_tokenize0(env, s);
	for(i = 0; i < env->token0Len; i++){
		NV_appendTerm(env, env->token0[i]);
	}
	NV_printTerms(&env->termRoot);

	return 0;
}

//
// Evaluate
//

void NV_Evaluate(NV_Env *env)
{
	NV_Term *t;
	NV_Operator *op;
	int maxPrecedence;

	env->changeFlag = 1;
	while(env->changeFlag){
		env->changeFlag = 0;
		t = env->termRoot.next;
		maxPrecedence = 0;
		for(t = env->termRoot.next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				if(op->precedence > maxPrecedence){
					maxPrecedence = op->precedence;
				}
			}
		}
		for(t = env->termRoot.next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				if(op->precedence == maxPrecedence){
					t = op->nativeFunc(env, t);
					if(!t){
						break;
					}
					NV_printTerms(&env->termRoot);
				}
			}
		}
	}
	t = env->termRoot.next;
	if(t->next == NULL && t->type == Imm32s){
		printf("= %d\n", *(int *)t->data);
	}
	fputs("OK.\n", stdout);
	NV_removeTermTree(&env->termRoot);
}

