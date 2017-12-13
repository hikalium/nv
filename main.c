#include "nv.h"
//
// main
//

NV_ID rootScope;

void test() {
  NV_ID nodeA = NV_Node_createWithString("A");
  NV_ID nodeB = NV_Node_createWithString("B");
  NV_ID nodeC = NV_Node_createWithString("C");
  NV_ID nodeD = NV_Node_createWithString("D");
  NV_ID nodeE = NV_Node_createWithString("E");
  NV_NodeID_createRelation(&nodeA, &nodeB, &nodeC);

  {
    NV_ID r = NV_NodeID_getRelatedNodeFrom(&nodeA, &nodeB);
    NV_NodeID_printForDebug(&r);
    if (NV_NodeID_isEqual(&nodeC, &r)) {
      printf("OK");
    } else {
      printf("BAD");
    }
  }

  NV_Dict_addUniqueIDKey(&nodeA, &nodeB, &nodeD);
  {
    NV_ID r = NV_NodeID_getRelatedNodeFrom(&nodeA, &nodeB);
    NV_NodeID_printForDebug(&r);
    if (NV_NodeID_isEqual(&nodeD, &r)) {
      printf("OK");
    } else {
      printf("BAD");
    }
  }

  NV_Dict_addUniqueIDKey(&nodeA, &nodeB, &nodeE);
  {
    NV_ID r = NV_NodeID_getRelatedNodeFrom(&nodeA, &nodeB);
    NV_NodeID_printForDebug(&r);
    if (NV_NodeID_isEqual(&nodeE, &r)) {
      printf("OK");
    } else {
      printf("BAD");
    }
  }
  {
    NV_ID ary = NV_Array_create();
    NV_ID nodeA = NV_Node_createWithString("A");
    NV_ID nodeB = NV_Node_createWithString("B");
    NV_Array_push(&ary, &nodeA);
    NV_Array_push(&ary, &nodeB);
    {
      NV_ID r0 = NV_Array_getByIndex(&ary, 0);
      NV_ID r1 = NV_Array_getByIndex(&ary, 1);
      NV_NodeID_printForDebug(&r0);
      NV_NodeID_printForDebug(&r1);
      if (NV_NodeID_isEqual(&nodeA, &r0) && NV_NodeID_isEqual(&nodeB, &r1)) {
        printf("OK");
      } else {
        printf("BAD");
      }
    }
    NV_Array_print(&ary);
  }
  exit(0);
}

#define MAX_FILE_NAME_LEN 128
typedef struct {
  char filePath[MAX_FILE_NAME_LEN];
  char dbPath[MAX_FILE_NAME_LEN];
} NV_Args;

void Internal_ParseArgument(NV_Args *args, int argc, char *argv[]) {
  args->filePath[0] = 0;
  args->dbPath[0] = 0;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'v') {
        NV_globalExecFlag |= NV_EXEC_FLAG_VERBOSE;
      }
      if (argv[i][1] == 'g') {
        NV_globalExecFlag |= NV_EXEC_FLAG_SAVECODEGRAPH;
      }
      if (argv[i][1] == 's') {
        i++;
        NV_strncpy(args->dbPath, argv[i], MAX_FILE_NAME_LEN, strlen(argv[i]));
      }
    } else {
      NV_strncpy(args->filePath, argv[i], MAX_FILE_NAME_LEN, strlen(argv[i]));
    }
  }
}

int main(int argc, char *argv[]) {
  NV_ID cTypeList, opDict;
  // set signal handler
  if (signal(SIGINT, NV_signalHandler) == SIG_ERR) return 1;
  //
  NV_Args *args = NV_malloc(sizeof(NV_Args));
  Internal_ParseArgument(args, argc, argv);
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
  rootScope = NV_Node_createWithString("root");
  //
  NV_insertInitialNode();
  NV_Dict_addUniqueEqKeyByCStr(&rootScope, "static", &NODEID_NV_STATIC_ROOT);
  //
  cTypeList = NV_Lang_createCharTypeList(NV_Lang02_charTypeList);
  NV_Dict_addUniqueEqKeyByCStr(&rootScope, "cTypeList", &cTypeList);
  //
  opDict = NV_Lang_createOpDict(NV_Lang02_opList);
  NV_Dict_addUniqueEqKeyByCStr(&rootScope, "opDict", &opDict);
  //
  /*
  NV_globalExecFlag |= NV_EXEC_FLAG_INTERACTIVE;
  NV_evalLine(&cTypeList, &ctx,
                  "loop={for{#args[0]=args[1]}{#args[0]<=args[2]}{#args[0]++}{args[3]()}}");
  */

  if (args->filePath[0]) {
    FILE *fp = fopen(args->filePath, "rb");
    if (!fp) {
      printf("file not found: %s\n", args->filePath);
      exit(EXIT_FAILURE);
    }
    char *buf = NV_malloc(4096 * 1024);
    fread(buf, 1, 4096 * 1024, fp);
    NV_ID tokenList = NV_tokenize(&cTypeList, buf);
    NV_Dict_addUniqueEqKeyByCStr(&rootScope, "currentTokenList", &tokenList);
    //
    NV_ID codeGraphRoot =
        NV_Lang_parseToCodeGraph(&tokenList, &opDict, NV_Lang02_funcList);
    NV_Dict_addUniqueEqKeyByCStr(&rootScope, "currentCodeGraph",
                                 &codeGraphRoot);
    //
    NV_ID result = NV_evalGraph(&codeGraphRoot, &rootScope, NV_Lang02_evalList);
    //
    if (!(NV_globalExecFlag & NV_EXEC_FLAG_SUPRESS_AUTOPRINT)) {
      printf(" = ");
      NV_ID prim = NV_Term_getPrimNodeID(&result, &rootScope);
      NV_Term_print(&prim);
      printf("\n");
    } else {
      NV_globalExecFlag &= ~NV_EXEC_FLAG_SUPRESS_AUTOPRINT;
    }

  } else {
    printf(
        "# nv interpreter\n"
        "# repo:   https://github.com/hikalium/nv \n"
        "# commit: %s\n"
        "# date:   %s\n",
        GIT_COMMIT_ID, GIT_COMMIT_DATE);
    //
    char line[MAX_INPUT_LEN];
    //
    while (NV_gets(line, sizeof(line)) != NULL) {
      NV_ID tokenList = NV_tokenize(&cTypeList, line);
      NV_Dict_addUniqueEqKeyByCStr(&rootScope, "currentTokenList", &tokenList);
      //
      NV_ID codeGraphRoot =
          NV_Lang_parseToCodeGraph(&tokenList, &opDict, NV_Lang02_funcList);
      NV_Dict_addUniqueEqKeyByCStr(&rootScope, "currentCodeGraph",
                                   &codeGraphRoot);
      //
      NV_ID result =
          NV_evalGraph(&codeGraphRoot, &rootScope, NV_Lang02_evalList);
      //
      if (!(NV_globalExecFlag & NV_EXEC_FLAG_SUPRESS_AUTOPRINT)) {
        printf(" = ");
        NV_ID prim = NV_Term_getPrimNodeID(&result, &rootScope);
        NV_Term_print(&prim);
        printf("\n");
      } else {
        NV_globalExecFlag &= ~NV_EXEC_FLAG_SUPRESS_AUTOPRINT;
      }
    }
  }

  //
  return 0;
}

