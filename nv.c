#include "nv.h"
//
// main
//
volatile sig_atomic_t NV_globalExecFlag;

NV_ID NV_getContextList()
{
	return NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_CONTEXT_LIST);
}

NV_ID NV_Context_create()
{
	char s[128];
	time_t td;
	td = time(NULL);
	strftime(s, sizeof(s), "%Y-%m-%d- %H:%M:%S", localtime(&td));
	NV_ID ctx = NV_Node_createWithString(s);
	//
	NV_ID evalStack = NV_Array_create();
	NV_Dict_addUniqueIDKey(&ctx, &RELID_EVAL_STACK, &evalStack);
	//
	NV_ID ctxList = NV_getContextList();
	NV_Array_push(&ctxList, &ctx);
	//
	return ctx;
}

NV_ID NV_Context_getEvalStack(const NV_ID *ctx)
{
	return NV_NodeID_getRelatedNodeFrom(ctx, &RELID_EVAL_STACK);
}

void NV_Context_pushToEvalStack
(const NV_ID *ctx, const NV_ID *code, const NV_ID *newScope)
{
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	//
	NV_ID currentScope;
	if(newScope){
		currentScope = *newScope;	// GLOBAL SCOPE
	} else{
		currentScope = NV_Node_createWithStringFormat(
			"scope level %d", NV_Array_count(&evalStack));
	}
	//
	NV_Dict_addUniqueIDKey(code, &RELID_CURRENT_SCOPE, &currentScope);
	NV_Array_push(&evalStack, code);
	if(IS_DEBUG_MODE()){
		printf("pushed to evalStack: ");
		NV_Array_print(code); putchar('\n');
		printf("evalStack: ");
		NV_Array_print(&evalStack); putchar('\n');
	}
}

NV_ID NV_Context_getCurrentCode(const NV_ID *ctx)
{
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	return NV_Array_last(&evalStack);
}

NV_ID NV_Context_getCurrentScope(const NV_ID *ctx)
{
	NV_ID currentCode = NV_Context_getCurrentCode(ctx);
	return NV_NodeID_getRelatedNodeFrom(&currentCode, &RELID_CURRENT_SCOPE);
}

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
/*
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
		if(NV_NodeID_isEqual(&itemID, &NODEID_NOT_FOUND)) break;
		item = NV_NodeID_getNode(&itemID);
		// check operator
		opID = NV_Dict_get(opList, &itemID);
		if(!NV_NodeID_isEqual(&opID, &NODEID_NULL)){
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
*/
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



void NV_evalLoop(const NV_ID *opList, const NV_ID *ctx)
{
	NV_ID currentBlock;
	NV_ID currentTermIndexNode;
	NV_ID currentTerm;
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	NV_ID t;
	int nextOpIndex, currentOpIndex;
	//
	currentBlock = NV_Array_last(&evalStack);
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
		if(IS_DEBUG_MODE()){
			printf("current evalStack: ");
			NV_printNodeByID(&evalStack); putchar('\n');
		}
		if(IS_DEBUG_MODE()){
			printf("currentBlock: ");
			NV_printNodeByID(&currentBlock); putchar('\n');
		}
		if(IS_DEBUG_MODE()){
			NV_Array_print(&currentBlock); putchar('\n');
		}
		if(NV_NodeID_isEqual(&currentBlock, &NODEID_NOT_FOUND)){
			// evalStack empty.
			if(IS_DEBUG_MODE()){
				printf("evalStack empty. break.\n");
			}
			break;
		}
		currentTermIndexNode = NV_NodeID_getRelatedNodeFrom(
			&currentBlock, &RELID_CURRENT_TERM_INDEX);
		if(!NV_NodeID_isEqual(&currentTermIndexNode, &NODEID_NOT_FOUND)){
			// do op
			currentOpIndex = NV_NodeID_getInt32(&currentTermIndexNode);
			currentTerm = NV_Array_getByIndex(&currentBlock, currentOpIndex);
			//if(IS_DEBUG_MODE()){
			//	printf("currentBlock[%d] ", currentOpIndex);
			//	NV_printNodeByID(&currentTerm); putchar('\n');
			//}
			NV_tryExecOpAt(&currentBlock, currentOpIndex, ctx);
			if(IS_DEBUG_MODE()){
				NV_Array_print(&currentBlock); putchar('\n');
			}
			if(IS_DEBUG_MODE()){
				printf("current evalStack2: ");
				NV_printNodeByID(&evalStack); putchar('\n');
			}
		}
		currentBlock = NV_Array_last(&evalStack);
		// search next term to do
		nextOpIndex = NV_getNextOpIndex(&currentBlock, opList);
		if(nextOpIndex == -1){
			// no more op
			t = NV_Array_pop(&evalStack);
			NV_NodeID_createUniqueIDRelation(
				&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT, &t);
			continue;
		}
		t = NV_Node_createWithInt32(nextOpIndex);
		//if(IS_DEBUG_MODE()){
		//	printf("nextOpIndex: %d\n", nextOpIndex);
		//}
		NV_NodeID_createUniqueIDRelation(
			&currentBlock, &RELID_CURRENT_TERM_INDEX, &t);
	}
}
