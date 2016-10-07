#include "nv.h"

//
// Graph
//
NV_Node *nodeRoot;

void NV_Graph_init()
{
	NV_ElementID id;
	nodeRoot = NULL;
	//
	id = NV_Node_addWithID(NODEID_NULL);
	NV_Node_setStrToID(id, "NullElement");
	//
	id = NV_Node_addWithID(NODEID_TREE_TYPE_ARRAY);
	NV_Node_setStrToID(id, "TreeType(Array)");
	//
	id = NV_Node_addWithID(NODEID_TREE_TYPE_VARIABLE);
	NV_Node_setStrToID(id, "TreeType(Variable)");
	//
	id = NV_Node_addWithID(RELID_TREE_TYPE);
	NV_Node_setStrToID(id, "relTreeType");
	//
	id = NV_Node_addWithID(RELID_ARRAY_NEXT);
	NV_Node_setStrToID(id, "relArrayNext");
	//
	id = NV_Node_addWithID(RELID_VARIABLE_DATA);
	NV_Node_setStrToID(id, "relVariableData");
	//
	id = NV_Node_addWithID(RELID_POINTER_TARGET);
	NV_Node_setStrToID(id, "relPointerTarget");
}

void NV_Graph_dump()
{
	NV_Node *n;
	//
	for(n = nodeRoot; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
}

int NV_isTreeType(NV_ElementID node, NV_ElementID tType)
{
	return NV_ElementID_isEqual(
		NV_Node_getRelatedNodeFrom(node, RELID_TREE_TYPE),
		tType
	);
}



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
	id2 = NV_Node_add();
	NV_Node_setStrToID(id2, "Hello");
	//
	id = NV_Node_add();
	NV_Node_setStrToID(id, "World");
	NV_Node_clone(id);
	NV_Node_setStrToID(id, "World2");
	//
	id3 = NV_Node_add();
	NV_Node_setInt32ToID(id3, 12345);
	//
	id = NV_Array_create();
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	NV_Array_push(id, id3);
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	//
	id = NV_Variable_create();
NV_Variable_print(id);
	NV_Node_setStrToID(id, "var1");
NV_Variable_print(id);
	NV_Variable_assign(id, id2);
NV_Variable_print(id);
	//
	NV_Graph_dump();
	//
	printf("mem: %d\n", NV_getMallocCount());

	return 0;
}


