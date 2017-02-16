#include "nv.h"
//
// Graph
//

//
// Internal
//
void NV_Graph_addStaticNode(const NV_ID *id, const char *s)
{
	NV_Node_createWithIDAndString(id, s);
	NV_Node_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_initStaticNodes()
{
	NV_Node_createWithIDAndString(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_Node_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_Graph_addStaticNode(&NODEID_NULL, "NullElement");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_ARRAY, "TreeType(Array)");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_VARIABLE, "TreeType(Variable)");
	NV_Graph_addStaticNode(&NODEID_TERM_TYPE_OP, "TreeType(Op)");
	//
	NV_Graph_addStaticNode(&RELID_TERM_TYPE, "relTreeType");
	NV_Graph_addStaticNode(&RELID_ARRAY_NEXT, "relArrayNext");
	NV_Graph_addStaticNode(&RELID_VARIABLE_DATA, "relVariableData");
	NV_Graph_addStaticNode(&RELID_POINTER_TARGET, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_PRECEDENCE, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_FUNC, "relPointerTarget");
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
	NV_Graph_initStaticNodes();
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

