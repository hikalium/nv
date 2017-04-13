#include "nv.h"
//
// main
//
volatile sig_atomic_t NV_globalExecFlag;

int NV_interactiveInput(const NV_ID *cTypeList, const NV_ID *ctx)
{
	char line[MAX_INPUT_LEN];
	//
	if(NV_gets(line, sizeof(line)) != NULL){
		NV_evalLine(cTypeList, ctx, line);
		return 0;
	}
	return 1;
}

int NV_evalLine(const NV_ID *cTypeList, const NV_ID *ctx, const char *line)
{
	NV_ID tokenList;
	tokenList = NV_tokenize(cTypeList, line);
	if(IS_DEBUG_MODE()){
		NV_Term_print(&tokenList); putchar('\n');
	}
	NV_Context_pushToEvalStack(ctx, &tokenList, &NODEID_NULL);
	if(IS_DEBUG_MODE()){
		NV_Term_print(&tokenList); putchar('\n');
	}
	return 0;
}

NV_ID NV_tokenize(const NV_ID *cTypeList, const char *input)
{
	// retv: tokenized str array
	const char *p;
	int i, lastCType, cType;
	char buf[MAX_TOKEN_LEN];
	NV_ID tokenList = NV_Array_create();
	NV_ID ns;
	lastCType = 0;
	p = input;
	for(i = 0; ; i++){
		cType = NV_Lang_getCharType(cTypeList, input[i]);
		if(cType != lastCType ||
			cType == 2 || lastCType == 2 || cType == 0 || lastCType == 0){
			if(input + i - p != 0){
				if((input + i - p) > MAX_TOKEN_LEN){
					NV_Error("%s", "Too long token.");
					exit(EXIT_FAILURE);
				}
				NV_strncpy(buf, p, MAX_TOKEN_LEN, input + i - p);
				//
				ns = NV_Node_createWithString(buf);
				NV_Array_push(&tokenList, &ns);
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
		
	}
	if(IS_DEBUG_MODE()){
		NV_Array_print(&tokenList);
	}
	return tokenList;
}

//
// Evaluate
//

int NV_getNextOpIndex(const NV_ID *currentBlock, const NV_ID *opDict)
{
	// 次に実行すべきオペレータを探し、そのインデックスを返す
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp, org;
	//
	lastOpPrec = -1;
	lastOpIndex = -1;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(currentBlock, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		t = NV_Term_tryReadAsOperator(&t, opDict);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_OP)) continue;
		opPrec = NV_getOpPrec(&t);
		/*
		if(IS_DEBUG_MODE()){
			printf("Op found. prec = %d ", opPrec);
			NV_Term_print(&t); putchar('\n');
		}
		*/
		if(lastOpPrec & 1 ? lastOpPrec <= opPrec : lastOpPrec < opPrec){
			// continue searching
			lastOpIndex = i;
			lastOpPrec = opPrec;
			lastOp = t;
			continue;
		}
		// found. lastOpID is target op.
		break;
	}
	if(lastOpIndex != -1){
		org = NV_Array_getByIndex(currentBlock, lastOpIndex);
		NV_Dict_addUniqueEqKeyByCStr(&org, "recogAsOp", &lastOp);
	}
	return lastOpIndex;
}

int NV_checkAndPrintErrorOfCodeBlock(const NV_ID *code)
{
	NV_ID t, failedOp, failedReason;
	int i;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(code, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		failedOp = NV_Dict_getByStringKey(&t, "failedOp");
		if(!NV_NodeID_isEqual(&failedOp, &NODEID_NOT_FOUND)){
			failedReason = NV_Dict_getByStringKey(&t, "failedReason");
			NV_Term_print(&failedOp);
			printf(" : ");
			NV_Term_print(&failedReason);
			printf("\n");
		}
	}
	return 0;
}

