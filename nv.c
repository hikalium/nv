#include "nv.h"
//
// Graph
//
NV_Node nodeRoot;

void NV_Graph_addStaticNode(const NV_ElementID *id, const char *s){
	NV_Node_createWithID(id);
	NV_Node_setStrToID(id, s);
	NV_Node_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_init()
{
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

int NV_isTreeType(const NV_ElementID *node, const NV_ElementID *tType)
{
	NV_ElementID typeID = NV_Node_getRelatedNodeFrom(node, &RELID_TREE_TYPE);
	return NV_ElementID_isEqual(&typeID, tType);
}
/*
NV_ElementID NV_putDataToPath(const NV_ElementID *base, const char s[], const NV_ElementID *data)
{
	
}
*/

void NV_Test_Memory()
{
	int memcount0;
	NV_ElementID id, id2, id3;
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
/*
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
	NV_Node_remove(&id);
	//
*/
	NV_Graph_dump();
	NV_Node_cleanup();
	NV_Graph_dump();

	
	//
	printf("mem not freed: %d\n", NV_getMallocCount() - memcount0);
}

//
// main
//
int main(int argc, char *argv[])
{
	srand(time(NULL));
	NV_Graph_init();
	//
	NV_Test_Memory();
	//
	return 0;
}


