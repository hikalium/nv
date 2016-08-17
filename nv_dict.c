#include "nv.h"

struct NV_DICT_ITEM {
	NV_Pointer key;
	NV_Pointer val;
	NV_Pointer prev;
	NV_Pointer next;	
};

NV_DictItem *NV_allocDictItem()
{
	NV_DictItem *di;
	di = NV_malloc(sizeof(NV_DictItem));
	di->key = NV_NullPointer;
	di->val = NV_NullPointer;
	di->prev = NV_NullPointer;
	di->next = NV_NullPointer;
	return di;
}

NV_DictItem * NV_DictItem_getRawDictItem(NV_Pointer item);
//
NV_Pointer NV_DictItem_getPrev(NV_Pointer item);
NV_Pointer NV_DictItem_getNext(NV_Pointer item);
void NV_DictItem_setPrev(NV_Pointer item, NV_Pointer prev);
void NV_DictItem_setNext(NV_Pointer item, NV_Pointer next);
void NV_DictItem_setKey(NV_Pointer item, NV_Pointer key);
void NV_DictItem_setVal(NV_Pointer item, NV_Pointer val);

//
// Item operations (not data)
//

NV_Pointer NV_Dict_allocRoot()
{
	return NV_E_malloc_type(EDict);	
}
/*
NV_Pointer NV_Dict_getNextItem(NV_Pointer item)
{
	// item: EList, EListItem
	NV_Pointer retv;
	NV_ListItem *li;
	if(NV_E_isType(item, EListItem)) li = NV_E_getRawPointer(item, EListItem);
	else if(NV_E_isType(item, EList)) li = NV_E_getRawPointer(item, EList);
	else return NV_NullPointer;
	//
	if(li)	retv = li->next;
	else	retv = NV_NullPointer;
	return retv;
}

NV_Pointer NV_List_getPrevItem(NV_Pointer item)
{
	// item: EList, EListItem
	NV_Pointer retv;
	NV_ListItem *li;
	if(NV_E_isType(item, EListItem)) li = NV_E_getRawPointer(item, EListItem);
	else if(NV_E_isType(item, EList)) li = NV_E_getRawPointer(item, EList);
	else return NV_NullPointer;
	//
	if(li)	retv = li->prev;
	else	retv = NV_NullPointer;
	return retv;
}

NV_Pointer NV_List_getItemByIndex(NV_Pointer rootItem, int i)
{
	NV_Pointer tItem;
	tItem = NV_List_getNextItem(rootItem);
	do {
		if(i == 0) break;
		i--;
		tItem = NV_List_getNextItem(tItem);
	} while(!NV_E_isNullPointer(tItem));
	return tItem;
}
*/
/*
NV_Pointer NV_List_lastItem(NV_Pointer root)
{
	// root: EList
	if(!NV_E_isType(root, EList))
		return NV_NullPointer;
	//
	NV_Pointer lastItem;
	lastItem = NV_List_getNextItem(root);
	while(!NV_E_isNullPointer(NV_List_getNextItem(lastItem))){
		lastItem = NV_List_getNextItem(lastItem);
	}
	return lastItem;
}
*/
/*
NV_Pointer NV_List_removeItem(NV_Pointer item)
{
	// retv: data of item removed.
	// note: this func does not free data of the item.
	// you should free data manually
	NV_Pointer tData;
	NV_Pointer prevItem, nextItem;
	//
	if(NV_E_isNullPointer(item)) return NV_NullPointer;
	//
	tData = NV_List_getItemData(item);
	prevItem = NV_List_getPrevItem(item);
	nextItem = NV_List_getNextItem(item);
	NV_List_setNextItem(prevItem, nextItem);
	NV_List_setPrevItem(nextItem, prevItem);
	return tData;
}

NV_Pointer NV_List_removeItemByIndex(NV_Pointer rootItem, int i)
{
	// retv: data of item removed.
	// note: this func does not free data of the item.
	// you should free data manually
	NV_Pointer tItem;
	//
	if(!NV_E_isValidPointer(rootItem)) return NV_NullPointer;
	tItem = NV_List_getItemByIndex(rootItem, i);
	return NV_List_removeItem(tItem);
}

void NV_List_insertItemAfter(NV_Pointer prevItem, NV_Pointer newItem)
{
	if(NV_E_isNullPointer(newItem)){
		NV_Error("%s", "newItem is NULL!!!!");
		return;
	}
	NV_Pointer nextItem = NV_List_getNextItem(prevItem);
	NV_List_setNextItem(prevItem, newItem);
	NV_List_setPrevItem(newItem, prevItem);
	NV_List_setNextItem(newItem, nextItem);
	NV_List_setPrevItem(nextItem, newItem);
}

void NV_List_insertAllAfter(NV_Pointer prevItem, NV_Pointer rootItem)
{
	// rootItem becomes empty.
	NV_Pointer firstItem, lastItem, nextItem;
	if(NV_E_isNullPointer(prevItem)){
		NV_Error("%s", "prevItem is NULL");
		return;
	}
	firstItem = NV_List_getNextItem(rootItem);
	lastItem = NV_List_lastItem(rootItem);
	nextItem = NV_List_getNextItem(prevItem);
	// update link
	NV_List_setNextItem(rootItem, NV_NullPointer);
	//
	NV_List_setNextItem(prevItem, firstItem);
	NV_List_setPrevItem(firstItem, prevItem);
	//
	NV_List_setNextItem(lastItem, nextItem);
	NV_List_setPrevItem(nextItem, lastItem);
}

void NV_List_insertAllAfterIndex(NV_Pointer dstRoot, int index, NV_Pointer rootItem)
{
	NV_List_insertAllAfter(NV_List_getItemByIndex(dstRoot, index), rootItem);
}

NV_Pointer NV_List_divideBefore(NV_Pointer dividerItem)
{
	// in:	[A, ..., dividerItem, ..., B]
	// out: [A, ...] and retv = [dividerItem, ..., B]
	NV_Pointer retvRoot;
	retvRoot = NV_List_allocRoot();
	//
	NV_List_setNextItem(NV_List_getPrevItem(dividerItem), NV_NullPointer);
	//
	NV_List_setNextItem(retvRoot, dividerItem);
	NV_List_setPrevItem(dividerItem, retvRoot);
	return retvRoot;
}

//
// Data operations
//

void NV_List_push(NV_Pointer rootItem, NV_Pointer newData)
{
	// [] <-
	NV_Pointer newItem, lastItem;
	//
	newItem = NV_E_malloc_type(EListItem);
	NV_List_setData(newItem, newData);
	// Update link
	lastItem = NV_List_lastItem(rootItem);
	if(NV_E_isNullPointer(lastItem)) lastItem = rootItem;
	NV_List_setNextItem(lastItem, newItem);
	NV_List_setPrevItem(newItem, lastItem);
}

NV_Pointer NV_List_pop(NV_Pointer pRoot)
{
	// [] ->
	NV_Pointer lastItem, lastData;
	//
	lastItem = NV_List_lastItem(pRoot);
	if(NV_E_isNullPointer(lastItem)) return NV_NullPointer;
	lastData = NV_List_getItemData(lastItem);
	//
	NV_List_setNextItem(NV_List_getPrevItem(lastItem), NV_NullPointer);
	NV_E_free(&lastItem);
	//
	return lastData;
}

NV_Pointer NV_List_shift(NV_Pointer rootItem)
{
	// <- []
	return NV_List_removeItemByIndex(rootItem, 0);
}

void NV_List_unshift(NV_Pointer rootItem, NV_Pointer newData)
{
	// -> []
	NV_List_insertDataAfterItem(rootItem, newData);
}

void NV_List_insertDataAfterItem(NV_Pointer itemInList, NV_Pointer newData)
{
	NV_Pointer newItem;
	//
	if(NV_E_isNullPointer(newData)){
		NV_Error("%s", "newData is NULL!!!!!");
		return;
	}
	newItem = NV_E_malloc_type(EListItem);
	NV_List_setData(newItem, newData);
	NV_List_insertItemAfter(itemInList, newItem);
}

void NV_List_insertDataBeforeItem(NV_Pointer itemInList, NV_Pointer newData)
{
	NV_List_insertDataAfterItem(NV_List_getPrevItem(itemInList), newData);
}

void NV_List_insertDataAfterIndex(NV_Pointer root, int index, NV_Pointer newData)
{
	NV_List_insertDataAfterItem(NV_List_getItemByIndex(root, index), newData);
}

NV_Pointer NV_List_getItemData(NV_Pointer item)
{
	NV_Pointer retv;
	NV_ListItem *li;
	if(!NV_E_isType(item, EListItem)) return NV_NullPointer;
	li = NV_E_getRawPointer(item, EListItem);
	if(li)	retv = li->data;
	else	retv = NV_NullPointer;
	return retv;
}

NV_Pointer NV_List_getDataByIndex(NV_Pointer rootItem, int i)
{
	return NV_List_getItemData(NV_List_getItemByIndex(rootItem, i));
}

void *NV_List_getItemRawData(NV_Pointer item, NV_ElementType et)
{
	return NV_E_getRawPointer(NV_List_getItemData(item), et);
}

int NV_List_isItemType(NV_Pointer item, NV_ElementType et)
{
	return NV_E_isType(NV_List_getItemData(item), et);
}

int NV_List_indexOfData(NV_Pointer root, NV_Pointer data)
{
	int i = 0;
	NV_Pointer li;
	for(li = NV_List_getNextItem(root); !NV_E_isNullPointer(li); i++){
		if(NV_List_getItemData(li).data == data.data) break;
		li = NV_List_getNextItem(li);
	}
	if(NV_E_isNullPointer(li)) return -1;
	return i;
}

NV_Pointer NV_List_setItemData(NV_Pointer item, NV_Pointer newData)
{
	NV_Pointer oldData = NV_List_getItemData(item);
	NV_List_setData(item, newData);
	return oldData;
}

//
// Print
//

void NV_ListItem_print(NV_Pointer t)
{
	NV_ListItem *li;
	li = NV_E_getRawPointer(t, EListItem);
	if(li){
		printf("(ListItem: data = %p)", li->data.data);
	}
}

void NV_List_printAll(NV_Pointer root, const char *prefix, const char *delimiter, const char *suffix)
{
	NV_Pointer li;
	if(!prefix) 	prefix = "[";
	if(!delimiter)	delimiter = ", ";
	if(!suffix) 	suffix = "]";
	//printf("List(%p): \n", NV_E_getRawPointer(root, EList));
	printf("%s", prefix);
	for(li = NV_List_getNextItem(root); !NV_E_isNullPointer(li);){
		NV_printElement(NV_List_getItemData(li));
		//
		li = NV_List_getNextItem(li);
		if(NV_E_isNullPointer(li)) break;
		printf("%s", delimiter);
	}
	printf("%s", suffix);
}
*/

