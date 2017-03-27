#include "nv.h"
//
// Graph
//

//
// Internal
//
void NV_Graph_addStaticNode(const NV_ID *id, const char *s)
{
	int f = NV_NodeID_exists(id);
	//
	NV_NodeID_createAndString(id, s);
	if(!f){
		NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
	}
}

typedef struct NV_STATIC_NODE_NAME_TAG NV_StaticNodeNameTag;
struct NV_STATIC_NODE_NAME_TAG {
	const NV_ID *id;
	const char *s;
};

NV_StaticNodeNameTag staticNodeNameList[] = {
	{&NODEID_NULL, "NullElement"},
	//
	{&NODEID_TERM_TYPE_ARRAY, "Type(Array)"},
	{&NODEID_TERM_TYPE_VARIABLE, "Type(Variable)"},
	{&NODEID_TERM_TYPE_OP, "Type(Op)"},
	//
	{&RELID_TERM_TYPE, "type"},
	{&RELID_ARRAY_NEXT, "next"},
	{&RELID_VARIABLE_DATA, "data"},
	{&RELID_POINTER_TARGET, "target"},
	{&RELID_OP_PRECEDENCE, "precedence"},
	{&RELID_OP_FUNC, "func"},
	{&RELID_EVAL_STACK, "evalStack"},
	{&RELID_LAST_RESULT, "lastResult"},
	{&RELID_CURRENT_TERM_INDEX, "currentTermIndex"},
	{&RELID_CURRENT_TERM_PHASE, "currentTermPhase"},
	{&RELID_CURRENT_SCOPE, "currentScope"},
	{&RELID_PARENT_SCOPE, "parentScope"},
	{&RELID_CONTEXT_LIST, "contextList"},
	//
	{NULL, NULL}
};

void NV_Graph_initStaticNodes()
{
	NV_NodeID_createAndString(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_NodeID_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_StaticNodeNameTag *t;
	int i;
	for(i = 0; ; i++){
		t = &staticNodeNameList[i];
		if(!t->s) break;
		NV_Graph_addStaticNode(t->id, t->s); 
	}
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
	// contextListが存在しなければ作成する
	NV_ID contextList;
	contextList = NV_getContextList();
	if(NV_NodeID_isEqual(&contextList, &NODEID_NOT_FOUND)){
		contextList = NV_Array_create();
		NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, 
			&RELID_CONTEXT_LIST, &contextList);
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
		NV_Node_dump(&n->id); putchar('\n');
	}
}

void NV_Graph_dumpToFile(FILE *fp)
{
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_fdump(fp, &n->id); fputc('\n', fp);
	}
}

void NV_Graph_restoreFromFile(FILE *fp)
{
	char s[MAX_SAVE_DATA_ENTRY_SIZE];

	while(fgets(s, sizeof(s), fp)){
		NV_Node_restoreFromString(s);
	}
}

