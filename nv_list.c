#include "nv.h"

struct NV_LIST_ITEM {
	NV_Pointer data;
	NV_Pointer prev;
	NV_Pointer next;
	int revision;
};

void NV_ListItem_setNext(NV_Pointer item, NV_Pointer nextItem);
void NV_ListItem_setPrev(NV_Pointer item, NV_Pointer prevItem);
void NV_ListItem_clearLink(NV_Pointer item);
//
// NV_Element
//

NV_ListItem *NV_E_allocListItem()
{
	NV_ListItem *li;
	li = NV_malloc(sizeof(NV_ListItem));
	li->data = NV_NullPointer;
	li->prev = NV_NullPointer;
	li->next = NV_NullPointer;
	li->revision = 0;
	return li;
}

void NV_E_free_internal_ListItem(NV_Pointer item)
{
	NV_Pointer data = NV_ListItem_getData(item);
	NV_E_free(&data);
	NV_ListItem_clearLink(item);
}

void NV_E_free_internal_List(NV_Pointer root)
{
	// free children
	NV_Pointer item;
	//
	for(;;){
		item = NV_List_getItemByIndex(root, 0);
		if(NV_E_isNullPointer(item)) break;
		NV_ListItem_clearLink(item);
		NV_E_free(&item);
	}
}

//
// Item operations
//
NV_Pointer NV_ListItem_getNext(NV_Pointer item)
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

NV_Pointer NV_ListItem_getPrev(NV_Pointer item)
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

NV_Pointer NV_ListItem_getData(NV_Pointer item)
{
	NV_Pointer retv;
	NV_ListItem *li;
	if(!NV_E_isType(item, EListItem)) return NV_NullPointer;
	li = NV_E_getRawPointer(item, EListItem);
	if(li)	retv = li->data;
	else	retv = NV_NullPointer;
	return retv;
}

void NV_ListItem_setData(NV_Pointer item, NV_Pointer data)
{
	// retains data, frees before data.
	NV_ListItem *li = NULL;
	NV_Pointer oldData = NV_NullPointer;
	if(NV_E_isType(item, EListItem)) li = NV_E_getRawPointer(item, EListItem);
	else if(NV_E_isType(item, EList)) li = NV_E_getRawPointer(item, EList);
	//
	if(li){
		oldData = li->data;
		li->data = NV_E_retain(data);
	}
	NV_E_free(&oldData);
	return;
}

void *NV_ListItem_getRawData(NV_Pointer item, NV_ElementType et)
{
	return NV_E_getRawPointer(NV_ListItem_getData(item), et);
}

int NV_ListItem_isDataType(NV_Pointer item, NV_ElementType et)
{
	return NV_E_isType(NV_ListItem_getData(item), et);
}

void NV_ListItem_convertUnknownToKnown(NV_Pointer vDict, NV_Pointer item)
{
	// if mayStr object is EString and EFUnknownToken (not string literal),
	// try to convert from string to variable,
	// if not, return original mayStr object.
	NV_Pointer new, data;
	data = NV_ListItem_getData(item);
	if(!NV_E_isType(data, EString) ||
		!NV_E_checkFlag(data, EFUnknownToken) ||
		!NV_Variable_isExisted(vDict, data)){
#ifdef DEBUG
		if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
			NV_DbgInfo("%s", "not converted");
		}
#endif
		NV_E_clearFlag(data, EFUnknownToken);
	} else{
#ifdef DEBUG
		if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
			NV_DbgInfo("%s", "converted to variable");
		}
#endif
		new = NV_Variable_allocExistedByStr(vDict, data);
		NV_ListItem_setData(item, NV_E_autorelease(new));
	}
}

void NV_ListItem_unbox(NV_Pointer item)
{
	NV_Pointer nData, cData;
	cData = NV_ListItem_getData(item);
	if(NV_E_isType(cData, EVariable)){
		nData = NV_Variable_getData(cData);
	} else if(NV_E_isType(cData, EListItem)){
		nData = NV_ListItem_getData(cData);
	} else if(NV_E_isType(cData, EDictItem)){
		nData = NV_DictItem_getVal(cData);
	} else{
		return;
	}
	NV_ListItem_setData(item, nData);
	NV_ListItem_unbox(item);
}

void NV_ListItem_convertToKnownUnboxed(NV_Pointer vDict, NV_Pointer item)
{
	NV_ListItem_convertUnknownToKnown(vDict, item);
	NV_ListItem_unbox(item);
}

void NV_ListItem_exchangeDataByIndex(NV_Pointer root, int i, int k)
{
	NV_Pointer a, b;
	a = NV_List_getItemByIndex(root, i);
	b = NV_List_getItemByIndex(root, k);
	NV_ListItem_exchangeData(a, b);
}

