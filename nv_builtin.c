#include "nv.h"

//
// builtin support functions
//
int
NV_Util_execItem
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer sentenceRootItem)
{
	// eval sentence Item. 
	NV_Pointer sentenceRoot;
	//
	if(!NV_ListItem_isDataType(sentenceRootItem, EList)){
		NV_Error("%s", "sentenceRootItem is not EList");
		NV_printElement(sentenceRootItem);
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return 1;
	}
	sentenceRoot = NV_ListItem_getData(sentenceRootItem);
	//
	NV_evaluateSentence(excFlag, lang, vDict, sentenceRoot);
	if(*excFlag & NV_EXC_FLAG_FAILED){
		NV_Error("%s", "Exec failed.");
		return 1;
	}
	NV_List_convertAllToKnownUnboxed(
		vDict, sentenceRoot);
	return 0;
}

int
NV_Util_execItemScalar
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer sentenceRootItem)
{
	// eval sentence
	// sentenceRootItem will be repaced by last value of eval tree.
	if(NV_Util_execItem(excFlag, lang, vDict, sentenceRootItem))
		return 1;
	NV_ListItem_setData(sentenceRootItem,
		NV_ListItem_getData(
			NV_List_getLastItem(
				NV_ListItem_getData(sentenceRootItem))));
	return 0;
}



//
// builtin operators
//

NV_Pointer
NV_Op_builtin_exec
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_exec [EList]
	// -> [EList]
	NV_Pointer nextItem;
	nextItem = NV_ListItem_getNext(thisItem);
	NV_Util_execItem(excFlag, lang, vDict, nextItem);
	if(*excFlag & NV_EXC_FLAG_FAILED)
		return NV_NullPointer;
	NV_E_free(&thisItem);
	return nextItem;
}

NV_Pointer
NV_Op_builtin_exec_scalar
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_exec_scalar [EList]
	// -> [Item]
	NV_Pointer nextItem;
	nextItem = NV_ListItem_getNext(thisItem);
	if(NV_Util_execItemScalar(excFlag, lang, vDict, nextItem))
		 return NV_NullPointer;
	NV_E_free(&thisItem);
	return nextItem;
}

NV_Pointer
NV_Op_builtin_pop
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_pop [EList]
	// -> [Item]
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_ListItem_convertToKnownUnboxed(vDict, nextItem);
	if(!NV_ListItem_isDataType(nextItem, EList)){
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return NV_NullPointer;
	}
	NV_ListItem_setData(nextItem, NV_List_pop(NV_ListItem_getData(nextItem)));
	NV_E_free(&thisItem);
	return nextItem;
}

NV_Pointer
NV_Op_builtin_get_item
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// [EList | EDict] builtin_get_item [Item]
	// -> [Item]
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
	} else if(NV_E_isType(prevData, EDict)){
		target = NV_Dict_getItemByKey(prevData, nextData);
		if(NV_E_isNullPointer(target)){
			// if key not found, add entry
			NV_Dict_add(prevData, nextData, NV_E_autorelease(NV_Integer_alloc(0)));
			target = NV_Dict_getItemByKey(prevData, nextData);
		}
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

