#include "nv.h"

void NV_initRootTerm(NV_Term *t)
{
	t->type = Root;
	t->data = NULL;
	t->prev = NULL;	
	t->next = NULL;
}

void NV_changeRootTerm(NV_Term *oldRoot, NV_Term *newRoot)
{
	NV_initRootTerm(newRoot);
	NV_appendAll(newRoot, oldRoot);
}

void NV_cloneTermTree(NV_Term *dstRoot, const NV_Term *srcRoot)
{
	NV_Term *t, *tNew;
	//
	if(!srcRoot || !dstRoot) return;
	NV_initRootTerm(dstRoot);
	//
	for(t = srcRoot->next; t; t = t->next){
		tNew = NV_cloneTerm(t);
		//
		tNew->prev = NULL;
		tNew->next = NULL;
		NV_appendTermRaw(dstRoot, tNew);
	}
}

void NV_insertTermAfter(NV_Term *base, NV_Term *new)
{
	new->prev = base;
	new->next = base->next;
	if(new->next) new->next->prev = new;
	if(new->prev) new->prev->next = new;
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
	target = target->prev;
	NV_removeTerm(target->next);
	NV_insertTermAfter(target, new);
	return new;
}

void NV_divideTerm(NV_Term *subRoot, NV_Term *subBegin)
{
	// It modifies tree which contains subBegin.
	NV_Term *mainLast = subBegin->prev;
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
	if(new) new->prev = t;
	t->next = new;
	return;
}

void NV_appendTerm(NV_LangDef *langDef, NV_Term *termRoot, const char *termStr)
{
	// retv: isValid
	NV_Term *new;
	int tmpNum;
	char *p;
	// check operator
	new = NV_createTerm_Operator(langDef, termStr);
	if(new){
		NV_appendTermRaw(termRoot, new);
		return;
	}
	// check Integer
	tmpNum = strtol(termStr, &p, 0);
	if(termStr != p && *p == 0){
		new = NV_createTerm_Imm32(tmpNum);
		NV_appendTermRaw(termRoot, new);
		return;
	}
	// unknown
	new = NV_allocTerm();
	new->type = Unknown;
	new->data = (void *)termStr;
	NV_appendTermRaw(termRoot, new);
	return;
}

void NV_removeTerm(NV_Term *t)
{
	// don't apply to the root Term.
	if(t->prev)	t->prev->next = t->next;
	if(t->next)		t->next->prev = t->prev;
	if(t->data){
		switch(t->type){
			case Unknown:
			case Variable:
			case Operator:
				// data is static so do nothing.
				break;
			case String:
			case Imm32s:
				NV_free(t->data);
				break;
			case Sentence:
				NV_removeTermTree(t->data);
				NV_free(t->data);
				break;
			default:
				fprintf(stderr, "NV_removeTerm: Not implemented for type %d.\n", t->type);
				exit(EXIT_FAILURE);
		}
	}
	NV_free(t);
}

void NV_removeTermTree(NV_Term *root)
{
	while(root->next != NULL){
		NV_removeTerm(root->next);
	}
	root->next = 0;
}

void NV_printTerms(NV_Term *root)
{
	NV_printTerms_noNewLine(root);
	putchar('\n');
}

void NV_printTerms_noNewLine(NV_Term *root)
{
	NV_Term *t;
	NV_Operator *op;
	if(!root) return;
	for(t = root->next; t; t = t->next){
		if(NV_canReadTermAsInt(t)){
			printf("%d", NV_getValueOfTermAsInt(t));
		} else if(t->type == Operator){
			op = t->data;
			printf("op(%s:%d)", op->name, op->precedence);
		} else if(t->type == Unknown){
			printf("[Unknown: %s]", t->data);
		} else{
			printf("[type %d]", t->type);
		}
		if(t->next){
			printf(", ");
		}
	};
}

NV_Term *NV_getTermByIndex(NV_Term *root, int index)
{
	NV_Term *t;
	if(!root || !root->next) return NULL;
	for(t = root->next; t; t = t->next){
		if(index == 0) return t;
		index--;
	}
	return NULL;
}

NV_Term *NV_getLastTerm(NV_Term *root)
{
	NV_Term *t;
	if(!root || !root->next) return NULL;
	for(t = root->next; t->next; t = t->next);	// skip
	if(t->type == Sentence){
		t = NV_getLastTerm(t->data);
	}
	return t;
}

void NV_printLastTermValue(NV_Term *root, NV_VariableSet *vs)
{
	NV_Term *t;
	t = NV_getLastTerm(root);
	NV_printRealTermValue(t, vs);
}

