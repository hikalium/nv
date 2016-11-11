#include "nv.h"

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

