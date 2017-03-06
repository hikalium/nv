//
// Node Internal
//

NV_ID NV_NodeID_createNew(const NV_ID *id);
void NV_Node_Internal_resetData(NV_Node *n);
void NV_Node_Internal_remove(NV_Node *n);
void NV_Node_Internal_removeAllRelationFrom(const NV_ID *from);
void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s);
void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v);