void NV_ListItem_exchangeData(NV_Pointer a, NV_Pointer b)
{
	NV_Pointer tmp;
	tmp = NV_E_retain(NV_ListItem_getData(a));
	NV_ListItem_setData(a, NV_ListItem_getData(b));
	NV_ListItem_setData(b, NV_E_autorelease(tmp));
	return;
}

void NV_ListItem_print(NV_Pointer t)
{
	NV_ListItem *li;
	li = NV_E_getRawPointer(t, EListItem);
	if(li){
		printf("(ListItem: ");
		NV_printElement(li->data);
	}
}

//
// List and Item operations (not data)
//
NV_Pointer NV_List_allocRoot()
{
	return NV_E_malloc_type(EList);	
}

NV_Pointer NV_List_clone(NV_Pointer p)
{
	NV_Pointer c, t, d;
	if(!NV_E_isType(p, EList)) return NV_NullPointer;
	c = NV_E_malloc_type(EList);
	//
	t = NV_ListItem_getNext(p);
	while(!NV_E_isNullPointer(t)){
		d = NV_E_clone(NV_ListItem_getData(t));
		NV_List_push(c, NV_E_autorelease(d));
		//
		t = NV_ListItem_getNext(t);
	}
	return c;
}

NV_Pointer NV_List_getItemByIndex(NV_Pointer rootItem, int i)
{
	NV_Pointer tItem;
	tItem = NV_ListItem_getNext(rootItem);
	do {
		if(i == 0) break;
		i--;
		tItem = NV_ListItem_getNext(tItem);
	} while(!NV_E_isNullPointer(tItem));
	return tItem;
}

NV_Pointer NV_List_getLastItem(NV_Pointer root)
{
	// root: EList
	if(!NV_E_isType(root, EList)/* && !NV_E_isType(root, EListItem)*/)
		return NV_NullPointer;
	//
	NV_Pointer lastItem;
	lastItem = NV_ListItem_getNext(root);
	while(!NV_E_isNullPointer(NV_ListItem_getNext(lastItem))){
		lastItem = NV_ListItem_getNext(lastItem);
	}
	return lastItem;
}

void NV_List_insertItemAfter(NV_Pointer prevItem, NV_Pointer newItem)
{
	if(NV_E_isNullPointer(newItem)){
		NV_Error("%s", "newItem is NULL!!!!");
		return;
	}
	NV_Pointer nextItem = NV_ListItem_getNext(prevItem);
	NV_ListItem_setNext(prevItem, newItem);
	NV_ListItem_setPrev(newItem, prevItem);
	NV_ListItem_setNext(newItem, nextItem);
	NV_ListItem_setPrev(nextItem, newItem);
}

void NV_List_insertAllAfter(NV_Pointer prevItem, NV_Pointer rootItem)
{
	// rootItem becomes empty.
	NV_Pointer firstItem, lastItem, nextItem;
	if(NV_E_isNullPointer(prevItem)){
		NV_Error("%s", "prevItem is NULL");
		return;
	}
	firstItem = NV_ListItem_getNext(rootItem);
	lastItem = NV_List_getLastItem(rootItem);
	nextItem = NV_ListItem_getNext(prevItem);
	// update link
	NV_ListItem_setNext(rootItem, NV_NullPointer);
	//
	NV_ListItem_setNext(prevItem, firstItem);
	NV_ListItem_setPrev(firstItem, prevItem);
	//
	NV_ListItem_setNext(lastItem, nextItem);
	NV_ListItem_setPrev(nextItem, lastItem);
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
	NV_ListItem_setNext(NV_ListItem_getPrev(dividerItem), NV_NullPointer);
	//
	NV_ListItem_setNext(retvRoot, dividerItem);
	NV_ListItem_setPrev(dividerItem, retvRoot);
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
	NV_ListItem_setData(newItem, newData);
	// Update link
	lastItem = NV_List_getLastItem(rootItem);
	if(NV_E_isNullPointer(lastItem)) lastItem = rootItem;
	NV_ListItem_setNext(lastItem, newItem);
	NV_ListItem_setPrev(newItem, lastItem);
}

NV_Pointer NV_List_pop(NV_Pointer pRoot)
{
	// [] ->
	// retains data
	NV_Pointer lastItem, lastData;
	//
	lastItem = NV_List_getLastItem(pRoot);
	if(NV_E_isNullPointer(lastItem)) return NV_NullPointer;
	lastData = NV_E_retain(NV_ListItem_getData(lastItem));
	//
	NV_ListItem_setNext(NV_ListItem_getPrev(lastItem), NV_NullPointer);
	NV_E_free(&lastItem);
	//
	return lastData;
}

