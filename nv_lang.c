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
	// eval next sentence of thisTerm
	// and replace [thisTerm, nextSentence] with return tree of nextSentence.
	// retv is last term of return tree.
	NV_Term *sentenceTerm, *sentenceRoot, *lastTerm;
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
	lastTerm = NV_getLastTerm(sentenceRoot);
	lastTerm = NV_getLastTerm(sentenceRoot);
	NV_insertAllTermAfter(thisTerm, sentenceRoot);
	NV_removeTerm(sentenceTerm);
	return lastTerm;
}

//
// Native Functions
//
NV_Term *NV_LANG00_Op_assign(NV_Env *env, NV_Term *thisTerm)
{
	int32_t tmp;
	NV_Term *left = thisTerm->before;
	NV_Term *right = thisTerm->next;
	// type check
	if(!left || !right) return NULL;
	if(right->type == Unknown) NV_tryConvertTermFromUnknownToVariable(env->varSet, &right, 0);
	if(left->type != Variable) NV_tryConvertTermFromUnknownToVariable(env->varSet, &left, 1);
	if(left->type != Variable) return NULL;
	// process
	if(NV_canTermReadAsInt(right)){
		tmp = NV_getValueOfTermAsInt(right);
		NV_assignVariable_Integer(left->data, tmp);
	} else if(right->type == String){
		NV_assignVariable_String(left->data, right->data);
	} else if(right->type == Variable){
		NV_assignVariable_Variable(left->data, right->data);
	} else if(right->type == Sentence){
		NV_assignVariable_Structure(left->data, right->data);
	} else{
		return NULL;
	}
	NV_removeTerm(thisTerm);
	NV_removeTerm(right);
	env->changeFlag = 1;
	return left;
}

NV_Term *NV_LANG00_Op_compoundAssign(NV_Env *env, NV_Term *thisTerm)
{
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	NV_Term *before = thisTerm->before;
	char s[2];
	//
	if(!before || (before->type != Unknown && before->type != Variable)) return NULL;
	//
	s[0] = op->name[0];
	s[1] = 0;
	//
	NV_insertTermAfter(thisTerm, NV_createTerm_Operator(env->langDef, s));
	NV_insertTermAfter(thisTerm, NV_cloneTerm(before));
	NV_insertTermAfter(thisTerm, NV_createTerm_Operator(env->langDef, "="));
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	return before->next;
}

NV_Term *NV_LANG00_Op_unaryOperator_prefix(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Term *result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	// type check
	if(!next) return NULL;
	if(before && (before->type != Operator && before->type != Root)) return NULL;
	if(next->type == Unknown)	NV_tryConvertTermFromUnknownToImm(env->varSet, &next);
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
			if(NV_isDebugMode) NV_printError("NV_LANG00_Op_unaryOperator: Not implemented %s\n", op->name);
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
	if(NV_isDebugMode) NV_printError("NV_LANG00_Op_unaryOperator: Bad operand. type %d\n", next->type);
	return NULL;
}

NV_Term *NV_LANG00_Op_unaryOperator_suffix_variableOnly(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *before = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	NV_Term *result;
	NV_Variable *var;
	// type check
	if(!before) return NULL;
	if(next && (next->type != Operator && next->type != Root)) return NULL;
	if(before->type == Unknown)	NV_tryConvertTermFromUnknownToVariable(env->varSet, &before, 1);
	// process
	if(before->type == Variable){
		var = before->data;
		if(var->type == VInteger){
			if(var->byteSize == sizeof(int32_t)){
				result = NV_createTerm_Imm32(*((int32_t *)var->data));
				if(strcmp("++", op->name) == 0){
					(*((int32_t *)var->data))++;
				} else if(strcmp("--", op->name) == 0){
					(*((int32_t *)var->data))--;
				} else{
					return NULL;
				}
			} else{
				return NULL;
			}
		} else{
			return NULL;
		}
		//
		NV_removeTerm(thisTerm);
		NV_overwriteTerm(before, result);
		//
		env->changeFlag = 1;
		return before;
	}
	if(NV_isDebugMode) NV_printError("NV_LANG00_Op_unaryOperator_suffix_variableOnly: Bad operand. type %d\n", before->type);
	return NULL;
}

