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

void
NV_Op_builtin_exec
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_exec [EList]
	// -> [EList]
	NV_Pointer nextItem;
	nextItem = NV_ListItem_getNext(thisItem);
	NV_E_free(&thisItem);
	NV_Util_execItem(excFlag, lang, vDict, nextItem);
}

void
NV_Op_builtin_exec_scalar
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_exec_scalar [EList]
	// -> [Item]
	NV_Pointer nextItem;
	nextItem = NV_ListItem_getNext(thisItem);
	NV_E_free(&thisItem);
	NV_Util_execItemScalar(excFlag, lang, vDict, nextItem);
}

void
NV_Op_builtin_push
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_push [EList] [Data]
	// -> (nothing)
	NV_Pointer listItem = NV_ListItem_getNext(thisItem);
	NV_Pointer dataItem = NV_ListItem_getNext(listItem);
	NV_ListItem_convertToKnownUnboxed(vDict, listItem);
	NV_ListItem_convertToKnownUnboxed(vDict, dataItem);
	if(!NV_ListItem_isDataType(listItem, EList)){
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	NV_List_push(NV_ListItem_getData(listItem), NV_ListItem_getData(dataItem));
	NV_E_free(&thisItem);
	NV_E_free(&listItem);
	NV_E_free(&dataItem);
}

void
NV_Op_builtin_pop
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// builtin_pop [EList]
	// -> [Item]
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_ListItem_convertToKnownUnboxed(vDict, nextItem);
	if(!NV_ListItem_isDataType(nextItem, EList)){
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	NV_ListItem_setData(nextItem, NV_List_pop(NV_ListItem_getData(nextItem)));
	NV_E_free(&thisItem);
}

void
NV_Op_builtin_del_index
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// del_index [EList] [EInteger]
	// -> nothing
	NV_Pointer listItem = NV_ListItem_getNext(thisItem);
	NV_Pointer indexItem = NV_ListItem_getNext(listItem);
	NV_Pointer list, index, item;
	NV_ListItem_convertToKnownUnboxed(vDict, listItem);
	if(!NV_ListItem_isDataType(listItem, EList)){
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	list = NV_ListItem_getData(listItem);
	index = NV_ListItem_getData(indexItem);
	item = NV_List_getItemByIndex(list, NV_Integer_getImm32(index));
	NV_E_free(&item);
	NV_E_free(&thisItem);
	NV_E_free(&listItem);
	NV_E_free(&indexItem);
}

void
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
			SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
			NV_Error("%s", "Can't use this type of index.");
			return;
		}
	} else if(NV_E_isType(prevData, EDict)){
		target = NV_Dict_getItemByKey(prevData, nextData);
		if(NV_E_isNullPointer(target)){
			// if key not found, add entry
			NV_Dict_add(prevData, nextData, NV_E_autorelease(NV_Integer_alloc(0)));
			target = NV_Dict_getItemByKey(prevData, nextData);
		}
	} else{
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		NV_Error("%s", "data is not enumerable.");
		NV_printElement(prevData);
		return;
	}
	var = NV_E_malloc_type(EVariable);
	NV_Variable_setTarget(var, target);
	//
	NV_ListItem_setData(thisItem, NV_E_autorelease(var));
	NV_E_free(&prev);
	NV_E_free(&next);
}

void
NV_Op_builtin_var_dump
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// dump [object]
	// -> nothing
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_Pointer nextData;
	//
	NV_ListItem_convertUnknownToKnown(vDict, nextItem);
	nextData = NV_ListItem_getData(nextItem);
	if(NV_E_isType(nextData, EVariable)){
		nextData = NV_Variable_getData(nextData);
	}
	NV_printElement(nextData);
	printf("\n");
	NV_E_free(&thisItem);
	NV_E_free(&nextItem);
	CLR_FLAG(*excFlag, NV_EXC_FLAG_AUTO_PRINT);
}

void
NV_Op_builtin_remove_item
(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer thisItem)
{
	// dump [object]
	// -> nothing
	NV_Pointer nextItem = NV_ListItem_getNext(thisItem);
	NV_Pointer nextData;
	//
	NV_ListItem_convertUnknownToKnown(vDict, nextItem);
	nextData = NV_ListItem_getData(nextItem);
	if(NV_E_isType(nextData, EVariable)){
		nextData = NV_Variable_getData(nextData);
	}
	NV_List_removeItem(nextData);
	NV_E_free(&thisItem);
	NV_E_free(&nextItem);
}

