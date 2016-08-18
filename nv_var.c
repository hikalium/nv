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

