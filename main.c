#include "nv.h"
//
// main
//

NV_ID rootScope;

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
			if(argv[i][1] == 'g') NV_globalExecFlag |= NV_EXEC_FLAG_SAVECODEGRAPH;
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
	/*
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
	*/
	//
	NV_insertInitialNode();
	//
	rootScope = NV_Node_createWithString("root");
	//
	cTypeList = NV_createCharTypeList();
	//
	opDict = NV_createOpDict();
	//
	{
		NV_ID opDictName = NV_Node_createWithString("opDict");
		NV_ID opDictVar = NV_Variable_createWithName(&rootScope, &opDictName);
		NV_Variable_assign(&opDictVar, &opDict);
	}
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
	// TEST CODE BEGIN vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
/*
	NV_ID rootAnchor = NV_Anchor_createWithName("root");

	NV_ID list1 = NV_Array_create();
	NV_ID e;
	e = NV_Node_createWithString("test1");
	NV_Array_push(&list1, &e);
	e = NV_Node_createWithString("hello");
	NV_Array_push(&list1, &e);

	NV_Array_print(&list1); putchar('\n');
	NV_Dict_printWithDepth(&list1, 6, 0);
	printf("Hash: %08X\n", NV_Term_calcHash(&list1));
	printf("Hash: %08X\n", NV_Term_calcHash(&opDict));

	FILE *fp = fopen("dump.txt", "w");
	if(fp){
		NV_Node_dumpAllToFile(fp);
		fclose(fp);
	}

	return 0;	
	*/
	// TEST CODE END ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	char line[MAX_INPUT_LEN];
	//
	while(NV_gets(line, sizeof(line)) != NULL){
		NV_ID tokenList = NV_tokenize(&cTypeList, line);
		NV_ID codeGraphRoot = NV_parseToCodeGraph(&tokenList, &opDict);
		NV_ID result = NV_evalGraph(&codeGraphRoot, &rootScope);
		//
		if(!(NV_globalExecFlag & NV_EXEC_FLAG_SUPRESS_AUTOPRINT)){
			printf(" = ");
			NV_ID prim = NV_Term_getPrimNodeID(&result, &rootScope);
			NV_Term_print(&prim);
			printf("\n");
		} else{
			NV_globalExecFlag &= ~NV_EXEC_FLAG_SUPRESS_AUTOPRINT;
		}
	}
	//
	return 0;
}


