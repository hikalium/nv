#include "nv.h"

int32_t NV_debugFlag;
int main(int argc, char *argv[])
{
	int i;
	char line[MAX_INPUT_LEN];
	NV_Pointer root, lastItem, lastData, lang, vDict;
	clock_t t0 = clock();
	int32_t excFlag = 0;
	// get interpreter args
	for(i = 1; i < argc; i++){
#ifdef DEBUG
		if(strcmp(argv[i], "-v") == 0) NV_debugFlag |= NV_DBG_FLAG_VERBOSE;
		if(strcmp(argv[i], "-m") == 0) NV_debugFlag |= NV_DBG_FLAG_MEM;
		if(strcmp(argv[i], "-t") == 0) NV_debugFlag |= NV_DBG_FLAG_TIME;
#endif
	}
	// init env
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_MEM) NV_E_printMemStat();
#endif
	lang = NV_allocDefaultLang();
	vDict = NV_E_malloc_type(EDict);
	// main loop
	while(NV_gets(line, sizeof(line)) != NULL){
		root = NV_E_malloc_type(EList);
		// init flag
		excFlag = 0;
		SET_FLAG(excFlag, NV_EXC_FLAG_AUTO_PRINT);
		//
		NV_tokenize(lang, root, line);
		if(NV_convertLiteral(root, lang)){
			NV_Error("%s\n", "Literal conversion failed.");
		} else{
			NV_evaluateSentence(&excFlag, lang, vDict, root);
			if(excFlag & NV_EXC_FLAG_FAILED){
				// Ended with error
				NV_Error("%s\n", "Bad Syntax");
			} else{
				// Ended with Success
				if(excFlag & NV_EXC_FLAG_AUTO_PRINT){
					lastItem = NV_List_getLastItem(root);
					NV_ListItem_convertUnknownToKnown(vDict, lastItem);
					NV_ListItem_unbox(lastItem);
					lastData = NV_ListItem_getData(lastItem);
					if(!NV_E_isNullPointer(lastData)){
						printf("= ");
						NV_printElement(lastData);
						printf("\n");
					}
				}
			}
		}
		// cleanup current code
		NV_E_free(&root);
		if(excFlag & NV_EXC_FLAG_EXIT) break;
	}
	NV_E_free(&lang);
	NV_E_free(&vDict);
	// cleanup
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_MEM)
		NV_E_printMemStat();
	if(NV_debugFlag & NV_DBG_FLAG_TIME)
		printf("Processed in %f seconds.\n", (double)(clock() - t0) / CLOCKS_PER_SEC);
#endif
	return 0;
}

//
// Tokenize
//


