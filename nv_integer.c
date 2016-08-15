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

void NV_Integer_setImm32(NV_Pointer t, int32_t data)
{
	NV_Integer *v = NV_E_getRawPointer(t, EInteger);
	if(!v) return;
	v->imm32 = data;
}

void NV_Integer_print(NV_Pointer t)
{
	NV_Integer *v = NV_E_getRawPointer(t, EInteger);
	if(!v) return;
	printf("%d", v->imm32);
}
