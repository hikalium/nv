#include "nv.h"
//
// main
//
volatile sig_atomic_t NV_globalExecFlag;

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

NV_OpPointer NV_getNextOp(const NV_ID *currentBlock, const NV_ID *opDict)
{
	// 次に実行すべきオペレータを探し、そのインデックスを返す
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp, org;
	NV_OpPointer p;
	//
	p.index = -1;
	lastOpPrec = -1;
	lastOpIndex = -1;
	for(i = 0; ; i++){
		t = NV_Array_getByIndex(currentBlock, i);
		if(NV_NodeID_isEqual(&t, &NODEID_NOT_FOUND)) break;
		t = NV_Term_tryReadAsOperator(&t, opDict);
		if(!NV_isTermType(&t, &NODEID_TERM_TYPE_OP)) continue;
		opPrec = NV_Lang_getOpPrec(&t);
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
		//
		p.op = lastOp;
		p.index = lastOpIndex;
		p.dict = *opDict;
		p.prec = lastOpPrec;
	}
	return p;
}
/*
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
*/
/*
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
*/
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
	NV_saveCodeGraph_digForKey(n, "inner");
}
void NV_saveCodeGraphForVisualization
(const NV_ID *codeGraphRoot, const char *path_prefix)
{
	char path[256];
	snprintf(path, sizeof(path), "%s%08X.dot", path_prefix, codeGraphRoot->d[0]);
	cgNodesUsed = 0;
	cgRelsUsed = 0;
	NV_saveCodeGraph_trace(codeGraphRoot);
	printf("Saved to %s (Node: %d, Rel: %d)\n", path, cgNodesUsed, cgRelsUsed);
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
	// nodes
	for(i = 0; i < cgNodesUsed; i++){
		NV_ID *n;
		char nStr[64];
		n = &cgNodes[i];
		if(NV_Variable_isVariable(n)){
			NV_Variable_snprintf(nStr, sizeof(nStr), n);
		} else if(NV_NodeID_isString(n)){
			snprintf(nStr, sizeof(nStr), "\\\"%s\\\"", NV_NodeID_getCStr(n));
		} else if(NV_NodeID_isInteger(n)){
			snprintf(nStr, sizeof(nStr), "%d", NV_NodeID_getInt32(n));
		} else{
			snprintf(nStr, sizeof(nStr), "");
		}
		//
		fprintf(fp, "n");
		NV_ID_dumpIDToFile(n, fp);
		fprintf(fp, " [label=\"%s\"]", nStr);
		fprintf(fp, ";\n");
	}
	// rels
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



