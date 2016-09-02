#include "nv.h"

#define NV_EXC_FLAG_LANG00_EXIT_BY_CONTINUE		0x010000
#define NV_EXC_FLAG_LANG00_EXIT_BY_BREAK		0x020000


//
// Support functions
//

NV_Pointer
NV_LANG00_makeBlock
(NV_Pointer thisItem, const char *closeStr)
{
	// support func
	// retv is prev term of thisItem.
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
	NV_E_free(&thisItem);
	NV_List_insertAllAfter(prevItem, remListRoot);
	NV_E_free(&remListRoot);
	NV_ListItem_setData(t, NV_E_autorelease(subListRoot));
	return prevItem;
}

void
NV_LANG00_execSentence
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer sentenceRootItem)
{
	// eval sentence
	// retv is sentenceRootItem(success) or NV_NullPointer(failed)
	NV_Pointer sentenceRoot;
	//
	if(!NV_ListItem_isDataType(sentenceRootItem, EList)){
		NV_Error("%s", "sentenceRootItem is not EList");
		NV_printElement(sentenceRootItem);
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	sentenceRoot = NV_ListItem_getData(sentenceRootItem);
	//
	NV_evaluateSentence(excFlag, lang, vDict, sentenceRoot);
	if(*excFlag & NV_EXC_FLAG_FAILED){
		NV_Error("%s", "Exec failed.");
		return;
	}
	return;
}


void
NV_LANG00_execSentenceScalar
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer sentenceRootItem)
{
	// eval sentence
	// sentenceRootItem will be repaced by last value of eval tree.
	NV_LANG00_execSentence(excFlag, lang, vDict, sentenceRootItem);
	if(*excFlag & NV_EXC_FLAG_FAILED){
		NV_Error("%s", "sentenceRootItem is not EList");
		NV_printElement(sentenceRootItem);
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	NV_ListItem_setData(sentenceRootItem,
		NV_ListItem_getData(
			NV_List_getLastItem(
				NV_ListItem_getData(sentenceRootItem))));
	return;
}

NV_BinOpType
NV_LANG00_getBinOpTypeFromString
(const char *s)
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
	else if(strcmp("^", 	s) == 0)	return BOpBitXOR;
	else if(strcmp("==", 	s) == 0)	return BOpCmpEq;
	else if(strcmp("!=", 	s) == 0)	return BOpCmpNEq;
	else if(strcmp("<", 	s) == 0)	return BOpCmpLt;
	else if(strcmp(">", 	s) == 0)	return BOpCmpGt;
	else if(strcmp("<=", 	s) == 0)	return BOpCmpLtE;
	else if(strcmp(">=", 	s) == 0)	return BOpCmpGtE;
	return BOpNone;
}

void
NV_LANG00_fetchNextSentenceItem
(NV_Pointer *t, NV_Pointer *list)
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
NV_Pointer
NV_LANG00_Op_assign
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer srcData;
	NV_Pointer dstData;
	{
		NV_Pointer src = NV_ListItem_getNext(thisItem);
		NV_Pointer dst = NV_ListItem_getPrev(thisItem);
		//
		if(NV_E_isNullPointer(src) || NV_E_isNullPointer(dst))
			return NV_NullPointer;
		NV_ListItem_convertUnknownToKnown(vDict, dst);
		NV_ListItem_convertUnknownToKnown(vDict, src);
		NV_ListItem_unbox(src);
		srcData = NV_E_retain(NV_ListItem_getData(src));
		dstData = NV_E_retain(NV_ListItem_getData(dst));
		//
		NV_E_free(&src);
		NV_E_free(&dst);
	}
	if(!NV_E_isType(dstData, EVariable)){
		// dst is not assignable
		NV_Error("%s", "Cannot assign data to following object.");
		NV_printElement(dstData);
		return NV_NullPointer;
	}
	NV_Variable_assignData(dstData, NV_E_autorelease(NV_E_clone(srcData)));
	NV_ListItem_setData(thisItem, NV_E_autorelease(dstData));
	//
	NV_E_free(&srcData);
	return thisItem;
}

NV_Pointer
NV_LANG00_Op_compoundAssign
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem){
	NV_Operator *op;
	NV_Pointer var, prevItem;
	char s[2];
	//
	if(!NV_ListItem_isDataType(thisItem, EOperator))
		return NV_NullPointer;
	
	prevItem = NV_ListItem_getPrev(thisItem);
	NV_ListItem_convertUnknownToKnown(vDict, prevItem);
	var = NV_ListItem_getData(prevItem);
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
	NV_List_insertDataAfterItem(thisItem, NV_E_autorelease(NV_E_clone(var)));
	return thisItem;
}

