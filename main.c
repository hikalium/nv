#include "nv.h"
//
// main
//

int main(int argc, char *argv[])
{
	NV_ID cTypeList, opDict;
	int i;
	char filename[MAX_TOKEN_LEN];
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
	// read arguments
	filename[0] = 0;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'v') NV_globalExecFlag |= NV_EXEC_FLAG_VERBOSE;
			if(argv[i][1] == 's'){
				i++;
				NV_strncpy(filename, argv[i], MAX_TOKEN_LEN, strlen(argv[i]));
			}
		} else{
			
		}
	}
	//
	NV_Node_initRoot();
	//
	if(filename[0]){
		// restore savedata
		printf("Restoring from %s ...\n", filename);
		FILE *fp = fopen(filename, "rb");
		if(fp){
			NV_Node_restoreFromFile(fp);
			fclose(fp);
			printf("done.\n");
			//
			NV_ID cl = NV_getContextList(); 
			printf("%d contexts found.\n", NV_Array_count(&cl));
		} else{
			printf("fopen failed.\n");
		}
	}
	//
	NV_insertInitialNode();
	//
	cTypeList = NV_createCharTypeList();
	NV_NodeID_retain(&cTypeList);
	//
	opDict = NV_createOpDict();
	NV_NodeID_retain(&opDict);
	//
/*
	NV_ID t;
	t = NV_Path_createWithCStr("/var/log/httpd");
	NV_Term_print(&t); putchar('\n');
*/
	/*
	NV_ID ctx = NV_Context_create();
	NV_Context_setOpDict(&ctx, &opList);
	if(IS_DEBUG_MODE()){
		NV_ID ctxList = NV_getContextList();
		NV_Array_print(&ctxList);
	}
	//
	NV_globalExecFlag |= NV_EXEC_FLAG_INTERACTIVE;
	NV_evalLine(&cTypeList, &ctx, 
			"loop={for{#args[0]=args[1]}{#args[0]<=args[2]}{#args[0]++}{args[3]()}}"); 
		*/
	/*
	for(;;){
		NV_evalLoop(&opList, &ctx);
		// check if context should be changed
		
		NV_ID nextContext = NV_Dict_getEqID(&ctx, &RELID_NEXT_CONTEXT);
		if(!NV_NodeID_isEqual(&nextContext, &NODEID_NOT_FOUND)){
			NV_Dict_removeUniqueIDKey(&ctx, &RELID_NEXT_CONTEXT);
			ctx = nextContext;
			continue;
		}
		
		//
		
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERACTIVE){
			// 入力を取得して継続する
			if(NV_interactiveInput(&cTypeList, &ctx)){
				break;
			}
		} else{
			break;
		}
		
	}
	*/
	char line[MAX_INPUT_LEN];
	while(NV_gets(line, sizeof(line)) != NULL){
		NV_ID tokenList = NV_tokenize(&cTypeList, line);
		NV_ID codeGraphRoot = NV_parseToCodeGraph(&tokenList, &opDict);
		NV_saveCodeGraphForVisualization(&codeGraphRoot, "note/code.dot");
	}
	//
	return 0;
}


