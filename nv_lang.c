#include "nv.h"

#define NV_LANG_CHAR_LIST_LEN	3
// type 0 chars divide tokens but can't be a part of a token. 
// (only to disappear)
// sequences of type 1 chars make tokens.
// type 2 chars make token separately.

struct NV_LANG {
	// interpreter params
	const char *charList[NV_LANG_CHAR_LIST_LEN]; // should be terminated with 0
	NV_Pointer opList;
	NV_Pointer pool;
};

//
// NV_Element
//

NV_Lang *NV_E_allocLang()
{
	NV_Lang *t;

	t = NV_malloc(sizeof(NV_Lang));
	//
	t->charList[0] = NULL;
	t->charList[1] = NULL;
	t->charList[2] = NULL;
	//
	t->opList = NV_List_allocRoot();
	t->pool = NV_List_allocRoot();

	return t;
	
}

void NV_E_free_internal_Lang(NV_Pointer p, NV_Pointer pool)
{
	NV_Lang *t = NV_E_getRawPointer(p, ELang);
	NV_E_freeWithPool(&t->opList, pool);
	NV_E_freeWithPool(&t->pool, pool);
}

//
// NV_Lang
//

NV_Pointer NV_allocDefaultLang()
{
	return NV_allocLang00();
}

int NV_Lang_getCharType(NV_Pointer lang, char c)
{
	NV_Lang *t = NV_E_getRawPointer(lang, ELang);
	int i;
	if(!t || c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		if(strchr(t->charList[i], c)) break;
	}
	return i;
}

void NV_Lang_setCharList(NV_Pointer lang, int type, const char *s)
{
	// s should be pointing a static string.
	NV_Lang *t = NV_E_getRawPointer(lang, ELang);
	if(!t || type < 0 || NV_LANG_CHAR_LIST_LEN <= type) return;
	t->charList[type] = s;
}

NV_Pointer NV_Lang_getOpList(NV_Pointer lang)
{
	NV_Lang *t = NV_E_getRawPointer(lang, ELang);
	if(t) return t->opList;
	return NV_NullPointer;	
}

NV_Pointer NV_Lang_getOperatorFromString(NV_Pointer lang, const char *termStr)
{
	NV_Pointer p;
	NV_Operator *op;
	p = NV_ListItem_getNext(NV_Lang_getOpList(lang));
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getRawData(p, EOperator);
		if(strcmp(op->name, termStr) == 0){
			return NV_ListItem_getData(p);
		}
	}
	return NV_NullPointer;
}

NV_Pointer NV_Lang_getFallbackOperator(NV_Pointer lang, NV_Pointer baseOp)
{
	NV_Pointer p;
	NV_Operator *op = NULL;
	NV_Operator *rawBaseOp = NV_E_getRawPointer(baseOp, EOperator);
	//
	p = NV_ListItem_getNext(NV_Lang_getOpList(lang));
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getRawData(p, EOperator);
		if(op == rawBaseOp){
			p = NV_ListItem_getNext(p);
			break;
		}
	}
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getRawData(p, EOperator);
		if(strcmp(op->name, rawBaseOp->name) == 0){
			return NV_ListItem_getData(p);
		}
	}
	return NV_NullPointer;
}

void NV_Lang_registerOperator(NV_Pointer lang, NV_Pointer op)
{
	// op list is sorted in a descending order of precedence.
	NV_Pointer tOpItem, opList, tOp;
	//
	opList = NV_Lang_getOpList(lang);
	tOpItem = NV_ListItem_getNext(opList);
	for(; !NV_E_isNullPointer(tOpItem); tOpItem = NV_ListItem_getNext(tOpItem)){
		tOp = NV_ListItem_getData(tOpItem);
		if(NV_getOperatorPrecedence(tOp) < NV_getOperatorPrecedence(op)) break;
	}
	if(NV_E_isNullPointer(tOpItem)){
		NV_List_push(opList, op);
	} else{
		NV_List_insertDataBeforeItem(tOpItem, op);
	}
	NV_E_setPool(op, opList);
}

void NV_Lang_addOp(NV_Pointer lang, int pr, const char *name, NV_OpFunc f)
{
	NV_Lang_registerOperator(lang, NV_Operator_alloc(pr, name, f));
}
