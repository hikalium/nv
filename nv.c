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

NV_Term *NV_LANG00_Op_assign(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *left = thisTerm->before;
	NV_Term *right = thisTerm->next;
	// type check
	if(!left || !right) return NULL;
	if(right->type == Unknown) NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &right);
	// process
	if(right->type == Imm32s){
		if(left->type == Unknown) left = NV_overwriteTerm(left, NV_createTerm_Variable(env, left->data));
		//
		if(
			left->type == Variable &&
			right->type == Imm32s
		){
			NV_assignVariable_Integer(left->data, *((int32_t *)right->data));
			NV_removeTerm(thisTerm);
			NV_removeTerm(right);
			env->changeFlag = 1;
			return left;	
		}
	}
	return NULL;
}

NV_Term *NV_LANG00_Op_binaryOperator(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Term *result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	// type check
	if(!before || !next) return NULL;
	if(before->type == Unknown)	NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &before);
	if(next->type == Unknown)	NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &next);
	// process
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
	printf("NV_LANG00_Op_binaryOperator: Bad operand. type %d and %d \n", before->type, next->type);
	return NULL;
}

NV_Term *NV_LANG00_Op_nothingButDisappear(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	return before;
}

NV_Term *NV_LANG00_Op_sentenceSeparator(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *b = thisTerm->before, *originalTree;
	NV_Term *sentenceTerm = NV_createTerm_Sentence();
	NV_Term *sentenceRoot, remRoot;

	sentenceRoot = sentenceTerm->data;

	for(b = thisTerm;  (b->before->type != Root && b->before->type != Sentence && !(b->before->type == Operator && b->before->data == thisTerm->data)); b = b->before){
		// skip
	}
	// now, b->before is ';' or Sentence or Root.
	if(b->before->type == Operator && b->before->data == thisTerm->data){
		NV_removeTerm(b->before);
	}
	originalTree = b->before;
	NV_divideTerm(sentenceRoot, b);
	NV_divideTerm(&remRoot, thisTerm);
	NV_appendAll(originalTree, &remRoot);
	NV_overwriteTerm(thisTerm, sentenceTerm);
	NV_insertTermAfter(thisTerm->before, NV_createTerm_Operator(env->langDef, "builtin_exec"));
	env->changeFlag = 1;
	return originalTree;
}

NV_Term *NV_LANG00_Op_builtin_exec(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *sentenceTerm, *sentenceRoot, *retv;
	sentenceTerm = thisTerm->next;
	if(sentenceTerm == NULL || sentenceTerm->type != Sentence){
		return NULL;
	}
	sentenceRoot = sentenceTerm->data;

	NV_EvaluateSentence(env, sentenceRoot);

	retv = thisTerm->next;
	NV_removeTerm(thisTerm);
	NV_insertAllTermAfter(retv, sentenceRoot);
	NV_removeTerm(sentenceTerm);
	env->changeFlag = 1;
	return retv;
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
	//
	t->opRoot = NULL;

	return t;
	
}

NV_LangDef *NV_getDefaultLang()
{
	NV_LangDef *lang = NV_allocLangDef();
	char *char0 = " \n";
	char *char1 = "+=*/;";
	//
	lang->char0Len = strlen(char0);
	lang->char0List = char0;
	lang->char1Len = strlen(char1);
	lang->char1List = char1;
	//
	NV_addOperator(lang, 7010,	";", NV_LANG00_Op_sentenceSeparator);
	NV_addOperator(lang, 7000,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	NV_addOperator(lang, 1024,	" ", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 1024,	"\n", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10,	"=", NV_LANG00_Op_assign);
	NV_addOperator(lang, 50,	"+", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 50,	"-", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 100,	"*", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 100,	"/", NV_LANG00_Op_binaryOperator);
	return lang;
}

//
// Varibale
//
NV_Variable *NV_allocVariable(NV_Env *env)
{
	NV_Variable *t;
	//
	if(env->varUsed >= MAX_VARS){
		fputs("No more variables.", stderr);
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
	//printf("NV_getVariableByName: Variable '%s' not found.\n", name);
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
	if(strchr(lang->char0List, c)){
		return 0;
	}
	if(strchr(lang->char1List, c)){
		return 1;
	}
	return 2;
}

void NV_tokenize0(NV_LangDef *langDef, char (*token0)[MAX_TOKEN_LEN], int token0Len, int *token0Used,  const char *s)
{
	const char *p;
	int i, lastCType, cType;
	lastCType = NV_getCharType(langDef, s[0]);
	p = s;
	*token0Used = 0;
	for(i = 0; ; i++){
		cType = NV_getCharType(langDef, s[i]);
		if(cType != lastCType){
			// division between tokens
			if((p - s + i) > MAX_TOKEN_LEN){
				fputs("Too long token.\n", stderr);
				exit(EXIT_FAILURE);
			}
			//
			NV_strncpy(token0[*token0Used], p, MAX_TOKEN_LEN, s + i - p);
			printf("[%s]", token0[*token0Used]);
			(*token0Used)++;
			//
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
	char token0[MAX_TOKENS][MAX_TOKEN_LEN];
	int token0Len = 0;
	//
	NV_tokenize0(env->langDef, token0, MAX_TOKENS, &token0Len, s);
	for(i = 0; i < token0Len; i++){
		NV_appendTerm(env->langDef, &env->termRoot, token0[i]);
	}
	NV_printTerms(&env->termRoot);
	//
	return 0;
}

//
// Evaluate
//
void NV_Evaluate(NV_Env *env)
{
	NV_Term *t; 
	//
	NV_EvaluateSentence(env, &env->termRoot);
	//
	t = env->termRoot.next;
	if(t && t->next == NULL && t->type == Imm32s){
		printf("= %d\n", *(int *)t->data);
	}
	fputs("OK.\n", stdout);
	NV_removeTermTree(&env->termRoot);
	NV_printVarsInVarList(env->varList, env->varUsed);
}

void NV_EvaluateSentence(NV_Env *env, NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op;
	int maxPrecedence;

	if(!root || !root->next) return;

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
						break;
					}
					NV_printTerms(root);
				}
			}
		}
	}
	//
	// print variables.
	//NV_printVarsInTerms(&env->termRoot);
}

