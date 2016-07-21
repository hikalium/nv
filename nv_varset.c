#include "nv.h"

NV_VariableSet *NV_allocVariableSet()
{
	NV_VariableSet *t;

	t = NV_malloc(sizeof(NV_VariableSet));
	//
	t->varUsed = 0;

	return t;
}

void NV_printVarsInVarSet(NV_VariableSet *vs)
{
	int i;
	printf("Variable Table (%p): %d\n", vs, vs->varUsed);
	for(i = 0; i < vs->varUsed; i++){
		NV_printVariable(&vs->varList[i], 2);
		putchar('\n');
	}
}