NV_Pointer NV_List_shift(NV_Pointer rootItem)
{
	// <- []
	// retains data
	NV_Pointer data, item;
	item = NV_ListItem_getNext(rootItem);
	data = NV_E_retain(NV_ListItem_getData(item));
	NV_E_free(&item);
	return data;
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
	NV_ListItem_setData(newItem, newData);
	NV_List_insertItemAfter(itemInList, newItem);
}

void NV_List_insertDataBeforeItem(NV_Pointer itemInList, NV_Pointer newData)
{
	NV_List_insertDataAfterItem(NV_ListItem_getPrev(itemInList), newData);
}

void NV_List_insertDataAfterIndex(NV_Pointer root, int index, NV_Pointer newData)
{
	NV_List_insertDataAfterItem(NV_List_getItemByIndex(root, index), newData);
}

NV_Pointer NV_List_getDataByIndex(NV_Pointer rootItem, int i)
{
	return NV_ListItem_getData(NV_List_getItemByIndex(rootItem, i));
}

int NV_List_indexOfData(NV_Pointer root, NV_Pointer data)
{
	int i = 0;
	NV_Pointer li;
	for(li = NV_ListItem_getNext(root); !NV_E_isNullPointer(li); i++){
		if(NV_ListItem_getData(li).data == data.data) break;
		li = NV_ListItem_getNext(li);
	}
	if(NV_E_isNullPointer(li)) return -1;
	return i;
}

void NV_List_convertAllToKnownUnboxed(NV_Pointer scope, NV_Pointer root)
{
	NV_Pointer li;
	li = NV_ListItem_getNext(root);
	for(; !NV_E_isNullPointer(li); li = NV_ListItem_getNext(li)){
		NV_ListItem_convertUnknownToKnown(scope, li);
		NV_ListItem_unbox(li);
	}
}

void NV_List_removeItem(NV_Pointer item)
{
	NV_ListItem_clearLink(item);
	NV_E_free(&item);
}

void NV_List_sortAsc(NV_Pointer root, int (*cf)(NV_Pointer a, NV_Pointer b))
{
	// cf: b - a
	NV_Pointer li, lk;
	int i, k;
	for(i = 0; ; i++){
		li = NV_List_getItemByIndex(root, i);
		if(NV_E_isNullPointer(li)) break;
		for(k = i + 1; ; k++){
			li = NV_List_getItemByIndex(root, i);
			lk = NV_List_getItemByIndex(root, k);
			if(NV_E_isNullPointer(lk)) break;
			if(cf(NV_ListItem_getData(li), NV_ListItem_getData(lk)) < 0){
				NV_ListItem_exchangeData(li, lk);
			}
		}
	}
}
//
// Print
//

void NV_List_printAll(NV_Pointer root, const char *prefix, const char *delimiter, const char *suffix)
{
	NV_Pointer li;
	if(!prefix) 	prefix = "[";
	if(!delimiter)	delimiter = ", ";
	if(!suffix) 	suffix = "]";
	//printf("List(%p): \n", NV_E_getRawPointer(root, EList));
	printf("%s", prefix);
	for(li = NV_ListItem_getNext(root); !NV_E_isNullPointer(li);){
		NV_printElement(NV_ListItem_getData(li));
		//
		li = NV_ListItem_getNext(li);
		if(NV_E_isNullPointer(li)) break;
		printf("%s", delimiter);
	}
	printf("%s", suffix);
}

//
// InternalFunc
//

void NV_ListItem_setNext(NV_Pointer item, NV_Pointer nextItem)
{
	NV_ListItem *li;
	if(NV_E_isType(item, EListItem)) li = NV_E_getRawPointer(item, EListItem);
	else if(NV_E_isType(item, EList)) li = NV_E_getRawPointer(item, EList);
	else return;
	//
	if(li)	li->next = nextItem;
}

void NV_ListItem_setPrev(NV_Pointer item, NV_Pointer prevItem)
{
	NV_ListItem *li;
	if(NV_E_isType(item, EListItem)) li = NV_E_getRawPointer(item, EListItem);
	else if(NV_E_isType(item, EList)) li = NV_E_getRawPointer(item, EList);
	else return;
	//
	if(li)	li->prev = prevItem;
}

void NV_ListItem_clearLink(NV_Pointer item)
{
	NV_Pointer prevItem, nextItem;
	//
	if(NV_E_isNullPointer(item)) return;
	//
	prevItem = NV_ListItem_getPrev(item);
	nextItem = NV_ListItem_getNext(item);
	NV_ListItem_setNext(prevItem, nextItem);
	NV_ListItem_setPrev(nextItem, prevItem);
	return;
}
