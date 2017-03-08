#include "nv.h"

//
// internal
//

void NV_Op_Internal_setCurrentPhase(const NV_ID *opList, int32_t phase)
{
	NV_ID r, n;
	n = NV_Node_createWithInt32(phase);
	r = NV_NodeID_getRelationFrom(opList, &RELID_CURRENT_TERM_PHASE);
	if(NV_NodeID_isEqual(&r, &NODEID_NOT_FOUND)){
		// create new one
		NV_NodeID_createRelation(opList, &RELID_CURRENT_TERM_PHASE, &n);
	} else{
		// update old link
		NV_NodeID_updateRelationTo(&r, &n);
	}
}

int32_t NV_Op_Internal_getCurrentPhase(const NV_ID *opList)
{
	// if not set, returns -1
	NV_ID n;
	n = NV_NodeID_getRelatedNodeFrom(opList, &RELID_CURRENT_TERM_PHASE);
	return NV_NodeID_getInt32(&n);
}

//
// public
//

#define NV_LANG_CHAR_LIST_LEN 3
int NV_Lang_getCharType(const NV_ID *cTypeList, char c)
{
	NV_ID t;
	int i;
	if(c == '\0') return -1;
	for(i = 0; i < NV_LANG_CHAR_LIST_LEN; i++){
		t = NV_Array_getByIndex(cTypeList, i);
		if(NV_Node_String_strchr(NV_NodeID_getNode(&t), c)) break;
	}
	return i;
}


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

void NV_addOp(const NV_ID *opList, const char *token, int32_t prec, const NV_ID *func)
{
	NV_ID opDir;
	NV_ID opEntry;
	NV_ID ePrec;
	// まずtokenごとに分けたDirがある
	opDir = NV_Dict_getByStringKey(opList, token);
	if(NV_NodeID_isEqual(&opDir, &NODEID_NOT_FOUND)){
		// このtokenは初出なので新規追加
		opDir = NV_Array_create();
		NV_Dict_addKeyByCStr(opList, token, &opDir);
	}
	// opEntry(ひとつのOpを表現)を作成
	opEntry = NV_Node_create();
	NV_NodeID_createRelation(
		&opEntry, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_OP);
	ePrec = NV_Node_createWithInt32(prec);
	NV_NodeID_createRelation(
		&opEntry, &RELID_OP_PRECEDENCE, &ePrec);
	NV_NodeID_createRelation(
		&opEntry, &RELID_OP_FUNC, func);
	// opEntryをopDirに追加
	NV_Array_push(&opDir, &opEntry);
}

void NV_addBuiltinOp(const NV_ID *opList, const char *token, int32_t prec, const char *funcStr)
{
	NV_ID funcStrID;
	funcStrID = NV_Node_createWithString(funcStr);
	NV_addOp(opList, token, prec, &funcStrID);
}

typedef struct NV_BUILTIN_OP_TAG {
	const char *token;
	int prec;
	const char *funcStr;
} NV_BuiltinOpTag;

NV_BuiltinOpTag builtinOpList[] = {
	{";",		0,		"NV_Op_nothing"},
	//
	{"}",		10,		"NV_Op_codeBlockClose"},
	{"ls",		10,		"NV_Op_ls"},
	{"ls2",		10,		"NV_Op_ls2"},
	{"last",	10,		"NV_Op_last"},
	{"save",	10,		"NV_Op_save"},
	{"restore",	10,		"NV_Op_restore"},
	{"print",	10,		"NV_Op_print"},
	{"info",	10,		"NV_Op_info"},
	{"clean",	10,		"NV_Op_clean"},
	//
	{"=",		101,	"NV_Op_assign"},
	//
	{"<",		500,	"NV_Op_lt"},
	{">=",		500,	"NV_Op_gte"},
	{"<=",		500,	"NV_Op_lte"},
	{">",		500,	"NV_Op_gt"},
	{"==",		500,	"NV_Op_eq"},
	{"!=",		500,	"NV_Op_neq"},
	//
	{"+",		1000,	"NV_Op_add"},
	{"-",		1000,	"NV_Op_sub"},
	{"*",		2000,	"NV_Op_mul"},
	{"/",		2000,	"NV_Op_div"},
	{"%",		2000,	"NV_Op_mod"},
	//
	{"+",		5001,	"NV_Op_sign_plus"},
	{"-",		5001,	"NV_Op_sign_minus"},
	
	//
	{"if",		10000,	"NV_Op_if"},
	{"for",		10000,	"NV_Op_for"},
	//
	{"(",		15000,	"NV_Op_callArgs"},
	//
	{" ",		20000,	"NV_Op_nothing"},
	//
	{"{",		30000,	"NV_Op_codeBlock"},
	//
	{"\"",		100000,	"NV_Op_strLiteral"},
	//
	{"", -1, ""}	// terminate tag
};