NV_Pointer
NV_LANG00_Op_declareVariable
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer next = NV_ListItem_getNext(thisItem);
	NV_Pointer var;
	if(!NV_ListItem_isDataType(next, EString)) return NV_NullPointer;
	//
	var = NV_Variable_allocByStr(vDict, NV_ListItem_getData(next));
	//
	NV_E_free(&next);
	NV_ListItem_setData(thisItem, NV_E_autorelease(var));
	return thisItem;
}

NV_Pointer
NV_LANG00_Op_unaryOperator_prefix
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prev = NV_ListItem_getPrev(thisItem);
	NV_Pointer next = NV_ListItem_getNext(thisItem);
	NV_Pointer data;
	NV_Operator *op = NV_ListItem_getRawData(thisItem, EOperator);
	int32_t val;
	// type check
	if(NV_E_isNullPointer(next)) return NV_NullPointer;
	if(!NV_ListItem_isDataType(prev, EOperator) && 
		!NV_E_isType(prev, EList)) return NV_NullPointer;
	NV_ListItem_convertUnknownToKnown(vDict, next);
	NV_ListItem_unbox(next);
	data = NV_ListItem_getData(next);
	// process
	if(NV_E_isType(data, EInteger)){
		val = NV_Integer_getImm32(data);
		if(strcmp("+", op->name) == 0){
			val = + val;
		} else if(strcmp("-", op->name) == 0){
			val = - val;
		}
		// comparison operators
		else if(strcmp("!", op->name) == 0){
			val = ! val;
		} else{
#ifdef DEBUG
			if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
				NV_Error("Not implemented %s\n", op->name);
#endif
			return NV_NullPointer;
		}
		NV_Integer_setImm32(data, val);
		NV_E_free(&thisItem);
		return next;
	}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
		NV_Error("%s", "Bad operand.");
#endif
	return NV_NullPointer;
}

NV_Pointer
NV_LANG00_Op_unaryOperator_varSuffix
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Operator *op;
	NV_Pointer var, cint, prevItem;
	char s[2];
	//
	if(!NV_ListItem_isDataType(thisItem, EOperator))
		return NV_NullPointer;
	prevItem = NV_ListItem_getPrev(thisItem);
	NV_ListItem_convertUnknownToKnown(vDict, prevItem);
	var = NV_ListItem_getData(prevItem);
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
	//
	NV_ListItem_setData(thisItem, NV_Lang_getOperatorFromString(lang, "="));
	NV_List_insertDataAfterItem(thisItem, NV_E_autorelease(cint));
	NV_List_insertDataAfterItem(thisItem, NV_Lang_getOperatorFromString(lang, s));
	NV_List_insertDataAfterItem(thisItem, NV_E_autorelease(NV_E_clone(var)));
	return thisItem;
}

NV_Pointer
NV_LANG00_Op_binaryOperator
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// for Integer values only.
	NV_Pointer prev = NV_ListItem_getPrev(thisItem);
	NV_Pointer next = NV_ListItem_getNext(thisItem);
	NV_Operator *op = NV_ListItem_getRawData(thisItem, EOperator);
	NV_Pointer resultData;
	NV_Pointer vL, vR;
	NV_BinOpType opType;
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
	NV_ListItem_convertUnknownToKnown(vDict, prev);
	NV_ListItem_convertUnknownToKnown(vDict, next);
	//
	NV_ListItem_unbox(prev);
	NV_ListItem_unbox(next);
	//
	vL = NV_E_retain(NV_ListItem_getData(prev)); NV_E_free(&prev);
	vR = NV_E_retain(NV_ListItem_getData(next)); NV_E_free(&next);
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
	NV_ListItem_setData(thisItem, NV_E_autorelease(resultData));
	//
	return thisItem;
}
NV_Pointer NV_LANG00_Op_nothingButDisappear(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prev = NV_ListItem_getPrev(thisItem);
	NV_E_free(&thisItem);
	return prev;
}

