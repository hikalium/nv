#include "nv.h"

//
// internal
//

void NV_Op_Internal_setCurrentPhase(const NV_ID *opList, int32_t phase)
{
	NV_ID r, n;
	n = NV_Node_createWithInt32(phase);
	r = NV_Node_getRelationFrom(opList, &RELID_CURRENT_TERM_PHASE);
	if(NV_ID_isEqual(&r, &NODEID_NOT_FOUND)){
		// create new one
		NV_Node_createRelation(opList, &RELID_CURRENT_TERM_PHASE, &n);
	} else{
		// update old link
		NV_Node_updateRelationTo(&r, &n);
	}
}

int32_t NV_Op_Internal_getCurrentPhase(const NV_ID *opList)
{
	// if not set, returns -1
	NV_ID n;
	n = NV_Node_getRelatedNodeFrom(opList, &RELID_CURRENT_TERM_PHASE);
	return NV_Node_getInt32FromID(&n);
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
		if(NV_Node_String_strchr(NV_Node_getByID(&t), c)) break;
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
	NV_ID opEntry;
	NV_ID ePrec;
	opEntry = NV_Node_create();
	NV_Node_createRelation(
		&opEntry, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_OP);
	ePrec = NV_Node_createWithInt32(prec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_PRECEDENCE, &ePrec);
	NV_Node_createRelation(
		&opEntry, &RELID_OP_FUNC, func);
	
	//
	NV_Dict_addByStringKey(opList, token, &opEntry);
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
	{"if",		10000,	"NV_Op_if"},
	{"for",		10000,	"NV_Op_for"},
	//
	{" ",		20000,	"NV_Op_nothing"},
	//
	{"{",		30000,	"NV_Op_codeBlock"},
	//
	{"", -1, ""}	// terminate tag
};

