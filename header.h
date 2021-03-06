
// @main.c
int main(int argc, char *argv[]);


// @nv.c
int NV_interactiveInput(const NV_ID *cTypeList, const NV_ID *ctx);
int NV_evalLine(const NV_ID *cTypeList, const NV_ID *ctx, const char *line);
NV_ID NV_tokenize(const NV_ID *cTypeList, const char *input);
int NV_getNextOpIndex(const NV_ID *currentBlock, const NV_ID *opDict);
int NV_checkAndPrintErrorOfCodeBlock(const NV_ID *code);
void NV_evalLoop(const NV_ID *opList, const NV_ID *ctx);
void NV_saveCodeGraph_digForKey(const NV_ID *n, const char *key);
void NV_saveCodeGraph_trace(const NV_ID *n);
void NV_saveCodeGraphForVisualization(const NV_ID *codeGraphRoot, const char *path);


// @nv_array.c
void NV_Array_Internal_setCount(const NV_ID *array, int32_t count);
int32_t NV_Array_Internal_getCount(const NV_ID *array);
void NV_Array_Internal_updateCountRel(const NV_ID *array, int32_t diff);
NV_ID NV_Array_create();
NV_ID NV_Array_clone(const NV_ID *base);
NV_ID NV_Array_push(const NV_ID *array, const NV_ID *data);
NV_ID NV_Array_pop(const NV_ID *array);
NV_ID NV_Array_last(const NV_ID *array);
int32_t NV_Array_count(const NV_ID *array);
NV_ID NV_Array_getAssignableByIndex(const NV_ID *array, int index);
NV_ID NV_Array_getByIndex(const NV_ID *array, int index);
void NV_Array_removeIndex(const NV_ID *array, int index);
void NV_Array_writeToIndex(const NV_ID *array, int index, const NV_ID *data);
NV_ID NV_Array_joinWithCStr(const NV_ID *array, const char *sep);
NV_ID NV_Array_getSorted(const NV_ID *array, int (*f)(const void *n1, const void *n2));
void NV_Array_print(const NV_ID *array);


