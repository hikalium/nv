#include "nv.h"

//
// Support functions
//

NV_Pointer NV_LANG00_makeBlock(NV_Pointer env, NV_Pointer thisItem, const char *closeStr)
{
	// support func
	// retv is prev term of thisTerm.
	NV_Pointer t, data, subListRoot, remListRoot, prevItem, thisData;
	int pairCount = 1;
	thisData = NV_ListItem_getData(thisItem);
	t = NV_ListItem_getNext(thisItem);
	for(; !NV_E_isNullPointer(t); t = NV_ListItem_getNext(t)){
		data = NV_ListItem_getData(t);
		if(NV_E_isType(data, EString) && NV_String_isEqualToCStr(data, closeStr)){
			pairCount --;
			if(pairCount == 0) break;
		} else if(NV_E_isType(data, EOperator) && 
			NV_E_isSamePointer(data, thisData)){
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

void NV_LANG00_fetchNextSentenceItem(NV_Pointer *t, NV_Pointer *list)
{
	// t will moved to next item.
	*list = NV_NullPointer;
	//
	*t = NV_ListItem_getNext(*t);
	if(!NV_ListItem_isDataType(*t, EList)) return;
	*list = *t;
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
	srcData = NV_E_clone(NV_E_convertToContents(vRoot, src));
	//
	NV_Variable_assignData(var, srcData);
	//
	
	NV_List_removeItem(thisItem);
	NV_List_removeItem(src);
	NV_ListItem_setData(dst, var);
	return dst;
}

NV_Pointer NV_LANG00_Op_compoundAssign(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Operator *op;
	NV_Pointer lang = NV_Env_getLang(env);
	NV_Pointer var;
	NV_Pointer vDict = NV_Env_getVarRoot(env);
	char s[2];
	//
	if(!NV_ListItem_isDataType(thisItem, EOperator))
		return NV_NullPointer;
	var = NV_ListItem_getData(NV_ListItem_getPrev(thisItem));
	var = NV_E_convertUnknownToKnown(vDict, var);
	if(!NV_E_isType(var, EVariable))
		return NV_NullPointer;
	//
	op = NV_ListItem_getRawData(thisItem, EOperator);
	if(!op) return NV_NullPointer;
	s[0] = op->name[0];
	s[1] = 0;
	//
	NV_ListItem_setData(thisItem, NV_Lang_getOperatorFromString(lang, "="));
	NV_List_insertDataAfterItem(thisItem, NV_Lang_getOperatorFromString(lang, s));
	NV_List_insertDataAfterItem(thisItem, var);
	return thisItem;
}
/*
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
*/

NV_Pointer NV_LANG00_Op_unaryOperator_varSuffix(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Operator *op;
	NV_Pointer lang = NV_Env_getLang(env);
	NV_Pointer var, cint;
	NV_Pointer vDict = NV_Env_getVarRoot(env);
	char s[2];
	//
	if(!NV_ListItem_isDataType(thisItem, EOperator))
		return NV_NullPointer;
	var = NV_ListItem_getData(NV_ListItem_getPrev(thisItem));
	var = NV_E_convertUnknownToKnown(vDict, var);
	if(!NV_E_isType(var, EVariable))
		return NV_NullPointer;
	//
	op = NV_ListItem_getRawData(thisItem, EOperator);
	if(!op) return NV_NullPointer;
	s[0] = op->name[0];
	s[1] = 0;
	//
	cint = NV_E_malloc_type(EInteger);
	NV_Integer_setImm32(cint, 1);
	NV_ListItem_setData(thisItem, NV_Lang_getOperatorFromString(lang, "="));
	NV_List_insertDataAfterItem(thisItem, var);
	NV_List_insertDataAfterItem(thisItem, NV_Lang_getOperatorFromString(lang, s));
	NV_List_insertDataAfterItem(thisItem, cint);
	return thisItem;
}

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
	if(NV_E_isNullPointer(prev) || NV_E_isNullPointer(next)){
		NV_Error("%s", "operand is NULL");
		return NV_NullPointer;
	}
	if(!op){
		NV_Error("%s", "op is NULL!");
		return NV_NullPointer;
	}
	//
	vL = NV_List_removeItem(prev);
	vR = NV_List_removeItem(next);
	// try variable conversion
	vL = NV_E_convertUnknownToKnown(vRoot, vL);
	vR = NV_E_convertUnknownToKnown(vRoot, vR);
	//
	opType = NV_LANG00_getBinOpTypeFromString(op->name);
	// process
	resultData = NV_Integer_evalBinOp(vL, vR, opType);
	if(NV_E_isNullPointer(resultData)){
		NV_Error("%s", "result is NULL!");
		return NV_NullPointer;
	}
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

NV_Pointer NV_LANG00_Op_sentenceSeparator(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer t, sentenceRoot, remRoot;
	NV_Operator *tOp;
	sentenceRoot = NV_E_malloc_type(EList);
	t = NV_ListItem_getPrev(thisItem);
	for(; !NV_E_isNullPointer(t); t = NV_ListItem_getPrev(t)){
		if(NV_E_isType(t, EList)) break;
		if(!NV_ListItem_isDataType(t, EOperator)) continue;
		tOp = NV_ListItem_getRawData(t, EOperator);
		if(strcmp(tOp->name, ";;") == 0){
			t = NV_ListItem_getPrev(t);
			NV_List_removeItem(NV_ListItem_getNext(t));
			break;
		}
	}
	sentenceRoot = NV_List_divideBefore(NV_ListItem_getNext(t));
	remRoot = NV_List_divideBefore(thisItem);
	NV_List_insertAllAfter(t, remRoot);
	NV_ListItem_setData(thisItem, sentenceRoot);
	NV_List_insertDataAfterItem(t, 
		NV_Lang_getOperatorFromString(NV_Env_getLang(env), "builtin_exec"));
	NV_List_insertDataAfterItem(t, 
		NV_Lang_getOperatorFromString(NV_Env_getLang(env), ";;"));
	return t;
}
/*
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
	NV_Pointer itemBeforeBlock, f;
	// NV_Pointer prev;
	//
	itemBeforeBlock = NV_LANG00_makeBlock(env, thisItem, ")");
	if(NV_E_isNullPointer(itemBeforeBlock)) return NV_NullPointer;
	// if prev term is sentence object, perform function call. 
	f = NV_E_convertToContents(
			NV_Env_getVarRoot(env), NV_ListItem_getData(itemBeforeBlock));
	if(NV_E_isType(f, EList)){
		NV_List_removeItem(NV_ListItem_getNext(itemBeforeBlock));
		NV_ListItem_setData(itemBeforeBlock, NV_E_clone(f));
		NV_LANG00_execSentence(env, itemBeforeBlock);
		return itemBeforeBlock;
	}
	NV_List_insertDataAfterItem(
		itemBeforeBlock,
		NV_Lang_getOperatorFromString(NV_Env_getLang(env), "builtin_exec"));
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

NV_Pointer NV_LANG00_Op_if(NV_Pointer env, NV_Pointer thisItem)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	int32_t cond;
	NV_Pointer condItem, doItem, t;
	//
	t = thisItem;
	NV_LANG00_fetchNextSentenceItem(&t, &condItem);
	NV_LANG00_fetchNextSentenceItem(&t, &doItem);
	for(;;){
		// evaluate cond
		NV_LANG00_execSentence(env, condItem);
		if(NV_E_isNullPointer(doItem)){
			// condItem was else block so break here.
			break;
		}
		// get cond value
		t = NV_ListItem_getData(condItem);	// t is root of the list.
		t = NV_E_unbox(NV_ListItem_getData(NV_List_getLastItem(t)));
		if(!NV_E_isType(t, EInteger)) return NV_NullPointer;
		cond = NV_Integer_getImm32(t);
		NV_List_removeItem(condItem);
		// evaluate do
		if(cond){
			// cond is true.
			t = NV_LANG00_execSentence(env, doItem);
			if(NV_E_isNullPointer(t)) return NV_NullPointer;
			// remove rest of sentence blocks.
			t = NV_ListItem_getNext(doItem);
			while(NV_ListItem_isDataType(t, EList)){
				t = NV_ListItem_getNext(t);
				NV_List_removeItem(NV_ListItem_getPrev(t));
			}
			break;
		} else{
			NV_List_removeItem(doItem);
		}
		// cond is false. check next cond.
		t = thisItem;
		NV_LANG00_fetchNextSentenceItem(&t, &condItem);
		NV_LANG00_fetchNextSentenceItem(&t, &doItem);
		// continue checking.
	}
	// remove 'if' term.
	t = NV_ListItem_getPrev(thisItem);
	NV_List_removeItem(thisItem);
	//
	return t;
}

NV_Pointer NV_LANG00_Op_for(NV_Pointer env, NV_Pointer thisItem)
{
	// for {init block}{conditional block}{update block}{statement}
	int cond;
	NV_Pointer t, initItem, condItem, updateItem, doItem;
	NV_Pointer tmpCondRoot, tmpUpdateRoot, tmpDoRoot;
	//
	t = thisItem;
	NV_LANG00_fetchNextSentenceItem(&t, &initItem);
	NV_LANG00_fetchNextSentenceItem(&t, &condItem);
	NV_LANG00_fetchNextSentenceItem(&t, &updateItem);
	NV_LANG00_fetchNextSentenceItem(&t, &doItem);
	if(NV_E_isNullPointer(initItem) || NV_E_isNullPointer(condItem) || 
		NV_E_isNullPointer(updateItem) || NV_E_isNullPointer(doItem))
		return NV_NullPointer;
	// do init block
	// initTerm is removed here.
	t = NV_LANG00_execSentence(env,initItem);
	if(NV_E_isNullPointer(t)) return NV_NullPointer;
	NV_List_removeItem(t);
	// 
	for(;;){
		// copy blocks
		tmpCondRoot = NV_E_clone(NV_ListItem_getData(condItem));
		tmpUpdateRoot = NV_E_clone(NV_ListItem_getData(updateItem));
		tmpDoRoot = NV_E_clone(NV_ListItem_getData(doItem));
		// check cond
		if(NV_EvaluateSentence(env, tmpCondRoot)) return NV_NullPointer;
		t = NV_E_unbox(NV_ListItem_getData(NV_List_getLastItem(tmpCondRoot)));
		if(!NV_E_isType(t, EInteger)) return NV_NullPointer;
		cond = NV_Integer_getImm32(t);
		if(!cond) break;
		// do
		if(NV_EvaluateSentence(env, tmpDoRoot)) return NV_NullPointer;
		// update
		if(NV_EvaluateSentence(env, tmpUpdateRoot)) return NV_NullPointer;
		// free tmp
		NV_resetEvalTree(tmpCondRoot);
		NV_resetEvalTree(tmpUpdateRoot);
		NV_resetEvalTree(tmpDoRoot);
		//
		NV_E_free(&tmpCondRoot);
		NV_E_free(&tmpUpdateRoot);
		NV_E_free(&tmpDoRoot);
	}
	// free tmp
	NV_resetEvalTree(tmpCondRoot);
	NV_resetEvalTree(tmpUpdateRoot);
	NV_resetEvalTree(tmpDoRoot);
	//
	NV_E_free(&tmpCondRoot);
	NV_E_free(&tmpUpdateRoot);
	NV_E_free(&tmpDoRoot);
	// remove original
	NV_List_removeItem(updateItem);
	NV_List_removeItem(condItem);
	NV_List_removeItem(doItem);
	// remove 'for' term.
	t = NV_ListItem_getPrev(thisItem);
	NV_List_removeItem(thisItem);
	//
	return t;
}

NV_Pointer NV_LANG00_Op_print(NV_Pointer env, NV_Pointer thisItem)
{
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_printElement(
		NV_E_convertToContents(
			NV_Env_getVarRoot(env), NV_ListItem_getData(nextItem)));
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
		NV_Lang_getOpList(NV_Env_getLang(env)), "\nOpList: [\n", ",\n", "\n]\n");
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

NV_Pointer NV_allocLang00()
{
	NV_Pointer lang = NV_E_malloc_type(ELang);
	//
	NV_Lang_setCharList(lang, 0, " \t\r\n");
	NV_Lang_setCharList(lang, 1, "!%&-=^~|+*:.<>/");
	NV_Lang_setCharList(lang, 2, "(){}[],;\"");
	// based on http://www.tutorialspoint.com/cprogramming/c_operators.htm
	//
	/*
	NV_Lang_addOp(lang, 200000,	"\"", NV_LANG00_Op_stringLiteral);
*/
	NV_Lang_addOp(lang, 100050,	"{", NV_LANG00_Op_sentenceBlock);
	NV_Lang_addOp(lang, 100040,	";", NV_LANG00_Op_sentenceSeparator);
	NV_Lang_addOp(lang, 100030,	"(", NV_LANG00_Op_precedentBlock);
	NV_Lang_addOp(lang, 100020,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	//
	NV_Lang_addOp(lang, 100000,	"mem", NV_LANG00_Op_mem);
	//
	NV_Lang_addOp(lang, 10000,	" ", NV_LANG00_Op_nothingButDisappear);
	NV_Lang_addOp(lang, 10000,	"\t", NV_LANG00_Op_nothingButDisappear);
	NV_Lang_addOp(lang, 10000,	"\r", NV_LANG00_Op_nothingButDisappear);	
	NV_Lang_addOp(lang, 10000,	"\n", NV_LANG00_Op_nothingButDisappear);
	NV_Lang_addOp(lang, 10000,	";;", NV_LANG00_Op_nothingButDisappear);
	NV_Lang_addOp(lang, 10000,	"else", NV_LANG00_Op_nothingButDisappear);
	NV_Lang_addOp(lang, 10000,	"elseif", NV_LANG00_Op_nothingButDisappear);
/*
	//
	NV_Lang_addOp(lang, 2010,	"[", NV_LANG00_Op_structureAccessor);	
	NV_Lang_addOp(lang, 2000,	"]", NV_LANG00_Op_nothingButDisappear);
	//
*/
	NV_Lang_addOp(lang, 1000,  "if", NV_LANG00_Op_if);
	NV_Lang_addOp(lang, 1000,  "for", NV_LANG00_Op_for);
	//
	NV_Lang_addOp(lang, 702,	"++", NV_LANG00_Op_unaryOperator_varSuffix);
	NV_Lang_addOp(lang, 702,	"--", NV_LANG00_Op_unaryOperator_varSuffix);
	//
	/*
	NV_Lang_addOp(lang, 701,	"+", NV_LANG00_Op_unaryOperator_prefix);
	NV_Lang_addOp(lang, 701,	"-", NV_LANG00_Op_unaryOperator_prefix);
	NV_Lang_addOp(lang, 701,	"!", NV_LANG00_Op_unaryOperator_prefix);
	//
*/
	NV_Lang_addOp(lang, 600,	"*", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 600,	"/", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 600,	"%", NV_LANG00_Op_binaryOperator);
	//
	NV_Lang_addOp(lang, 500,	"+", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 500,	"-", NV_LANG00_Op_binaryOperator);
	//
	NV_Lang_addOp(lang, 400,	"<", NV_LANG00_Op_binaryOperator);	
	NV_Lang_addOp(lang, 400,	">", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	"<=", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	">=", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	"==", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	"!=", NV_LANG00_Op_binaryOperator);
	//
	NV_Lang_addOp(lang, 300,	"||", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 300,	"&&", NV_LANG00_Op_binaryOperator);
	//
	NV_Lang_addOp(lang, 200,	"+=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 200,	"-=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 200,	"*=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 200,	"/=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 200,	"%=", NV_LANG00_Op_compoundAssign);
	//
	NV_Lang_addOp(lang, 101,	"=", NV_LANG00_Op_assign);
	//
	NV_Lang_addOp(lang, 10,	"print", NV_LANG00_Op_print);
	NV_Lang_addOp(lang, 12,	"showop", NV_LANG00_Op_showOpList);
	NV_Lang_addOp(lang, 12,	"vlist", NV_LANG00_Op_showVarList);
	NV_Lang_addOp(lang, 10,	"exit", NV_LANG00_Op_exit);

	if(NV_isDebugMode) NV_List_printAll(NV_Lang_getOpList(lang), "\n[\n", ",\n", "\n]\n");
	
	return lang;
}