int NV_isBuiltinOp(const NV_ID *term, const char *ident)
{
	NV_ID func = NV_Node_getRelatedNodeFrom(term, &RELID_OP_FUNC);
	return NV_Node_String_compareWithCStr(NV_Node_getByID(&func), ident) == 0;
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

int32_t NV_getOpPrecAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	NV_ID ePrec = NV_Node_getRelatedNodeFrom(&op, &RELID_OP_PRECEDENCE);
	return NV_Node_getInt32FromID(&ePrec);
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



void NV_Op_ExecBuiltinInfix(const NV_ID *tList, int index, int func)
{
	NV_ID nL, nR, ans;
	int vL, vR, v;
	const NV_ID *ctx = &NODEID_NULL;
	//
	nL = NV_Array_getByIndex(tList, index - 1);
	nR = NV_Array_getByIndex(tList, index + 1);
	if(!NV_Term_isInteger(&nL, ctx) || !NV_Term_isInteger(&nR, ctx)){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	vL = NV_Term_getInt32(&nL, ctx);
	vR = NV_Term_getInt32(&nR, ctx);
	//
	index--;
	NV_Array_removeIndex(tList, index);
	NV_Array_removeIndex(tList, index);
	//
	if((func == 3 || func == 4) && vR == 0){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Div by 0");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
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
}

void NV_Op_save(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_Node_isString(&operand[0])){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	fname = NV_Node_getCStr(&operand[0]);
	if(!fname){
		NV_ID errObj = NV_Node_createWithString(
			"fname is null");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	FILE *fp = fopen(fname, "wb");
	if(!fp){
		NV_ID errObj = NV_Node_createWithString(
			"fopen failed");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	ans = NV_Node_createWithInt32(0);
	NV_Graph_dumpToFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_restore(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	const char *fname;
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(!NV_Node_isString(&operand[0])){
		NV_ID errObj = NV_Node_createWithString(
			"Error: Invalid Operand Type.");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	fname = NV_Node_getCStr(&operand[0]);
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
		NV_ID errObj = NV_Node_createWithString(
			"fopen failed");
		NV_Array_writeToIndex(tList, index, &errObj);
		return;
	}
	ans = NV_Node_createWithString("restore");
	NV_Graph_restoreFromFile(fp);
	fclose(fp);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_ls(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Dict_print(&NODEID_NULL);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_ls2(const NV_ID *tList, int index)
{
	NV_ID ans;
	//
	NV_Dict_print(&NODEID_NV_STATIC_ROOT);
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_last(const NV_ID *tList, int index)
{
	NV_ID ans, n;
	//
	n = NV_Node_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
	NV_printNodeByID(&n); putchar('\n');
	//
	ans = NV_Node_createWithInt32(0);
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_Op_info(const NV_ID *tList, int index)
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
}

void NV_Op_clean(const NV_ID *tList, int index)
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
}

void NV_Op_assign(const NV_ID *tList, int index)
{
	const int operandCount = 2;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {-1, 1};
	//
	const NV_ID *ctx = &NODEID_NULL;
	NV_ID v = operand[0];
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	if(NV_Term_isAssignable(&operand[0], ctx)){
		// 既存変数への代入
		v = NV_Term_getAssignableNode(&operand[0], ctx);
	} else{
		// 新規変数を作成して代入
		if(!NV_Node_isString(&operand[0])){
			NV_ID errObj = NV_Node_createWithString(
				"Error: Invalid Operand Type.");
			NV_Array_writeToIndex(tList, index, &errObj);
			return;
		}
		v = NV_Variable_createWithName(ctx, &operand[0]);
	}
	//
	NV_Variable_assign(&v, &operand[1]);	
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	NV_Array_writeToIndex(tList, index - 1, &v);
}

void NV_Op_codeBlock(const NV_ID *tList, int index)
{
	NV_ID v;
	//
	NV_ID root;
	//
	root = NV_Array_create();
	for(;;){
		v = NV_Array_getByIndex(tList, index + 1);
		if(NV_ID_isEqual(&v, &NODEID_NOT_FOUND)){
			// おかしい
			NV_ID errObj = NV_Node_createWithString(
				"Error: Expected } but not found.");
			NV_Array_writeToIndex(tList, index, &errObj);
			return;
		}
		NV_Array_removeIndex(tList, index + 1);
		if(NV_isBuiltinOp(&v, "NV_Op_codeBlockClose")){
			// 終了
			break;
		}
		NV_Array_push(&root, &v);
	}
	NV_Array_writeToIndex(tList, index, &root);
}
void NV_Op_if(const NV_ID *tList, int index)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	NV_ID t, tRes;
	const NV_ID *ctx = &NODEID_NULL;
	int phase;
	NV_ID evalStack = NV_Node_getRelatedNodeFrom(
		&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK);
	// phaseには、次に実行すべき項のoffsetが格納されていることとする。
	phase = NV_Op_Internal_getCurrentPhase(tList);
	if(phase == -1) phase = 1;
	//
	if(phase >= 1){
		if(phase & 1){
			// 奇数: 条件節の実行、もしくはelse節
			t = NV_Array_getByIndex(tList, index + phase);
			if(!NV_Term_isArray(&t, ctx)){
				// どの条件節も成立しないまま、if文が終了した
				tRes = NODEID_NULL;
				// 終了処理へ
			} else{
				// 条件節を実行スタックに追加。この文が実行されてから現在の文に戻ってくる。
				NV_Array_push(&evalStack, &t);
				NV_Op_Internal_setCurrentPhase(tList, phase + 1);
				return;
			}
		} else{
			// 偶数: 実行
			tRes = NV_Node_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
			t = NV_Array_getByIndex(tList, index + phase);
			if(!NV_Term_isArray(&t, ctx)){
				// この直前に実行した文はelse節だった。
				// 終了処理へ
			} else{
				// 条件を評価し、もしもtrueなら実行部分を実行スタックに追加。
				tRes = NV_Array_last(&tRes);
				if(NV_Term_getInt32(&tRes, ctx)){
					NV_Array_push(&evalStack, &t);
				}
				NV_Op_Internal_setCurrentPhase(tList, phase + 1);
				return;
			}
		}
	}
	// 終了処理
	// store eval result
	NV_Array_writeToIndex(tList, index, &tRes);
	// remove operands
	for(;;){
		t = NV_Array_getByIndex(tList, index + 1);
		if(!NV_Term_isArray(&t, ctx)) break;
		NV_Array_removeIndex(tList, index + 1);
	}
	//
	NV_Op_Internal_setCurrentPhase(tList, -1);
}

void NV_Op_for(const NV_ID *tList, int index)
{
	// for {init block}{conditional block}{update block}[{statement}]
	NV_ID t, tRes;
	const NV_ID *ctx = &NODEID_NULL;
	int phase;
	NV_ID evalStack = NV_Node_getRelatedNodeFrom(
		&NODEID_NV_STATIC_ROOT, &RELID_EVAL_STACK);
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
			if(!NV_Term_isArray(&t, ctx)) break;
		}
		if(i <= 3){
			NV_ID errObj = NV_Node_createWithString(
				"Error: Expected >= 3 blocks but not found.");
			NV_Array_writeToIndex(tList, index, &errObj);
			return;
		}
		// 問題ないのでphase1から始める
		phase = 1;
	}
	//printf("Op_for: phase = %d\n", phase);
	t = NV_Array_getByIndex(tList, index + phase);
	if(phase == 1){
		// 初期化式を実行スタックに積んで終了
		NV_Array_push(&evalStack, &t);
		NV_Op_Internal_setCurrentPhase(tList, 2);
		return;
	} else if(phase == 2){
		// 条件式のコピーを実行スタックに積んで終了
		t = NV_Array_clone(&t);
		NV_Array_push(&evalStack, &t);
		NV_Op_Internal_setCurrentPhase(tList, 4);
		return;
	} else if(phase == 4){
		// 条件を判定して、本体部分のコピーを実行スタックに積んで終了
		tRes = NV_Node_getRelatedNodeFrom(&NODEID_NV_STATIC_ROOT, &RELID_LAST_RESULT);
		tRes = NV_Array_last(&tRes);
		if(NV_Term_getInt32(&tRes, ctx)){
			t = NV_Array_clone(&t);
			NV_Array_push(&evalStack, &t);
			NV_Op_Internal_setCurrentPhase(tList, 3);
			return;
		}
		// 条件が偽ならば終了処理へ
	} else if(phase == 3){
		// 更新式のコピーを実行スタックに積んで終了
		t = NV_Array_clone(&t);
		NV_Array_push(&evalStack, &t);
		NV_Op_Internal_setCurrentPhase(tList, 2);
		return;
	}
	// 終了処理
	// store eval result
	NV_Array_writeToIndex(tList, index, &t);
	// remove operands
	NV_Array_removeIndex(tList, index + 1);
	NV_Array_removeIndex(tList, index + 1);
	NV_Array_removeIndex(tList, index + 1);
	if(NV_Term_isArray(&t, ctx)){
		NV_Array_removeIndex(tList, index + 1);
	}
	NV_Op_Internal_setCurrentPhase(tList, -1);
}

void NV_Op_print(const NV_ID *tList, int index)
{
	const int operandCount = 1;
	NV_ID operand[operandCount];
	int operandIndex[operandCount] = {1};
	//
	const NV_ID *ctx = &NODEID_NULL;
	//
	NV_ID ans;
	//
	NV_getOperandByList(tList, index, operandIndex, operand, operandCount);
	//
	operand[0] = NV_Term_tryConvertToVariable(&operand[0], ctx);
	NV_printNodeByID(&operand[0]); putchar('\n');
	//
	NV_removeOperandByList(tList, index, operandIndex, operandCount);
	//
	ans = NV_Node_createWithString("success");
	NV_Array_writeToIndex(tList, index, &ans);
}

void NV_tryExecOpAt(const NV_ID *tList, int index)
{
	NV_ID op = NV_Array_getByIndex(tList, index);
	//
	if(IS_DEBUG_MODE()){
		printf("begin op ");
		NV_printNodeByID(&op);
		putchar('\n');
	}
	//
	if(NV_isBuiltinOp(&op, "NV_Op_nothing")){
		NV_Array_removeIndex(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_add")){
		NV_Op_ExecBuiltinInfix(tList, index, 0);
	} else if(NV_isBuiltinOp(&op, "NV_Op_sub")){
		NV_Op_ExecBuiltinInfix(tList, index, 1);
	} else if(NV_isBuiltinOp(&op, "NV_Op_mul")){
		NV_Op_ExecBuiltinInfix(tList, index, 2);
	} else if(NV_isBuiltinOp(&op, "NV_Op_div")){
		NV_Op_ExecBuiltinInfix(tList, index, 3);
	} else if(NV_isBuiltinOp(&op, "NV_Op_mod")){
		NV_Op_ExecBuiltinInfix(tList, index, 4);
	//
	} else if(NV_isBuiltinOp(&op, "NV_Op_lt")){
		NV_Op_ExecBuiltinInfix(tList, index, 10);
	} else if(NV_isBuiltinOp(&op, "NV_Op_gte")){
		NV_Op_ExecBuiltinInfix(tList, index, 11);
	} else if(NV_isBuiltinOp(&op, "NV_Op_lte")){
		NV_Op_ExecBuiltinInfix(tList, index, 12);
	} else if(NV_isBuiltinOp(&op, "NV_Op_gt")){
		NV_Op_ExecBuiltinInfix(tList, index, 13);
	} else if(NV_isBuiltinOp(&op, "NV_Op_eq")){
		NV_Op_ExecBuiltinInfix(tList, index, 14);
	} else if(NV_isBuiltinOp(&op, "NV_Op_neq")){
		NV_Op_ExecBuiltinInfix(tList, index, 15);
	//
	} else if(NV_isBuiltinOp(&op, "NV_Op_save")){
		NV_Op_save(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_restore")){
		NV_Op_restore(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_ls")){
		NV_Op_ls(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_ls2")){
		NV_Op_ls2(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_last")){
		NV_Op_last(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_assign")){
		NV_Op_assign(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_codeBlock")){
		NV_Op_codeBlock(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_if")){
		NV_Op_if(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_print")){
		NV_Op_print(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_for")){
		NV_Op_for(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_info")){
		NV_Op_info(tList, index);
	} else if(NV_isBuiltinOp(&op, "NV_Op_clean")){
		NV_Op_clean(tList, index);
	} else{
		NV_ID errObj = NV_Node_createWithString(
			"Error: Op NOT found or NOT implemented.");
		NV_Array_writeToIndex(tList, index, &errObj);
	}
	//
	if(IS_DEBUG_MODE()){
		printf("end op ");
		NV_printNodeByID(&op);
		putchar('\n');
	}
}

void NV_printOp(const NV_ID *op)
{
	NV_ID eFunc;
	NV_ID ePrec;
	eFunc = NV_Node_getRelatedNodeFrom(op, &RELID_OP_FUNC);
	ePrec = NV_Node_getRelatedNodeFrom(op, &RELID_OP_PRECEDENCE);
	printf("(op ");
	NV_printNodeByID(&eFunc);
	printf("/");
	NV_printNodeByID(&ePrec);
	printf(")");
}
