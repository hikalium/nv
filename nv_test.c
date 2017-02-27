#include "nv.h"
/*
void NV_Test_Memory()
{
	int memcount0;
	NV_ID id, id2, id3, id4;
	//
	memcount0 = NV_getMallocCount();
	//
	id2 = NV_Node_createWithString("Hello");
	//
	id = NV_Node_createWithString("World");
	//
	id3 = NV_Node_createWithInt32(12345);
	//

	id = NV_Array_create();
	NV_Array_print(&id); putchar('\n');

	NV_Array_push(&id, &id2);
	NV_Array_print(&id); putchar('\n');

	NV_Array_push(&id, &id3);
	NV_Array_print(&id); putchar('\n');

	id4 = NV_Array_pop(&id);
	NV_Array_print(&id); putchar('\n');
	NV_Node_printForDebug(NV_NodeID_getNode(&id4)); putchar('\n');

	id4 = NV_Array_pop(&id);
	NV_Array_print(&id); putchar('\n');
	NV_Node_printForDebug(NV_NodeID_getNode(&id4)); putchar('\n');

	id4 = NV_Array_pop(&id);
	NV_Array_print(&id); putchar('\n');
	NV_Node_printForDebug(NV_NodeID_getNode(&id4)); putchar('\n');

	id4 = NV_Array_pop(&id);
	NV_Array_print(&id); putchar('\n');
	NV_Node_printForDebug(NV_NodeID_getNode(&id4)); putchar('\n');

	NV_Array_push(&id, &id2);
	NV_Array_print(&id); putchar('\n');

	id4 = NV_Array_pop(&id);
	NV_Array_print(&id); putchar('\n');
	NV_Node_printForDebug(NV_NodeID_getNode(&id4)); putchar('\n');

	//
	//NV_NodeID_retain(&id);
	//
	//
	printf("mem not freed: %d\n", NV_getMallocCount() - memcount0);
}
*/
/*
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
	NV_NodeID_retain(&root);
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
	//NV_NodeID_retain(&id);
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
*/
