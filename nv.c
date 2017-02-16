#include "nv.h"
//
// main
//

volatile sig_atomic_t NV_globalExecFlag;

int main(int argc, char *argv[])
{
	NV_ID cTypeList, opList;
	int i;
	//
	if(signal(SIGINT, NV_signalHandler) == SIG_ERR) return 1;
	printf(
		"# nv interpreter\n"
		"# repository: https://github.com/hikalium/nv \n"
		"# commit: %s\n# commit date: %s\n", 
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
	NV_runInteractive(&cTypeList, &opList);
	//
	return 0;
}

int NV_runInteractive(const NV_ID *cTypeList, const NV_ID *opList)
{
	char line[MAX_INPUT_LEN];
	NV_ID tokenList;
	//
	while(NV_gets(line, sizeof(line)) != NULL){
		tokenList = NV_tokenize(cTypeList, line);
		NV_convertLiteral(&tokenList, opList);
		if(IS_DEBUG_MODE()){
			NV_printNodeByID(&tokenList); putchar('\n');
		}
		NV_evaluateSetence(&tokenList);
		if(IS_DEBUG_MODE()){
			NV_printNodeByID(&tokenList); putchar('\n');
		}
	}
	return 0;
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
NV_ID NV_evaluateSetence(const NV_ID *tokenizedList)
{
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp;
	for(;;){
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERRUPT){
			fprintf(stderr, "Saving env to `save.bin`...\n");
			FILE *fp = fopen("save.bin", "wb");
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
		lastOpPrec = -1;
		for(i = 0; ; i++){
			t = NV_Array_getByIndex(tokenizedList, i);
			if(NV_ID_isEqual(&t, &NODEID_NOT_FOUND)) break;
			if(!NV_isTermType(&t, &NODEID_TERM_TYPE_OP)) continue;
			opPrec = NV_getOpPrecAt(tokenizedList, i);
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
		if(lastOpPrec == -1) break;	// no more op
		NV_tryExecOpAt(tokenizedList, lastOpIndex);
		if(IS_DEBUG_MODE()){
			NV_Array_print(tokenizedList); putchar('\n');
		}
	}
	return NV_Array_getByIndex(tokenizedList, 0);
}