int NV_Dict_add(NV_Pointer dict, NV_Pointer key, NV_Pointer val)
{
	// retv: error?
	// you can not add data which (key or val) == NV_NullPointer.
	// if the same key exists, data will be overwritten by a new val.
	NV_Pointer item, firstItem;
	if(!NV_E_isType(dict, EDict)) return 1;
	if(NV_E_isNullPointer(key) || NV_E_isNullPointer(val)) return 1;
	//
	firstItem = NV_DictItem_getNext(dict);
	item = NV_Dict_getItemByKey(dict, key);
	if(NV_E_isNullPointer(item)){
		// alloc item and create link
		item = NV_E_malloc_type(EDictItem);
		NV_DictItem_setKey(item, key);
		NV_DictItem_setNext(dict, item);
		NV_DictItem_setPrev(item, dict);
		NV_DictItem_setNext(item, firstItem);
		NV_DictItem_setPrev(firstItem, item);
	}
	// update val.
	NV_DictItem_setVal(item, val);
	return 0;
}

NV_Pointer NV_Dict_getItemByKey(NV_Pointer dict, NV_Pointer key)
{
	NV_Pointer di;
	di = NV_List_getNextItem(dict);
	for(; !NV_E_isNullPointer(di); di = NV_List_getNextItem(di)){
		if(NV_E_isEqual(key, NV_DictItem_getKey(di))) break;
	}
	return di;
}

