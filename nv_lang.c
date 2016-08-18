#include "nv.h"

//
// Support functions
//

NV_Pointer NV_LANG00_makeBlock(NV_Pointer env, NV_Pointer thisItem, const char *closeStr)
{
	// support func
	// retv is prev term of thisTerm.
	NV_Pointer t, data, subListRoot, remListRoot, prevItem;
	int pairCount = 1;
	t = NV_ListItem_getNext(thisItem);
	for(; !NV_E_isNullPointer(t); t = NV_ListItem_getNext(t)){
		data = NV_ListItem_getData(t);
		if(NV_E_isType(data, EString) && NV_String_isEqualToCStr(data, closeStr)){
			pairCount --;
			if(pairCount == 0) break;
		} else if(NV_E_isType(data, EOperator) && 
			NV_E_isSamePointer(data, thisItem)){
			pairCount++;
		}
	}
	if(pairCount != 0) return NV_NullPointer;
	// t is item which is close str.
	prevItem = NV_ListItem_getPrev(thisItem);
	subListRoot = NV_List_divideBefore(thisItem);
	remListRoot = NV_List_divideBefore(t);
	NV_List_removeItem(thisItem);
	NV_List_insertAllAfter(prevItem, remListRoot);
	data = NV_ListItem_setData(t, subListRoot);
	NV_E_free(&data);	// free closeStr instance 
	return prevItem;
}

NV_Pointer NV_LANG00_execSentence(NV_Pointer env, NV_Pointer sentenceRootItem)
{
	// eval sentence
	NV_Pointer sentenceRoot;
	//
	if(!NV_ListItem_isDataType(sentenceRootItem, EList)) return NV_NullPointer;
	sentenceRoot = NV_ListItem_getData(sentenceRootItem);
	//
	if(NV_EvaluateSentence(env, sentenceRoot)){
		NV_Error("%s", "Exec failed.");
		return NV_NullPointer;
	}
	return sentenceRootItem;
}

NV_BinOpType NV_LANG00_getBinOpTypeFromString(const char *s)
{
	     if(strcmp("+", 	s) == 0)	return BOpAdd;
	else if(strcmp("-", 	s) == 0)	return BOpSub;
	else if(strcmp("*", 	s) == 0)	return BOpMul;
	else if(strcmp("/", 	s) == 0)	return BOpDiv;
	else if(strcmp("%", 	s) == 0)	return BOpMod;
	else if(strcmp("||",	s) == 0)	return BOpLogicOR;
	else if(strcmp("&&",	s) == 0)	return BOpLogicAND;
	else if(strcmp("|",		s) == 0)	return BOpBitOR;
	else if(strcmp("&", 	s) == 0)	return BOpBitAND;
	else if(strcmp("==", 	s) == 0)	return BOpCmpEq;
	else if(strcmp("!=", 	s) == 0)	return BOpCmpNEq;
	else if(strcmp("<", 	s) == 0)	return BOpCmpLt;
	else if(strcmp(">", 	s) == 0)	return BOpCmpGt;
	else if(strcmp("<=", 	s) == 0)	return BOpCmpLtE;
	else if(strcmp(">=", 	s) == 0)	return BOpCmpGtE;
	return BOpNone;
}