// @nv_dict.c
NV_ID NV_Dict_addKey
(const NV_ID *root, const NV_ID *key, const NV_ID *value);
NV_ID NV_Dict_addKeyByCStr
(const NV_ID *root, const char *key, const NV_ID *value);
NV_ID NV_Dict_addUniqueIDKey
(const NV_ID *root, const NV_ID *key, const NV_ID *value);
NV_ID NV_Dict_addUniqueEqKeyByCStr
(const NV_ID *root, const char *key, const NV_ID *value);
NV_ID NV_Dict_removeUniqueIDKey(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_removeUniqueEqKeyByCStr(const NV_ID *root, const char *key);
NV_ID NV_Dict_get(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getEqID(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getAll(const NV_ID *root, const NV_ID *key);
NV_ID NV_Dict_getByStringKey
(const NV_ID *root, const char *key);
void NV_Dict_print(const NV_ID *root);


// @nv_driver.c
char *NV_gets(char *str, int size);
void atexit_recover_termios(void);
char *NV_gets(char *str, int size);


// @nv_fix.c
char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size);
long NV_strtolSeq(const char **restrict p, int base);
int NV_getMallocCount();
void *NV_malloc(size_t size);
void NV_free(void *p);


// @nv_id.c
NV_ID NV_ID_generateRandom();
int NV_ID_setFromString(NV_ID *id, const char *s);
void NV_ID_dumpIDToFile(const NV_ID *id, FILE *fp);


// @nv_node.c
NV_ID NV_NodeID_createNew(const NV_ID *id);
void NV_Node_Internal_resetData(NV_Node *n);
void NV_Node_Internal_remove(NV_Node *n);
void NV_Node_Internal_removeAllRelationFrom(const NV_ID *from);
int NV_Node_Internal_isEqualInValue(const NV_Node *na, const NV_Node *nb);
void NV_Node_initRoot();
int NV_Node_getNodeCount();
NV_ID NV_NodeID_create(const NV_ID *id);
NV_ID NV_Node_create();
int NV_NodeID_isEqual(const NV_ID *a, const NV_ID *b);
int NV_NodeID_isEqualInValue(const NV_ID *a, const NV_ID *b);
int NV_NodeID_exists(const NV_ID *id);
NV_Node *NV_NodeID_getNode(const NV_ID *id);
NV_NodeType NV_Node_getType(const NV_ID *id);
void *NV_Node_getDataAsType(const NV_ID *id, NV_NodeType type);
void NV_Node_dumpAll();
void NV_Node_dumpAllToFile(FILE *fp);
void NV_Node_restoreFromFile(FILE *fp);
void NV_NodeID_remove(const NV_ID *baseID);
NV_ID NV_NodeID_clone(const NV_ID *baseID);
NV_ID NV_Node_restoreFromString(const char *s);
void NV_NodeID_retain(const NV_ID *id);
void NV_NodeID_release(const NV_ID *id);
void NV_Node_cleanup();
void NV_Node_fdump(FILE *fp, const NV_ID *id);
void NV_Node_dump(const NV_ID *id);
void NV_Node_printPrimVal(const NV_ID *id);
void NV_NodeID_printForDebug(const NV_ID *id);


// @nv_op.c
void NV_Op_Internal_setCurrentPhase(const NV_ID *opList, int32_t phase);
int32_t NV_Op_Internal_getCurrentPhase(const NV_ID *opList);
int NV_Lang_getCharType(const NV_ID *cTypeList, char c);
NV_ID NV_createCharTypeList();
void NV_addOp(const NV_ID *opDict, const char *token, int32_t prec, const NV_ID *func);
void NV_addBuiltinOp(const NV_ID *opDict, const char *token, int32_t prec, const char *funcStr);
int NV_isBuiltinOp(const NV_ID *term, const char *ident);
NV_ID NV_createOpDict();
int32_t NV_getOpPrec(const NV_ID *op);
void NV_getOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, NV_ID *idBuf, int count);
void NV_removeOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, int count);
NV_ID NV_Op_ExecBuiltinInfix
(const NV_ID *tList, int index, int func, const NV_ID *ctx);
NV_ID NV_Op_save(const NV_ID *tList, int index);
NV_ID NV_Op_restore(const NV_ID *tList, int index);
NV_ID NV_Op_ls(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_ls2(const NV_ID *tList, int index);
NV_ID NV_Op_lsctx(const NV_ID *tList, int index);
NV_ID NV_Op_swctx(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_last(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_info(const NV_ID *tList, int index);
NV_ID NV_Op_clean(const NV_ID *tList, int index);
NV_ID NV_Op_assign(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_push(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_codeBlock
(const NV_ID *tList, int index, const char *openTerm, const char *closeTerm);
NV_ID NV_Op_strLiteral(const NV_ID *tList, int index);
NV_ID NV_Op_if(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_for(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_print(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_out(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_fmt(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_unaryPrefix(const NV_ID *tList, int index, int mod, const NV_ID *ctx);
NV_ID NV_Op_unbox(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_unaryPostfixAssignable(const NV_ID *tList, int index, int mod, const NV_ID *ctx);
NV_ID NV_Op_callArgs(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_arrayAccessor(const NV_ID *tList, int index, const NV_ID *ctx);
NV_ID NV_Op_pathSeparator(const NV_ID *tList, int index, const NV_ID *ctx);
void NV_tryExecOpAt(const NV_ID *tList, int index, const NV_ID *ctx);
void NV_printOp(const NV_ID *op);


// @nv_static.c
const NV_ID NODEID_NV_STATIC_ROOT
const NV_ID NODEID_NULL
const NV_ID NODEID_NOT_FOUND
const NV_ID NODEID_TERM_TYPE_ARRAY
const NV_ID NODEID_TERM_TYPE_VARIABLE
const NV_ID NODEID_TERM_TYPE_OP
const NV_ID NODEID_TERM_TYPE_PATH
const NV_ID RELID_ARRAY_NEXT
const NV_ID RELID_ARRAY_COUNT
const NV_ID RELID_VARIABLE_DATA
const NV_ID RELID_POINTER_TARGET
const NV_ID RELID_OP_PRECEDENCE
const NV_ID RELID_OP_FUNC
const NV_ID RELID_TERM_TYPE
const NV_ID RELID_EVAL_STACK
const NV_ID RELID_LAST_RESULT
const NV_ID RELID_CURRENT_TERM_INDEX
const NV_ID RELID_CURRENT_TERM_PHASE
const NV_ID RELID_CURRENT_SCOPE
const NV_ID RELID_PARENT_SCOPE
const NV_ID RELID_CONTEXT_LIST
const NV_ID RELID_NEXT_CONTEXT
const NV_ID RELID_OP_DICT
void NV_Graph_addStaticNode(const NV_ID *id, const char *s);
void NV_Graph_initStaticNodes();
void NV_insertInitialNode();


// @nv_variable.c
NV_ID NV_Variable_create();
NV_ID NV_Variable_createWithName(const NV_ID *parentNode, const NV_ID *nameNode);
NV_ID NV_Variable_getNamed(const NV_ID *parentNode, const NV_ID *nameNode);
void NV_Variable_assign(const NV_ID *vid, const NV_ID *data);
NV_ID NV_Variable_getData(const NV_ID *vid);
void NV_Variable_print(const NV_ID *vid);
void NV_Variable_printPrimVal(const NV_ID *vid);


// @nv_term.c
int NV_isTermType(const NV_ID *node, const NV_ID *tType);
NV_ID NV_Term_tryReadAsVariableData(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_tryReadAsVariable(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_getPrimNodeID(const NV_ID *id, const NV_ID *scope);
int NV_Term_f_OpPrec_Dec(const void *n1, const void *n2);
NV_ID NV_Term_tryReadAsOperator(const NV_ID *id, const NV_ID *opDict);
int NV_Term_isIntegerNotVal(const NV_ID *id);
int NV_Term_isInteger(const NV_ID *id, const NV_ID *scope);
int NV_Term_isAssignable(const NV_ID *id, const NV_ID *scope);
int NV_Term_isArray(const NV_ID *id, const NV_ID *scope);
int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_getAssignableNode(const NV_ID *id, const NV_ID *scope);
NV_ID NV_Term_assign(const NV_ID *v, const NV_ID *data);
void NV_Term_print(const NV_ID *id);


// @nv_signal.c
void NV_signalHandler(int signum);


// @nv_path.c
NV_ID NV_Path_createWithOrigin(const NV_ID *origin);
NV_ID NV_Path_createAbsoluteWithCodeBlock(NV_ID *code);
NV_ID NV_Path_createAbsoluteWithCStr(const char *pathStr);
void NV_Path_appendRoute(const NV_ID *path, const NV_ID *r);
NV_ID NV_Path_getTarget(const NV_ID *path);
int NV_Path_statTarget(const NV_ID *path);
void NV_Path_assign(const NV_ID *path, const NV_ID *data);
NV_ID NV_Path_print(const NV_ID *path);


// @nv_integer.c
void NV_Node_Internal_setInt32ToID(const NV_ID *id, int32_t v);
int NV_NodeID_isInteger(const NV_ID *id);
NV_ID NV_Node_createWithInt32(int32_t v);
int32_t NV_NodeID_getInt32(const NV_ID *id);


// @nv_string.c
void NV_Node_Internal_setStrToID(const NV_ID *id, const char *s);
long NV_Node_String_Internal_strtol(const NV_Node *ns, int *endptrindex, int base);
int NV_NodeID_isString(const NV_ID *id);
NV_ID NV_Node_createWithString(const char *s);
NV_ID NV_Node_createWithStringFormat(const char *fmt, ...);
void NV_NodeID_createAndString(const NV_ID *id, const char *s);
const char *NV_NodeID_getCStr(const NV_ID *id);
int NV_Node_String_compare(const NV_ID *ida, const NV_ID *idb);
int NV_Node_String_compareWithCStr(const NV_ID *ida, const char *s);
char *NV_Node_String_strchr(const NV_ID *id, char c);
long NV_Node_String_strtol(const NV_ID *ns, int *endptrindex, int base);
size_t NV_Node_String_strlen(const NV_ID *id);


// @nv_relation.c
NV_ID NV_NodeID_createRel_OnDupUpdate
(const NV_ID *from, const NV_ID *rel, const NV_ID *to, 
NV_ID (*find)(const NV_ID *from, const NV_ID *rel));
int NV_Node_isLiveRelation(const NV_ID *relnid);
NV_ID NV_NodeID_createRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to);
NV_ID NV_NodeID_createUniqueIDRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to);
NV_ID NV_NodeID_createUniqueEqRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to);
void NV_Node_setRelation
(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to);
NV_Node *NV_NodeID_Relation_getLinkFrom(const NV_ID *relnid);
NV_ID NV_NodeID_Relation_getIDLinkTo(const NV_ID *relnid);
NV_Node *NV_NodeID_Relation_getLinkTo(const NV_ID *relnid);
NV_ID NV_NodeID_Relation_getIDLinkRel(const NV_ID *relnid);
NV_Node *NV_NodeID_Relation_getLinkRel(const NV_ID *relnid);
void NV_NodeID_updateRelationTo(const NV_ID *relnid, const NV_ID *to);
const NV_Node *NV_NodeID_getRelNodeFromWithCmp
(const NV_ID *from, const NV_ID *rel, int (*cmp)(const NV_ID *p, const NV_ID *q));
NV_ID NV_NodeID_getRelationFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getEqRelationFrom(const NV_ID *from, const NV_ID *rel);
NV_ID NV_NodeID_getEqRelatedNodeFrom(const NV_ID *from, const NV_ID *rel);


// @nv_context.c
NV_ID NV_getContextList();
NV_ID NV_Context_create();
NV_ID NV_Context_getEvalStack(const NV_ID *ctx);
NV_ID NV_Context_createChildScopeWithArgs(const NV_ID *ctx, const NV_ID *argsBlock);
void NV_Context_pushToEvalStack
(const NV_ID *ctx, const NV_ID *code, const NV_ID *newScope);
NV_ID NV_Context_getCurrentCode(const NV_ID *ctx);
NV_ID NV_Context_getCurrentScope(const NV_ID *ctx);
NV_ID NV_Context_getLastResult(const NV_ID *ctx);
void NV_Context_setOpDict(const NV_ID *ctx, const NV_ID *opDict);
NV_ID NV_Context_getOpDict(const NV_ID *ctx);


// @lang/02/parse.c
void NV_parseToCodeGraph_infixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName);
void NV_parseToCodeGraph_prefixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName);
void NV_parseToCodeGraph_postfixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName);
void NV_parseToCodeGraph_if
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict);
NV_ID NV_parseToCodeGraph_for
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict);
NV_ID NV_parseToCodeGraph(const NV_ID *tokenList, const NV_ID *opDict);


// @lang/02/eval.c
NV_ID NV_Lang02_OpFunc_infixOp(const NV_ID *p, NV_ID *lastEvalVal);
NV_ID NV_Lang02_OpFunc_prefixOp(const NV_ID *p, NV_ID *lastEvalVal);
NV_ID NV_Lang02_OpFunc_postfixOp(const NV_ID *p, NV_ID *lastEvalVal);
NV_ID NV_Lang02_OpFunc_cond(NV_ID *p, NV_ID *lastEvalVal);
NV_ID NV_Lang02_OpFunc_do(NV_ID *p, NV_ID *lastEvalVal);
NV_ID NV_evalGraph(const NV_ID *codeGraphRoot);

