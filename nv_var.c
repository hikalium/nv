#include "nv.h"

struct NV_VARIABLE {
	NV_Pointer target;
};

void NV_Variable_setParentScope(NV_Pointer vDict, NV_Pointer pDict);
NV_Pointer NV_Variable_getParentScope(NV_Pointer vDict);

//
// NV_Element
//

NV_Variable *NV_E_allocVariable()
{
	NV_Variable *v;
	v = NV_malloc(sizeof(NV_Variable));
	v->target = NV_NullPointer;
	return v;
}

void NV_E_free_internal_Variable(NV_Pointer p)
{
	NV_Variable *v;
	v = NV_E_getRawPointer(p, EVariable);
	if(!v) return;
	NV_E_free(&v->target);
}

//
// NV_Variable
//

NV_Pointer NV_Variable_clone(NV_Pointer p)
{
	NV_Pointer c;
	if(!NV_E_isType(p, EVariable)) return NV_NullPointer;
	c = NV_E_malloc_type(EVariable);
	NV_Variable_setTarget(c, NV_Variable_getTarget(p));
	return c;
}

NV_Pointer NV_Variable_allocNewScope(NV_Pointer baseScope)
{
	NV_Pointer scope = NV_E_malloc_type(EDict);
	NV_Variable_setParentScope(scope, baseScope);
	return scope;
}

int NV_Variable_isExisted(NV_Pointer vDict, NV_Pointer key)
{
	NV_Pointer target;
	//
	target = NV_Dict_getItemByKey(vDict, key);
	if(!NV_E_isNullPointer(target)){
		// found target
		return 1;
	}
	// search parent
	target = NV_Variable_getParentScope(vDict);
	if(NV_E_isType(target, EDict)){
		return NV_Variable_isExisted(target, key);
	}
	return 0;
}

NV_Pointer NV_Variable_allocExistedByStr(NV_Pointer vDict, NV_Pointer key)
{
	NV_Pointer var, target;
	//
	target = NV_Dict_getItemByKey(vDict, key);
	if(!NV_E_isNullPointer(target)){
		// found target
		var = NV_E_malloc_type(EVariable);
		NV_Variable_setTarget(var, target);
		return var;
	}
	// search parent
	target = NV_Variable_getParentScope(vDict);
	if(NV_E_isType(target, EDict)){
		return NV_Variable_allocExistedByStr(target, key);
	}
	return NV_NullPointer;
}

NV_Pointer NV_Variable_allocExistedByStrInCurrentScope(NV_Pointer vDict, NV_Pointer key)
{
	NV_Pointer var, target;
	//
	target = NV_Dict_getItemByKey(vDict, key);
	if(!NV_E_isNullPointer(target)){
		// found target
		var = NV_E_malloc_type(EVariable);
		NV_Variable_setTarget(var, target);
		return var;
	}
	return NV_NullPointer;
}

NV_Pointer NV_Variable_allocByStr(NV_Pointer vDict, NV_Pointer key)
{
	NV_Pointer var;
	var = NV_Variable_allocExistedByStr(vDict, key);
	if(NV_E_isNullPointer(var)){
		NV_Dict_add(vDict, key, NV_E_autorelease(NV_E_malloc_type(EString)));
		var = NV_Variable_allocExistedByStr(vDict, key);
	}
	return var;
}

NV_Pointer NV_Variable_allocByStrInCurrentScope(NV_Pointer vDict, NV_Pointer key)
{
	NV_Pointer var;
	var = NV_Variable_allocExistedByStrInCurrentScope(vDict, key);
	if(NV_E_isNullPointer(var)){
		NV_Dict_add(vDict, key, NV_E_autorelease(NV_E_malloc_type(EString)));
		var = NV_Variable_allocExistedByStr(vDict, key);
	}
	return var;
}


NV_Pointer NV_Variable_allocByCStr(NV_Pointer vDict, const char *s)
{
	NV_Pointer str = NV_E_malloc_type(EString);
	NV_Pointer var;
	NV_String_setString(str, s);
	var = NV_Variable_allocByStr(vDict, str);
	NV_E_free(&str);
	return var;
}

NV_Pointer NV_Variable_getTarget(NV_Pointer var)
{
	NV_Variable *v;
	v = NV_E_getRawPointer(var, EVariable);
	if(v) return v->target;
	return NV_NullPointer;
}

void NV_Variable_setTarget(NV_Pointer var, NV_Pointer target)
{
	// retains target.
	NV_Variable *v;
	// var type check
	if(!NV_E_isType(var, EVariable)) return;
	// target type check
	if(!NV_E_isType(target, EDictItem) && !NV_E_isType(target, EListItem)){
		NV_Error("%s", "Cannot set following object as Target of Variable.");
		NV_printElement(target);
		return;
	}
	//
	v = NV_E_getRawPointer(var, EVariable);
	if(v) v->target = NV_E_retain(target);
}

void NV_Variable_assignData(NV_Pointer var, NV_Pointer data)
{
	NV_Pointer target, old;
	if(!NV_E_isType(var, EVariable)) return;
	target = NV_Variable_getTarget(var);
	if(NV_E_isType(target, EDictItem)){
		old = NV_DictItem_getVal(target);
		NV_E_free(&old);
		NV_DictItem_setVal(target, data);
	} else if(NV_E_isType(target, EListItem)){
		old = NV_ListItem_getData(target);
		NV_E_free(&old);
		NV_ListItem_setData(target, data);
	} else{
		NV_Error("%s", "Cannot assign data to following object.");
		NV_printElement(target);
		return;
	}
}

NV_Pointer NV_Variable_getData(NV_Pointer var)
{
	NV_Pointer target;
	if(!NV_E_isType(var, EVariable)) return NV_NullPointer;
	target = NV_Variable_getTarget(var);
	if(NV_E_isType(target, EDictItem)){	
		return NV_DictItem_getVal(target);
	} else if(NV_E_isType(target, EListItem)){
		return NV_ListItem_getData(target);
	} else{
		NV_Error("%s", "Cannot get data from following object.");
		NV_printElement(target);
	}
	return NV_NullPointer;
}

void NV_Variable_print(NV_Pointer p)
{
	NV_Pointer target;
	if(!NV_E_isType(p, EVariable)) return;
	target = NV_Variable_getTarget(p);
	printf("(Variable ");
	if(NV_E_isType(target, EDictItem) || NV_E_isType(target, EListItem)){
		NV_printElement(target);
	} else{
		NV_Error("%s", "Not implemented.");
		NV_printElement(target);
		return;
	}
	printf(")");
}

//
// internal func
//

void NV_Variable_setParentScope(NV_Pointer vDict, NV_Pointer pDict)
{
	NV_Pointer pKey = NV_String_alloc(NV_VARIABLE_PARENT_SCOPE_NAME);
	NV_Dict_add(vDict, pKey, pDict);
	NV_E_free(&pKey);
}

NV_Pointer NV_Variable_getParentScope(NV_Pointer vDict)
{
	NV_Pointer pKey = NV_String_alloc(NV_VARIABLE_PARENT_SCOPE_NAME);
	NV_Pointer parent = NV_Dict_getValByKey(vDict, pKey);
	NV_E_free(&pKey);
	return parent;
}

