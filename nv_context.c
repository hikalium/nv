#include "nv.h"

NV_ID NV_getContextList()
{
	return NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_CONTEXT_LIST);
}

NV_ID NV_Context_create()
{
	char st[128];
	char s[128];
	time_t td;
	td = time(NULL);
	strftime(st, sizeof(st), "%F %H:%M:%S", localtime(&td));
	snprintf(s, sizeof(s), "%s %lu", st, clock());
	NV_ID ctx = NV_Node_createWithString(s);
	//
	NV_ID evalStack = NV_Array_create();
	NV_Dict_addUniqueIDKey(&ctx, &RELID_EVAL_STACK, &evalStack);
	//
	NV_ID ctxList = NV_getContextList();
	NV_Array_push(&ctxList, &ctx);
	//
	return ctx;
}

NV_ID NV_Context_getEvalStack(const NV_ID *ctx)
{
	return NV_NodeID_getRelatedNodeFrom(ctx, &RELID_EVAL_STACK);
}

NV_ID NV_Context_createChildScopeWithArgs(const NV_ID *ctx, const NV_ID *argsBlock)
{
	NV_ID newScope;
	NV_ID parentScope = NV_Context_getCurrentScope(ctx);
	newScope = NV_Node_createWithStringFormat("scope");
	NV_Dict_addUniqueIDKey(&newScope, &RELID_PARENT_SCOPE, &parentScope);
	if(argsBlock){
		NV_ID argsVar = NV_Variable_create();
		NV_Variable_assign(&argsVar, argsBlock);
		NV_Dict_addKeyByCStr(&newScope, "args", &argsVar);
	}
	return newScope;
}

void NV_Context_pushToEvalStack
(const NV_ID *ctx, const NV_ID *code, const NV_ID *newScope)
{
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	//
	NV_ID currentScope;
	if(newScope){
		// スコープが指定されていれば、そのスコープをcodeの実行時に使うようにする。
		currentScope = *newScope;	// GLOBAL SCOPE
	} else{
		// NULLならば、現在のコンテキストを親に持つ新しいコンテキストに設定する。
		currentScope = NV_Context_createChildScopeWithArgs(ctx, NULL);
	}
	//
	NV_Dict_addUniqueIDKey(code, &RELID_CURRENT_SCOPE, &currentScope);
	NV_Array_push(&evalStack, code);
	if(IS_DEBUG_MODE()){
		printf("pushed to evalStack: ");
		NV_Array_print(code); putchar('\n');
		printf("evalStack: ");
		NV_Array_print(&evalStack); putchar('\n');
	}
}

NV_ID NV_Context_getCurrentCode(const NV_ID *ctx)
{
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	return NV_Array_last(&evalStack);
}

NV_ID NV_Context_getCurrentScope(const NV_ID *ctx)
{
	NV_ID currentCode = NV_Context_getCurrentCode(ctx);
	return NV_NodeID_getRelatedNodeFrom(&currentCode, &RELID_CURRENT_SCOPE);
}

NV_ID NV_Context_getLastResult(const NV_ID *ctx)
{	
	return NV_NodeID_getRelatedNodeFrom(ctx, &RELID_LAST_RESULT);
}

void NV_Context_setOpDict(const NV_ID *ctx, const NV_ID *opDict)
{
	NV_Dict_addUniqueIDKey(ctx, &RELID_OP_DICT, opDict);
}

NV_ID NV_Context_getOpDict(const NV_ID *ctx)
{	
	return NV_NodeID_getRelatedNodeFrom(ctx, &RELID_OP_DICT);
}
