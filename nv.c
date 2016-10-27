#include "nv.h"
//
// Graph
//
NV_Node nodeRoot;

void NV_Graph_addStaticNode(const NV_ID *id, const char *s){
	NV_Node_createWithID(id);
	NV_Node_setStrToID(id, s);
	NV_Node_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_init()
{
	srand(time(NULL));
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
	//
	NV_Node_createWithID(&NODEID_NV_STATIC_ROOT);
	NV_Node_setStrToID(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_Node_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_Graph_addStaticNode(&NODEID_NULL, "NullElement");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_ARRAY, "TreeType(Array)");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_VARIABLE, "TreeType(Variable)");
	NV_Graph_addStaticNode(&RELID_TREE_TYPE, "relTreeType");
	NV_Graph_addStaticNode(&RELID_ARRAY_NEXT, "relArrayNext");
	NV_Graph_addStaticNode(&RELID_VARIABLE_DATA, "relVariableData");
	NV_Graph_addStaticNode(&RELID_POINTER_TARGET, "relPointerTarget");
}


void NV_Graph_dump()
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
}

void NV_Graph_saveToFileName(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
	fclose(fp);
}

int NV_isTreeType(const NV_ID *node, const NV_ID *tType)
{
	NV_ID typeID = NV_Node_getRelatedNodeFrom(node, &RELID_TREE_TYPE);
	return NV_ID_isEqual(&typeID, tType);
}
/*
NV_ID NV_putDataToPath(const NV_ID *base, const char s[], const NV_ID *data)
{
	
}
*/

void NV_Test_Memory()
{
	int memcount0;
	NV_ID id, id2, id3;
	//
	memcount0 = NV_getMallocCount();
	//
	id2 = NV_Node_create();
	NV_Node_setStrToID(&id2, "Hello");
	//
	id = NV_Node_create();
	NV_Node_setStrToID(&id, "World");
	//
	id3 = NV_Node_create();
	NV_Node_setInt32ToID(&id3, 12345);
	//

	id = NV_Array_create();
	NV_Array_print(&id);
	NV_Array_push(&id, &id2);
	NV_Array_print(&id);
	NV_Array_push(&id, &id3);
	NV_Array_print(&id);
	//
	NV_Node_setStrToID(&id2, "World");
	NV_Array_push(&id, &id2);
	NV_Array_print(&id);
	//
	//NV_Node_retain(&id);
	//
	NV_Graph_dump();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Graph_dump();

	//
	printf("mem not freed: %d\n", NV_getMallocCount() - memcount0);
}

void NV_Test_Dict()
{
	int memcount0;
	NV_ID root, k, v;
	//
	memcount0 = NV_getMallocCount();
	//
	root = NV_Node_create();
	NV_Node_setStrToID(&root, "testDict");
	k = NV_Node_createWithString("hello");
	v = NV_Node_createWithString("world");
	NV_Dict_add(&root, &k, &v);
	NV_Dict_add(&root, &v, &k);
	//
	NV_Dict_print(&root);
	//
	NV_Node_retain(&root);
	//
	NV_Graph_dump();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Graph_dump();
	NV_Dict_print(&root);

	//
	printf("mem not freed: %d\n", NV_getMallocCount() - memcount0);
}

void NV_Test_Data()
{
	int memcount0;
	NV_ID id, id2;
	//
	memcount0 = NV_getMallocCount();
	//
	id2 = NV_Node_create();
	NV_Node_setStrToID(&id2, "");
	//
	id = NV_Node_create();
	NV_Node_setStrToID(&id, "World");
	printf("%d %d\n",
		NV_ID_isEqualInValue(&id, &id2),
		NV_ID_isEqual(&id, &id2));
	NV_Node_setStrToID(&id, "");
	printf("%d %d\n",
		NV_ID_isEqualInValue(&id, &id2),
		NV_ID_isEqual(&id, &id2));
	//
	//NV_Node_retain(&id);
	//
	NV_Graph_dump();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Node_cleanup();
	NV_Graph_dump();

	//
	printf("mem not freed: %d\n", NV_getMallocCount() - memcount0);
}

int NV_runInteractive(const NV_ID *cTypeList)
{
	char line[MAX_INPUT_LEN];
	//
	while(NV_gets(line, sizeof(line)) != NULL){
		NV_tokenize(cTypeList, line);
	}
	return 0;
}

#define NV_LANG_CHAR_LIST_LEN 3
int NV_Lang_getCharType(const NV_ID *cTypeList, char c)
{
	NV_ID t;
	int i;
	if(c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		t = NV_Array_getByIndex(cTypeList, i);
		if(NV_Node_String_strchr(NV_Node_getByID(&t), c)) break;
	}
	return i;
}


NV_ID NV_createCharTypeList()
{
	NV_ID ns;
	NV_ID cList = NV_Array_create();
	//
	ns = NV_Node_createWithString(" \t\r\n");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("#!%&-=^~|+*:.<>/");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("(){}[],;\"`\\");
	NV_Array_push(&cList, &ns);
	//
	return cList;
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
				//NV_E_setFlag(t, EFUnknownToken);
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
	NV_Array_print(&tokenList);
	return tokenList;
}
//
// main
//
int32_t NV_debugFlag;
int main(int argc, char *argv[])
{
	NV_ID cTypeList;
	NV_Graph_init();
	//
	cTypeList = NV_createCharTypeList();
	NV_Node_retain(&cTypeList);
	//NV_Test_Data();
	//NV_Test_Dict();
	NV_runInteractive(&cTypeList);
	//NV_tokenize(&cTypeList, "test 123");
	return 0;
}