NV_Pointer NV_Dict_getValByKey(NV_Pointer dict, NV_Pointer key)
{
	return NV_DictItem_getVal(NV_Dict_getItemByKey(dict, key));
}

NV_Pointer NV_DictItem_getKey(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	return di->key;
	return NV_NullPointer;
}

NV_Pointer NV_DictItem_getVal(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	return di->val;
	return NV_NullPointer;
}

//
// InternalFunc
//

NV_DictItem * NV_DictItem_getRawDictItem(NV_Pointer item)
{
	if(NV_E_isType(item, EDictItem)) return NV_E_getRawPointer(item, EDictItem);
	else if(NV_E_isType(item, EDict)) return NV_E_getRawPointer(item, EDict);
	else return NULL;
}

NV_Pointer NV_DictItem_getPrev(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	return di->prev;
	return NV_NullPointer;
}

NV_Pointer NV_DictItem_getNext(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	return di->next;
	return NV_NullPointer;
}

void NV_DictItem_setPrev(NV_Pointer item, NV_Pointer prev)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->next = prev;
}

void NV_DictItem_setNext(NV_Pointer item, NV_Pointer next)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->next = next;
}

void NV_DictItem_setKey(NV_Pointer item, NV_Pointer key)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->next = key;
}

void NV_DictItem_setVal(NV_Pointer item, NV_Pointer val)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->next = val;
}