void NV_evalLoop(const NV_ID *opList, const NV_ID *ctx)
{
	NV_ID currentBlock;
	NV_ID currentTermIndexNode;
	NV_ID currentTerm;
	NV_ID evalStack = NV_Context_getEvalStack(ctx);
	NV_ID t;
	int nextOpIndex, currentOpIndex;
	//
	for(;;){
		currentBlock = NV_Array_last(&evalStack);
		if(NV_globalExecFlag & NV_EXEC_FLAG_INTERRUPT){
			// env saving
			fprintf(stderr, "Saving env to `savefile`...\n");
			FILE *fp = fopen("savefile", "wb");
			if(!fp){
				fprintf(stderr, "fopen failed.\n");
			} else{
				NV_Node_dumpAllToFile(fp);
				fclose(fp);
				fprintf(stderr, "Saving env done.\n");
				exit(EXIT_SUCCESS);
			}
			NV_globalExecFlag &= ~NV_EXEC_FLAG_INTERRUPT;
		}
		//
		if(IS_DEBUG_MODE()){
			printf("---- eval begin\n ");
			//
			//printf("current evalStack: ");
			//NV_Term_print(&evalStack); putchar('\n');
		}
		if(NV_NodeID_isEqual(&currentBlock, &NODEID_NOT_FOUND)){
			// evalStack is empty.
			if(IS_DEBUG_MODE()){
				printf("evalStack empty. break.\n");
			}
			break;
		}
		// find next op
		currentTermIndexNode = NV_NodeID_getRelatedNodeFrom(
			&currentBlock, &RELID_CURRENT_TERM_INDEX);
		if(!NV_NodeID_isEqual(&currentTermIndexNode, &NODEID_NOT_FOUND)){
			// do op
			currentOpIndex = NV_NodeID_getInt32(&currentTermIndexNode);
			currentTerm = NV_Array_getByIndex(&currentBlock, currentOpIndex);
			if(IS_DEBUG_MODE()){
				printf("op at currentBlock[%d]: ", currentOpIndex);
				NV_Term_print(&currentTerm); putchar('\n');
			}
			NV_tryExecOpAt(&currentBlock, currentOpIndex, ctx);
			if(IS_DEBUG_MODE()){
				printf("evalStack after op exec: ");
				NV_Term_print(&evalStack); putchar('\n');
			}
		}
		currentBlock = NV_Array_last(&evalStack);
		// search next term to do
		nextOpIndex = NV_getNextOpIndex(&currentBlock, opList);
		if(nextOpIndex == -1){
			// no more op
			if(IS_DEBUG_MODE()){
				printf("nextOp not found\n");
			}
			t = NV_Array_pop(&evalStack);
			// error check
			NV_checkAndPrintErrorOfCodeBlock(&t);
			// store last result
			NV_NodeID_createUniqueIDRelation(ctx, &RELID_LAST_RESULT, &t);
			continue;
		}
		t = NV_Node_createWithInt32(nextOpIndex);
		//if(IS_DEBUG_MODE()){
		//	printf("nextOpIndex: %d\n", nextOpIndex);
		//}
		NV_NodeID_createUniqueIDRelation(
			&currentBlock, &RELID_CURRENT_TERM_INDEX, &t);
	}
}

#define CG_BUF_SIZE	128
int cgNodesUsed;
NV_ID cgNodes[CG_BUF_SIZE];
int cgRelsUsed;
NV_ID cgRels[CG_BUF_SIZE][3];	// [from, rel, to]

