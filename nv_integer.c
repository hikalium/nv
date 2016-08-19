#include "nv.h"

struct NV_INTEGER {
	int32_t imm32;
};

NV_Integer *NV_allocInteger()
{
	NV_Integer *t;
	t = NV_malloc(sizeof(NV_Integer));
	t->imm32 = 0;
	return t;
}

NV_Pointer NV_Integer_clone(NV_Pointer p)
{
	NV_Pointer c;
	if(!NV_E_isType(p, EInteger)) return NV_NullPointer;
	c = NV_E_malloc_type(EInteger);
	NV_Integer_setImm32(c, NV_Integer_getImm32(p));
	return c;
}

void NV_Integer_setImm32(NV_Pointer t, int32_t data)
{
	NV_Integer *v = NV_E_getRawPointer(t, EInteger);
	if(!v) return;
	v->imm32 = data;
}

int32_t NV_Integer_getImm32(NV_Pointer t)
{
	NV_Integer *v = NV_E_getRawPointer(t, EInteger);
	if(!v) return 0;
	return v->imm32;
}

void NV_Integer_print(NV_Pointer t)
{
	NV_Integer *v = NV_E_getRawPointer(t, EInteger);
	if(!v) return;
	printf("%d", v->imm32);
}

//
// Arithmetic operations
//

NV_Pointer NV_Integer_evalBinOp(NV_Pointer vL, NV_Pointer vR, NV_BinOpType type)
{
	NV_Pointer result;
	int32_t rv, ivL, ivR;
	//
	vL = NV_E_unbox(vL);
	vR = NV_E_unbox(vR);
	//
	if(!NV_E_isType(vL, EInteger) || !NV_E_isType(vR, EInteger)){
		NV_Error("%s", "Operand is not Integer");
		return NV_NullPointer;
	}
	//
	ivL = NV_Integer_getImm32(vL);
	ivR = NV_Integer_getImm32(vR);
	     if(type == BOpAdd)			rv = ivL + ivR;
	else if(type == BOpSub)			rv = ivL - ivR;
	else if(type == BOpMul)			rv = ivL * ivR;
	else if(type == BOpDiv)			rv = ivL / ivR;
	else if(type == BOpMod)			rv = ivL % ivR;
	else if(type == BOpLogicOR)		rv = ivL || ivR;
	else if(type == BOpLogicAND)	rv = ivL && ivR;
	else if(type == BOpBitOR)		rv = ivL | ivR;
	else if(type == BOpBitAND)		rv = ivL & ivR;
	else if(type == BOpCmpEq)		rv = ivL == ivR;
	else if(type == BOpCmpNEq)		rv = ivL != ivR;
	else if(type == BOpCmpLt)		rv = ivL < ivR;
	else if(type == BOpCmpGt)		rv = ivL > ivR;
	else if(type == BOpCmpLtE)		rv = ivL <= ivR;
	else if(type == BOpCmpGtE)		rv = ivL >= ivR;
	else {
		NV_Error("unknown op type %d.", type);
		return NV_NullPointer;
	}
	result = NV_E_malloc_type(EInteger);
	NV_Integer_setImm32(result, rv);
	return result;
}
