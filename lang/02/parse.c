#include "../../nv.h"

NV_ID NV_createCharTypeList()
{
	NV_ID ns;
	NV_ID cList = NV_Array_create();
	//
	ns = NV_Node_createWithString(" \t\r\n");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("#!%&-=^~|+*:.<>/$");
	NV_Array_push(&cList, &ns);
	ns = NV_Node_createWithString("(){}[],;\"`\\");
	NV_Array_push(&cList, &ns);
	//
	return cList;
}

typedef struct NV_BUILTIN_OP_TAG {
	const char *token;
	int prec;
	const char *funcStr;
	//int (*parser)(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident);
} NV_BuiltinOpTag;

NV_BuiltinOpTag builtinOpList[] = {
	{";",		0,		"nothing"},
	//
	{"print",	10,		"prefix"},
	{"ls",		10,		"prefix"},
	{"lsdep",	10,		"prefix"},
	{"dump",	10,		"prefix"},
	{"clean",	10,		"prefix"},
	/*
	{"}",		10,		"NV_Op_codeBlockClose"},
	{"ls2",		10,		"NV_Op_ls2"},
	{"lsctx",	10,		"NV_Op_lsctx"},
	{"swctx",	10,		"NV_Op_swctx"},
	{"last",	10,		"NV_Op_last"},
	{"save",	10,		"NV_Op_save"},
	{"restore",	10,		"NV_Op_restore"},
	{"out",		10,		"NV_Op_out"},
	{"fmt",		10,		"NV_Op_fmt"},
	{"info",	10,		"NV_Op_info"},
	{"clean",	10,		"NV_Op_clean"},
	{"push",	10,		"NV_Op_push"},
	*/
	{",",		40,		"infix"},
	{":",		50,		"infix"},
	//
	{"=",		101,	"infix"},
	//
	{"<",		500,	"infix"},
	{">=",		500,	"infix"},
	{"<=",		500,	"infix"},
	{">",		500,	"infix"},
	{"==",		500,	"infix"},
	{"!=",		500,	"infix"},
	//
	{"+",		1000,	"infix"},
	{"-",		1000,	"infix"},
	{"*",		2000,	"infix"},
	{"/",		2000,	"infix"},
	{"%",		2000,	"infix"},
	//
	{"+",		5001,	"prefix"},
	{"-",		5001,	"prefix"},
	//
	{"++",		6000,	"postfix"},
	{"--",		6000,	"postfix"},
	//
	{"if",		10000,	"if"},
	{"for",		10000,	"for"},
	//
	//{"#",       14000,  "NV_Op_unbox"},
	//
	{"(",		15000,	"parentheses"},
	//{"[",		15000,	"NV_Op_arrayAccessor"},
	{".",		15000,	"infix"},
	//
	{" ",		20000,	"nothing"},
	//
	{"{",		30000,	"codeblock"},
	{"\"",		40000,	"strliteral"},
	//
	//{"\"",		100000,	"NV_Op_strLiteral"},
	//
	{"", -1, ""}	// terminate tag
};

NV_ID NV_createOpDict()
{
	NV_ID opDict = NV_Node_createWithString("NV_OpList");
	//
	int i;
	for(i = 0; builtinOpList[i].prec >= 0; i++){
		NV_addBuiltinOp(&opDict,
			builtinOpList[i].token, builtinOpList[i].prec, builtinOpList[i].funcStr);
	}
	//
	if(IS_DEBUG_MODE()){
		NV_Dict_print(&opDict);
	}
	return opDict;
}

