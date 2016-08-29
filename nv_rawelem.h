// @nv_dict.c
NV_DictItem *NV_E_allocDictItem();
void NV_E_free_internal_DictItem(NV_Pointer item);
void NV_E_free_internal_Dict(NV_Pointer dict);
// @nv_env.c
NV_Env *NV_E_allocEnv();
void NV_E_free_internal_Env(NV_Pointer item);
// @nv_integer.c
NV_Integer *NV_E_allocInteger();
// @nv_lang.c
NV_Lang *NV_E_allocLang();
void NV_E_free_internal_Lang(NV_Pointer p);
// @nv_list.c
NV_ListItem *NV_E_allocListItem();
void NV_E_free_internal_ListItem(NV_Pointer item);
void NV_E_free_internal_List(NV_Pointer root);
// @nv_operator.c
NV_Operator *NV_E_allocOperator();
// @nv_string.c
NV_String *NV_E_allocString();
void NV_E_free_internal_String(NV_Pointer p);
// @nv_var.c
NV_Variable *NV_E_allocVariable();
void NV_E_free_internal_Variable(NV_Pointer p);

