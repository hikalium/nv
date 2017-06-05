#include "nv.h"
#include "nv_node.h"

//
// Internal
//

void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(n){
		if(n->type != kNone){
			// NV_Node_Internal_resetData(n);
			printf("Try to modify data existed. abort.");
			exit(EXIT_FAILURE);
		}
		n->type = kInteger;
		n->size = sizeof(int32_t);
		n->data = NV_malloc(n->size);
		*((int32_t *)n->data) = v;
	}
}


//
// Integer
//

int NV_NodeID_isInteger(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(!n || n->type != kInteger) return 0;
	return 1;
}

NV_ID NV_Node_createWithInt32(int32_t v)
{
	NV_ID id;
	id = NV_Node_create();
	NV_Node_Internal_setInt32ToID(&id, v);
	return id;
}

int32_t NV_NodeID_getInt32(const NV_ID *id)
{
	NV_Node *n;
	//
	n = NV_NodeID_getNode(id);
	if(!n || n->type != kInteger) return -1;
	return *((int32_t *)n->data);
}

