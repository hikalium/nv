#include "nv.h"
//
// main
//

volatile sig_atomic_t NV_globalExecFlag;

int main(int argc, char *argv[])
{
	NV_ID cTypeList, opList;
	int i;
	// set signal handler
	if(signal(SIGINT, NV_signalHandler) == SIG_ERR) return 1;
	//
	printf(
		"# nv interpreter\n"
		"# repo:   https://github.com/hikalium/nv \n"
		"# commit: %s\n"
		"# date:   %s\n", 
		GIT_COMMIT_ID, GIT_COMMIT_DATE
	);
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'v') NV_globalExecFlag |= NV_EXEC_FLAG_VERBOSE;
		}
	}
	//
	NV_Graph_init();
	//
	cTypeList = NV_createCharTypeList();
	NV_Node_retain(&cTypeList);
	//
	opList = NV_createOpList();
	NV_Node_retain(&opList);
	//
	NV_globalExecFlag |= NV_EXEC_FLAG_INTERACTIVE;
	for(;;){
		NV_evalLoop();
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERACTIVE){
			// 入力を取得して継続する
			NV_interactiveInput(&cTypeList, &opList);
		} else{
			break;
		}
	}
	//
	return 0;
}

int NV_interactiveInput(const NV_ID *cTypeList, const NV_ID *opList)
{
	char line[MAX_INPUT_LEN];
	NV_ID tokenList;
	NV_ID evalStack = NV_Node_getRelatedNodeFrom(
		&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK);
	//
	if(NV_gets(line, sizeof(line)) != NULL){
		tokenList = NV_tokenize(cTypeList, line);
		NV_convertLiteral(&tokenList, opList);
		if(IS_DEBUG_MODE()){
			NV_printNodeByID(&tokenList); putchar('\n');
		}
		NV_Array_push(&evalStack, &tokenList);
		if(IS_DEBUG_MODE()){
			NV_printNodeByID(&tokenList); putchar('\n');
		}
		return 0;
	}
	return 1;
}

NV_ID NV_tokenize(const NV_ID *cTypeList, const char *input)
{
	// retv: tokenized str array
	const char *p;
	int i, lastCType, cType;
	char buf[MAX_TOKEN_LEN];
	NV_ID tokenList = NV_Array_create();
	NV_ID ns;
	lastCType = 0;
	p = input;
	for(i = 0; ; i++){
		cType = NV_Lang_getCharType(cTypeList, input[i]);
		if(cType != lastCType ||
			cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(input + i - p != 0){
				if((input + i - p) > MAX_TOKEN_LEN){
					NV_Error("%s", "Too long token.");
					exit(EXIT_FAILURE);
				}
				NV_strncpy(buf, p, MAX_TOKEN_LEN, input + i - p);
				//
				ns = NV_Node_createWithString(buf);
				NV_Array_push(&tokenList, &ns);
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
		
	}
	if(IS_DEBUG_MODE()){
		NV_Array_print(&tokenList);
	}
	return tokenList;
}

int NV_convertLiteral(const NV_ID *tokenizedList, const NV_ID *opList)
{
	// retv: converted token list
	int pIndex;
	int32_t tmpNum;
	NV_ID itemID, opID;
	NV_Node *item;
	int i;
	//
	for(i = 0; ; i++){
		itemID = NV_Array_getByIndex(tokenizedList, i);
		if(NV_ID_isEqual(&itemID, &NODEID_NOT_FOUND)) break;
		item = NV_Node_getByID(&itemID);
		// check operator
		opID = NV_Dict_get(opList, &itemID);
		if(!NV_ID_isEqual(&opID, &NODEID_NULL)){
			NV_Array_writeToIndex(tokenizedList, i, &opID);
			continue;
		}
		// check Integer
		tmpNum = NV_Node_String_strtol(item, &pIndex, 0);
		if(pIndex != 0 && (int)NV_Node_String_strlen(item) == pIndex){
			// converted entire string to number.
			itemID = NV_Node_createWithInt32(tmpNum);
			NV_Array_writeToIndex(tokenizedList, i, &itemID);
			continue;
		}
	}
	return 0;
}

//
// Evaluate
//

int NV_getNextOpIndex(const NV_ID *currentBlock)
{
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp;
	//
	lastOpPrec = -1;
	lastOpIndex = -1;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(currentBlock, i);
		if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_OP)) continue;
		opPrec = NV_getOpPrecAt(currentBlock, i);
		if(lastOpPrec & 1 ? lastOpPrec <= opPrec : lastOpPrec < opPrec){
			// continue searching
			lastOpIndex = i;
			lastOpPrec = opPrec;
			lastOp = t;
			continue;
		}
		// found. lastOpID is target op.
		break;
	}
	return lastOpIndex;
}



void NV_evalLoop()
{
	NV_ID currentBlock;
	NV_ID currentTermIndexNode;
	NV_ID currentTerm;
	NV_ID evalStack = NV_Node_getRelatedNodeFrom(
		&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK);
	NV_ID t, r;
	int nextOpIndex, currentOpIndex;
	for(;;){
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERRUPT){
			// env saving
			fprintf(stderr, "Saving env to `savefile`...\n");
			FILE *fp = fopen("savefile", "wb");
			if(!fp){
				fprintf(stderr, "fopen failed.\n");
			} else{
				NV_Graph_dumpToFile(fp);
				fclose(fp);
				fprintf(stderr, "Saving env done.\n");
				exit(EXIT_SUCCESS);
			}
			NV_globalExecFlag &= ~NV_EXEC_FLAG_INTERRUPT;
		}
		//
		currentBlock = NV_Array_last(&evalStack);
		if(NV_ID_isEqual(&currentBlock, &NODEID_NOT_FOUND)){
			// evalStack empty.
			if(IS_DEBUG_MODE()){
				printf("evalStack empty. break.\n");
			}
			break;
		}
		//NV_Array_print(&currentBlock); putchar('\n');
		currentTermIndexNode = NV_Node_getRelatedNodeFrom(
			&currentBlock, &RELID_CURRENT_TERM_INDEX);
		if(!NV_ID_isEqual(&currentTermIndexNode, &NODEID_NOT_FOUND)){
			// do op
			currentOpIndex = NV_Node_getInt32FromID(&currentTermIndexNode);
			currentTerm = NV_Array_getByIndex(&currentBlock, currentOpIndex);
			if(IS_DEBUG_MODE()){
				printf("currentBlock[%d] ", currentOpIndex);
				NV_printNodeByID(&currentTerm); putchar('\n');
			}
			NV_tryExecOpAt(&currentBlock, currentOpIndex);
		}
		// search next term to do
		nextOpIndex = NV_getNextOpIndex(&currentBlock);
		if(nextOpIndex == -1){
			// no more op
			t = NV_Array_pop(&evalStack);
			NV_Node_createUniqueRelation(
				&NODEID_NV_STATIC_ROOT, &RELID_CURRENT_TERM_PHASE, &t);
			continue;
		}
		t = NV_Node_createWithInt32(nextOpIndex);
		if(IS_DEBUG_MODE()){
			printf("nextOpIndex: %d\n", nextOpIndex);
		}
		r = NV_Node_getRelationFrom(&currentBlock, &RELID_CURRENT_TERM_INDEX);
		if(NV_ID_isEqual(&r, &NODEID_NOT_FOUND)){
			NV_Node_createRelation(&currentBlock, &RELID_CURRENT_TERM_INDEX, &t);
		} else{
			NV_Node_updateRelationTo(&r, &t);
		}
	}
}
