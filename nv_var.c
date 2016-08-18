#include "nv.h"

struct NV_VARIABLE {
	NV_Pointer target;
};

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
		return;
	}
	//
	v = NV_E_getRawPointer(var, EVariable);
	v->target = target;
}
