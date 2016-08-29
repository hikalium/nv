#include "nv.h"
#include "nv_rawelem.h"

#define NV_ELEMENT_TYPES	11
#define NV_DEBUG_MEMORY		1

struct NV_ELEMENT {
	NV_Pointer pool;	// EList
    NV_ElementType type;
    int32_t token;
	int32_t flag;
    void *data;
};

NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data);
void NV_E_free_internal_ListItem(NV_Pointer item, NV_Pointer pool);
void NV_E_free_internal_List(NV_Pointer root, NV_Pointer pool);

NV_Element *freeRoot = NULL;
int NV_E_NumOfElementsAlloced;
int NV_E_NumOfElementsUsing;
int NV_E_NumOfElements[NV_ELEMENT_TYPES];

const NV_Pointer NV_NullPointer = {
	0,
	NULL
};

NV_Pointer NV_E_malloc_type(NV_ElementType type)
{
	NV_Pointer p;
	switch(type){
		case EList:
			return NV_E_malloc_internal(EList,		NV_E_allocListItem());
		case EListItem:
			return NV_E_malloc_internal(EListItem,	NV_E_allocListItem());
		case EDict:
			return NV_E_malloc_internal(EDict,		NV_E_allocDictItem());
		case EDictItem:
			return NV_E_malloc_internal(EDictItem,	NV_E_allocDictItem());
		case EVariable:
			return NV_E_malloc_internal(EVariable,	NV_E_allocVariable());
		case EEnv:
			return NV_E_malloc_internal(EEnv,		NV_E_allocEnv());
		case ELang:
			return NV_E_malloc_internal(ELang,		NV_E_allocLang());
		case EOperator:
			return NV_E_malloc_internal(EOperator,	NV_E_allocOperator());
		case EInteger:
			return NV_E_malloc_internal(EInteger, 	NV_E_allocInteger());
		case EString:
			return NV_E_malloc_internal(EString, 	NV_E_allocString());
		default:
			NV_Error("Unknown element type %d\n", type);
			p = NV_NullPointer;
	}
	return p;
}

int NV_E_isNullPointer(NV_Pointer p)
{
	if(p.data == NULL) return 1;
	return 0;
}

void NV_E_free(NV_Pointer *p)
{
	NV_E_freeWithPool(p, NV_NullPointer);
}

void NV_E_freeWithPool(NV_Pointer *p, NV_Pointer pool)
{
	NV_Element *e;
	//
	if(!p) return;
	if(!NV_E_isValidPointer(*p)) return;
	e = p->data;
	//NV_DbgInfo("fe: %d %d %d %d %d", e->type, 
	//NV_E_isNullPointer(e->pool), NV_E_isSamePointer(e->pool, pool),
	//NV_E_isType(*p, EListItem), NV_E_isType(*p, EDictItem));
	if(NV_E_isNullPointer(e->pool) || NV_E_isSamePointer(e->pool, pool)){
		
		// free contents in complex type
		if(NV_E_isType(*p, EList)){
			NV_E_free_internal_List(*p, pool);
		} else if(NV_E_isType(*p, EListItem)){
			NV_E_free_internal_ListItem(*p, pool);
		} else if(NV_E_isType(*p, EDict)){
			NV_E_free_internal_Dict(*p, pool);
		} else if(NV_E_isType(*p, EDictItem)){
			NV_E_free_internal_DictItem(*p, pool);
		} else if(NV_E_isType(*p, EVariable)){
			NV_E_free_internal_Variable(*p, pool);
		} else if(NV_E_isType(*p, EEnv)){
			NV_E_free_internal_Env(*p, pool);
		} else if(NV_E_isType(*p, ELang)){
			NV_E_free_internal_Lang(*p, pool);
		} else if(NV_E_isType(*p, EString)){
			NV_E_free_internal_String(*p, pool);
		}
		//
		if(e->type < NV_ELEMENT_TYPES) NV_E_NumOfElements[e->type]--;
		NV_E_NumOfElementsUsing--;
#if NV_DEBUG_MEMORY
		NV_DbgInfo("free elem! (type: %d)", e->type);
#endif
		e->token = rand();
		e->type = ENone;
		NV_free(e->data);
		// add freeRoot to reuse
		e->data = freeRoot;
		freeRoot = e;
		//
		*p = NV_NullPointer;
	}
}