NV_Pointer
NV_LANG00_Op_sentenceSeparator
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer t, sentenceRoot, remRoot, tmp;
	NV_Operator *tOp;
	t = NV_ListItem_getPrev(thisItem);
	for(; !NV_E_isNullPointer(t); t = NV_ListItem_getPrev(t)){
		if(NV_E_isType(t, EList)) break;
		if(!NV_ListItem_isDataType(t, EOperator)) continue;
		tOp = NV_ListItem_getRawData(t, EOperator);
		if(strcmp(tOp->name, ";;") == 0){
			t = NV_ListItem_getPrev(t);
			tmp = NV_ListItem_getNext(t);
			NV_E_free(&tmp);
			break;
		}
	}
	sentenceRoot = NV_List_divideBefore(NV_ListItem_getNext(t));
	remRoot = NV_List_divideBefore(thisItem);
	NV_List_insertAllAfter(t, remRoot);
	NV_E_free(&remRoot);
	NV_ListItem_setData(thisItem, NV_E_autorelease(sentenceRoot));
	NV_List_insertDataAfterItem(t, 
		NV_Lang_getOperatorFromString(lang, "builtin_exec_scalar"));
	NV_List_insertDataAfterItem(t, 
		NV_Lang_getOperatorFromString(lang, ";;"));
	return t;
}

NV_Pointer
NV_LANG00_Op_sentenceBlock
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	return NV_LANG00_makeBlock(thisItem, "}");
}

NV_Pointer
NV_LANG00_Op_precedentBlock
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// ()
	NV_Pointer itemBeforeBlock, itemBlock, f, tmp;
	// NV_Pointer prev;
	//
	itemBeforeBlock = NV_LANG00_makeBlock(thisItem, ")");
	itemBlock = NV_ListItem_getNext(itemBeforeBlock);
	if(NV_E_isNullPointer(itemBeforeBlock)) return NV_NullPointer;
	// if prev term is sentence object, perform function call.
	NV_ListItem_convertUnknownToKnown(vDict, itemBeforeBlock);
	NV_ListItem_unbox(itemBeforeBlock);
	f = NV_ListItem_getData(itemBeforeBlock);
	if(NV_E_isType(f, EList)){
		tmp = NV_ListItem_getNext(itemBeforeBlock);
		NV_E_free(&tmp);
		NV_ListItem_setData(itemBeforeBlock, NV_E_autorelease(NV_E_clone(f)));
		NV_LANG00_execSentence(excFlag, lang, vDict, itemBeforeBlock);
		return itemBeforeBlock;
	}
	NV_List_insertDataAfterItem(
		itemBeforeBlock,
		NV_Lang_getOperatorFromString(lang, "builtin_exec_scalar")
	);
	return itemBeforeBlock;
}


NV_Pointer
NV_LANG00_Op_builtin_get_item
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// vpa*']'
	// []
	NV_Pointer prev, next, prevData, nextData, target, var;
	int index;
	//
	prev = NV_ListItem_getPrev(thisItem);
	next = NV_ListItem_getNext(thisItem);
	NV_ListItem_convertUnknownToKnown(vDict, prev);
	NV_ListItem_convertUnknownToKnown(vDict, next);
	NV_ListItem_unbox(prev);
	NV_ListItem_unbox(next);
	prevData = NV_ListItem_getData(prev);
	nextData = NV_ListItem_getData(next);
	//
	if(NV_E_isType(prevData, EList)){
		if(NV_E_isType(nextData, EInteger)){
			index = NV_Integer_getImm32(nextData);
			target = NV_List_getItemByIndex(prevData, index);
		} else{
			NV_Error("%s", "Can't use this type of index.");
			return NV_NullPointer;
		}
	//} else if(NV_E_isType(prevData, EDict)){
	} else{
		NV_Error("%s", "data is not enumerable.");
		return NV_NullPointer;
	}
	var = NV_E_malloc_type(EVariable);
	NV_Variable_setTarget(var, target);
	//
	NV_ListItem_setData(thisItem, NV_E_autorelease(var));
	NV_E_free(&prev);
	NV_E_free(&next);
	return thisItem;
}

NV_Pointer
NV_LANG00_Op_structureAccessor
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer itemBeforeBlock;
	itemBeforeBlock = NV_LANG00_makeBlock(thisItem, "]");

	NV_List_insertDataAfterItem(
		itemBeforeBlock,
		NV_Lang_getOperatorFromString(lang, "builtin_exec_scalar")
	);
	NV_List_insertDataAfterItem(
		itemBeforeBlock,
		NV_Lang_getOperatorFromString(lang, "builtin_get_item")
	);
	return itemBeforeBlock;
}