/*

//
// Native Functions
//
*/
NV_Pointer NV_LANG00_Op_assign(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer src = NV_ListItem_getNext(thisItem);
	NV_Pointer dst = NV_ListItem_getPrev(thisItem);
	NV_Pointer var, srcData;
	NV_Pointer vRoot = NV_Env_getVarRoot(env);
	// type check
	if(NV_E_isNullPointer(src) || NV_E_isNullPointer(dst)) return NV_NullPointer;
	if(NV_ListItem_isDataType(dst, EString)){
		// create / get val.
		var = NV_Variable_allocByStr(vRoot, NV_ListItem_getData(dst));
	} else if(NV_ListItem_isDataType(dst, EVariable)){
		// use existed val.
		var = NV_ListItem_getData(dst);
	} else{
		// dst is not assignable
		NV_Error("%s", "Cannot assign data to following object.");
		NV_printElement(dst);
		return NV_NullPointer;
	}
	//
	srcData = NV_ListItem_getData(src);
	srcData = NV_Variable_tryAllocVariableExisted(vRoot, srcData);
	srcData = NV_E_getPrimitive(srcData);
	srcData = NV_E_clone(srcData);
	//
	NV_Variable_assignData(var, srcData);
	//
	
	NV_List_removeItem(thisItem);
	NV_List_removeItem(src);
	NV_ListItem_setData(dst, var);
	return dst;
}
/*
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
*/
NV_Pointer NV_LANG00_Op_binaryOperator(NV_Pointer env, NV_Pointer thisTerm)
{
	// for Integer values only.
	NV_Pointer prev = NV_ListItem_getPrev(thisTerm);
	NV_Pointer next = NV_ListItem_getNext(thisTerm);
	NV_Operator *op = NV_ListItem_getRawData(thisTerm, EOperator);
	NV_Pointer resultData;
	NV_Pointer vL, vR;
	NV_BinOpType opType;
	NV_Pointer vRoot = NV_Env_getVarRoot(env);
	// type check
	if(NV_E_isNullPointer(prev) || NV_E_isNullPointer(next)) return NV_NullPointer;
	if(!op) return NV_NullPointer;
	//
	vL = NV_List_removeItem(prev);
	vR = NV_List_removeItem(next);
	// try variable conversion
	vL = NV_Variable_tryAllocVariableExisted(vRoot, vL);
	vR = NV_Variable_tryAllocVariableExisted(vRoot, vR);
	//
	opType = NV_LANG00_getBinOpTypeFromString(op->name);
	// process
	resultData = NV_Integer_evalBinOp(vL, vR, opType);
	if(NV_E_isNullPointer(resultData)) return NV_NullPointer;
	//
	NV_E_free(&vL);
	NV_E_free(&vR);
	NV_ListItem_setData(thisTerm, resultData);
	//
	return resultData;
}
NV_Pointer NV_LANG00_Op_nothingButDisappear(NV_Pointer env, NV_Pointer thisTerm)
{
	NV_Pointer prev = NV_ListItem_getPrev(thisTerm);
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
*/
NV_Pointer NV_LANG00_Op_sentenceBlock(NV_Pointer env, NV_Pointer thisTerm)
{
	return NV_LANG00_makeBlock(env, thisTerm, "}");
}

NV_Pointer NV_LANG00_Op_precedentBlock(NV_Pointer env, NV_Pointer thisItem)
{
	// ()
	NV_Pointer itemBeforeBlock;
	// NV_Pointer prev;
	//
	itemBeforeBlock = NV_LANG00_makeBlock(env, thisItem, ")");
	if(NV_E_isNullPointer(itemBeforeBlock)) return NV_NullPointer;
/*
	// if prev term is sentence object, perform function call. 
	if(originalTree->type == Unknown) NV_tryConvertTermFromUnknownToVariable(NV_Env_getVarSet(env), &originalTree, 0);
	if(NV_canReadTermAsSentence(originalTree)){
		NV_removeTerm(originalTree->next);
		prev = originalTree->prev;
		NV_LANG00_execSentence(env, originalTree);
		return prev;
	}
*/
	NV_List_insertDataAfterItem(
		itemBeforeBlock,
		NV_getOperatorFromString(NV_Env_getLangDef(env), "builtin_exec"));
	return itemBeforeBlock;
}
/*
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
*/
NV_Pointer NV_LANG00_Op_builtin_exec(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer prevItem, nextItem;
	prevItem = NV_ListItem_getPrev(thisItem);
	nextItem = NV_ListItem_getNext(thisItem);
	if(NV_E_isNullPointer(NV_LANG00_execSentence(env, nextItem)))
		return NV_NullPointer;
	NV_List_removeItem(thisItem);
	//
	return prevItem;
}
/*
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
NV_Pointer NV_LANG00_Op_print(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_printElement(
		NV_Variable_tryAllocVariableExisted(
			NV_Env_getVarRoot(env),
			NV_ListItem_getData(nextItem)));
	printf("\n");
	NV_List_removeItem(thisItem);
	NV_Env_setAutoPrintValueEnabled(env, 0);
	return nextItem;
}

NV_Pointer NV_LANG00_Op_showOpList(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer prevItem = NV_ListItem_getPrev(thisItem);
	//
	NV_List_printAll(
		NV_Env_getLangDef(env)->opRoot, "\nOpList: [\n", ",\n", "\n]\n");
	//
	NV_List_removeItem(thisItem);
	NV_Env_setAutoPrintValueEnabled(env, 0);
	return prevItem;
}

NV_Pointer NV_LANG00_Op_showVarList(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer prevItem = NV_ListItem_getPrev(thisItem);
	//
	NV_Dict_printAll(
		NV_Env_getVarRoot(env), "\nVarList: [\n", ",\n", "\n]\n");
	//
	NV_List_removeItem(thisItem);
	NV_Env_setAutoPrintValueEnabled(env, 0);
	return prevItem;
}

NV_Pointer NV_LANG00_Op_mem(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer memUsingSize = NV_E_malloc_type(EInteger);
	NV_Integer_setImm32(memUsingSize, NV_getMallocCount());
	NV_ListItem_setData(thisItem, memUsingSize);
	return thisItem;
}

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
*/
	NV_addOperator(lang, 100050,	"{", NV_LANG00_Op_sentenceBlock);
//	NV_addOperator(lang, 100040,	";", NV_LANG00_Op_sentenceSeparator);
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
*/
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
/*
	NV_addOperator(lang, 200,	"+=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"-=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"*=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"/=", NV_LANG00_Op_compoundAssign);
	NV_addOperator(lang, 200,	"%=", NV_LANG00_Op_compoundAssign);
*/
	//
	NV_addOperator(lang, 101,	"=", NV_LANG00_Op_assign);
	//
	NV_addOperator(lang, 10,	"print", NV_LANG00_Op_print);
	NV_addOperator(lang, 12,	"showop", NV_LANG00_Op_showOpList);
	NV_addOperator(lang, 12,	"vlist", NV_LANG00_Op_showVarList);
	NV_addOperator(lang, 10,	"exit", NV_LANG00_Op_exit);

	if(NV_isDebugMode) NV_List_printAll(lang->opRoot, "\n[\n", ",\n", "\n]\n");
	
	return lang;
}