NV_ID NV_parseToCodeGraph_nothing
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(lastNode);
	PARAM_UNUSED(ident);
	//
	NV_Array_removeIndex(tokenList, p->index);
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_infixOp
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	NV_ID funcNode = NV_Node_createWithString("infixOp");
	NV_ID op = NV_Node_createWithString(ident);
	NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
	NV_ID opR = NV_Array_getByIndex(tokenList, p->index + 1);
	NV_ID result = NV_Variable_create();
	//
	if(NV_Term_canBeOperator(&opL, &p->dict)){
		puts("canBeOp!");
		return NV_Node_createWithString("Expected opL is a value");
	}
	if(NV_Term_canBeOperator(&opR, &p->dict)){
		puts("canBeOp!");
		return NV_Node_createWithString("Expected opR is a value");
	}
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, p->index - 1);
	NV_Array_removeIndex(tokenList, p->index - 1);
	NV_Array_writeToIndex(tokenList, p->index - 1, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_prefixOp
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	//puts("parse: prefix: begin");
	NV_ID funcNode = NV_Node_createWithString("prefixOp");
	NV_ID op = NV_Node_createWithString(ident);
	NV_ID opR = NV_Array_getByIndex(tokenList, p->index + 1);
	NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
	NV_ID result = NV_Variable_create();
	//
	if(!NV_Term_isNotFound(&opL) && !NV_Term_canBeOperator(&opL, &p->dict)){
		return NV_Node_createWithString("Expected opL is not a value");
	}
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, p->index);
	NV_Array_writeToIndex(tokenList, p->index, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_postfixOp
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	NV_ID funcNode = NV_Node_createWithString("postfixOp");
	NV_ID op = NV_Node_createWithString(ident);
	NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, p->index - 1);
	NV_Array_writeToIndex(tokenList, p->index - 1, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_codeblock
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(lastNode);
	PARAM_UNUSED(ident);
	NV_Op_codeBlock(tokenList, p->index, "{", "}");
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_strLiteral
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(lastNode);
	PARAM_UNUSED(ident);
	NV_Op_strLiteral(tokenList, p->index);
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_parentheses
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(ident);
	NV_ID *opDict = &p->dict;
	NV_Op_codeBlock(tokenList, p->index, "(", ")");
	NV_ID funcNode = NV_Node_createWithString("()");
	NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
	NV_ID inner = NV_Array_getByIndex(tokenList, p->index);
	NV_ID result = NV_Variable_create();
	//
	inner = NV_parseToCodeGraph(&inner, opDict);
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "inner", &inner);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_writeToIndex(tokenList, p->index, &result);
	if(!NV_Term_isNotFound(&opL) && !NV_Term_canBeOperator(&opL, &p->dict)){
		NV_Array_removeIndex(tokenList, p->index - 1);
	}
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_if
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(ident);
	// if {cond} {do} [{cond} {do}] [{else}]
	int i, count;
	NV_ID t;
	NV_ID condT = NODEID_NOT_FOUND, doT;
	NV_ID terminateNode = NV_Node_createWithString("endif");
	NV_ID *opDict = &p->dict;

	for(i = 1; ; i++){
		t = NV_Array_getByIndex(tokenList, p->index + i);
		if(i & 1){
			// 奇数: 条件節、もしくはelse節
			if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
				// 終了
				break;
			}
			condT = t;
		} else{
			// 偶数：実行部分
			if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
				// もうブロックがない
				if(!NV_NodeID_isEqual(&condT, &NODEID_NOT_FOUND)){
					// 直前の節(condT)はelseだった。
					condT = NV_parseToCodeGraph(&condT, opDict);
					NV_ID doFunc = NV_Node_createWithString("do");
					NV_Dict_addUniqueEqKeyByCStr(&doFunc, "call", &condT);
					//
					NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &doFunc);
					*lastNode = doFunc;
				}
				break;
			}
			doT = t;
			//
			condT = NV_parseToCodeGraph(&condT, opDict);
			doT = NV_parseToCodeGraph(&doT, opDict);
			//
			NV_ID func = NV_Node_createWithString("cond");
			NV_ID doFunc = NV_Node_createWithString("do");
			//
			NV_Dict_addUniqueEqKeyByCStr(&func, "flag", &condT);
			NV_Dict_addUniqueEqKeyByCStr(&func, "truePath", &doFunc);
			NV_Dict_addUniqueEqKeyByCStr(&doFunc, "call", &doT);
			//
			NV_Dict_addUniqueEqKeyByCStr(&doFunc, "next", &terminateNode);
			//
			NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &func);
			*lastNode = func;
		}
	}
	count = i;
	for(i = 0; i < count; i++){
		NV_Array_removeIndex(tokenList, p->index);
	}
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &terminateNode);
	*lastNode = terminateNode;
	return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_for
