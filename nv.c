#include "nv.h"
//
// main
//
volatile sig_atomic_t NV_globalExecFlag;

int NV_interactiveInput(const NV_ID *cTypeList, const NV_ID *ctx)
{
	char line[MAX_INPUT_LEN];
	NV_ID tokenList;
	//
	if(NV_gets(line, sizeof(line)) != NULL){
		tokenList = NV_tokenize(cTypeList, line);
		if(IS_DEBUG_MODE()){
			NV_printNodeByID(&tokenList); putchar('\n');
		}
		NV_Context_pushToEvalStack(ctx, &tokenList, &NODEID_NULL);
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

//
// Evaluate
//

int NV_getNextOpIndex(const NV_ID *currentBlock, const NV_ID *ctx)
{
	// 次に実行すべきオペレータを探し、そのインデックスを返す
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp, org;
	//
	lastOpPrec = -1;
	lastOpIndex = -1;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(currentBlock, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		t = NV_Term_tryReadAsOperator(&t, ctx);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_OP)) continue;
		opPrec = NV_getOpPrec(&t);
		if(IS_DEBUG_MODE()){
			printf("Op found. prec = %d ", opPrec);
			NV_printNodeByID(&t); putchar('\n');
		}
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
	if(lastOpIndex != -1){
		org = NV_Array_getByIndex(currentBlock, lastOpIndex);
		NV_Dict_addUniqueEqKeyByCStr(&org, "recogAsOp", &lastOp);
	}
	return lastOpIndex;
}

int NV_checkAndPrintErrorOfCodeBlock(const NV_ID *code)
{
	NV_ID t, failedOp, failedReason;
	int i;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(code, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		failedOp = NV_Dict_getByStringKey(&t, "failedOp");
		if(!NV_NodeID_isEqual(&failedOp, &NODEID_NOT_FOUND)){
			failedReason = NV_Dict_getByStringKey(&t, "failedReason");
			NV_printNodeByID(&failedOp);
			printf(" : ");
			NV_printNodeByID(&failedReason);
			printf("\n");
		}
	}
	return 0;
}

void NV_evalLoop(const NV_ID *opList, const NV_ID *ctx)
{
	NV_ID currentBlock;
	NV_ID currentTermIndexNode;
	NV_ID currentTerm;
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	NV_ID t;
	int nextOpIndex, currentOpIndex;
	//
	for(;;){
		currentBlock = NV_Array_last(&evalStack);
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
		if(IS_DEBUG_MODE()){
			printf("---- eval begin\n ");
			//
			printf("current evalStack: ");
			NV_printNodeByID(&evalStack); putchar('\n');
		}
		if(NV_NodeID_isEqual(&currentBlock, &NODEID_NOT_FOUND)){
			// evalStack is empty.
			if(IS_DEBUG_MODE()){
				printf("evalStack empty. break.\n");
			}
			break;
		}
		// find next op
		currentTermIndexNode = NV_NodeID_getRelatedNodeFrom(
			&currentBlock, &RELID_CURRENT_TERM_INDEX);
		if(!NV_NodeID_isEqual(&currentTermIndexNode, &NODEID_NOT_FOUND)){
			// do op
			currentOpIndex = NV_NodeID_getInt32(&currentTermIndexNode);
			currentTerm = NV_Array_getByIndex(&currentBlock, currentOpIndex);
			if(IS_DEBUG_MODE()){
				printf("op at currentBlock[%d]: ", currentOpIndex);
				NV_printNodeByID(&currentTerm); putchar('\n');
			}
			NV_tryExecOpAt(&currentBlock, currentOpIndex, ctx);
			if(IS_DEBUG_MODE()){
				printf("evalStack after op exec: ");
				NV_printNodeByID(&evalStack); putchar('\n');
			}
		}
		currentBlock = NV_Array_last(&evalStack);
		// search next term to do
		nextOpIndex = NV_getNextOpIndex(&currentBlock, opList);
		if(nextOpIndex == -1){
			// no more op
			if(IS_DEBUG_MODE()){
				printf("nextOp not found\n");
			}
			t = NV_Array_pop(&evalStack);
			// error check
			NV_checkAndPrintErrorOfCodeBlock(&t);
			// store last result
			NV_NodeID_createUniqueIDRelation(ctx, &RELID_LAST_RESULT, &t);
			continue;
		}
		t = NV_Node_createWithInt32(nextOpIndex);
		if(IS_DEBUG_MODE()){
			printf("nextOpIndex: %d\n", nextOpIndex);
		}
		NV_NodeID_createUniqueIDRelation(
			&currentBlock, &RELID_CURRENT_TERM_INDEX, &t);
	}
}