int NV_E_isValidPointer(NV_Pointer p)
{
	NV_Element *e = p.data;
	if(!e){
		//NV_Error("NULL pointer passed: token %d\n", p.token);
		return 0;
	}
	if(e->token != p.token){
		return 0;
	}
	return 1;
}

int NV_E_isType(NV_Pointer p, NV_ElementType et)
{
	if(!NV_E_isValidPointer(p)) return 0;
	if(p.data->type != et) return 0;
	return 1;
}

int NV_E_isSamePointer(NV_Pointer p, NV_Pointer q)
{
	return (p.data == q.data && p.token == q.token);
}

int NV_E_isEqual(NV_Pointer p, NV_Pointer q)
{
	if(NV_E_isType(p, EInteger) && NV_E_isType(q, EInteger)){
		// Integer vs Integer
		return (NV_Integer_getImm32(p) == NV_Integer_getImm32(q));
	} else if(NV_E_isType(p, EString) && NV_E_isType(q, EString)){
		// Integer vs Integer
		return NV_String_isEqual(p, q);
	}
	NV_Error("%s", 
		"Equality comparison between following two elements are not implemented.");
	NV_printElement(p);
	NV_printElement(q);
	return 0;
}

void *NV_E_getRawPointer(NV_Pointer p, NV_ElementType et)
{
	if(!NV_E_isType(p, et)){
		NV_Error("Attempting to get raw pointer for wrong type (type: %d)\n", et);
		NV_printElement(p); printf("\n");
		return NULL;
	}
	return p.data->data;
}

void NV_E_unbox(NV_Pointer *maybeBoxedItem)
{
	// EVariable, EListItem, EDictItem -> content object
	if(NV_E_isType(*maybeBoxedItem, EVariable)){
		*maybeBoxedItem = NV_Variable_getData(*maybeBoxedItem);
		NV_E_unbox(maybeBoxedItem);
	} else if(NV_E_isType(*maybeBoxedItem, EListItem)){
		*maybeBoxedItem = NV_ListItem_getData(*maybeBoxedItem);
		NV_E_unbox(maybeBoxedItem);
	} else if(NV_E_isType(*maybeBoxedItem, EDictItem)){
		*maybeBoxedItem = NV_DictItem_getVal(*maybeBoxedItem);
		NV_E_unbox(maybeBoxedItem);
	}
}

void NV_E_convertUnknownToKnown(NV_Pointer vDict, NV_Pointer *mayStr)
{
	// if mayStr object is EString and EFUnknownToken (not string literal),
	// try to convert from string to variable,
	// if not, return original mayStr object.
/*
	NV_DbgInfo("%d %d %d ", 
		!NV_E_isType(mayStr, EString),
		!NV_E_checkFlag(mayStr, EFUnknownToken),
		NV_E_isNullPointer(NV_Dict_getItemByKey(vDict, mayStr))
	);
*/
	NV_Pointer new;
	if(!NV_E_isType(*mayStr, EString) ||
		!NV_E_checkFlag(*mayStr, EFUnknownToken) ||
		NV_E_isNullPointer(NV_Dict_getItemByKey(vDict, *mayStr))){
		NV_E_clearFlag(*mayStr, EFUnknownToken);
	} else{
		new = NV_Variable_allocByStr(vDict, *mayStr);
		NV_E_free(mayStr);
		*mayStr = new;
	}
}

void NV_E_convertToContents(NV_Pointer vDict, NV_Pointer *item)
{
	NV_E_convertUnknownToKnown(vDict, item);
	NV_E_unbox(item);
}

void NV_E_setFlag(NV_Pointer p, int32_t flag)
{
	if(!NV_E_isValidPointer(p)) return;
	p.data->flag |= flag;
}

void NV_E_clearFlag(NV_Pointer p, int32_t flag)
{
	if(!NV_E_isValidPointer(p)) return;
	p.data->flag &= ~flag;
}

int NV_E_checkFlag(NV_Pointer p, int32_t pattern)
{
	if(!NV_E_isValidPointer(p)) return 0;
	return p.data->flag & pattern;
}

void NV_E_addToPool(NV_Pointer p, NV_Pointer pool)
{
	if(!NV_E_isValidPointer(p)) return;
	if(!NV_E_isType(pool, EList)) return;
	//
	if(!NV_E_isNullPointer(p.data->pool)){
		NV_List_unlinkItemByIndex(
			p.data->pool, NV_List_indexOfData(p.data->pool, p));
	}
	p.data->pool = pool;
	NV_List_push(p.data->pool, p);
}