int NV_isBuiltinOp(const NV_ID *term, const char *ident)
{
	NV_ID func = NV_NodeID_getRelatedNodeFrom(term, &RELID_OP_FUNC);
	return NV_Node_String_compareWithCStr(NV_NodeID_getNode(&func), ident) == 0;
}


NV_ID NV_createOpList()
{
	NV_ID opList = NV_Node_createWithString("NV_OpList");
	//
	int i;
	for(i = 0; builtinOpList[i].prec >= 0; i++){
		NV_addBuiltinOp(&opList,
			builtinOpList[i].token, builtinOpList[i].prec, builtinOpList[i].funcStr);
	}
	//
	if(IS_DEBUG_MODE()){
		NV_Dict_print(&opList);
	}
	return opList;
}

int32_t NV_getOpPrec(const NV_ID *op)
{
	NV_ID ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	return NV_NodeID_getInt32(&ePrec);
}

void NV_getOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, NV_ID *idBuf, int count)
{
	int i;
	for(i = 0; i < count; i++){
		idBuf[i] = NV_Array_getByIndex(tList,  baseIndex + relIndexList[i]);
	}
}

void NV_removeOperandByList(const NV_ID *tList, int baseIndex, const int *relIndexList, int count)
{
	// relIndexListが昇順にソートされていると仮定している．
	int i;
	for(i = count - 1; i >= 0; i--){
		NV_Array_removeIndex(tList, relIndexList[i] + baseIndex);
	}
}



