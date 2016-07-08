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

void NV_changeRootTerm(NV_Term *oldRoot, NV_Term *newRoot)
{
	NV_initRootTerm(newRoot);
	NV_appendAll(newRoot, oldRoot);
}

void NV_insertTermAfter(NV_Term *base, NV_Term *new)
{
	new->before = base;
	new->next = base->next;
	if(new->next) new->next->before = new;
	if(new->before) new->before->next = new;
}

void NV_insertAllTermAfter(NV_Term *base, NV_Term *srcRoot)
{
	// src becomes empty
	NV_Term *next, *new;
	for(new = srcRoot->next; new; new = next){
		next = new->next;
		NV_insertTermAfter(base, new);
		base = new;
	}
	NV_initRootTerm(srcRoot); 
}

NV_Term *NV_overwriteTerm(NV_Term *target, NV_Term *new)
{
	// target should not be a Root Term.
	// retv: term in tree.
	target = target->before;
	NV_removeTerm(target->next);
	NV_insertTermAfter(target, new);
	return new;
}

void NV_divideTerm(NV_Term *subRoot, NV_Term *subBegin)
{
	// It modifies tree which contains subBegin.
	NV_Term *mainLast = subBegin->before;
	mainLast->next = NULL;
	NV_initRootTerm(subRoot);
	NV_appendTermRaw(subRoot, subBegin);
}

void NV_appendAll(NV_Term *dstRoot, NV_Term *srcRoot)
{
	// src becomes empty
	// dstRoot can be any Term (not only root).
	NV_appendTermRaw(dstRoot, srcRoot->next);
	NV_initRootTerm(srcRoot);
}

void NV_appendTermRaw(NV_Term *root, NV_Term *new)
{
	NV_Term *t;
	for(t = root; t->next; t = t->next);
	if(new) new->before = t;
	t->next = new;
	return;
}

void NV_appendTerm(NV_LangDef *langDef, NV_Term *termRoot, const char *termStr)
{
	// retv: isValid
	NV_Term *new;
	int tmpNum;
	char *p;
	//
	new = NV_createTerm_Operator(langDef, termStr);
	if(new){
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

void NV_printTerms(NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op;
	if(!root) return;
	for(t = root->next; t; t = t->next){
		if(t->type == Operator){
			op = t->data;
			printf("(%s:%d)", op->name, op->precedence);
		} else if(t->type == Unknown){
			printf("[%d: %s]", t->type, t->data);
		} else{
			printf("[%d]", t->type);
		}
	};
	putchar('\n');
}

void NV_printLastTermValue(NV_Term *root)
{
	NV_Term *t;
	if(!root || !root->next) return;
	for(t = root->next; t->next; t = t->next);	// skip
	if(t->type == Sentence){
		NV_printLastTermValue(t->data);
	}
	NV_printValueOfTerm(t);
}

void NV_printValueOfTerm(NV_Term *t)
{
	NV_Variable *var;
	int32_t *tmp_sint32;
	NV_Operator *op;

	if(t->type == Operator){
		op = t->data;
		printf("(Operator) %s:%d", op->name, op->precedence);
	} else if(t->type == Unknown){
		printf("(Unknown)[%s]", t->data);
	} else if(t->type == Variable){
		var = t->data;
		printf("(Variable)");
		if(var->type == Integer){
			if(var->byteSize == sizeof(int32_t)){
				printf("(Integer %d Byte)", var->byteSize);
				tmp_sint32 = var->data;
				printf(" %d", *tmp_sint32);
			}
		}
	} else if(t->type == Imm32s){
		tmp_sint32 = t->data;
		printf("%d", *tmp_sint32);
	} else{
		printf("[type: %d]", t->type);
	}	
}

