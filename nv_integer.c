#include "nv.h"

int NV_NodeID_isInteger(const NV_ID *id)
{
	return (NV_Node_getType(id) == kInteger);
}

NV_ID NV_Node_createWithInt32(int32_t v)
{
	NV_ID id = NV_ID_generateRandom();	
	return NV_Node_createWith_ID_Int32(&id, v);
}

NV_ID NV_Node_createWith_ID_Int32(const NV_ID *id, int32_t v)
{
	size_t size = sizeof(int32_t);
	int32_t *buf = NV_malloc(size);
	*buf = v;
	return NV_Node_createWith_ID_Type_Data_Size(id, kInteger, buf, size);
}

int32_t NV_NodeID_getInt32(const NV_ID *id)
{
	const int32_t *v = NV_NodeID_getDataAsType(id, kInteger);
	return v ? *v : -1;
}

