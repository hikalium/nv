#include "nv.h"

//
// Support functions
//
NV_Term *NV_LANG00_makeBlock(NV_Env *env, NV_Term *thisTerm, const char *closeStr)
{
	// support func
	NV_Term *t, *sentenceTerm, *sentenceRoot, remRoot, *originalTree;
	int pairCount = 1;
	for(t = thisTerm->next; t; t = t->next){
		if(t->type == Unknown && strcmp(closeStr, t->data) == 0){
			pairCount --;
			if(pairCount == 0) break;
		} else if(t->type == Operator && t->data == thisTerm->data){
			pairCount++;
		}
	}
	if(pairCount != 0) return NULL;
	//
	originalTree = thisTerm->before;
	sentenceTerm = NV_createTerm_Sentence();
	sentenceRoot = sentenceTerm->data;
	NV_divideTerm(sentenceRoot, thisTerm->next);
	NV_divideTerm(&remRoot, t);
	NV_removeTerm(t);
	NV_appendAll(originalTree, &remRoot);
	NV_overwriteTerm(thisTerm, sentenceTerm);
	return originalTree;
}

NV_Term *NV_LANG00_execNextSentence(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *sentenceTerm, *sentenceRoot;
	//
	sentenceTerm = thisTerm->next;
	if(sentenceTerm == NULL || sentenceTerm->type != Sentence){
		return NULL;
	}
	sentenceRoot = sentenceTerm->data;
	//
	if(NV_EvaluateSentence(env, sentenceRoot)){
		NV_printError("NV_LANG00_Op_builtin_exec: Exec failed.\n");
		return NULL;
	}
	//
	NV_removeTerm(sentenceTerm);
	NV_insertAllTermAfter(thisTerm, sentenceRoot);
	return thisTerm;
}

//
// Native Functions
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

NV_Term *NV_LANG00_Op_unaryOperator(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Term *result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	// type check
	if(!next) return NULL;
	if(before && (before->type != Operator)) return NULL;
	if(next->type == Unknown)	NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &next);
	// process
	if(next->type == Imm32s){
		int resultVal;
		if(strcmp("+", op->name) == 0){
			resultVal = + *((int *)next->data);
		} else if(strcmp("-", op->name) == 0){
			resultVal = - *((int *)next->data);
		}
		// comparison operators
		else if(strcmp("!", op->name) == 0){
			resultVal = ! *((int *)next->data);
		} else{
			return NULL;
		}
		result = NV_createTerm_Imm32(resultVal);
		//
		NV_removeTerm(next);
		NV_overwriteTerm(thisTerm, result);
		//
		env->changeFlag = 1;
		return result;	
	}
	// NV_printError("NV_LANG00_Op_unaryOperator: Bad operand. type %d\n", next->type);
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
		}
		// comparison operators
		else if(strcmp("<", op->name) == 0){
			resultVal = *((int *)before->data) < *((int *)next->data);
		} else if(strcmp(">", op->name) == 0){
			resultVal = *((int *)before->data) > *((int *)next->data);
		} else if(strcmp("<=", op->name) == 0){
			resultVal = *((int *)before->data) <= *((int *)next->data);
		} else if(strcmp(">=", op->name) == 0){
			resultVal = *((int *)before->data) >= *((int *)next->data);
		} else if(strcmp("==", op->name) == 0){
			resultVal = *((int *)before->data) == *((int *)next->data);
		} else if(strcmp("!=", op->name) == 0){
			resultVal = *((int *)before->data) != *((int *)next->data);
		} else{
			return NULL;
		}
		result = NV_createTerm_Imm32(resultVal);
		//
		NV_removeTerm(before);
		NV_removeTerm(next);
		NV_overwriteTerm(thisTerm, result);
		//
		env->changeFlag = 1;
		return result;	
	}
	NV_printError("NV_LANG00_Op_binaryOperator: Bad operand. type %d and %d \n", before->type, next->type);
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
	NV_Term *b;
	NV_Term *sentenceTerm = NV_createTerm_Sentence();
	NV_Term *sentenceRoot, remRoot;

	sentenceRoot = sentenceTerm->data;

	for(b = thisTerm->before; b; b = b->before){
		if(b->type == Root) break;
		if(b->type == Operator && strcmp(";;", ((NV_Operator *)b->data)->name) == 0){
			b = b->before;
			NV_removeTerm(b->next);
			break;
		}
	}
	NV_divideTerm(sentenceRoot, b->next);
	NV_divideTerm(&remRoot, thisTerm);
	NV_appendAll(b, &remRoot);
	NV_overwriteTerm(thisTerm, sentenceTerm);
	NV_insertTermAfter(b, NV_createTerm_Operator(env->langDef, "builtin_exec"));
	NV_insertTermAfter(b, NV_createTerm_Operator(env->langDef, ";;"));
	env->changeFlag = 1;
	return b;
}

