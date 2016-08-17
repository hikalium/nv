#include "nv.h"

//
// Support functions
//
/*
NV_Pointer NV_LANG00_makeBlock(NV_Pointer env, NV_Pointer thisTerm, const char *closeStr)
{
	// support func
	// retv is prev term of thisTerm.
	NV_Pointer t, *sentenceTerm, *sentenceRoot, remRoot, *originalTree;
	int pairCount = 1;
	t = NV_List_getNextItem(t);
	for(; !NV_E_isNullPointer(t); t = NV_List_getNextItem(t)){
		if(NV_E_isType(t, EString)){}
		if(t->type == Unknown && strcmp(closeStr, t->data) == 0){
			pairCount --;
			if(pairCount == 0) break;
		} else if(t->type == Operator && t->data == thisTerm->data){
			pairCount++;
		}
	}
	if(pairCount != 0) return NULL;
	//
	originalTree = thisTerm->prev;
	sentenceTerm = NV_createTerm_Sentence(NULL);
	sentenceRoot = sentenceTerm->data;
	NV_divideTerm(sentenceRoot, thisTerm->next);
	NV_divideTerm(&remRoot, t);
	NV_removeTerm(t);
	NV_appendAll(originalTree, &remRoot);
	NV_overwriteTerm(thisTerm, sentenceTerm);
	return originalTree;
}

NV_Pointer NV_LANG00_execSentence(NV_Pointer env, NV_Pointer sentenceTerm)
{
	// eval next sentence of thisTerm
	// and replace [thisTerm, nextSentence] with return tree of nextSentence.
	NV_Term sentenceRoot, *retvTerm;
	//
	if(!NV_canReadTermAsSentence(sentenceTerm)){
		return NULL;
	}
	NV_getValueOfTermAsSentence(sentenceTerm, &sentenceRoot);
	//
	if(NV_EvaluateSentence(env, &sentenceRoot)){
		NV_printError("NV_LANG00_Op_builtin_exec: Exec failed.\n");
		return NULL;
	}
	retvTerm = NV_createTerm_Sentence(&sentenceRoot);
	NV_overwriteTerm(sentenceTerm, retvTerm);
	NV_removeTermTree(&sentenceRoot);
	return retvTerm;
}

//
// Native Functions
//
NV_Pointer NV_LANG00_Op_assign(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer left = thisTerm->prev;
	NV_Pointer right = thisTerm->next;
	// type check
	if(!left || !right) return NULL;
	if(right->type == Unknown)
		NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &right, 0);
	if(left->type != Variable)
		NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &left, 1);
	if(left->type != Variable) return NULL;
	// process
	if(!NV_Variable_assignTermValue(left->data, right)){
		return NULL;
	}
	NV_removeTerm(thisTerm);
	NV_removeTerm(right);
	return left;
}

NV_Pointer NV_LANG00_Op_compoundAssign(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	NV_Pointer prev = thisTerm->prev;
	char s[2];
	//
	if(!prev || (prev->type != Unknown && prev->type != Variable)) return NULL;
	//
	s[0] = op->name[0];
	s[1] = 0;
	//
	NV_insertTermAfter(thisTerm, NV_createTerm_Operator(NV_Env_getLangDef(env), s));
	NV_insertTermAfter(thisTerm, NV_cloneTerm(prev));
	NV_insertTermAfter(thisTerm, NV_createTerm_Operator(NV_Env_getLangDef(env), "="));
	NV_removeTerm(thisTerm);
	return prev->next;
}

NV_Pointer NV_LANG00_Op_unaryOperator_prefix(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer prev = thisTerm->prev;
	NV_Pointer next = thisTerm->next;
	NV_Pointer result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	// type check
	if(!next) return NULL;
	if(prev && (prev->type != Operator && prev->type != Root)) return NULL;
	if(next->type == Unknown)
		NV_tryConvertTermFromUnknownToImm(NV_Env_getVarSet(env), &next);
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
		return result;	
	}
	if(NV_isDebugMode) NV_printError("NV_LANG00_Op_unaryOperator: Bad operand. type %d\n", next->type);
	return NULL;
}

NV_Pointer NV_LANG00_Op_unaryOperator_suffix_variableOnly(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer prev = thisTerm->prev;
	NV_Pointer next = thisTerm->next;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	NV_Pointer result;
	NV_Variable *var;
	// type check
	if(!prev) return NULL;
	if(next && (next->type != Operator && next->type != Root)) return NULL;
	if(prev->type == Unknown)
		NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &prev, 1);
	// process
	if(prev->type == Variable){
		var = prev->data;
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
		NV_overwriteTerm(prev, result);
		//
		return prev;
	}
	if(NV_isDebugMode) NV_printError("NV_LANG00_Op_unaryOperator_suffix_variableOnly: Bad operand. type %d\n", prev->type);
	return NULL;
}

NV_Pointer NV_LANG00_Op_binaryOperator(NV_Pointer env, NV_Pointer thisTerm)
{
	// for Integer values only.
	NV_Pointer prev = thisTerm->prev;
	NV_Pointer next = thisTerm->next;
	NV_Pointer result;
	NV_Operator *op = (NV_Operator *)thisTerm->data;
	int vL, vR;
	int resultVal;
	// type check
	if(!prev || !next) return NULL;
	// try variable conversion
	NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &prev, 0);
	NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &next, 0);
	// check type
	if(!NV_canReadTermAsInt(prev) || !NV_canReadTermAsInt(next)) return NULL;
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
	return result;	
}
*/
NV_Pointer NV_LANG00_Op_nothingButDisappear(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer prev = NV_List_getPrevItem(thisTerm);
	NV_List_removeItem(thisTerm);
	return prev;
}
/*
NV_Pointer NV_LANG00_Op_sentenceSeparator(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer b;
	NV_Pointer sentenceTerm = NV_createTerm_Sentence(NULL);
	NV_Pointer sentenceRoot, remRoot;

	sentenceRoot = sentenceTerm->data;

	for(b = thisTerm->prev; b; b = b->prev){
		if(b->type == Root) break;
		if(b->type == Operator && strcmp(";;", ((NV_Operator *)b->data)->name) == 0){
			b = b->prev;
			NV_removeTerm(b->next);
			break;
		}
	}
	NV_divideTerm(sentenceRoot, b->next);
	NV_divideTerm(&remRoot, thisTerm);
	NV_appendAll(b, &remRoot);
	NV_overwriteTerm(thisTerm, sentenceTerm);
	NV_insertTermAfter(b, NV_createTerm_Operator(NV_Env_getLangDef(env), "builtin_exec"));
	NV_insertTermAfter(b, NV_createTerm_Operator(NV_Env_getLangDef(env), ";;"));
	return b;
}

NV_Pointer NV_LANG00_Op_stringLiteral(NV_Pointer env, NV_Pointer thisTerm)
{
	// "string literal"
	int len = 0, sp;
	NV_Pointer t, *endTerm;
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
		NV_removeTerm(t->prev);
	}
	s[len] = 0;
	NV_removeTerm(endTerm);
	t = NV_createTerm_String(s);
	NV_free(s);
	NV_overwriteTerm(thisTerm, t);
	return t;
}

NV_Pointer NV_LANG00_Op_sentenceBlock(NV_Pointer env, NV_Pointer thisTerm)
{
	// {}
	NV_Pointer originalTree;
	//
	originalTree = NV_LANG00_makeBlock(env, thisTerm, "}");
	if(!originalTree) return NULL;
	return originalTree;
}

NV_Pointer NV_LANG00_Op_precedentBlock(NV_Pointer env, NV_Pointer thisTerm)
{
	// ()
	// if prev term is sentence object, perform function call. 
	NV_Pointer originalTree;
	NV_Pointer prev;
	//
	originalTree = NV_LANG00_makeBlock(env, thisTerm, ")");
	if(!originalTree) return NULL;
	if(originalTree->type == Unknown) NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &originalTree, 0);
	if(NV_canReadTermAsSentence(originalTree)){
		NV_removeTerm(originalTree->next);
		prev = originalTree->prev;
		NV_LANG00_execSentence(env, originalTree);
		return prev;
	}
	NV_insertTermAfter(originalTree, NV_createTerm_Operator(NV_Env_getLangDef(env), "builtin_exec"));
	return originalTree;
}

NV_Pointer NV_LANG00_Op_structureAccessor(NV_Pointer env, NV_Pointer thisTerm)
{
	// []
	NV_Pointer structTerm, *indexTerm, *t, *v;
	int index;
	char s[32];
	//
	t = thisTerm->prev;
	if(!t) return NULL;
	if(t->type != Variable) NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &t, 1);
	if(t->type != Variable) return NULL;
	structTerm = t;
	//
	t = thisTerm->next;
	if(!t) return NULL;
	if(t->type != Imm32s) return NULL;
	if(!t->next || t->next->type != Operator || strcmp("]", ((NV_Operator *)t->next->data)->name) != 0) return NULL;
	indexTerm = t;
	//
	if(!NV_canReadTermAsInt(indexTerm)) return NULL;
	index = NV_getValueOfTermAsInt(indexTerm);
	t = NV_getItemFromStructureByIndex(structTerm->data, index);
	if(!t) return NULL;
	NV_removeTerm(indexTerm->next);
	NV_removeTerm(indexTerm->prev);
	NV_removeTerm(indexTerm);
	snprintf(s, sizeof(s) - 1, "%d", rand());
	v = NV_createTerm_Variable(NV_Env_getVarSet(env), s);
	NV_Variable_assignStructureItem(v->data, t);
	NV_overwriteTerm(structTerm, v);
	return v;
}

NV_Pointer NV_LANG00_Op_builtin_exec(NV_Pointer env, NV_Pointer thisTerm)
{
	//
	if(!NV_LANG00_execSentence(env, thisTerm->next)){
		return NULL;
	}
	thisTerm = thisTerm->prev;
	NV_removeTerm(thisTerm->next);
	//
	return thisTerm;
}

NV_Pointer NV_LANG00_Op_if(NV_Pointer env, NV_Pointer thisTerm)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	int cond;
	NV_Pointer condTerm, *doTerm, *t, *next;
	//
	t = thisTerm;
	//
	t = t->next; if(!NV_canReadTermAsSentence(t)) return NULL;
	condTerm = t;
	t = t->next; if(!NV_canReadTermAsSentence(t)) return NULL;
	doTerm = t;
	for(;;){
		// evaluate cond
		if(!NV_LANG00_execSentence(env, thisTerm->next))
			return NULL;
		condTerm = thisTerm->next;
		if(condTerm->next != doTerm)
			return NULL;
		// get cond value
		if(!NV_canReadTermAsInt(condTerm)) return NULL;
		cond = NV_getValueOfTermAsInt(condTerm);
		NV_removeTerm(condTerm);
		// evaluate do
		if(cond){
			// cond is true.
			t = NV_LANG00_execSentence(env, thisTerm->next);
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
			t = NV_LANG00_execSentence(env, thisTerm->next);
			if(!t) return NULL;
			break;
		};
		doTerm = t;
		// continue checking.
	}
	// remove 'if' term.
	thisTerm = thisTerm->prev;
	NV_removeTerm(thisTerm->next);
	//
	return thisTerm;
}

NV_Pointer NV_LANG00_Op_for(NV_Pointer env, NV_Pointer thisTerm)
{
	// for {init block}{conditional block}{update block}{statement}
	//int cond;
	NV_Pointer t, *initTerm, *condTerm, *updateTerm, *doTerm;
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
	if(NV_LANG00_execSentence(env, thisTerm->next) == NULL) return NULL;
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
	return t;
}
*/
NV_Pointer NV_LANG00_Op_print(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Env_setAutoPrintValueEnabled(env, 0);
	return NV_NullPointer;
}
/*
NV_Pointer NV_LANG00_Op_showOpList(NV_Pointer env, NV_Pointer thisTerm)
{

	NV_Pointer prev = thisTerm->prev;
	//
	NV_Operator *p;
	printf("Precedence: [opName]\n");
	for(p = NV_Env_getLangDef(env)->opRoot; p; p = p->next){
		printf("%10d: [%s]\n", p->precedence, p->name);
	}
	//
	NV_removeTerm(thisTerm);

	return NV_NullPointer;
}
*/
/*
NV_Pointer NV_LANG00_Op_mem(NV_Pointer env, NV_Pointer thisTerm)
{
	//thisTerm = NV_overwriteTerm(thisTerm, NV_createTerm_Imm32(NV_getMallocCount()));
	return thisTerm;
}
*/
NV_Pointer NV_LANG00_Op_exit(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Env_setEndFlag(env, 1);
	NV_Env_setAutoPrintValueEnabled(env, 0);
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
	/*
	NV_addOperator(lang, 200000,	"\"", NV_LANG00_Op_stringLiteral);
	NV_addOperator(lang, 100050,	"{", NV_LANG00_Op_sentenceBlock);
	NV_addOperator(lang, 100040,	";", NV_LANG00_Op_sentenceSeparator);
	NV_addOperator(lang, 100030,	"(", NV_LANG00_Op_precedentBlock);
	NV_addOperator(lang, 100020,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	//
	NV_addOperator(lang, 100000,	"mem", NV_LANG00_Op_mem);
	//
*/
	NV_addOperator(lang, 10000,	" ", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"\t", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"\r", NV_LANG00_Op_nothingButDisappear);	
	NV_addOperator(lang, 10000,	"\n", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	";;", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"else", NV_LANG00_Op_nothingButDisappear);
	NV_addOperator(lang, 10000,	"elseif", NV_LANG00_Op_nothingButDisappear);
/*
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
	*/
	//
	NV_addOperator(lang, 10,	"print", NV_LANG00_Op_print);
	//NV_addOperator(lang, 12,	"showop", NV_LANG00_Op_showOpList);
	NV_addOperator(lang, 10,	"exit", NV_LANG00_Op_exit);

	if(NV_isDebugMode) NV_List_printAll(lang->opRoot, ",\n");
	
	return lang;
}

