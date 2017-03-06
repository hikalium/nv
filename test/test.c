#include "../nv.h"

int main(int argc, char *argv[])
{
	NV_Graph_insertInitialNode();
	//
	NV_ID a, s;
	a = NV_Array_create();
	NV_Array_print(&a); putchar('\n');
	printf("%d\n", NV_Array_count(&a));
	//
	s = NV_Node_createWithString("test");
	NV_Array_push(&a, &s);
	NV_Array_print(&a); putchar('\n');
	printf("%d\n", NV_Array_count(&a));
	//
	s = NV_Node_createWithString("hoge");
	NV_Array_push(&a, &s);
	NV_Array_print(&a); putchar('\n');
	printf("%d\n", NV_Array_count(&a));
	//
	NV_Dict_print(&a); putchar('\n');
	//
	return 0;
}