NV_Term *NV_LANG00_Op_sentenceBlock(NV_Env *env, NV_Term *thisTerm)
{
	// {}
	NV_Term *originalTree;
	//
	originalTree = NV_LANG00_makeBlock(env, thisTerm, "}");
	if(originalTree) env->changeFlag = 1;
	return originalTree;
}

NV_Term *NV_LANG00_Op_precedentBlock(NV_Env *env, NV_Term *thisTerm)
{
	// {}
	NV_Term *originalTree;
	//
	originalTree = NV_LANG00_makeBlock(env, thisTerm, ")");
	if(originalTree){
		NV_insertTermAfter(originalTree, NV_createTerm_Operator(env->langDef, "builtin_exec"));
		env->changeFlag = 1;
	}
	return originalTree;
}

NV_Term *NV_LANG00_Op_builtin_exec(NV_Env *env, NV_Term *thisTerm)
{
	//
	if(!NV_LANG00_execNextSentence(env, thisTerm)){
		return NULL;
	}
	thisTerm = thisTerm->before;
	NV_removeTerm(thisTerm->next);
	//
	env->changeFlag = 1;
	return thisTerm;
}

NV_Term *NV_LANG00_Op_if(NV_Env *env, NV_Term *thisTerm)
{
	// if {cond} {do} 
	int cond;
	NV_Term *condTerm, *doTerm;
	//
	condTerm = thisTerm->next;
	if(condTerm == NULL || condTerm->type != Sentence) return NULL;
	doTerm = condTerm->next;
	if(doTerm == NULL || doTerm->type != Sentence) return NULL;
	// evaluate cond
	if(!NV_LANG00_execNextSentence(env, thisTerm)) return NULL;
	condTerm = thisTerm->next;
	if(condTerm->next != doTerm) return NULL;
	if(condTerm->type == Variable) NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &condTerm);
	if(condTerm->type == Imm32s){
		cond = *((int32_t *)condTerm->data);
	} else{
		return NULL;
	}
	NV_removeTerm(condTerm);
	//
	thisTerm = thisTerm->before;
	NV_removeTerm(thisTerm->next);
	// evaluate do
	if(cond){
		if(!NV_LANG00_execNextSentence(env, thisTerm)) return NULL;
	} else{
		NV_removeTerm(doTerm);
	}
	//
	env->changeFlag = 1;
	return thisTerm;
}

NV_Term *NV_LANG00_Op_print(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *target = thisTerm->next;
	NV_Variable *var;
	int32_t *tmpint32;
	//
	if(!target)return NULL;
	if(target->type == Unknown)	NV_tryConvertTermFromVariableToImm(env->varList, env->varUsed, &target);
	if(target->type == Variable){
		var = target->data;
		if(var->type == Integer){
			if(var->byteSize == sizeof(int32_t)){
				tmpint32 = var->data;
				printf("%d\n", *tmpint32);
			} else{
				return NULL;
			}
		} else{
			return NULL;
		}
	} else if(target->type == Imm32s){
		tmpint32 = target->data;
		printf("%d\n", *tmpint32);
	} else{
		return NULL;
	}
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	return target;
}

NV_Term *NV_LANG00_Op_showOpList(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	//
	NV_Operator *p;
	printf("Precedence: [opName]\n");
	for(p = env->langDef->opRoot; p; p = p->next){
		printf("%10d: [%s]\n", p->precedence, p->name);
	}
	//
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	return before;
}

NV_LangDef *NV_getDefaultLang()
{
	NV_LangDef *lang = NV_allocLangDef();
	char *char0 = " \t\r\n";
	char *char1 = "!%&-=^~|+*:.<>/";
	char *char2 = "(){}[],;";
	//
	lang->char0Len = strlen(char0);
	lang->char0List = char0;
	lang->char1Len = strlen(char1);
	lang->char1List = char1;
	lang->char2Len = strlen(char2);
	lang->char2List = char2;
	//
	NV_addOperator(lang, 100040,	"{", NV_LANG00_Op_sentenceBlock);
	NV_addOperator(lang, 100030,	";", NV_LANG00_Op_sentenceSeparator);
	NV_addOperator(lang, 100020,	"(", NV_LANG00_Op_precedentBlock);
	NV_addOperator(lang, 100010,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	NV_addOperator(lang, 10000,	";;", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 1000,  "if", NV_LANG00_Op_if);
	NV_addOperator(lang, 501,	"+", NV_LANG00_Op_unaryOperator);
	NV_addOperator(lang, 501,	"-", NV_LANG00_Op_unaryOperator);
	NV_addOperator(lang, 501,	"!", NV_LANG00_Op_unaryOperator);
	NV_addOperator(lang, 400,	"<", NV_LANG00_Op_binaryOperator);	
	NV_addOperator(lang, 400,	">", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"<=", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	">=", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"==", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"!=", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 300,	"*", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 300,	"/", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 200,	"+", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 200,	"-", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 100,	"=", NV_LANG00_Op_assign);
	NV_addOperator(lang, 10,	"print", NV_LANG00_Op_print);
	NV_addOperator(lang, 10,	"showop", NV_LANG00_Op_showOpList);
	
	return lang;
}