NV_Pointer
NV_LANG00_Op_builtin_exec
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prevItem, nextItem;
	prevItem = NV_ListItem_getPrev(thisItem);
	nextItem = NV_ListItem_getNext(thisItem);
	NV_LANG00_execSentence(excFlag, lang, vDict, nextItem);
	if(*excFlag & NV_EXC_FLAG_FAILED)
		return NV_NullPointer;
	NV_E_free(&thisItem);
	//
	return prevItem;
}

NV_Pointer
NV_LANG00_Op_builtin_exec_scalar
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prevItem, nextItem;
	prevItem = NV_ListItem_getPrev(thisItem);
	nextItem = NV_ListItem_getNext(thisItem);
	NV_LANG00_execSentenceScalar(excFlag, lang, vDict, nextItem);
	if(*excFlag & NV_EXC_FLAG_FAILED)
		return NV_NullPointer;
	NV_E_free(&thisItem);
	//
	return prevItem;
}

NV_Pointer
NV_LANG00_Op_if
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	int32_t cond;
	NV_Pointer condItem, doItem, t, tmp, lastItem;
	//
	t = thisItem;
	NV_LANG00_fetchNextSentenceItem(&t, &condItem);
	NV_LANG00_fetchNextSentenceItem(&t, &doItem);
	for(;;){
		if(NV_E_isNullPointer(condItem)){
			// end here
			break;
		}
		// evaluate cond
		NV_LANG00_execSentence(excFlag, lang, vDict, condItem);
		if(*excFlag & NV_EXC_FLAG_FAILED){
			NV_Error("%s", "exec cond failed.");
			return NV_NullPointer;
		}
		if(NV_E_isNullPointer(doItem)){
			// condItem was else block so break here.
			break;
		}
		// get cond value
		t = NV_ListItem_getData(condItem);	// t is root of the list.
		lastItem = NV_List_getLastItem(t);
		NV_ListItem_unbox(lastItem);
		t = NV_ListItem_getData(lastItem);
		if(!NV_E_isType(t, EInteger)){
			NV_Error("%s", "cond is not integer");
			return NV_NullPointer;
		}
		cond = NV_Integer_getImm32(t);
		NV_E_free(&condItem);
		// evaluate do
		if(cond){
			// cond is true.
			NV_LANG00_execSentence(excFlag, lang, vDict, doItem);
			if(*excFlag & NV_EXC_FLAG_FAILED){
				NV_Error("%s", "exec do failed.");
				return NV_NullPointer;
			}
			// remove rest of sentence blocks.
			t = NV_ListItem_getNext(doItem);
			while(NV_ListItem_isDataType(t, EList)){
				t = NV_ListItem_getNext(t);
				tmp = NV_ListItem_getPrev(t);
				NV_E_free(&tmp);
			}
			break;
		} else{
			NV_E_free(&doItem);
		}
		// cond is false. check next cond.
		t = thisItem;
		NV_LANG00_fetchNextSentenceItem(&t, &condItem);
		NV_LANG00_fetchNextSentenceItem(&t, &doItem);
		// continue checking.
	}
	// remove 'if' term.
	t = NV_ListItem_getPrev(thisItem);
	NV_E_free(&thisItem);
	//
	return t;
}

