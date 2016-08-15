#include "nv.h"

struct NV_LIST {
	NV_Pointer data;
	NV_Pointer prev;
	NV_Pointer next;	
};

NV_List *NV_allocList()
{
	NV_List *li;
	li = NV_malloc(sizeof(NV_List));
	li->data = NV_NullPointer;
	li->prev = NV_NullPointer;
	li->next = NV_NullPointer;
	return li;
}

NV_Pointer NV_List_getNextItem(NV_Pointer item)
{
	NV_Pointer retv;
	NV_List *li = NV_E_getRawPointer(item, EList);
	if(li)	retv = li->next;
	else	retv = NV_NullPointer;
	return retv;
}

NV_Pointer NV_List_getPrevItem(NV_Pointer item)
{
	NV_Pointer retv;
	NV_List *li = NV_E_getRawPointer(item, EList);
	if(li)	retv = li->prev;
	else	retv = NV_NullPointer;
	return retv;
}

void NV_List_push(NV_Pointer *pRoot, NV_Pointer pData)
{
	NV_Pointer pTarget, pNew;
	NV_List *liTarget, *liNew;
	//
	pNew = NV_E_malloc_type(EList);
	liNew = NV_E_getRawPointer(pNew, EList);
	if(NV_E_isNullPointer(*pRoot)){
		*pRoot = pNew;
	} else{
		pTarget = *pRoot;
		while(!NV_E_isNullPointer(NV_List_getNextItem(pTarget))){
			pTarget = NV_List_getNextItem(pTarget);
		}
		liTarget = NV_E_getRawPointer(pTarget, EList);
		liTarget->next = pNew;
		liNew->prev = pTarget;
	}
	liNew->data = pData;
}

NV_Pointer NV_List_getItemData(NV_Pointer item)
{
	NV_Pointer retv;
	NV_List *li = NV_E_getRawPointer(item, EList);
	if(li)	retv = li->data;
	else	retv = NV_NullPointer;
	return retv;
}

void *NV_List_getItemRawData(NV_Pointer item, NV_ElementType et)
{
	return NV_E_getRawPointer(NV_List_getItemData(item), et);
}

NV_Pointer NV_List_setItemData(NV_Pointer item, NV_Pointer newData)
{
	NV_Pointer oldData;
	NV_List *li = NV_E_getRawPointer(item, EList);
	if(li){
		oldData = li->data;
		li->data = newData;
	} else{
		oldData = NV_NullPointer;
	}
	return oldData;
}

int NV_List_isItemType(NV_Pointer item, NV_ElementType et)
{
	return NV_E_isType(NV_List_getItemData(item), et);
}

//
// Term
//


void NV_List_printAll(NV_Pointer root, const char *delimiter)
{
	NV_Pointer li;
	//printf("List(%p): \n", NV_E_getRawPointer(root, EList));
	printf("[");
	for(li = root; !NV_E_isNullPointer(li); li = NV_List_getNextItem(li)){
		if(li.data != root.data) printf("%s", delimiter);
		NV_printElement(NV_List_getItemData(li));
	}
	printf("]\n");
}
