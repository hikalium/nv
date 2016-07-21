#include "nv.h"

NV_Variable *NV_allocVariable(NV_VariableSet *vs)
{
	NV_Variable *t;
	//
	if(vs->varUsed >= MAX_VARS){
		NV_printError("No more variables.", stderr);
		exit(EXIT_FAILURE);
	}
	t = &vs->varList[vs->varUsed++];
	//
	t->name[0] = 0;
	t->type = None;
	t->byteSize = 0;
	t->revision = 0;
	t->data = NULL;
	//
	return t;
}

void NV_resetVariable(NV_Variable *v)
{
	// excludes namestr.
	if(v->type == None) return;
	v->type = None;
	v->byteSize = 0;
	if(v->data) NV_free(v->data);
	v->data = NULL;
}

void NV_assignVariable_Integer(NV_Variable *v, int32_t newVal)
{
	NV_resetVariable(v);
	v->type = Integer;
	v->byteSize = sizeof(int32_t);	// int32s only now.
	v->data = NV_malloc(v->byteSize);
	v->revision++;
	*((int32_t *)v->data) = newVal;
}

void NV_assignVariable_Variable(NV_Variable *dst, const NV_Variable *src)
{
	NV_resetVariable(dst);
	//
	dst->type = src->type;
	dst->byteSize = src->byteSize;
	dst->revision = src->revision;
	dst->data = NV_malloc(src->byteSize);
	memcpy(dst->data, src->data, src->byteSize);
	return;
}

void NV_tryConvertTermFromUnknownToVariable(NV_VariableSet *vs, NV_Term **term)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(vs, (*term)->data);
	if(var) new = NV_createTerm_Variable(vs, var->name);
	else new = NV_createTerm_Variable(vs, (*term)->data);
	if(new){
		NV_overwriteTerm((*term), new);
		*term = new;
		return;
	}
}

void NV_tryConvertTermFromUnknownToImm(NV_VariableSet *vs, NV_Term **term)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(vs, (*term)->data);
	if(var){
		if(var->type == Integer && var->byteSize == sizeof(int32_t)){
			new = NV_createTerm_Imm32(*((int32_t *)var->data));
			NV_overwriteTerm((*term), new);
			*term = new;
			return;
		}
	}
}

NV_Variable *NV_getVariableByName(NV_VariableSet *vs, const char *name)
{
	NV_Variable *var;
	int i;
	for(i = 0; i < vs->varUsed; i++){
		var = &vs->varList[i];
		if(strncmp(var->name, name, MAX_TOKEN_LEN) == 0){
			return var;
		}
	}
	if(NV_isDebugMode) printf("NV_getVariableByName: Variable '%s' not found.\n", name);
	return NULL;
}