NV_Pointer
NV_LANG00_Op_for
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// for {init block}{conditional block}{update block}{statement}
	int cond;
	NV_Pointer t, initItem, condItem, updateItem, doItem, lastItem;
	NV_Pointer condRoot, updateRoot, doRoot;
	NV_Pointer tmpCondRoot, tmpUpdateRoot, tmpDoRoot;
	//
	t = thisItem;
	NV_LANG00_fetchNextSentenceItem(&t, &initItem);
	NV_LANG00_fetchNextSentenceItem(&t, &condItem);
	NV_LANG00_fetchNextSentenceItem(&t, &updateItem);
	NV_LANG00_fetchNextSentenceItem(&t, &doItem);
	if(NV_E_isNullPointer(initItem) || NV_E_isNullPointer(condItem) || 
		NV_E_isNullPointer(updateItem) || NV_E_isNullPointer(doItem)){
		NV_Error("%s", "Operand mismatched.");
		return NV_NullPointer;
	}
	//
	condRoot = NV_ListItem_getData(condItem);
	updateRoot = NV_ListItem_getData(updateItem);
	doRoot = NV_ListItem_getData(doItem);
	// do init block
	// initItem is removed here.
	NV_LANG00_execSentence(excFlag, lang, vDict, initItem);
	if(*excFlag & NV_EXC_FLAG_FAILED){
		NV_Error("%s", "Exec init failed.");
		return NV_NullPointer;
	}
	NV_E_free(&initItem);
	// 
	for(;;){
		// copy blocks
		tmpCondRoot = NV_E_clone(condRoot);
		tmpUpdateRoot = NV_E_clone(updateRoot);
		tmpDoRoot = NV_E_clone(doRoot);
		// check cond
		NV_evaluateSentence(excFlag, lang, vDict, tmpCondRoot);
		if(*excFlag & NV_EXC_FLAG_FAILED){
			NV_Error("%s", "Exec cond failed.");
			return NV_NullPointer;
		}
		//
		lastItem = NV_List_getLastItem(tmpCondRoot);
		NV_ListItem_unbox(lastItem);
		t = NV_ListItem_getData(lastItem);
		if(!NV_E_isType(t, EInteger)){
			NV_Error("%s", "cond type invalid");
			return NV_NullPointer;
		}
		cond = NV_Integer_getImm32(t);
		if(!cond) break;
		// do
		NV_evaluateSentence(excFlag, lang, vDict, tmpDoRoot);
		if(*excFlag & NV_EXC_FLAG_FAILED){
			NV_Error("%s", "Exec do failed");
			return NV_NullPointer;
		}
		if(*excFlag & NV_EXC_FLAG_LANG00_EXIT_BY_BREAK){
			// break
			CLR_FLAG(*excFlag, NV_EXC_FLAG_LANG00_EXIT_BY_BREAK);
			CLR_FLAG(*excFlag, NV_EXC_FLAG_EXIT);
			break;
		}
		// update
		NV_evaluateSentence(excFlag, lang, vDict, tmpUpdateRoot);
		if(*excFlag & NV_EXC_FLAG_FAILED){
			NV_Error("%s", "Exec update failed");
			return NV_NullPointer;
		}
		// free tmp
		NV_E_free(&tmpCondRoot);
		NV_E_free(&tmpUpdateRoot);
		NV_E_free(&tmpDoRoot);
	}
	// free tmp
	NV_E_free(&tmpCondRoot);
	NV_E_free(&tmpUpdateRoot);
	NV_E_free(&tmpDoRoot);
	// remove original
	NV_E_free(&updateItem);
	NV_E_free(&condItem);
	NV_E_free(&doItem);
	// remove 'for' term.
	t = NV_ListItem_getPrev(thisItem);
	NV_E_free(&thisItem);
	//
	return t;
}

NV_Pointer NV_LANG00_Op_print(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_Pointer nextData;
	NV_ListItem_convertUnknownToKnown(vDict, nextItem);
	NV_ListItem_unbox(nextItem);
	nextData = NV_ListItem_getData(nextItem);
	NV_printElement(nextData);
	printf("\n");
	NV_E_free(&thisItem);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	return nextItem;
}

NV_Pointer NV_LANG00_Op_showOpList(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prevItem = NV_ListItem_getPrev(thisItem);
	//
	NV_List_printAll(
		NV_Lang_getOpList(lang), "\nOpList: [\n", ",\n", "\n]\n");
	//
	NV_E_free(&thisItem);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	return prevItem;
}

NV_Pointer NV_LANG00_Op_showVarList(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer prevItem = NV_ListItem_getPrev(thisItem);
	//
	NV_Dict_printAll(
		vDict, "\nVarList: [\n", ",\n", "\n]\n");
	//
	NV_E_free(&thisItem);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	return prevItem;
}

NV_Pointer NV_LANG00_Op_mem(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	NV_Pointer memUsingSize = NV_E_malloc_type(EInteger);
	NV_Integer_setImm32(memUsingSize, NV_getMallocCount());
	NV_ListItem_setData(thisItem, NV_E_autorelease(memUsingSize));
	return thisItem;
}

NV_Pointer NV_LANG00_Op_exit(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	SET_FLAG(*excFlag, NV_EXC_FLAG_EXIT);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	return thisItem;
}

NV_Pointer NV_LANG00_Op_continue(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	SET_FLAG(*excFlag, NV_EXC_FLAG_EXIT);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	SET_FLAG(*excFlag, NV_EXC_FLAG_LANG00_EXIT_BY_CONTINUE);
	return thisItem;
}

NV_Pointer NV_LANG00_Op_break(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	SET_FLAG(*excFlag, NV_EXC_FLAG_EXIT);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
	SET_FLAG(*excFlag, NV_EXC_FLAG_LANG00_EXIT_BY_BREAK);
	return thisItem;
}

