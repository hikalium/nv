#include "nv.h"

struct NV_VARIABLE {
	NV_Pointer target;
};

NV_Pointer NV_Variable_getTarget(NV_Pointer var);

NV_Variable *NV_allocVariable()
{
	NV_Variable *v;
	v = NV_malloc(sizeof(NV_Variable));
	v->target = NV_NullPointer;
	return v;
}

NV_Pointer NV_Variable_clone(NV_Pointer p)
{
	NV_Pointer c;
	if(!NV_E_isType(p, EVariable)) return NV_NullPointer;
	c = NV_E_malloc_type(EVariable);
	NV_Variable_setTarget(c, NV_Variable_getTarget(p));
	return c;
}

NV_Pointer NV_Variable_allocByStr(NV_Pointer vDict, NV_Pointer str)
{
	NV_Pointer var = NV_E_malloc_type(EVariable);
	NV_Pointer target = NV_Dict_getItemByKey(vDict, str);
	NV_Pointer strKey = NV_String_clone(str);
	NV_Pointer strVal = NV_E_malloc_type(EString);
	if(NV_E_isNullPointer(target)){
		NV_Dict_add(vDict, strKey, strVal);
		target = NV_Dict_getItemByKey(vDict, str);
	}
	NV_Variable_setTarget(var, target);
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

void NV_Variable_setTarget(NV_Pointer var, NV_Pointer target)
{
	NV_Variable *v;
	// var type check
	if(!NV_E_isType(var, EVariable)) return;
	// target type check
	if(!NV_E_isType(target, EDictItem)){
		NV_Error("%s", "Cannot set following object as Target of Variable.");
		NV_printElement(target);
		return;
	}
	//
	v = NV_E_getRawPointer(var, EVariable);
	if(v) v->target = target;
}

void NV_Variable_assignData(NV_Pointer var, NV_Pointer data)
{
	NV_Pointer target;
	if(!NV_E_isType(var, EVariable)) return;
	target = NV_Variable_getTarget(var);
	if(NV_E_isType(target, EDictItem)){
		NV_DictItem_setVal(target, data);
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
	if(NV_E_isType(target, EDictItem)){
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

NV_Pointer NV_Variable_getTarget(NV_Pointer var)
{
	NV_Variable *v;
	v = NV_E_getRawPointer(var, EVariable);
	if(v) return v->target;
	return NV_NullPointer;
}

