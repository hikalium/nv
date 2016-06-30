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

void NV_initRootTerm(NV_Term *t)
{
	t->type = Root;
	t->data = NULL;
	t->before = NULL;	
	t->next = NULL;
}

void NV_insertTermAfter(NV_Term *base, NV_Term *new)
{
	new->before = base;
	new->next = base->next;
	new->next->before = new;
	new->before->next = new;
}

void NV_appendTermRaw(NV_Term *root, NV_Term *new)
{
	NV_Term *t;
	for(t = root; t->next; t = t->next);
	new->next = NULL;
	new->before = t;
	t->next = new;
	return;
}

void NV_removeTerm(NV_Term *t)
{
	// don't apply to the root Term.
	if(t->before)	t->before->next = t->next;
	if(t->next)		t->next->before = t->before;
	if(t->data && 
		(t->type == Imm32s)){
		free(t->data);
	}
	free(t);
}

void NV_removeTermTree(NV_Term *root)
{
	while(root->next != NULL){
		NV_removeTerm(root->next);
	}
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

NV_Term *NV_createTerm_Variable(NV_Env *env, const char *name)
{
	NV_Term *new;
	NV_Variable *data;
	//
	new = NV_allocTerm();
	new->type = Variable;
	data = NV_allocVariable(env); 
	strncpy(data->name, name, MAX_TOKEN_LEN);
	new->data = data;
	return new;
}

void NV_appendTerm(NV_LangDef *langDef, NV_Term *termRoot, const char *termStr)
{
	// retv: isValid
	NV_Term *new;
	NV_Operator *op;
	int tmpNum;
	char *p;
	//
	op = NV_isOperator(langDef, termStr);
	if(op){
		new = NV_allocTerm();
		new->type = Operator;
		new->data = op;
		NV_appendTermRaw(termRoot, new);
		return;
	}
	tmpNum = strtol(termStr, &p, 0);
	if(termStr != p && *p == 0){
		new = NV_createTerm_Imm32(tmpNum);
		NV_appendTermRaw(termRoot, new);
		return;
	}
	//
	new = NV_allocTerm();
	new->type = Unknown;
	new->data = (void *)termStr;
	NV_appendTermRaw(termRoot, new);
	return;
}

void NV_printTerms(NV_Term *root)
{
	NV_Term *t;
	if(!root) return;
	for(t = root->next; ; t = t->next){
		printf("[%d]", t->type);
		if(!t->next) break;
	};
	putchar('\n');
	
}

void NV_printVarsInTerms(NV_Term *root)
{
	NV_Term *t;
	NV_Variable *var;
	int32_t *tmpint32;

	for(t = root->next; t; t = t->next){
		if(t->type != Variable) continue;
		var = t->data;
		if(var->type == Integer){
			printf("%s Integer(%d):", var->name, var->byteSize);
			if(var->byteSize == sizeof(int32_t)){
				tmpint32 = var->data;
				printf("%d", *tmpint32);
			}
			putchar('\n');
		}
	};
}