void NV_saveCodeGraph_trace(const NV_ID *n);
void NV_saveCodeGraph_digForKey(const NV_ID *n, const char *key)
{
	NV_ID relStr;
	NV_ID to, rel;
	//
	relStr = NV_Node_createWithString(key);
	rel = NV_NodeID_getEqRelationFrom(n, &relStr);
	if(!NV_NodeID_isEqual(&rel, &NODEID_NOT_FOUND)){
		to = NV_NodeID_Relation_getIDLinkTo(&rel);
		cgRels[cgRelsUsed][0] = *n;
		cgRels[cgRelsUsed][1] = NV_NodeID_Relation_getIDLinkRel(&rel);
		cgRels[cgRelsUsed][2] = to;
		cgRelsUsed++;
		NV_saveCodeGraph_trace(&to);
	}
}
void NV_saveCodeGraph_trace(const NV_ID *n)
{
	int i;
	//
	for(i = 0; i < cgNodesUsed; i++){
		if(NV_NodeID_isEqual(n, &cgNodes[i])) return;
	}
	//
	cgNodes[cgNodesUsed++] = *n;
	//
	NV_saveCodeGraph_digForKey(n, "next");
	NV_saveCodeGraph_digForKey(n, "op");
	NV_saveCodeGraph_digForKey(n, "opL");
	NV_saveCodeGraph_digForKey(n, "opR");
	NV_saveCodeGraph_digForKey(n, "result");
	NV_saveCodeGraph_digForKey(n, "data");
	NV_saveCodeGraph_digForKey(n, "truePath");
	NV_saveCodeGraph_digForKey(n, "flag");
	NV_saveCodeGraph_digForKey(n, "call");
	//NV_saveCodeGraph_digForKey(n, "type");
}
void NV_saveCodeGraphForVisualization(const NV_ID *codeGraphRoot, const char *path)
{
	cgNodesUsed = 0;
	cgRelsUsed = 0;
	NV_saveCodeGraph_trace(codeGraphRoot);
	printf("Node: %d, Rel: %d\n", cgNodesUsed, cgRelsUsed);
	FILE *fp = fopen(path, "wb");
	if(!fp){
		return;
	}
	fprintf(fp, "digraph code {\n");
	int i;
	
	fprintf(fp, "node [shape = oval] \n");
	for(i = 0; i < cgNodesUsed; i++){
		NV_ID *n;
		n = &cgNodes[i];
		//
		fprintf(fp, "n");
		NV_ID_dumpIDToFile(n, fp);
		fprintf(fp, "\n");
	}
	fprintf(fp, ";\n");
	
	for(i = 0; i < cgNodesUsed; i++){
		NV_ID *n;
		const char *nStr;
		n = &cgNodes[i];
		if(NV_NodeID_isString(n)){
			nStr = NV_NodeID_getCStr(n);
		} else{
			nStr = "";
		}
		//
		if(nStr){
			fprintf(fp, "n");
			NV_ID_dumpIDToFile(n, fp);
			fprintf(fp, " [label=\"%s\"]", nStr);
			fprintf(fp, ";\n");
		}
	}

	for(i = 0; i < cgRelsUsed; i++){
		NV_ID *from, *rel, *to;
		const char *relStr;
		from = &cgRels[i][0];
		rel = &cgRels[i][1];
		to = &cgRels[i][2];
		if(NV_NodeID_isString(rel)){
			relStr = NV_NodeID_getCStr(rel);
		} else{
			relStr = "";
		}
		//
		fprintf(fp, "n");
		NV_ID_dumpIDToFile(from, fp);
		fprintf(fp, " -> ");
		fprintf(fp, "n");
		NV_ID_dumpIDToFile(to, fp);
		if(relStr){
			if(strcmp(relStr, "next") == 0){
				fprintf(fp, " [label=\"%s\", color=red]", relStr);
			} else if(strcmp(relStr, "truePath") == 0){
				fprintf(fp, " [label=\"%s\", color=green]", relStr);
			} else{
				fprintf(fp, " [label=\"%s\"]", relStr);
			}
		}
		fprintf(fp, ";\n");
	}
	fprintf(fp, "}\n");
	fclose(fp);
}

void NV_parseToCodeGraph_infixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName)
{
	NV_ID funcNode = NV_Node_createWithString("infixOp");
	NV_ID op = NV_Node_createWithString(opName);
	NV_ID opL = NV_Array_getByIndex(tokenList, opIndex - 1);
	NV_ID opR = NV_Array_getByIndex(tokenList, opIndex + 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, opIndex - 1);
	NV_Array_removeIndex(tokenList, opIndex - 1);
	NV_Array_writeToIndex(tokenList, opIndex - 1, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
}

void NV_parseToCodeGraph_prefixOp
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const char *opName)
{
	NV_ID funcNode = NV_Node_createWithString("prefixOp");
	NV_ID op = NV_Node_createWithString(opName);
	NV_ID opR = NV_Array_getByIndex(tokenList, opIndex + 1);
	NV_ID result = NV_Variable_create();
	//
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "op", &op);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opR", &opR);
	NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
	//
	NV_Array_removeIndex(tokenList, opIndex);
	NV_Array_writeToIndex(tokenList, opIndex, &result);
	//
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
	*lastNode = funcNode;
}

void NV_parseToCodeGraph_if
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict)
{
	// if {cond} {do} [{cond} {do}] [{else}]
	int i, count;
	NV_ID t;
	NV_ID condT = NODEID_NOT_FOUND, doT;
	NV_ID terminateNode = NV_Node_createWithString("endif");

	for(i = 1; ; i++){
		t = NV_Array_getByIndex(tokenList, opIndex + i);
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
				// 直前の節(condT)はelseだった。
				if(!NV_NodeID_isEqual(&condT, &NODEID_NOT_FOUND)){
					NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &condT);
					*lastNode = condT;
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
		NV_Array_removeIndex(tokenList, opIndex);
	}
	NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &terminateNode);
	*lastNode = terminateNode;
}

