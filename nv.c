#include "nv.h"
//
// Graph
//
NV_Node nodeRoot;

void NV_Graph_init()
{
	NV_ElementID id;
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
	//
	id = NV_Node_createWithID(&NODEID_NULL);
	NV_Node_setStrToID(&id, "NullElement");
	//
	id = NV_Node_createWithID(&NODEID_TREE_TYPE_ARRAY);
	NV_Node_setStrToID(&id, "TreeType(Array)");
	//
	id = NV_Node_createWithID(&NODEID_TREE_TYPE_VARIABLE);
	NV_Node_setStrToID(&id, "TreeType(Variable)");
	//
	id = NV_Node_createWithID(&RELID_TREE_TYPE);
	NV_Node_setStrToID(&id, "relTreeType");
	//
	id = NV_Node_createWithID(&RELID_ARRAY_NEXT);
	NV_Node_setStrToID(&id, "relArrayNext");
	//
	id = NV_Node_createWithID(&RELID_VARIABLE_DATA);
	NV_Node_setStrToID(&id, "relVariableData");
	//
	id = NV_Node_createWithID(&RELID_POINTER_TARGET);
	NV_Node_setStrToID(&id, "relPointerTarget");
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


