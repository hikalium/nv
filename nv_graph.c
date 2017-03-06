#include "nv.h"
//
// Graph
//

//
// Internal
//
void NV_Graph_addStaticNode(const NV_ID *id, const char *s)
{
	NV_NodeID_createAndString(id, s);
	NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_initStaticNodes()
{
	NV_NodeID_createAndString(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_NodeID_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_Graph_addStaticNode(&NODEID_NULL, "NullElement");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_ARRAY, "TreeType(Array)");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_VARIABLE, "TreeType(Variable)");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_OP, "TreeType(Op)");
	//
	NV_Graph_addStaticNode(&RELID_TERM_TYPE, "relTreeType");
	NV_Graph_addStaticNode(&RELID_ARRAY_NEXT, "relArrayNext");
	//NV_Graph_addStaticNode(&RELID_ARRAY_COUNT, "relArrayCount");
	NV_Graph_addStaticNode(&RELID_VARIABLE_DATA, "relVariableData");
	NV_Graph_addStaticNode(&RELID_POINTER_TARGET, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_PRECEDENCE, "relOpPrec");
	NV_Graph_addStaticNode(&RELID_OP_FUNC, "relOpFunc");
	NV_Graph_addStaticNode(&RELID_EVAL_STACK, "relEvalStack");
	NV_Graph_addStaticNode(&RELID_LAST_RESULT, "relLastResult");
	NV_Graph_addStaticNode(&RELID_CURRENT_TERM_INDEX, "relCurrentTermIndex");
	NV_Graph_addStaticNode(&RELID_CURRENT_TERM_PHASE, "relCurrentTermPhase");
}

void NV_Graph_init()
{
	srand(time(NULL));
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
}

void NV_Graph_insertInitialNode()
{
	NV_Graph_initStaticNodes();
	// evalStackが存在しなければ作成する
	NV_ID evalStack;
	evalStack = NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK);
	if(NV_ID_isEqual(&evalStack, &NODEID_NOT_FOUND)){
		evalStack = NV_Array_create();
		NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK, &evalStack);
	}
}

//
// Public
//
NV_Node nodeRoot;

void NV_Graph_dump()
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
}

void NV_Graph_dumpToFile(FILE *fp)
{
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_fdump(fp, n); fputc('\n', fp);
	}
}

void NV_Graph_restoreFromFile(FILE *fp)
{
	char s[MAX_SAVE_DATA_ENTRY_SIZE];

	while(fgets(s, sizeof(s), fp)){
		NV_Node_restoreFromString(s);
	}
}