NV_Pointer NV_allocLang00()
{
	NV_Pointer lang = NV_E_malloc_type(ELang);
	//
	NV_Lang_setCharList(lang, 0, " \t\r\n");
	NV_Lang_setCharList(lang, 1, "#!%&-=^~|+*:.<>/");
	NV_Lang_setCharList(lang, 2, "(){}[],;\"");

	// based on http://www.tutorialspoint.com/cprogramming/c_operators.htm
	NV_Lang_addOp(lang, 100050,	"{", NV_LANG00_Op_sentenceBlock);
	NV_Lang_addOp(lang, 100030,	"(", NV_LANG00_Op_precedentBlock);
	NV_Lang_addOp(lang, 100020,	"builtin_exec", NV_LANG00_Op_builtin_exec);
	NV_Lang_addOp(lang, 100020,	"builtin_exec_scalar", NV_LANG00_Op_builtin_exec_scalar);
	NV_Lang_addOp(lang, 100010,	";", NV_LANG00_Op_sentenceSeparator);
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
	//
	NV_Lang_addOp(lang, 2010,	"[", NV_LANG00_Op_structureAccessor);	
	NV_Lang_addOp(lang, 2008,	"builtin_get_item", NV_LANG00_Op_builtin_get_item);	
	//
	NV_Lang_addOp(lang, 1000,  "if", NV_LANG00_Op_if);
	NV_Lang_addOp(lang, 1000,  "for", NV_LANG00_Op_for);
	//
	NV_Lang_addOp(lang, 800,	"#", NV_LANG00_Op_declareVariable);
	// Postfix
	NV_Lang_addOp(lang, 702,	"++", NV_LANG00_Op_unaryOperator_varSuffix);
	NV_Lang_addOp(lang, 702,	"--", NV_LANG00_Op_unaryOperator_varSuffix);
	// Unary
	NV_Lang_addOp(lang, 701,	"+", NV_LANG00_Op_unaryOperator_prefix);
	NV_Lang_addOp(lang, 701,	"-", NV_LANG00_Op_unaryOperator_prefix);
	NV_Lang_addOp(lang, 701,	"!", NV_LANG00_Op_unaryOperator_prefix);
	// Multiplicative
	NV_Lang_addOp(lang, 600,	"*", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 600,	"/", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 600,	"%", NV_LANG00_Op_binaryOperator);
	// Additive
	NV_Lang_addOp(lang, 500,	"+", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 500,	"-", NV_LANG00_Op_binaryOperator);
	// Relational
	NV_Lang_addOp(lang, 400,	"<", NV_LANG00_Op_binaryOperator);	
	NV_Lang_addOp(lang, 400,	">", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	"<=", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	">=", NV_LANG00_Op_binaryOperator);
	// Equality
	NV_Lang_addOp(lang, 400,	"==", NV_LANG00_Op_binaryOperator);
	NV_Lang_addOp(lang, 400,	"!=", NV_LANG00_Op_binaryOperator);
	// Bitwise AND
	NV_Lang_addOp(lang, 340,	"&", NV_LANG00_Op_binaryOperator);
	// Bitwise XOR
	NV_Lang_addOp(lang, 330,	"^", NV_LANG00_Op_binaryOperator);
	// Bitwise OR
	NV_Lang_addOp(lang, 320,	"|", NV_LANG00_Op_binaryOperator);
	// Logical AND
	NV_Lang_addOp(lang, 310,	"&&", NV_LANG00_Op_binaryOperator);
	// Logical OR
	NV_Lang_addOp(lang, 300,	"||", NV_LANG00_Op_binaryOperator);
	// Assignment
	NV_Lang_addOp(lang, 201,	"+=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 201,	"-=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 201,	"*=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 201,	"/=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 201,	"%=", NV_LANG00_Op_compoundAssign);
	NV_Lang_addOp(lang, 201,	"=", NV_LANG00_Op_assign);
	//
	NV_Lang_addOp(lang, 50,	"print", NV_LANG00_Op_print);
	NV_Lang_addOp(lang, 52,	"showop", NV_LANG00_Op_showOpList);
	NV_Lang_addOp(lang, 52,	"vlist", NV_LANG00_Op_showVarList);
	NV_Lang_addOp(lang, 50,	"exit", NV_LANG00_Op_exit);
	NV_Lang_addOp(lang, 50,	"continue", NV_LANG00_Op_continue);
	NV_Lang_addOp(lang, 50,	"break", NV_LANG00_Op_break);
	//
	
	//
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
		NV_List_printAll(NV_Lang_getOpList(lang), "\n[\n", ",\n", "\n]\n");
#endif
	return lang;
}

