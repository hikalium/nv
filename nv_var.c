#include "nv.h"

NV_Variable *NV_allocVariable(NV_VariableSet *vs)
{
	NV_Variable *t;
	//
	if(vs->varUsed >= MAX_VARS){
		NV_Error("No more variables. %d\n", MAX_VARS);
		exit(EXIT_FAILURE);
	}
	t = &vs->varList[vs->varUsed++];
	//
	t->name[0] = 0;
	t->type = VNone;
	t->byteSize = 0;
	t->revision = 0;
	t->data = NULL;
	//
	return t;
}

void NV_resetVariable(NV_Variable *v)
{
	// excludes namestr.
	if(v->type == VNone) return;
	v->byteSize = 0;
	if(v->data && v->type != VAlias) NV_free(v->data);
	v->data = NULL;
	v->revision++;
	v->type = VNone;
}

void NV_assignVariable_Variable(NV_Variable *dst, const NV_Variable *src)
{
	NV_resetVariable(dst);
	//
	dst->type = src->type;
	dst->byteSize = src->byteSize;
	dst->revision = src->revision + 1;
	dst->data = NV_malloc(src->byteSize);
	if(src->type == VInteger || src->type == VString){
		memcpy(dst->data, src->data, src->byteSize);
	} else{
		NV_Error("Not implemented. type %d\n", src->type);
		exit(EXIT_FAILURE);
	}
	return;
}

void NV_assignVariable_Integer(NV_Variable *v, int32_t newVal)
{
	NV_resetVariable(v);
	v->type = VInteger;
	v->byteSize = sizeof(int32_t);	// int32s only now.
	v->data = NV_malloc(v->byteSize);
	v->revision++;
	*((int32_t *)v->data) = newVal;
}

void NV_assignVariable_String(NV_Variable *dst, const char *src)
{
	NV_resetVariable(dst);
	//
	dst->type = VString;
	dst->byteSize = strlen(src) + 1;
	dst->revision++;
	dst->data = NV_malloc(dst->byteSize);
	NV_strncpy(dst->data, src, dst->byteSize, dst->byteSize);
	return;
}

void NV_assignVariable_Structure(NV_Variable *dst, const NV_Term *srcRoot)
{
	NV_resetVariable(dst);
	//
	dst->type = VStructure;
	dst->byteSize = sizeof(NV_Term);
	dst->revision++;
	dst->data = NV_malloc(dst->byteSize);
	NV_cloneTermTree(dst->data, srcRoot);
	return;
}

void NV_assignVariable_StructureItem(NV_Variable *dst, NV_Term *term)
{
	NV_resetVariable(dst);
	//
	dst->type = VStructureItem;
	dst->byteSize = 0;
	dst->revision++;
	dst->data = term;
	return;
}

void NV_tryConvertTermFromUnknownToVariable(NV_VariableSet *vs, NV_Term **term, int allowCreateNewVar)
{
	NV_Variable *var;
	NV_Term *new;

	if((*term)->type != Unknown) return;
	var = NV_getVariableByName(vs, (*term)->data);
	if(var) new = NV_createTerm_Variable(vs, var->name);
	else if(allowCreateNewVar) new = NV_createTerm_Variable(vs, (*term)->data);
	else return;
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
		if(var->type == VInteger && var->byteSize == sizeof(int32_t)){
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
	if(NV_isDebugMode) NV_Error("Variable '%s' not found.\n", name);
	return NULL;
}

NV_Term *NV_getItemFromStructureByIndex(NV_Variable *v, int index)
{
	NV_Term *t = v->data;
	if(v->type != VStructure) return NULL;
	return NV_getTermByIndex(t, index);
}

#define NUM_OF_VTYPES	6
char *VTypeNameList[NUM_OF_VTYPES] = {
	"None",
	"Alias",
	"Integer",
	"String",
	"Structure",
	"StructureItem",
};

void NV_printVariable(NV_Variable *var, int verbose)
{
	// verbose: 0 -> Value
	// verbose: 1 -> Type Value
	// verbose: 2 -> VarName Rev Type Value
	if(verbose >= 2){
		printf("%s\trev:%d\t", var->name, var->revision);
	}
	if(verbose >= 1) printf("(%s(%d)) ", VTypeNameList[var->type], var->byteSize);
	if(var->type == VInteger){
		if(var->byteSize == sizeof(int32_t)){
			printf("%d", *(int32_t *)var->data);
		} else{
			NV_Error("(Not implemented size: %d)", var->byteSize);
		}
	} else if(var->type == VString){
		printf("%s", var->data);
	} else if(var->type == VStructure){
		NV_printTerms_noNewLine(var->data);
	} else if(var->type == VStructureItem){
		printf("[");
		NV_printValueOfTerm(var->data);
		printf("]");
	} else if(var->type == VNone){
		printf("(No data)");
	} else{
		NV_Error("(Not implemented type: %d)", var->type);
	}
}

