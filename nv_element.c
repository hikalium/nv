#include "nv.h"

struct NV_ELEMENT {
    NV_ElementType type;
    int32_t token;
	int32_t flag;
    void *data;
};

NV_Pointer NV_E_malloc_internal(NV_ElementType type, void *data);

NV_Element *freeRoot = NULL;
const NV_Pointer NV_NullPointer = {
	0,
	NULL
};

NV_Pointer NV_E_malloc_type(NV_ElementType type)
{
	NV_Pointer p;
	switch(type){
		case EList:
			return NV_E_malloc_internal(EList,		NV_allocListItem());
		case EListItem:
			return NV_E_malloc_internal(EListItem,	NV_allocListItem());
		case EDict:
			return NV_E_malloc_internal(EDict,		NV_allocDictItem());
		case EDictItem:
			return NV_E_malloc_internal(EDictItem,	NV_allocDictItem());
		case EVariable:
			return NV_E_malloc_internal(EVariable,	NV_allocVariable());
		case EEnv:
			return NV_E_malloc_internal(EEnv,		NV_allocEnv());
		case EOperator:
			return NV_E_malloc_internal(EOperator,	NV_allocOperator());
		case EInteger:
			return NV_E_malloc_internal(EInteger, 	NV_allocInteger());
		case EString:
			return NV_E_malloc_internal(EString, 	NV_allocString());
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
	if(!p) return;
	NV_Element *e = p->data;
	if(!NV_E_isValidPointer(*p)) return;
	e->token = rand();
	e->type = ENone;
	NV_free(e->data);
	//
	e->data = freeRoot;
	freeRoot = e;
	//
	*p = NV_NullPointer;
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
		return NULL;
	}
	return p.data->data;
}

NV_Pointer NV_E_unbox(NV_Pointer maybeBoxedItem)
{
	if(NV_E_isType(maybeBoxedItem, EVariable)){
		return NV_E_unbox(NV_Variable_getData(maybeBoxedItem));
	}
	return maybeBoxedItem;
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
	} else{
		e = NV_malloc(sizeof(NV_Element));
	}
	e->type = type;
	e->token = rand();
	e->flag = 0;
	e->data = data;
	//
	p.token = e->token;
	p.data = e;
	return p;
}