void NV_E_setPool(NV_Pointer p, NV_Pointer pool)
{
	if(!NV_E_isValidPointer(p)) return;
	if(!NV_E_isType(pool, EList) && !NV_E_isType(pool, EDict)) return;
	//
	p.data->pool = pool;
}

NV_Pointer NV_E_clone(NV_Pointer p)
{
	NV_Pointer c = NV_NullPointer;
	if(NV_E_isType(p, EInteger)){
		c = NV_Integer_clone(p);
	} else if(NV_E_isType(p, EOperator)){
		c = NV_Operator_clone(p);
	} else if(NV_E_isType(p, EString)){
		c = NV_String_clone(p);
	} else if(NV_E_isType(p, EList)){
		c = NV_List_clone(p);
	} else if(NV_E_isType(p, EDict)){
		c = NV_Dict_clone(p);
	} else if(NV_E_isType(p, EVariable)){
		c = NV_Variable_clone(p);
	}
	if(NV_E_isNullPointer(c)){
		NV_Error("%s", "Failed to cloning following element.");
		NV_printElement(p);
		return NV_NullPointer;
	}
	c.data->flag = p.data->flag;
	return c;
}

void NV_printElement(NV_Pointer p)
{
	if(NV_E_isValidPointer(p)){
		if(NV_E_isType(p, EInteger)){
			NV_Integer_print(p);		
		} else if(NV_E_isType(p, EOperator)){
			NV_Operator_print(p);		
		} else if(NV_E_isType(p, EString)){
			NV_String_print(p);		
		} else if(NV_E_isType(p, EListItem)){
			NV_ListItem_print(p);
		} else if(NV_E_isType(p, EList)){
			NV_List_printAll(p, NULL, NULL, NULL);
		} else if(NV_E_isType(p, EDictItem)){
			NV_DictItem_print(p);
		} else if(NV_E_isType(p, EDict)){
			NV_Dict_printAll(p, NULL, NULL, NULL);
		} else if(NV_E_isType(p, EVariable)){
			NV_Variable_print(p);
		} else{
			printf("(type: %d)", p.data->type);
		}
	} else{
		if(p.data){
			printf("(Invalid) %p ", p.data);
		} else{
			printf("(null)");
		}
	}
}

void NV_E_printMemStat()
{
	int i;
	printf("Malloc count: %d\n", NV_getMallocCount());
	printf("Elem using: %d, (pooling: %d)\n",
		NV_E_NumOfElementsUsing,
		NV_E_NumOfElementsAlloced - NV_E_NumOfElementsUsing);
	printf("RawMem using: %d\n", NV_getMallocCount() - NV_E_NumOfElementsAlloced);
	for(i = 0; i < NV_ELEMENT_TYPES; i++){
		printf("%d, ", NV_E_NumOfElements[i]);
	}
	putchar('\n');
}

//
// internal function
//


NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data)
{
	NV_Element *e;
	NV_Pointer p;
	//
	if(freeRoot){
		e = freeRoot;
		freeRoot = e->data;
#if NV_DEBUG_MEMORY
		NV_DbgInfo("reuse elem! (type: %d)", type);
#endif
	} else{
		NV_E_NumOfElementsAlloced++;
		e = NV_malloc(sizeof(NV_Element));
#if NV_DEBUG_MEMORY
		NV_DbgInfo("malloc elem! (type: %d)", type);
#endif
	}
	//
	if(type < NV_ELEMENT_TYPES) NV_E_NumOfElements[type]++;
	NV_E_NumOfElementsUsing++;
	//
	e->pool = NV_NullPointer;
	e->type = type;
	e->token = rand();
	e->flag = 0;
	e->data = data;
	//
	p.token = e->token;
	p.data = e;
	return p;
}

void NV_E_free_internal_ListItem(NV_Pointer item, NV_Pointer pool)
{
	NV_Pointer data = NV_ListItem_getData(item);
	NV_E_freeWithPool(&data, pool);
	NV_List_unlinkItem(item);
}

void NV_E_free_internal_List(NV_Pointer root, NV_Pointer pool)
{
	// free children
	NV_Pointer item;
	//
	for(;;){
		item = NV_List_getItemByIndex(root, 0);
		if(NV_E_isNullPointer(item)) break;
		NV_E_freeWithPool(&item, root);
	}
}
