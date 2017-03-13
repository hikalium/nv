#include "nv.h"
//
// main
//

int main(int argc, char *argv[])
{
	NV_ID cTypeList, opList;
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
		} else{
			NV_strncpy(filename, argv[i], MAX_TOKEN_LEN, strlen(argv[i]));
		}
	}
	//
	NV_Graph_init();
	//
	if(filename[0]){
		// restore savedata
		printf("Restoring from %s ...\n", filename);
		FILE *fp = fopen(filename, "rb");
		if(fp){
			NV_Graph_restoreFromFile(fp);
			fclose(fp);
			printf("done.\n");
		} else{
			printf("fopen failed.\n");
		}
	}
	//
	NV_Graph_insertInitialNode();
	//
	cTypeList = NV_createCharTypeList();
	NV_NodeID_retain(&cTypeList);
	//
	opList = NV_createOpList();
	NV_NodeID_retain(&opList);
	//
/*
	NV_ID t;
	t = NV_Path_createWithCStr("/var/log/httpd");
	NV_printNodeByID(&t); putchar('\n');
*/
	NV_ID ctx = NV_Context_create();
	if(IS_DEBUG_MODE()){
		NV_ID ctxList = NV_getContextList();
		NV_Array_print(&ctxList);
	}
	//
	NV_globalExecFlag |= NV_EXEC_FLAG_INTERACTIVE;
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
	//
	return 0;
}

