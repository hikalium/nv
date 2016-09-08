#include "nv.h"

#define NV_LANG_CHAR_LIST_LEN	3
// type 0 chars divide tokens but can't be a part of a token. 
// (only to disappear)
// sequences of type 1 chars make tokens.
// type 2 chars make token separately.
/*
Directory Lang {
	
	"opIdentifier": [
		[opIdentifier]
	]
}
*/
struct NV_LANG {
	// interpreter params
	NV_Pointer charList;	// EList<EString>
	NV_Pointer opList;		// EList<EOperator>
};

//
// NV_Element
//

NV_Lang *NV_E_allocLang()
{
	NV_Lang *t;
	int i;

	t = NV_malloc(sizeof(NV_Lang));
	//
	t->charList = NV_List_allocRoot();
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		NV_List_push(t->charList, NV_E_autorelease(NV_E_malloc_type(EString)));
	}

	t->opList = NV_List_allocRoot();

	return t;
	
}

void NV_E_free_internal_Lang(NV_Pointer p)
{
	NV_Lang *t = NV_E_getRawPointer(p, ELang);
	NV_E_free(&t->opList);
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
		if(NV_String_strchr(NV_List_getDataByIndex(t->charList, i), c)) break;
	}
	return i;
}

void NV_Lang_setCharList(NV_Pointer lang, int type, const char *s)
{
	// s should be pointing a static string.
	NV_Lang *t = NV_E_getRawPointer(lang, ELang);
	NV_Pointer item;	// EListItem
	if(!t || type < 0 || NV_LANG_CHAR_LIST_LEN <= type) return;
	item = NV_List_getItemByIndex(t->charList, type);
	NV_ListItem_setData(item, NV_E_autorelease(NV_String_alloc(s)));
}

NV_Pointer NV_Lang_getOpList(NV_Pointer lang)
{
	NV_Lang *t = NV_E_getRawPointer(lang, ELang);
	if(t) return t->opList;
	return NV_NullPointer;	
}

NV_Pointer NV_Lang_getOperatorFromString(NV_Pointer lang, const char *termStr)
{
	NV_Pointer p, op;
	p = NV_ListItem_getNext(NV_Lang_getOpList(lang));
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getData(p);
		if(NV_String_isEqualToCStr(NV_Operator_getName(op), termStr)){
			return NV_ListItem_getData(p);
		}
	}
	return NV_NullPointer;
}

NV_Pointer NV_Lang_getFallbackOperator(NV_Pointer lang, NV_Pointer baseOp)
{
	NV_Pointer p, op;
	//
	p = NV_ListItem_getNext(NV_Lang_getOpList(lang));
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getData(p);
		if(NV_E_isSamePointer(op, baseOp)){
			p = NV_ListItem_getNext(p);
			break;
		}
	}
	for(; !NV_E_isNullPointer(p); p = NV_ListItem_getNext(p)){
		op = NV_ListItem_getData(p);
		if(NV_E_isEqual(NV_Operator_getName(op), NV_Operator_getName(baseOp))){
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
}

void NV_Lang_addOpN(NV_Pointer lang, int pr, const char *name, NV_OpFunc f)
{
	NV_Lang_registerOperator(lang, 
		NV_E_autorelease(NV_Operator_allocNative(pr, name, f)));
}

void NV_Lang_addOp(NV_Pointer lang, NV_Pointer pr, NV_Pointer name, NV_Pointer f)
{
	NV_Lang_registerOperator(lang, 
		NV_E_autorelease(NV_Operator_alloc(pr, name, f)));
}
