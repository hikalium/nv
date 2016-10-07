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
//
// main
//
int main(int argc, char *argv[])
{
	NV_ElementID id, id2, id3;
	//
	srand(time(NULL));
	printf("mem: %d\n", NV_getMallocCount());
	NV_Graph_init();
	//

printf("mem: %d\n", NV_getMallocCount());
	id = NV_Node_create();
	NV_Node_setStrToID(&id, "Hello, world!");
printf("mem: %d\n", NV_getMallocCount());
	NV_Node_remove(&id);
printf("mem: %d\n", NV_getMallocCount());
	//
	
/*
	id2 = NV_Node_add();
	NV_Node_setStrToID(&id2, "Hello");
	//
	id = NV_Node_add();
	NV_Node_setStrToID(&id, "World");
	NV_Node_clone(&id);
	NV_Node_setStrToID(&id, "World2");
	//
	id3 = NV_Node_add();
	NV_Node_setInt32ToID(&id3, 12345);
	//
	id = NV_Array_create();
	NV_Array_print(&id);
	NV_Array_push(&id, &id2);
	NV_Array_print(&id);
	NV_Array_push(&id, &id3);
	NV_Array_print(&id);
	NV_Array_push(&id, &id2);
	NV_Array_print(&id);
	//
	id = NV_Variable_create();
NV_Variable_print(&id);
	NV_Node_setStrToID(&id, "var1");
NV_Variable_print(&id);
	NV_Variable_assign(&id, &id2);
NV_Variable_print(&id);
	//
*/	

	NV_Graph_dump();
	printf("mem: %d\n", NV_getMallocCount());
	return 0;
}


