#include "nv.h"

NV_Variable *NV_allocVariable()
{
	NV_Variable *t;
	//
	t = NV_malloc(sizeof(NV_Variable));
	t->name[0] = 0;
	t->revision = 0;
	t->data = NV_NullPointer;
	//
	return t;
}

int NV_resetVariable(NV_Pointer v)
{
	// retv: failed?
	// excludes namestr.
	NV_Variable *pv = NV_E_getRawPointer(v, EVariable);
	if(!pv) return 1;
	NV_E_free(&pv->data);
	pv->revision++;
	return 0;
}

//
// assign to Variable
//
/*
int NV_Variable_assignTermValue(NV_Pointer v, NV_Pointer src)
{
	// retv: assigned?
	if(v->type == VStructureItem){
		NV_Variable_assignStructureItem(v, src);
	} else{
		switch(src->type){
			case Variable:
				NV_Variable_assignVariable(v, src->data);
				break;
			case String:
				NV_Variable_assignString(v, src->data);
				break;
			case Sentence:
				NV_Variable_assignStructure(v, src->data);
				break;
			case Imm32s:
				NV_Variable_assignInteger(v, NV_getValueOfTermAsInt(src));
				break;
			default:
				return 0;
		}
	}
	return 1;
}
*/

void NV_Variable_assignVariable(NV_Pointer dst, NV_Pointer src)
{
	NV_Variable *dstp, *srcp;
	if(!NV_E_isType(dst, EVariable) || !NV_E_isType(src, EVariable)) return;
	NV_resetVariable(dst);
	dstp = NV_E_getRawPointer(dst, EVariable);
	srcp = NV_E_getRawPointer(src, EVariable);
	dstp->data = srcp->data;
	dstp->revision++;
	return;
}
/*
void NV_Variable_assignInteger(NV_Variable *v, int32_t newVal)
{
	NV_resetVariable(v);
	v->type = VInteger;
	v->byteSize = sizeof(int32_t);	// int32s only now.
	v->data = NV_malloc(v->byteSize);
	v->revision++;
	*((int32_t *)v->data) = newVal;
}

void NV_Variable_assignString(NV_Variable *dst, const char *src)
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

void NV_Variable_assignStructure(NV_Variable *dst, const NV_Term *srcRoot)
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

void NV_Variable_assignStructureItem(NV_Variable *dst, NV_Term *term)
{
	NV_Term *t;
	t = NV_cloneTerm(term);
	if(dst->type == VStructureItem){
		if(dst->data != NULL){
			NV_overwriteTerm(dst->data, t);
		}
		dst->data = t;
		dst->revision++;
	} else{
		NV_resetVariable(dst);
		//
		dst->type = VStructureItem;
		dst->byteSize = 0;
		dst->data = term;
		dst->revision++;
	}
	return;
}
*/
//
// Conversion
//
/*
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
*/
//
// Lookup variable
//
/*
NV_Pointer NV_getVariableByName(NV_VariableSet *vs, const char *name)
{
	NV_Variable *v;
	int i;
	for(i = 0; i < vs->varUsed; i++){
		v = NV_E_getRawPointer(vs->varList[i], EVariable);
		if(strncmp(v->name, name, MAX_TOKEN_LEN) == 0){
			return vs->varList[i];
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
*/
//
// Print variable
//
/*
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
		NV_printRealTermValue(var->data, NULL);
		printf("]");
	} else if(var->type == VNone){
		printf("(No data)");
	} else{
		NV_Error("(Not implemented type: %d)", var->type);
	}
}
*/
