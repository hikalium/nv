#include "nv.h"

//
// Graph
//
NV_Node *nodeRoot;
NV_Edge *edgeRoot;

void NV_Graph_init()
{
	NV_ElementID id;
	nodeRoot = NULL;
	edgeRoot = NULL;
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

void NV_Graph_dumpNode(NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	printf("%08X %d ", n->id.d[0], n->type);
	if(n->type == kString){
		printf("%s", n->data);
	}
}

void NV_Graph_printNodePrimVal(NV_Node *n)
{
	if(!n){
		printf("(null)");
		return;
	}
	if(n->type == kString){
		printf("\"%s\"", n->data);
	}
}

void NV_Graph_dumpEdge(NV_Edge *e)
{
	if(!e){
		printf("(null)");
		return;
	}
	printf("%08X %08X -- %08X -> %08X", e->id.d[0], e->from.d[0], e->rel.d[0], e->to.d[0]);
}

void NV_Graph_dump()
{
	NV_Node *n;
	NV_Edge *e;
	//
	puts("Node:");
	for(n = nodeRoot; n; n = n->next){
		NV_Graph_dumpNode(n); putchar('\n');
	}
	puts("Edge:");
	for(e = edgeRoot; e; e = e->next){
		NV_Graph_dumpEdge(e); putchar('\n');
	}
}

int NV_isTreeType(NV_ElementID node, NV_ElementID tType)
{
	return NV_ElementID_isEqual(
		NV_Node_getConnectedFrom(node, RELID_TREE_TYPE), tType);
}



//
// main
//


int main(int argc, char *argv[])
{
	NV_ElementID id, id2;
	//
	srand(time(NULL));
	NV_Graph_init();
	//
	id2 = NV_Node_add();
	NV_Node_setStrToID(id2, "Hello");
	//
	id = NV_Node_add();
	NV_Node_setStrToID(id, "World");
	id = NV_Node_clone(id);
	//
	id = NV_Array_create();
	NV_Array_print(id);
	NV_Array_push(id, id2);
	NV_Array_print(id);
	NV_Array_push(id, id2);
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

	return 0;
}