NV_Term *NV_LANG00_Op_binaryOperator(NV_Env *env, NV_Term *thisTerm)
{
	// for Integer values only.
	NV_Term *prev = thisTerm->before;
	NV_Term *next = thisTerm->next;
	NV_Term *result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	int vL, vR;
	int resultVal;
	// type check
	if(!prev || !next) return NULL;
	if(!NV_canTermReadAsInt(prev) || !NV_canTermReadAsInt(next)) return NULL;
	vL = NV_getValueOfTermAsInt(prev);
	vR = NV_getValueOfTermAsInt(next);
	// process
	if(strcmp("+", op->name) == 0){
		resultVal = vL + vR;
	} else if(strcmp("-", op->name) == 0){
		resultVal = vL - vR;
	} else if(strcmp("*", op->name) == 0){
		resultVal = vL * vR;
	} else if(strcmp("/", op->name) == 0){
		resultVal = vL / vR;
	} else if(strcmp("%", op->name) == 0){
		resultVal = vL % vR;
	} else if(strcmp("||", op->name) == 0){
		resultVal = vL || vR;
	} else if(strcmp("&&", op->name) == 0){
		resultVal = vL && vR;
	}
	// comparison operators
	else if(strcmp("<", op->name) == 0){
		resultVal = vL < vR;
	} else if(strcmp(">", op->name) == 0){
		resultVal = vL > vR;
	} else if(strcmp("<=", op->name) == 0){
		resultVal = vL <= vR;
	} else if(strcmp(">=", op->name) == 0){
		resultVal = vL >= vR;
	} else if(strcmp("==", op->name) == 0){
		resultVal = vL == vR;
	} else if(strcmp("!=", op->name) == 0){
		resultVal = vL != vR;
	} else{
		return NULL;
	}
	result = NV_createTerm_Imm32(resultVal);
	//
	NV_removeTerm(prev);
	NV_removeTerm(next);
	NV_overwriteTerm(thisTerm, result);
	//
	env->changeFlag = 1;
	return result;	
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

NV_Term *NV_LANG00_Op_stringLiteral(NV_Env *env, NV_Term *thisTerm)
{
	// "string literal"
	int len = 0, sp;
	NV_Term *t, *endTerm;
	NV_Operator *op;
	char *s, *st;
	for(t = thisTerm->next;;t = t->next){
		if(!t) return NULL;
		if(t->type == Operator && ((NV_Operator *)t->data)->nativeFunc == NV_LANG00_Op_stringLiteral){
			endTerm = t;
			break;
		}
		switch(t->type){
			case Operator:
				op = t->data;
				len += strlen(op->name);
				break;
			case Unknown:
				len += strlen((const char *)t->data);
				break;
			default:
				return NULL;
		}
	}
	s = NV_malloc(len + 1);
	sp = 0;
	for(t = thisTerm->next; t != endTerm;){
		switch(t->type){
			case Operator:
				op = t->data;
				st = op->name;
				break;
			case Unknown:
				st = t->data;
				break;
			default:
				return NULL;
		}
		strcpy(&s[sp], st);
		sp += strlen(st);
		t = t->next;
		NV_removeTerm(t->before);
	}
	s[len] = 0;
	NV_removeTerm(endTerm);
	t = NV_createTerm_String(s);
	NV_free(s);
	NV_overwriteTerm(thisTerm, t);
	env->changeFlag = 1;
	return t;
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

NV_Term *NV_LANG00_Op_structureAccessor(NV_Env *env, NV_Term *thisTerm)
{
	// []
	NV_Term *structTerm, *indexTerm, *t, *v;
	int index;
	char s[32];
	//
	t = thisTerm->before;
	if(!t) return NULL;
	if(t->type != Variable) NV_tryConvertTermFromUnknownToVariable(env->varSet, &t, 1);
	if(t->type != Variable) return NULL;
	structTerm = t;
	//
	t = thisTerm->next;
	if(!t) return NULL;
	if(t->type != Imm32s) return NULL;
	if(!t->next || t->next->type != Operator || strcmp("]", ((NV_Operator *)t->next->data)->name) != 0) return NULL;
	indexTerm = t;
	//
	if(!NV_canTermReadAsInt(indexTerm)) return NULL;
	index = NV_getValueOfTermAsInt(indexTerm);
	t = NV_getItemFromStructureByIndex(structTerm->data, index);
	if(!t) return NULL;
	NV_removeTerm(indexTerm->next);
	NV_removeTerm(indexTerm->before);
	NV_removeTerm(indexTerm);
	snprintf(s, sizeof(s) - 1, "%d", rand());
	v = NV_createTerm_Variable(env->varSet, s);
	NV_assignVariable_StructureItem(v->data, t);
	NV_overwriteTerm(structTerm, v);
	//
	env->changeFlag = 1;
	return v;
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
	// if {cond} {do} [{cond} {do}] [{else}]
	int cond;
	NV_Term *condTerm, *doTerm, *t, *next;
	//
	t = thisTerm;
	//
	t = t->next; if(t == NULL || t->type != Sentence) return NULL;
	condTerm = t;
	t = t->next; if(t == NULL || t->type != Sentence) return NULL;
	doTerm = t;
	for(;;){
		// evaluate cond
		if(!NV_LANG00_execNextSentence(env, thisTerm)) return NULL;
		condTerm = thisTerm->next;
		if(condTerm->next != doTerm) return NULL;
		if(condTerm->type == Variable) NV_tryConvertTermFromUnknownToImm(env->varSet, &condTerm);
		if(condTerm->type == Imm32s){
			cond = *((int32_t *)condTerm->data);
		} else{
			return NULL;
		}
		NV_removeTerm(condTerm);
		// evaluate do
		if(cond){
			// cond is true.
			t = NV_LANG00_execNextSentence(env, thisTerm);
			if(!t) return NULL;
			// remove rest of sentence blocks.
			for(t = t->next; t; t = next){
				if(t->type != Sentence) break;
				next = t->next;
				NV_removeTerm(t);
			}
			break;
		} else{
			NV_removeTerm(doTerm);
		}
		// cond is false. check next cond.
		t = thisTerm;
		//
		t = t->next; if(t == NULL || t->type != Sentence) return NULL;
		condTerm = t;
		t = t->next; if(t == NULL || t->type != Sentence){
			// next term is else block.
			t = NV_LANG00_execNextSentence(env, thisTerm);
			if(!t) return NULL;
			break;
		};
		doTerm = t;
		// continue checking.
	}
	// remove 'if' term.
	thisTerm = thisTerm->before;
	NV_removeTerm(thisTerm->next);
	//
	env->changeFlag = 1;
	return thisTerm;
}

NV_Term *NV_LANG00_Op_for(NV_Env *env, NV_Term *thisTerm)
{
	// for {init block}{conditional block}{update block}{statement}
	//int cond;
	NV_Term *t, *initTerm, *condTerm, *updateTerm, *doTerm;
	NV_Term tmpCondRoot, tmpUpdateRoot, tmpDoRoot;
	//
	t = thisTerm;
	//
	t = t->next;
	if(t == NULL || t->type != Sentence) return NULL;
	initTerm = t;
	//
	t = t->next;
	if(t == NULL || t->type != Sentence) return NULL;
	condTerm = t;
	//
	t = t->next;
	if(t == NULL || t->type != Sentence) return NULL;
	updateTerm = t;
	//
	t = t->next;
	if(t == NULL || t->type != Sentence) return NULL;
	doTerm = t;
	//
	// do init block
	// initTerm is removed here.
	if(NV_LANG00_execNextSentence(env, thisTerm) == NULL) return NULL;
	for(;;){
		// copy blocks
		NV_cloneTermTree(&tmpCondRoot, condTerm->data);
		NV_cloneTermTree(&tmpUpdateRoot, updateTerm->data);
		NV_cloneTermTree(&tmpDoRoot, doTerm->data);
		// check cond
		if(NV_EvaluateSentence(env, &tmpCondRoot)) return NULL;
		t = NV_getLastTerm(&tmpCondRoot);
		if(!NV_getValueOfTermAsInt(t)) break;
		// do
		if(NV_EvaluateSentence(env, &tmpDoRoot)) return NULL;
		// update
		if(NV_EvaluateSentence(env, &tmpUpdateRoot)) return NULL;
		// free tmp
		NV_removeTermTree(&tmpCondRoot);
		NV_removeTermTree(&tmpUpdateRoot);
		NV_removeTermTree(&tmpDoRoot);
	}
	// free tmp
	NV_removeTermTree(&tmpCondRoot);
	NV_removeTermTree(&tmpUpdateRoot);
	NV_removeTermTree(&tmpDoRoot);
	// remove
	NV_removeTerm(updateTerm);
	NV_removeTerm(condTerm);
	NV_removeTerm(doTerm);
	//
	t = thisTerm;
	NV_removeTerm(thisTerm);
	//
	env->changeFlag = 1;
	return t;
}

NV_Term *NV_LANG00_Op_print(NV_Env *env, NV_Term *thisTerm)
{
	NV_Term *target = thisTerm->next;
	int32_t *tmpint32;
	//
	if(!target)return NULL;
	if(target->type == Unknown)	NV_tryConvertTermFromUnknownToVariable(env->varSet, &target, 0);
	if(target->type == Variable){
		NV_printVariable(target->data, 0);
		putchar('\n');
	} else if(target->type == Imm32s){
		tmpint32 = target->data;
		printf("%d\n", *tmpint32);
	} else if(target->type == String){
		printf("%s\n", target->data);
	} else{
		return NULL;
	}
	NV_removeTerm(thisTerm);
	env->changeFlag = 1;
	env->autoPrintValue = 0;
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

NV_Term *NV_LANG00_Op_mem(NV_Env *env, NV_Term *thisTerm)
{
	thisTerm = NV_overwriteTerm(thisTerm, NV_createTerm_Imm32(NV_getMallocCount()));
	env->changeFlag = 1;
	return thisTerm;
}

NV_LangDef *NV_getDefaultLang()
{
	NV_LangDef *lang = NV_allocLangDef();
	char *char0 = " \t\r\n";
	char *char1 = "!%&-=^~|+*:.<>/";
	char *char2 = "(){}[],;\"";
	//
	lang->char0Len = strlen(char0);
	lang->char0List = char0;
	lang->char1Len = strlen(char1);
	lang->char1List = char1;
	lang->char2Len = strlen(char2);
	lang->char2List = char2;
	// based on http://www.tutorialspoint.com/cprogramming/c_operators.htm
	//
	NV_addOperator(lang, 200000,	"\"", NV_LANG00_Op_stringLiteral);
	NV_addOperator(lang, 100050,	"{", NV_LANG00_Op_sentenceBlock);
	NV_addOperator(lang, 100040,	";", NV_LANG00_Op_sentenceSeparator);
	NV_addOperator(lang, 100030,	"(", NV_LANG00_Op_precedentBlock);
	NV_addOperator(lang, 100020,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	//
	NV_addOperator(lang, 100000,	"mem", NV_LANG00_Op_mem);
	//
	NV_addOperator(lang, 10000,	" ", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"\t", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"\r", NV_LANG00_Op_nothingButDisappear);	
	NV_addOperator(lang, 10000,	"\n", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	";;", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"else", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"elseif", NV_LANG00_Op_nothingButDisappear);
	//
	NV_addOperator(lang, 2010,	"[", NV_LANG00_Op_structureAccessor);	
	NV_addOperator(lang, 2000,	"]", NV_LANG00_Op_nothingButDisappear);
	//
	NV_addOperator(lang, 1000,  "if", NV_LANG00_Op_if);
	NV_addOperator(lang, 1000,  "for", NV_LANG00_Op_for);
	//
	NV_addOperator(lang, 702,	"++", NV_LANG00_Op_unaryOperator_suffix_variableOnly);
	NV_addOperator(lang, 702,	"--", NV_LANG00_Op_unaryOperator_suffix_variableOnly);
	//
	NV_addOperator(lang, 701,	"+", NV_LANG00_Op_unaryOperator_prefix);
	NV_addOperator(lang, 701,	"-", NV_LANG00_Op_unaryOperator_prefix);
	NV_addOperator(lang, 701,	"!", NV_LANG00_Op_unaryOperator_prefix);
	//
	NV_addOperator(lang, 600,	"*", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 600,	"/", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 600,	"%", NV_LANG00_Op_binaryOperator);
	//
	NV_addOperator(lang, 500,	"+", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 500,	"-", NV_LANG00_Op_binaryOperator);
	//
	NV_addOperator(lang, 400,	"<", NV_LANG00_Op_binaryOperator);	
	NV_addOperator(lang, 400,	">", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"<=", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	">=", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"==", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 400,	"!=", NV_LANG00_Op_binaryOperator);
	//
	NV_addOperator(lang, 300,	"||", NV_LANG00_Op_binaryOperator);
	NV_addOperator(lang, 300,	"&&", NV_LANG00_Op_binaryOperator);
	//
	NV_addOperator(lang, 200,	"+=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"-=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"*=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"/=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"%=", NV_LANG00_Op_compoundAssign);
	//
	NV_addOperator(lang, 101,	"=", NV_LANG00_Op_assign);
	//
	NV_addOperator(lang, 10,	"print", NV_LANG00_Op_print);
	NV_addOperator(lang, 10,	"showop", NV_LANG00_Op_showOpList);
	
	return lang;
}

