#include "nv.h"

NV_Term *NV_allocTerm()
{
	NV_Term *t;

	t = NV_malloc(sizeof(NV_Term));

	t->type = Unknown;
	t->data = NULL;
	t->before = NULL;
	t->next = NULL;

	return t;
}

NV_Term *NV_cloneTerm(const NV_Term *src)
{
	NV_Term *new = NULL;
	switch(src->type){
		case Unknown:
		case Variable:
		case Operator:
			// data is static so copy address simply.
			new = NV_allocTerm();
			*new = *src;
			break;
		case Imm32s:
			new = NV_createTerm_Imm32(*(int32_t *)src->data);
			break;
		case String:
			new = NV_createTerm_String((const char *)src->data);
			break;
		/*
		case Sentence:
			NV_removeTermTree(t->data);
			NV_free(t->data);
			break;
		*/
		default:
			fprintf(stderr, "%s: Not implemented for type %d.\n", __func__, src->type);
			exit(EXIT_FAILURE);
	}
	if(!new){
		fprintf(stderr, "%s: Clone failed for type %d.\n", __func__, src->type);
		exit(EXIT_FAILURE);
	}
	new->next = NULL;
	new->before = NULL;
	return new;
}

NV_Term *NV_createTerm_Operator(NV_LangDef *langDef, const char *opName)
{
	NV_Operator *op;
	NV_Term *new;

	op = NV_isOperator(langDef, opName);
	if(!op) return NULL;
	new = NV_allocTerm();
	new->type = Operator;
	new->data = op;
	return new;
}

NV_Term *NV_createTerm_Imm32(int imm32)
{
	NV_Term *new;
	//
	new = NV_allocTerm();
	new->type = Imm32s;
	new->data = NV_malloc(sizeof(int));

	*((int *)new->data) = imm32;
	return new;
}

NV_Term *NV_createTerm_String(const char *s)
{
	NV_Term *new;
	//
	new = NV_allocTerm();
	new->type = String;
	new->data = NV_malloc(strlen(s) + 1);
	strcpy(new->data, s);
	return new;
}

NV_Term *NV_createTerm_Variable(NV_VariableSet *vs, const char *name)
{
	NV_Term *new;
	NV_Variable *data;
	//
	new = NV_allocTerm();
	new->type = Variable;
	//
	data = NV_getVariableByName(vs, name);
	if(!data){
		data = NV_allocVariable(vs); 
		strncpy(data->name, name, MAX_TOKEN_LEN);
	}
	new->data = data;
	return new;
}

NV_Term *NV_createTerm_Sentence()
{
	NV_Term *new;
	NV_Term *root;
	//
	new = NV_allocTerm();
	root = NV_allocTerm();
	//
	new->type = Sentence;
	new->data = root;
	return new;
}

void NV_printValueOfTerm(NV_Term *t, NV_VariableSet *vs)
{
	NV_Operator *op;

	if(!t) return;
	
	if(vs && t->type == Unknown){
		NV_tryConvertTermFromUnknownToVariable(vs, &t, 0);
	}

	if(NV_canReadTermAsInt(t)){
		printf("%d", NV_getValueOfTermAsInt(t));
		return;
	}

	if(t->type == Operator){
		op = t->data;
		printf("(Operator) %s:%d", op->name, op->precedence);
	} else if(t->type == Unknown){
		printf("(Unknown)[%s]", t->data);
	} else if(t->type == String){
		printf("(String)\"%s\"", t->data);
	} else if(t->type == Variable){
		printf("(Variable)");
		NV_printVariable(t->data, 1);
	} else{
		printf("[type: %d]", t->type);
	}	
}

//
// Evaluate term
//

int NV_canReadTermAsInt(NV_Term *t)
{
	NV_Variable *var;
	if(t->type == Variable){
		var = t->data;
		if(var->type == VInteger){
			if(var->byteSize == sizeof(int32_t)){
				return 1;
			}
		} else if(var->type == VStructureItem){
			return NV_canReadTermAsInt(var->data);
		}
	} else if(t->type == Imm32s){
		return 1;
	}
	return 0;
}

int NV_getValueOfTermAsInt(NV_Term *t)
{
	NV_Variable *var;
	int32_t *tmp_sint32;

	if(t->type == Variable){
		var = t->data;
		if(var->type == VInteger){
			if(var->byteSize == sizeof(int32_t)){
				tmp_sint32 = var->data;
				return *tmp_sint32;
			}
		} else if(var->type == VStructureItem){
			return NV_getValueOfTermAsInt(var->data);
		}
	} else if(t->type == Imm32s){
		tmp_sint32 = t->data;
		return *tmp_sint32;
	}
	return 0;
}

int NV_canAssignToTerm(NV_Term *t){
	if(t->type == Variable){
		return 1;
	}
	return 0;
}

