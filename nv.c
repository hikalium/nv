#include "nv.h"

int NV_isDebugMode;
int main(int argc, char *argv[])
{
	int i;
	char line[MAX_LINE_LEN];
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0) NV_isDebugMode = 1;
	}

	NV_Env *env = NV_allocEnv();
	env->langDef = NV_getDefaultLang();
	
	while(fgets(line, sizeof(line), stdin) != NULL){
		NV_tokenize(env, line);
		NV_Evaluate(env);
	}

	return 0;
}

//
// LangDef
//
NV_LangDef *NV_allocLangDef()
{
	NV_LangDef *t;

	t = NV_malloc(sizeof(NV_LangDef));
	//
	t->char0Len = 0;
	t->char0List = "";
	t->char1Len = 0;
	t->char1List = "";
	t->char2Len = 0;
	t->char2List = "";
	//
	t->opRoot = NULL;

	return t;
	
}

//
// Varibale
//
NV_Variable *NV_allocVariable(NV_Env *env)
{
	NV_Variable *t;
	//
	if(env->varUsed >= MAX_VARS){
		NV_printError("No more variables.", stderr);
		exit(EXIT_FAILURE);
	}
	t = &env->varList[env->varUsed++];
	//
	t->name[0] = 0;
	t->type = None;
	t->byteSize = 0;
	t->revision = 0;
	t->data = NULL;
	//
	return t;
}

void NV_resetVariable(NV_Variable *v)
{
	// excludes namestr.
	if(v->type == None) return;
	v->type = None;
	v->byteSize = 0;
	if(v->data) free(v->data);
	v->data = NULL;
}

void NV_assignVariable_Integer(NV_Variable *v, int32_t newVal)
{
	NV_resetVariable(v);
	v->type = Integer;
	v->byteSize = sizeof(int32_t);	// int32s only now.
	v->data = NV_malloc(v->byteSize);
	v->revision++;
	*((int32_t *)v->data) = newVal;
}

void NV_tryConvertTermFromVariableToImm(NV_Variable *varList, int varUsed, NV_Term **term)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(varList, varUsed, (*term)->data);
	if(var){
		if(var->type == Integer && var->byteSize == sizeof(int32_t)){
			new = NV_createTerm_Imm32(*((int32_t *)var->data));
			NV_overwriteTerm((*term), new);
			*term = new;
			return;
		}
	}
}

NV_Variable *NV_getVariableByName(NV_Variable *varList, int varUsed, const char *name)
{
	NV_Variable *var;
	int i;
	for(i = 0; i < varUsed; i++){
		var = &varList[i];
		if(strncmp(var->name, name, MAX_TOKEN_LEN) == 0){
			return var;
		}
	}
	if(NV_isDebugMode) printf("NV_getVariableByName: Variable '%s' not found.\n", name);
	return NULL;
}

void NV_printVarsInVarList(NV_Variable *varList, int varUsed)
{
	NV_Variable *var;
	int32_t *tmpint32;
	int i;

	printf("Variable Table (%p): %d\n", varList, varUsed);
	for(i = 0; i < varUsed; i++){
		var = &varList[i];
		printf("%s", var->name);
		printf("\t rev: %d", var->revision);
		if(var->type == Integer){
			printf("\t Integer(%d)", var->byteSize);
			if(var->byteSize == sizeof(int32_t)){
				tmpint32 = var->data;
				printf("\t = %d", *tmpint32);
			}
		}
		putchar('\n');
	}
}

//
// Operator
//

NV_Operator *NV_allocOperator()
{
	NV_Operator *t;

	t = NV_malloc(sizeof(NV_Operator));
	//
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
	strlcpy(t->name, name, sizeof(t->name));
	t->name[sizeof(t->name) - 1] = 0;
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

	t = NV_malloc(sizeof(NV_Env));
	//
	t->langDef = NULL;
	NV_initRootTerm(&t->termRoot);
	t->varUsed = 0;

	return t;
}

//
// Tokenize
//

int NV_getCharType(NV_LangDef *lang, char c)
{
	if(c == '\0'){
		return -1;
	} else if(strchr(lang->char0List, c)){
		// type 0 chars divide tokens but can't be a part of a token.
		return 0;
	} else if(strchr(lang->char1List, c)){
		// sequences of type 1 chars make tokens.
		return 1;
	} else if(strchr(lang->char2List, c)){
		// type 2 chars make token separately.
		return 2;
	}
	// sequences of type 2 chars make tokens.
	return 3;
}

void NV_tokenize0(NV_LangDef *langDef, char (*token0)[MAX_TOKEN_LEN], int token0Len, int *token0Used,  const char *s)
{
	const char *p;
	int i, lastCType, cType;
	lastCType = 0;
	p = s;
	*token0Used = 0;
	for(i = 0; ; i++){
		cType = NV_getCharType(langDef, s[i]);
		if(cType != lastCType || cType == 2 || lastCType == 2){
			if(lastCType != 0){
				if((s + i - p) > MAX_TOKEN_LEN){
					NV_printError("Too long token.\n", stderr);
					exit(EXIT_FAILURE);
				}
				NV_strncpy(token0[*token0Used], p, MAX_TOKEN_LEN, s + i - p);
				(*token0Used)++;
			}
			p = s + i;
		}
		lastCType = cType;
		if(!s[i]) break;
	}
}

int NV_tokenize(NV_Env *env, const char *s)
{
	int i;
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len = 0;
	//
	NV_tokenize0(env->langDef, token0, MAX_TOKENS, &token0Len, s);
	for(i = 0; i < token0Len; i++){
		NV_appendTerm(env->langDef, &env->termRoot, token0[i]);
	}
	if(NV_isDebugMode) NV_printTerms(&env->termRoot);
	//
	return 0;
}

//
// Evaluate
//
void NV_Evaluate(NV_Env *env)
{
	NV_EvaluateSentence(env, &env->termRoot);
	NV_removeTermTree(&env->termRoot);
	if(NV_isDebugMode) NV_printVarsInVarList(env->varList, env->varUsed);
}

int NV_EvaluateSentence(NV_Env *env, NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op;
	int maxPrecedence;

	if(!root || !root->next) return 1;

	env->changeFlag = 1;
	while(env->changeFlag){
		env->changeFlag = 0;
		maxPrecedence = 0;
		for(t = root->next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				if(op->precedence > maxPrecedence){
					maxPrecedence = op->precedence;
				}
			}
		}
		for(t = root->next; t; t = t->next){
			if(t->type == Operator){
				op = (NV_Operator *)t->data;
				if(op->precedence == maxPrecedence){
					t = op->nativeFunc(env, t);
					if(!t){
						NV_printError("Operator mismatched: %s\n", op->name);
						return 1;
					}
					if(NV_isDebugMode) NV_printTerms(root);
				}
			}
		}
	}
	return 0;
}

void NV_printError(const char *format, ...)
{
	va_list args;
	printf("Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