(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident)
{
	PARAM_UNUSED(ident);
	// for {init block}{conditional block}{update block}[{statement}]
	NV_ID initT, condT, updtT, doT;
	NV_ID initF, condF, updtF, doF;
	NV_ID terminateF, t;
	NV_ID *opDict = &p->dict;
	int i;
	for(i = 1; i <= 3; i++){
		t = NV_Array_getByIndex(tokenList, p->index + i);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
			fprintf(stderr, "too few codeblocks for for.\n");
			return NODEID_NULL;
		}
	}
	initT = NV_Array_getByIndex(tokenList, p->index + 1);
	condT = NV_Array_getByIndex(tokenList, p->index + 2);
	updtT = NV_Array_getByIndex(tokenList, p->index + 3);
	doT   = NV_Array_getByIndex(tokenList, p->index + 4);
	for(i = 0; i < 4; i++){
		NV_Array_removeIndex(tokenList, p->index);
	}
	if(!NV_isTermType(&doT, &NODEID_TERM_TYPE_ARRAY)){
		doT = NV_Array_create();
	} else{
		NV_Array_removeIndex(tokenList, p->index);	
	}
	//
	initT = NV_parseToCodeGraph(&initT, opDict);
	if(NV_NodeID_isEqual(&initT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	condT = NV_parseToCodeGraph(&condT, opDict);
	if(NV_NodeID_isEqual(&condT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	updtT = NV_parseToCodeGraph(&updtT, opDict);
	if(NV_NodeID_isEqual(&updtT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	doT   = NV_parseToCodeGraph(&doT  , opDict);
	if(NV_NodeID_isEqual(&doT, &NODEID_NULL)){
		return NODEID_NULL;
	}
	//
	initF = NV_Node_createWithString("init");
	NV_Dict_addUniqueEqKeyByCStr(&initF, "call", &initT);
	condF = NV_Node_createWithString("cond");
	NV_Dict_addUniqueEqKeyByCStr(&condF, "flag", &condT);
	updtF = NV_Node_createWithString("updt");
	NV_Dict_addUniqueEqKeyByCStr(&updtF, "call", &updtT);
	doF   = NV_Node_createWithString("do");
	NV_Dict_addUniqueEqKeyByCStr(&doF  , "call", &doT);
	terminateF = NV_Node_createWithString("endfor");
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &initF);
	NV_Dict_addUniqueEqKeyByCStr(&initF, "next", &condF);
	NV_Dict_addUniqueEqKeyByCStr(&condF, "next", &terminateF);
	NV_Dict_addUniqueEqKeyByCStr(&condF, "truePath", &doF);
	NV_Dict_addUniqueEqKeyByCStr(&doF, "next", &updtF);
	NV_Dict_addUniqueEqKeyByCStr(&updtF, "next", &condF);
	//
	*lastNode = terminateF;
	return *lastNode;
}



typedef struct NV_LANG02_BUILTIN_FUNCTION_TAG {
	const char *name;
	NV_ID (*parser)
		(const NV_ID *tokenList, NV_ID *lastNode, NV_OpPointer *p, const char *ident);
} NV_Lang02_BuiltinFunctionTag;

NV_Lang02_BuiltinFunctionTag builtinFuncList[] = {
	{"nothing", NV_parseToCodeGraph_nothing},
	{"infix", NV_parseToCodeGraph_infixOp},
	{"prefix", NV_parseToCodeGraph_prefixOp},
	{"postfix", NV_parseToCodeGraph_postfixOp},
	{"codeblock", NV_parseToCodeGraph_codeblock},
	{"strliteral", NV_parseToCodeGraph_strLiteral},
	{"parentheses", NV_parseToCodeGraph_parentheses},
	{"if", NV_parseToCodeGraph_if},
	{"for", NV_parseToCodeGraph_for},
	{NULL, NULL},	// terminate tag
};

NV_ID NV_parseToCodeGraph(const NV_ID *baseTokenList, const NV_ID *opDict)
{
	// retv: codeGraphRoot
	NV_ID tokenList = NV_Array_clone(baseTokenList);
	/*
	printf("parsing tokens: ");
	NV_Array_print(&tokenList); putchar('\n');
	*/
	//printf("tokenList hash = %08X\n", NV_Term_calcHash(baseTokenList));
	NV_ID codeGraphRoot = NV_Node_createWithString("eval");
	NV_ID lastNode = codeGraphRoot;
	NV_OpPointer p;
	const char *reqFuncName = NULL;
	int i;
	NV_ID retv;

	//NV_Dict_print(opDict);

	for(;;){
		p = NV_getNextOp(&tokenList, opDict);
		if(p.index == -1) break;
		NV_ID n = NV_Array_getByIndex(&tokenList, p.index);
		if(NV_NodeID_isEqual(&n, &NODEID_NOT_FOUND)) break;
		reqFuncName = NV_Op_getOpFuncNameCStr(&p.op);
		for(i = 0; builtinFuncList[i].name; i++){
			if(strcmp(reqFuncName, builtinFuncList[i].name) == 0) break;
		}
		if(builtinFuncList[i].name){
			if(IS_DEBUG_MODE()){
				printf("parse: %s\n", builtinFuncList[i].name);
			}
			retv = builtinFuncList[i].parser(
					&tokenList, &lastNode, &p, NV_NodeID_getCStr(&n));
			if(!NV_Term_isNull(&retv)){
				NV_ID triedPrec = NV_Node_createWithInt32(p.prec);
				NV_Dict_addUniqueEqKeyByCStr(&n, "triedPrec", &triedPrec);
			}
		} else{
			fprintf(stderr, "NV_parseToCodeGraph: op not implemented for");
			NV_NodeID_printForDebug(&n);
			putchar('\n');
			return NODEID_NULL;
		}
	}
	if(NV_globalExecFlag & NV_EXEC_FLAG_SAVECODEGRAPH){
		NV_saveCodeGraphForVisualization(&codeGraphRoot, "note/code");
	}
	return codeGraphRoot;
}