NV_ID NV_parseToCodeGraph_for
(const NV_ID *tokenList, NV_ID *lastNode, int opIndex, const NV_ID *opDict)
{
	// for {init block}{conditional block}{update block}[{statement}]
	NV_ID initT, condT, updtT, doT;
	NV_ID initF, condF, updtF, doF;
	NV_ID terminateF, t;
	int i;
	for(i = 1; i <= 3; i++){
		t = NV_Array_getByIndex(tokenList, opIndex + i);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)){
			fprintf(stderr, "too few codeblocks for for.\n");
			return NODEID_NULL;
		}
	}
	initT = NV_Array_getByIndex(tokenList, opIndex + 1);
	condT = NV_Array_getByIndex(tokenList, opIndex + 2);
	updtT = NV_Array_getByIndex(tokenList, opIndex + 3);
	doT   = NV_Array_getByIndex(tokenList, opIndex + 4);
	for(i = 0; i < 4; i++){
		NV_Array_removeIndex(tokenList, opIndex);
	}
	if(!NV_isTermType(&doT, &NODEID_TERM_TYPE_ARRAY)){
		doT = NV_Array_create();
	} else{
		NV_Array_removeIndex(tokenList, opIndex);	
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
	NV_Dict_addUniqueEqKeyByCStr(&condF, "call", &condT);
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

NV_ID NV_parseToCodeGraph(const NV_ID *tokenList, const NV_ID *opDict)
{
	NV_ID codeGraphRoot = NV_Node_createWithString("eval");
	NV_ID lastNode = codeGraphRoot, e;
	int opIndex;

	for(;;){
		opIndex = NV_getNextOpIndex(tokenList, opDict);
		if(opIndex == -1) break;
		NV_ID n = NV_Array_getByIndex(tokenList, opIndex);
		if(NV_NodeID_isEqual(&n, &NODEID_NOT_FOUND)) break;
		fprintf(stderr, "NV_parseToCodeGraph: begin parse.\n");
		NV_NodeID_printForDebug(&n);
		if(NV_Node_String_compareWithCStr(&n, " ") == 0){
			NV_Array_removeIndex(tokenList, opIndex);
			continue;
		} else if(NV_Node_String_compareWithCStr(&n, "+") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "plus");
		} else if(NV_Node_String_compareWithCStr(&n, "-") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "sub");
		} else if(NV_Node_String_compareWithCStr(&n, "*") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "mul");
		} else if(NV_Node_String_compareWithCStr(&n, "/") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "div");
		} else if(NV_Node_String_compareWithCStr(&n, "%") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "mod");
		} else if(NV_Node_String_compareWithCStr(&n, "<") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_lt");
		} else if(NV_Node_String_compareWithCStr(&n, ">=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_gte");
		} else if(NV_Node_String_compareWithCStr(&n, "<=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_lte");
		} else if(NV_Node_String_compareWithCStr(&n, ">") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_gt");
		} else if(NV_Node_String_compareWithCStr(&n, "==") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_eq");
		} else if(NV_Node_String_compareWithCStr(&n, "!=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "cmp_neq");
		} else if(NV_Node_String_compareWithCStr(&n, "=") == 0){
			NV_parseToCodeGraph_infixOp(tokenList, &lastNode, opIndex, "assign");
		} else if(NV_Node_String_compareWithCStr(&n, "++") == 0){
			NV_parseToCodeGraph_prefixOp(tokenList, &lastNode, opIndex, "inc");
		} else if(NV_Node_String_compareWithCStr(&n, "print") == 0){
			NV_parseToCodeGraph_prefixOp(tokenList, &lastNode, opIndex, "print");
		} else if(NV_Node_String_compareWithCStr(&n, "{") == 0){
			NV_Op_codeBlock(tokenList, opIndex, "{", "}");
		} else if(NV_Node_String_compareWithCStr(&n, "if") == 0){
			NV_parseToCodeGraph_if(tokenList, &lastNode, opIndex, opDict);
		} else if(NV_Node_String_compareWithCStr(&n, "for") == 0){
			e = NV_parseToCodeGraph_for(tokenList, &lastNode, opIndex, opDict);
			if(NV_NodeID_isEqual(&e, &NODEID_NULL)){
				return NODEID_NULL;
			}
		} else{
			fprintf(stderr, "NV_parseToCodeGraph: op not implemented.\n");
			NV_NodeID_printForDebug(&n);
			return NODEID_NULL;
		}
	}
	//NV_Dict_print(&codeGraphRoot); putchar('\n');
	NV_Array_print(tokenList); putchar('\n');
	//NV_saveCodeGraphForVisualization(&codeGraphRoot, "note/code.dot");
	return codeGraphRoot;
}