NV_ID NV_Op_ExecBuiltinInfix(const NV_ID *tList, int index, int func)
{
	NV_ID nL, nR, ans;
	int vL, vR, v;
	const NV_ID *scope = &NODEID_NULL;
	//
	nL = NV_Array_getByIndex(tList, index - 1);
	nR = NV_Array_getByIndex(tList, index + 1);
	if(!NV_Term_isInteger(&nL, scope) || !NV_Term_isInteger(&nR, scope)){
		return NV_Node_createWithString(
			"Error: Invalid Operand Type.");
	}
	vL = NV_Term_getInt32(&nL, scope);
	vR = NV_Term_getInt32(&nR, scope);
	//
	index--;
	NV_Array_removeIndex(tList, index);
	NV_Array_removeIndex(tList, index);
	//
	if((func == 3 || func == 4) && vR == 0){
		return NV_Node_createWithString(
			"Error: Div by 0");
	}
	switch(func){
		//
		case 0:		v = vL + vR; break;
		case 1:		v = vL - vR; break;
		case 2:		v = vL * vR; break;
		case 3:		v = vL / vR; break;
		case 4:		v = vL % vR; break;
		//
		case 10:	v = (vL < vR); break;
		case 11:	v = (vL >= vR); break;
		case 12:	v = (vL <= vR); break;
		case 13:	v = (vL > vR); break;
		case 14:	v = (vL == vR); break;
		case 15:	v = (vL != vR); break;
	}
	//
	ans = NV_Node_createWithInt32(v);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_save(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_NodeID_isString(&operand[0])){
		return NV_Node_createWithString(
			"Error: Invalid Operand Type.");
	}
	fname = NV_NodeID_getCStr(&operand[0]);
	if(!fname){
		return NV_Node_createWithString(
			"fname is null");
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	FILE *fp = fopen(fname, "wb");
	if(!fp){
		return NV_Node_createWithString(
			"fopen failed");
	}
	ans = NV_Node_createWithInt32(0);
	NV_Graph_dumpToFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_restore(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_NodeID_isString(&operand[0])){
		return NV_Node_createWithString(
			"Error: Invalid Operand Type.");
	}
	fname = NV_NodeID_getCStr(&operand[0]);
	if(!fname){
		NV_ID errObj = NV_Node_createWithString(
			"fname is null");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	FILE *fp = fopen(fname, "rb");
	if(!fp){
		return NV_Node_createWithString(
			"fopen failed");
	}
	ans = NV_Node_createWithString("restore");
	NV_Graph_restoreFromFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_ls(const NV_ID *tList, int index)
{
	NV_ID ans, pathStr;
	//
	pathStr = NV_Array_getByIndex(tList, index + 1);
	if(NV_NodeID_isString(&pathStr)){
		NV_ID path = NV_Path_createAbsoluteWithCStr(NV_NodeID_getCStr(&pathStr));
		NV_ID d;
		//NV_Dict_print(&path);
		d = NV_Path_getTarget(&path);
		NV_Dict_print(&d);
		NV_Array_removeIndex(tList, index + 1);
	} else{
		NV_Dict_print(&NODEID_NULL);
	}
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_ls2(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Dict_print(&NODEID_NV_STATIC_ROOT);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_last(const NV_ID *tList, int index)
{
	NV_ID ans, n;
	//
	n = NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
	NV_printNodeByID(&n); putchar('\n');
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_info(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Node *n;
	int i;
	//
	i = 0;
	for(n = nodeRoot.next; n; n = n->next){
		i++;
	}
	printf("%d nodes\n", i);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_clean(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Node *n;
	int i;
	//
	i = 0;
	for(n = nodeRoot.next; n; n = n->next){
		i++;
	}
	printf("%d nodes\n", i);
	//
	NV_Node_cleanup();
	//
	i = 0;
	for(n = nodeRoot.next; n; n = n->next){
		i++;
	}
	printf("%d nodes\n", i);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_assign(const NV_ID *tList, int index)
{
	const int operandCount = 2;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {-1, 1};
	//
	const NV_ID *scope = &NODEID_NULL;
	NV_ID v = operand[0];
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(NV_Term_isAssignable(&operand[0], scope)){
		// 既存変数への代入
		v = NV_Term_getAssignableNode(&operand[0], scope);
	} else{
		// 新規変数を作成して代入
		if(!NV_NodeID_isString(&operand[0])){
			return NV_Node_createWithString(
				"Error: Invalid Operand Type.");
		}
		v = NV_Variable_createWithName(scope, &operand[0]);
	}
	//
	NV_Variable_assign(&v, &operand[1]);	
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	NV_Array_writeToIndex(tList, index - 1, &v);
	return NODEID_NULL;
}

NV_ID NV_Op_codeBlock
(const NV_ID *tList, int index, const char *openTerm, const char *closeTerm)
{
	NV_ID v;
	//
	NV_ID root;
	int nc = 1;	// nest count
	//
	root = NV_Array_create();
	for(;;){
		v = NV_Array_getByIndex(tList, index + 1);
		if(NV_NodeID_isEqual(&v, &NODEID_NOT_FOUND)){
			// おかしい
			return NV_Node_createWithString(
				"Error: Expected closeTerm but not found.");
		}
		NV_Array_removeIndex(tList, index + 1);
		if(NV_NodeID_String_compareWithCStr(&v, openTerm) == 0){
			// 開きかっこ
			nc++;
		}
		if(NV_NodeID_String_compareWithCStr(&v, closeTerm) == 0){
			// 終了
			nc--;
			if(nc == 0) break;
		}
		NV_Array_push(&root, &v);
	}
	NV_Array_writeToIndex(tList, index, &root);
	return NODEID_NULL;
}

NV_ID NV_Op_strLiteral(const NV_ID *tList, int index)
{
	NV_ID v, s;
	//
	NV_ID root;
	int esc = 0;	// escape flag
	//
	root = NV_Array_create();
	for(;;){
		v = NV_Array_getByIndex(tList, index + 1);
		if(NV_NodeID_isEqual(&v, &NODEID_NOT_FOUND)){
			// おかしい
			return NV_Node_createWithString(
				"Error: Expected \" but not found.");
		}
		NV_Array_removeIndex(tList, index + 1);
		if(esc){
			esc = 0;
			NV_Array_push(&root, &v);
		} else{
			if(NV_NodeID_String_compareWithCStr(&v, "\\") == 0){
				esc = 1;
				continue;
			}
			if(NV_NodeID_String_compareWithCStr(&v, "\"") == 0){
				// 終了
				break;
			}
			NV_Array_push(&root, &v);
		}
	}
	s = NV_Array_joinWithCStr(&root, "");
	NV_Array_writeToIndex(tList, index, &s);
	return NODEID_NULL;
}

NV_ID NV_Op_if(const NV_ID *tList, int index, const NV_ID *ctx)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	NV_ID t, tRes;
	const NV_ID *scope = &NODEID_NULL;
	int phase;
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	// phaseには、次に実行すべき項のoffsetが格納されていることとする。
	phase = NV_Op_Internal_getCurrentPhase(tList);
	if(phase == -1) phase = 1;
	//
	if(phase >= 1){
		if(phase & 1){
			// 奇数: 条件節の実行、もしくはelse節
			t = NV_Array_getByIndex(tList, index + phase);
			if(!NV_Term_isArray(&t, scope)){
				// どの条件節も成立しないまま、if文が終了した
				tRes = NODEID_NULL;
				// 終了処理へ
			} else{
				// 条件節を実行スタックに追加。この文が実行されてから現在の文に戻ってくる。
				NV_Array_push(&evalStack, &t);
				NV_Op_Internal_setCurrentPhase(tList, phase + 1);
				return NODEID_NULL;
			}
		} else{
			// 偶数: 実行
			tRes = NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
			t = NV_Array_getByIndex(tList, index + phase);
			if(!NV_Term_isArray(&t, scope)){
				// この直前に実行した文はelse節だった。
				// 終了処理へ
			} else{
				// 条件を評価し、もしもtrueなら実行部分を実行スタックに追加。
				tRes = NV_Array_last(&tRes);
				if(NV_Term_getInt32(&tRes, scope)){
					NV_Array_push(&evalStack, &t);
				}
				NV_Op_Internal_setCurrentPhase(tList, phase + 1);
				return NODEID_NULL;
			}
		}
	}
	// 終了処理
	// store eval result
	NV_Array_writeToIndex(tList, index, &tRes);
	// remove operands
	for(;;){
		t = NV_Array_getByIndex(tList, index + 1);
		if(!NV_Term_isArray(&t, scope)) break;
		NV_Array_removeIndex(tList, index + 1);
	}
	//
	NV_Op_Internal_setCurrentPhase(tList, -1);
	return NODEID_NULL;
}

NV_ID NV_Op_for(const NV_ID *tList, int index, const NV_ID *ctx)
{
	// for {init block}{conditional block}{update block}[{statement}]
	NV_ID t, tRes;
	const NV_ID *scope = &NODEID_NULL;
	int phase;
	// phaseには、次に実行すべき項のoffsetが格納されていることとする。
	// つまり、for文の場合は、
	// 1 > 2 > 4 > 3 > 2 > 4 > 3 > ...
	// という順序で進行する。
	phase = NV_Op_Internal_getCurrentPhase(tList);
	if(phase == -1){
		// 初めてこのforを実行する
		// check
		// 少なくとも後続の3つはArrayでなければならない。
		int i;
		for(i = 1; i <= 3; i++){
			t = NV_Array_getByIndex(tList, index + i);
			if(!NV_Term_isArray(&t, scope)) break;
		}
		if(i <= 3){
			return NV_Node_createWithString(
				"Error: Expected >= 3 blocks but not found.");
		}
		// 問題ないのでphase1から始める
		phase = 1;
	}
	//printf("Op_for: phase = %d\n", phase);
	t = NV_Array_getByIndex(tList, index + phase);
	if(phase == 1){
		// 初期化式を実行スタックに積んで終了
		NV_Context_pushToEvalStack(ctx, &t); 
		NV_Op_Internal_setCurrentPhase(tList, 2);
		return NODEID_NULL;
	} else if(phase == 2){
		// 条件式のコピーを実行スタックに積んで終了
		t = NV_Array_clone(&t);
		NV_Context_pushToEvalStack(ctx, &t); 
		NV_Op_Internal_setCurrentPhase(tList, 4);
		return NODEID_NULL;
	} else if(phase == 4){
		// 条件を判定して、本体部分のコピーを実行スタックに積んで終了
		tRes = NV_NodeID_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
		tRes = NV_Array_last(&tRes);
		if(NV_Term_getInt32(&tRes, scope)){
			t = NV_Array_clone(&t);
			NV_Context_pushToEvalStack(ctx, &t); 
			NV_Op_Internal_setCurrentPhase(tList, 3);
			return NODEID_NULL;
		}
		// 条件が偽ならば終了処理へ
	} else if(phase == 3){
		// 更新式のコピーを実行スタックに積んで終了
		t = NV_Array_clone(&t);
		NV_Context_pushToEvalStack(ctx, &t); 
		NV_Op_Internal_setCurrentPhase(tList, 2);
		return NODEID_NULL;
	}
	// 終了処理
	// store eval result
	NV_Array_writeToIndex(tList, index, &t);
	// remove operands
	NV_Array_removeIndex(tList, index + 1);
	NV_Array_removeIndex(tList, index + 1);
	NV_Array_removeIndex(tList, index + 1);
	if(NV_Term_isArray(&t, scope)){
		NV_Array_removeIndex(tList, index + 1);
	}
	NV_Op_Internal_setCurrentPhase(tList, -1);
	return NODEID_NULL;
}

NV_ID NV_Op_print(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	const NV_ID *scope = &NODEID_NULL;
	//
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	//
	operand[0] = NV_Term_tryReadAsVariable(&operand[0], scope);
	NV_printNodeByID(&operand[0]); putchar('\n');
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	ans = NV_Node_createWithString("success");
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_unaryPrefix(const NV_ID *tList, int index, int mod)
{
	NV_ID nL, nR, ans;
	int vR, v;
	const NV_ID *scope = &NODEID_NULL;
	//
	nL = NV_Array_getByIndex(tList, index - 1);
	nR = NV_Array_getByIndex(tList, index + 1);
	if(!NV_Term_isInteger(&nR, scope)){
		return NV_Node_createWithString(
			"Error: Expected type(nR) == Integer, but not.");
	}
	if(NV_Term_isInteger(&nL, scope)){
		return NV_Node_createWithString(
			"Error: Expected type(nL) != Integer, but not.");
	}
	vR = NV_Term_getInt32(&nR, scope);
	//
	NV_Array_removeIndex(tList, index);
	//
	switch(mod){
		//
		case 0:		v = vR; break;
		case 1:		v = -vR; break;
/*
		case 2:		v = vL * vR; break;
		case 3:		v = vL / vR; break;
		case 4:		v = vL % vR; break;
		//
		case 10:	v = (vL < vR); break;
		case 11:	v = (vL >= vR); break;
		case 12:	v = (vL <= vR); break;
		case 13:	v = (vL > vR); break;
		case 14:	v = (vL == vR); break;
		case 15:	v = (vL != vR); break;
*/
		default:
			return NV_Node_createWithString(
				"Error: Invalid mod");
	}
	//
	ans = NV_Node_createWithInt32(v);
	NV_Array_writeToIndex(tList, index, &ans);
	return NODEID_NULL;
}

NV_ID NV_Op_callArgs(const NV_ID *tList, int index, const NV_ID *ctx)
{
	// {code block}(arg1, arg2, ...)
	NV_ID t, r;
	const NV_ID *scope = &NODEID_NULL;
	int phase;
	phase = NV_Op_Internal_getCurrentPhase(tList);
	// 初めてこのOpを実行する
	// 実行すべきコードブロックを取得
	t = NV_Array_getByIndex(tList, index - 1);
	t = NV_Term_tryReadAsVariable(&t, scope);
	if(!NV_Term_isArray(&t, scope)){
		return NV_Node_createWithString("pre term is not an Array");
	}
	if(IS_DEBUG_MODE()){
		printf("Exec block: ");
		NV_Array_print(&t); putchar('\n');
	}
	// 引数ブロックをまとめてもらう
	r = NV_Op_codeBlock(tList, index, "(", ")");
	//
	//NV_Array_removeIndex(tList, index);
	// 問題ないので実行スタックに積む
	NV_Context_pushToEvalStack(ctx, &t); 
	//
	return NODEID_NULL;
}

void NV_tryExecOpAt(const NV_ID *tList, int index, const NV_ID *ctx)
{
	NV_ID opStr = NV_Array_getByIndex(tList, index);
	NV_ID opRecog = NV_Dict_getByStringKey(&opStr, "recogAsOp");
	//
	if(IS_DEBUG_MODE()){
		printf("begin op at index: %d ", index);
		NV_printNodeByID(&opRecog);
		NV_printNodeByID(tList);
		putchar('\n');
	}
	//
	NV_ID r;
	if(NV_isBuiltinOp(&opRecog, "NV_Op_nothing")){
		NV_Array_removeIndex(tList, index);
		r = NODEID_NULL;
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_add")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 0);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_sub")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 1);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_mul")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 2);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_div")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 3);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_mod")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 4);
	//
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_lt")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 10);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_gte")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 11);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_lte")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 12);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_gt")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 13);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_eq")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 14);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_neq")){
		r = NV_Op_ExecBuiltinInfix(tList, index, 15);
	//
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_save")){
		r = NV_Op_save(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_restore")){
		r = NV_Op_restore(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_ls")){
		r = NV_Op_ls(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_ls2")){
		r = NV_Op_ls2(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_last")){
		r = NV_Op_last(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_assign")){
		r = NV_Op_assign(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_codeBlock")){
		r = NV_Op_codeBlock(tList, index, "{", "}");
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_if")){
		r = NV_Op_if(tList, index, ctx);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_print")){
		r = NV_Op_print(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_for")){
		r = NV_Op_for(tList, index, ctx);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_info")){
		r = NV_Op_info(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_clean")){
		r = NV_Op_clean(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_strLiteral")){
		r = NV_Op_strLiteral(tList, index);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_sign_plus")){
		r = NV_Op_unaryPrefix(tList, index, 0);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_sign_minus")){
		r = NV_Op_unaryPrefix(tList, index, 1);
	} else if(NV_isBuiltinOp(&opRecog, "NV_Op_callArgs")){
		r = NV_Op_callArgs(tList, index, ctx);
	} else{
		r = NV_Node_createWithString(
			"Error: Op NOT found or NOT implemented.");
	}
	if(!NV_NodeID_isEqual(&r, &NODEID_NULL)){
		// error ocuured
		NV_Dict_removeUniqueEqKeyByCStr(&opStr, "recogAsOp");
		NV_Dict_addKeyByCStr(&opStr, "failedOp", &opRecog);
		NV_Dict_addKeyByCStr(&opStr, "failedReason", &r);
		NV_ID prec = NV_Node_createWithInt32(NV_getOpPrec(&opRecog));
		NV_Dict_addUniqueEqKeyByCStr(&opStr, "triedPrec", &prec);
		NV_Dict_addKeyByCStr(&opStr, "failedOp", &opRecog);
		if(IS_DEBUG_MODE()){
			printf("op failed:");
			NV_printNodeByID(&r);
			NV_printNodeByID(&opRecog);
			putchar('\n');
		}
	}
	//
	if(IS_DEBUG_MODE()){
		printf("end op ");
		NV_printNodeByID(&opRecog);
		putchar('\n');
	}
}

void NV_printOp(const NV_ID *op)
{
	NV_ID eFunc;
	NV_ID ePrec;
	eFunc = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	ePrec = NV_NodeID_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	printf("(op ");
	NV_printNodeByID(&eFunc);
	printf("/");
	NV_printNodeByID(&ePrec);
	printf(")");
}
