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
	NV_Variable *var;
	int32_t *tmpint32;
	int i;

	printf("Variable Table (%p): %d\n", vs, vs->varUsed);
	for(i = 0; i < vs->varUsed; i++){
		var = &vs->varList[i];
		printf("%s", var->name);
		printf("\t rev: %d", var->revision);
		if(var->type == Integer){
			printf("\t Integer(%d)", var->byteSize);
			if(var->byteSize == sizeof(int32_t)){
				tmpint32 = var->data;
				printf("\t = %d", *tmpint32);
			}
		}
		putchar('\n');
	}
}