void NV_tokenize(NV_Pointer lang, NV_Pointer termRoot, const char *input)
{
	const char *p;
	int i, lastCType, cType;
	char buf[MAX_TOKEN_LEN];
	NV_Pointer t;
	lastCType = 0;
	p = input;
	for(i = 0; ; i++){
		cType = NV_Lang_getCharType(lang, input[i]);
		if(cType != lastCType ||
			cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(input + i - p != 0){
				if((input + i - p) > MAX_TOKEN_LEN){
					NV_Error("%s", "Too long token.");
					exit(EXIT_FAILURE);
				}
				NV_strncpy(buf, p, MAX_TOKEN_LEN, input + i - p);
				//
				t = NV_E_malloc_type(EString);
				NV_String_setString(t, buf);
				NV_E_setFlag(t, EFUnknownToken);
				NV_List_push(termRoot, NV_E_autorelease(t));
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
	}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
		NV_List_printAll(termRoot, NULL, NULL, "]\n");
#endif
}

int NV_convertLiteral(NV_Pointer root, NV_Pointer lang)
{
	NV_Pointer item, t, strLiteral = NV_NullPointer;
	const char *termStr;
	char *p;
	int32_t tmpNum;
	//
	if(!NV_E_isType(root, EList)) return 1;
	item = root;
	NV_DbgInfo("%s", "start");
	for(;;){
#ifdef DEBUG
		if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
			NV_List_printAll(root, NULL, NULL, "]\n");
#endif
		item = NV_ListItem_getNext(item);
		if(NV_E_isNullPointer(item)) break;
		// get CStr
		termStr = NV_String_getCStr(NV_ListItem_getData(item));
		if(!termStr){
			NV_Error("%s", "termStr is NULL!!!");
			return 1;
		}
		if(!NV_E_isNullPointer(strLiteral)){
			if(termStr[0] == '"'){
				// end of string literal
				NV_ListItem_setData(item, strLiteral);
				NV_E_free(&strLiteral);
				strLiteral = NV_NullPointer;
				continue;
			} else{
				// body of string literal
				NV_String_concatenateCStr(strLiteral, termStr);
				t = NV_ListItem_getPrev(item);
				NV_E_free(&item);
				item = t;
				continue;
			}
		}
		if(termStr[0] == '"'){
			// begin of str literal
			t = NV_ListItem_getPrev(item);
			NV_E_free(&item);
			item = t;
			strLiteral = NV_E_malloc_type(EString);
			continue;
		}
		// check operator	
		t = NV_Lang_getOperatorFromString(lang, termStr);
		if(!NV_E_isNullPointer(t)){
			NV_ListItem_setData(item, t);
			continue;
		}
		// check Integer
		tmpNum = strtol(termStr, &p, 0);
		if(termStr != p && *p == 0){
			t = NV_E_malloc_type(EInteger);
			NV_Integer_setImm32(t, tmpNum);
			NV_ListItem_setData(item, NV_E_autorelease(t));
			continue;
		}
	}
	if(!NV_E_isNullPointer(strLiteral)){
		NV_Error("%s", "Missing end of string literal.");
		return 1;
	}
	return 0;
}

//
// Evaluate
//

void NV_evaluateSentence(int32_t *excFlag, NV_Pointer lang, NV_Pointer vDict, NV_Pointer root)
{
	NV_Pointer t, last = NV_NullPointer;
	NV_Pointer op;
	int lastOpPrec, opPrec, d;

	if(!NV_E_isType(root, EList)){
		NV_Error("%s", "root is not EList");
		SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
		return;
	}
	CLR_FLAG(*excFlag, NV_EXC_FLAG_EXIT);
	d = 0;
	for(;;){
		t = NV_ListItem_getNext(root);
		lastOpPrec = -1;
		last = NV_NullPointer;
		for(; !NV_E_isNullPointer(t); t = NV_ListItem_getNext(t)){
#ifdef DEBUG
			NV_DbgInfo("%s", "check(1)");
			if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
				NV_printElement(t);
			}
#endif
			if(!NV_ListItem_isDataType(t, EOperator)) continue;
			op = NV_ListItem_getData(t);
			opPrec = NV_getOperatorPrecedence(op);
			if(opPrec == -1){
				NV_Error("%s", "Internal error: Op not found");
				SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
				return;
			}
			if(lastOpPrec & 1 ? lastOpPrec <= opPrec : lastOpPrec < opPrec){
				lastOpPrec = opPrec;
				last = t;
				continue;
			}
			// found
			break;
		}
		if(NV_E_isNullPointer(last)){
			NV_DbgInfo("%s", "Evaluate end (no more op)");
			return;
		}
		t = NV_tryExecOp(excFlag, lang, last, vDict, root);
		if(NV_E_isNullPointer(t)){
			NV_DbgInfo("%s", "Evaluate end (Op Mismatched)");
			SET_FLAG(*excFlag, NV_EXC_FLAG_FAILED);
			return;
		}
		if(*excFlag & NV_EXC_FLAG_EXIT){
			NV_DbgInfo("%s", "Evaluate end (End flag)");
			return;
		}
#ifdef DEBUG
		NV_DbgInfo("%s", "Continue from:");
		if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
			NV_printElement(t);
			NV_printElement(root);
		}
#endif
	}
	return;
}

NV_Pointer NV_tryExecOp(int32_t *excFlag, NV_Pointer lang, NV_Pointer thisTerm, NV_Pointer vDict, NV_Pointer root)
{
	NV_Pointer fallbackOp, op;
	NV_Pointer orgTerm = thisTerm;
	//
	op = NV_ListItem_getData(thisTerm);	
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_DbgInfo("%s", "Begin native op: ");
		NV_Operator_print(op); putchar('\n');
	}
#endif
	thisTerm = NV_Operator_exec(op, excFlag, lang, vDict, thisTerm);
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE){
		NV_DbgInfo("%s", "End native op:");
		NV_Operator_print(op); putchar('\n');
	}
#endif
	if(NV_E_isNullPointer(thisTerm)){
		// try fallback
		fallbackOp = NV_Lang_getFallbackOperator(lang, op);
		if(NV_E_isNullPointer(fallbackOp)){
			NV_Error("%s", "Operator mismatched: ");
			NV_Operator_print(op); putchar('\n');
			NV_List_printAll(root, NULL, NULL, "]\n");
			return NV_NullPointer;
		}
		NV_ListItem_setData(orgTerm, fallbackOp);
		thisTerm = orgTerm;
	}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
		NV_List_printAll(root, NULL, NULL, "]\n");
#endif
	return thisTerm;
}

