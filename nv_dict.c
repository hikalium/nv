#include "nv.h"

//
// NV_DictItem
//
struct NV_DICT_ITEM {
	NV_Pointer key;
	NV_Pointer val;
	NV_Pointer prev;
	NV_Pointer next;
	int32_t revision;
};

//
// Internal functions
//
NV_DictItem * NV_DictItem_getRawDictItem(NV_Pointer item);
//
NV_Pointer NV_DictItem_getPrev(NV_Pointer item);
NV_Pointer NV_DictItem_getNext(NV_Pointer item);
void NV_DictItem_setPrev(NV_Pointer item, NV_Pointer prev);
void NV_DictItem_setNext(NV_Pointer item, NV_Pointer next);
void NV_DictItem_setKey(NV_Pointer item, NV_Pointer key);
void NV_DictItem_unlink(NV_Pointer item);

//
// NV_Element
//
NV_DictItem *NV_E_allocDictItem()
{
	NV_DictItem *di;
	di = NV_malloc(sizeof(NV_DictItem));
	di->key = NV_NullPointer;
	di->val = NV_NullPointer;
	di->prev = NV_NullPointer;
	di->next = NV_NullPointer;
	return di;
}

NV_Pointer NV_Dict_clone(NV_Pointer p)
{
	NV_Pointer c, t, k, v;
	if(!NV_E_isType(p, EDict)) return NV_NullPointer;
	c = NV_E_malloc_type(EDict);
	//
	t = NV_DictItem_getNext(p);
	while(!NV_E_isNullPointer(t)){
		k = NV_E_clone(NV_DictItem_getKey(t));
		v = NV_E_clone(NV_DictItem_getVal(t));
		NV_Dict_add(c, k, v);
		//
		t = NV_DictItem_getNext(t);
	}
	return c;
}

void NV_E_free_internal_DictItem(NV_Pointer item)
{
	NV_Pointer d;
	if(!NV_E_isType(item, EDictItem)) return;
	//
	NV_DictItem_unlink(item);
	d = NV_DictItem_getKey(item); NV_E_free(&d);
	d = NV_DictItem_getVal(item); NV_E_free(&d);
}

void NV_E_free_internal_Dict(NV_Pointer dict)
{
	NV_Pointer item;
	if(!NV_E_isType(dict, EDict)) return;
	//
	for(;;){
		item = NV_DictItem_getNext(dict);
		if(NV_E_isNullPointer(item)) break;
		NV_E_free(&item);
	}
}

//
// NV_Dict / NV_DictItem
//

NV_Pointer NV_Dict_allocRoot()
{
	return NV_E_malloc_type(EDict);	
}

int NV_Dict_add(NV_Pointer dict, NV_Pointer key, NV_Pointer val)
{
	// key will be cloned.
	// val will retain.
	// retv: error?
	// you can not add data which (key or val) == NV_NullPointer.
	// if the same key exists, data will be overwritten by a new val.
	NV_Pointer item, nextItem;
	if(!NV_E_isType(dict, EDict)) return 1;
	if(NV_E_isNullPointer(key) || NV_E_isNullPointer(val)) return 1;
	//
	nextItem = NV_DictItem_getNext(dict);
	item = NV_Dict_getItemByKey(dict, key);
	if(NV_E_isNullPointer(item)){
		// alloc item and create link
		key = NV_E_clone(key);
		item = NV_E_malloc_type(EDictItem);
		NV_DictItem_setKey(item, key);
		NV_DictItem_setNext(dict, item);
		NV_DictItem_setPrev(item, dict);
		NV_DictItem_setNext(item, nextItem);
		NV_DictItem_setPrev(nextItem, item);
	}
	// update val.
	NV_DictItem_setVal(item, val);
	return 0;
}

void NV_Dict_removeItem(NV_Pointer item)
{
	NV_DictItem_unlink(item);
	NV_E_free(&item);
}

NV_Pointer NV_Dict_getItemByKey(NV_Pointer dict, NV_Pointer key)
{
	NV_Pointer di;
	if(!NV_E_isType(dict, EDict)) return NV_NullPointer;
	di = NV_DictItem_getNext(dict);
	for(; !NV_E_isNullPointer(di); di = NV_DictItem_getNext(di)){
		if(NV_E_isEqual(key, NV_DictItem_getKey(di))) break;
	}
	return di;
}

NV_Pointer NV_Dict_getValByKey(NV_Pointer dict, NV_Pointer key)
{
	if(!NV_E_isType(dict, EDict)) return NV_NullPointer;
	return NV_DictItem_getVal(NV_Dict_getItemByKey(dict, key));
}

NV_Pointer NV_Dict_getValByKeyCStr(NV_Pointer dict, const char *kc)
{
	NV_Pointer key, val;
	key = NV_String_alloc(kc);
	val = NV_Dict_getValByKey(dict, key);
	NV_E_free(&key);
	//
	return val;
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

int32_t NV_DictItem_getRevision(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	return di->revision;
	return -1;
}

void NV_DictItem_updateRevision(NV_Pointer item)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->revision++;
}

void NV_DictItem_setVal(NV_Pointer item, NV_Pointer val)
{
	// retains val
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di){
		NV_E_free(&di->val);
		di->val = NV_E_retain(val);
	}
	NV_DictItem_updateRevision(item);
}

void NV_DictItem_print(NV_Pointer item)
{
	if(!NV_E_isType(item, EDictItem)){
		NV_Error("%s", "Type mismatched.");
		NV_printElement(item);
		return;
	};
	NV_printElement(NV_DictItem_getKey(item));
	printf(": ");
	NV_printElement(NV_DictItem_getVal(item));
}

void NV_Dict_printAll(NV_Pointer dict, const char *prefix, const char *delimiter, const char *suffix)
{
	NV_Pointer di;
	//
	if(!NV_E_isType(dict, EDict)) return;
	//
	if(!prefix)		prefix = "[";
	if(!delimiter)	delimiter = ", ";
	if(!suffix)		suffix = "]";
	//
	printf("%s", prefix);
	for(di = NV_DictItem_getNext(dict); !NV_E_isNullPointer(di);){
		NV_DictItem_print(di);
		//
		di = NV_DictItem_getNext(di);
		if(NV_E_isNullPointer(di)) break;
		printf("%s", delimiter);
	}
	printf("%s", suffix);
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
	if(di)	di->prev = prev;
}

void NV_DictItem_setNext(NV_Pointer item, NV_Pointer next)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->next = next;
}

void NV_DictItem_setKey(NV_Pointer item, NV_Pointer key)
{
	NV_DictItem *di = NV_DictItem_getRawDictItem(item);
	if(di)	di->key = key;
}

void NV_DictItem_unlink(NV_Pointer item)
{
	NV_Pointer prevItem, nextItem;
	//
	if(!NV_E_isType(item, EDictItem)) return;
	//
	prevItem = NV_DictItem_getPrev(item);
	nextItem = NV_DictItem_getNext(item);
	NV_DictItem_setNext(prevItem, nextItem);
	NV_DictItem_setPrev(nextItem, prevItem);
}


