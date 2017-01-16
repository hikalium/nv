#include "nv.h"
//
// Graph
//
NV_Node nodeRoot;

void NV_Graph_addStaticNode(const NV_ID *id, const char *s){
	NV_Node_createWithID(id);
	NV_Node_setStrToID(id, s);
	NV_Node_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_init()
{
	srand(time(NULL));
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
	//
	NV_Node_createWithID(&NODEID_NV_STATIC_ROOT);
	NV_Node_setStrToID(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_Node_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_Graph_addStaticNode(&NODEID_NULL, "NullElement");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_ARRAY, "TreeType(Array)");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_VARIABLE, "TreeType(Variable)");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_OP, "TreeType(Op)");
	//
	NV_Graph_addStaticNode(&RELID_TREE_TYPE, "relTreeType");
	NV_Graph_addStaticNode(&RELID_ARRAY_NEXT, "relArrayNext");
	NV_Graph_addStaticNode(&RELID_VARIABLE_DATA, "relVariableData");
	NV_Graph_addStaticNode(&RELID_POINTER_TARGET, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_PRECEDENCE, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_FUNC, "relPointerTarget");
}


void NV_Graph_dump()
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
}

/*
void NV_Graph_dumpToFile(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
	fclose(fp);
}
*/
void NV_Graph_dumpToFile(FILE *fp)
{
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_fdump(fp, n); fputc('\n', fp);
	}
}

void NV_Graph_restoreFromFile(FILE *fp)
{
	char s[128], c;
	int i;
	NV_ID id;

	for(;;){
		s[0] = fgetc(fp);
		if(s[0] == EOF) break;
		for(i = 1; i < 32; i++){
			s[i] = fgetc(fp);
		}
		s[i] = 0;
		if(NV_ID_setFromString(&id, s)){
			printf("Invalid id format.\n");
			return;
		}
		NV_ID_dumpIDToFile(&id, stdout);
		putchar('\n');
		//
		for(;;){
			c = fgetc(fp);
			if(c == EOF || c == '\n') break;
		}
	}
}


int NV_isTreeType(const NV_ID *node, const NV_ID *tType)
{
	NV_ID typeID = NV_Node_getRelatedNodeFrom(node, &RELID_TREE_TYPE);
	return NV_ID_isEqual(&typeID, tType);
}

int NV_runInteractive(const NV_ID *cTypeList, const NV_ID *opList)
{
	char line[MAX_INPUT_LEN];
	NV_ID tokenList;
	//
	while(NV_gets(line, sizeof(line)) != NULL){
		tokenList = NV_tokenize(cTypeList, line);
		NV_convertLiteral(&tokenList, opList);
		NV_printNodeByID(&tokenList);
		NV_evaluateSetence(&tokenList);
		NV_printNodeByID(&tokenList);
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
				//NV_E_setFlag(t, EFUnknownToken);
			}
			p = input + i;
		}
		lastCType = cType;
		if(input[i] == 0) break;
		
	}
#ifdef DEBUG
	if(NV_debugFlag & NV_DBG_FLAG_VERBOSE)
		NV_List_printAll(termRoot, NULL, NULL, "]\n");
#endif
	NV_Array_print(&tokenList);
	return tokenList;
}


int NV_convertLiteral(const NV_ID *tokenizedList, const NV_ID *opList)
{
	// retv: converted token list
	int pIndex;
	int32_t tmpNum;
	NV_ID itemID, opID;
	NV_Node *item;
	int i;
	//
	for(i = 0; ; i++){
		itemID = NV_Array_getByIndex(tokenizedList, i);
		if(NV_ID_isEqual(&itemID, &NODEID_NULL)) break;
		item = NV_Node_getByID(&itemID);
		// check operator
		opID = NV_Dict_get(opList, &itemID);
		if(!NV_ID_isEqual(&opID, &NODEID_NULL)){
			NV_Array_writeToIndex(tokenizedList, i, &opID);
			continue;
		}
		// check Integer
		tmpNum = NV_Node_String_strtol(item, &pIndex, 0);
		if(pIndex != 0 && (int)NV_Node_String_strlen(item) == pIndex){
			// converted entire string to number.
			NV_Node_setInt32ToID(&itemID, tmpNum);
			continue;
		}
	}
	return 0;
}

//
// Evaluate
//
void NV_evaluateSetence(const NV_ID *tokenizedList)
{
	int i, lastOpIndex;
	int32_t lastOpPrec, opPrec;
	NV_ID t, lastOp;
	for(;;){
		lastOpPrec = -1;
		for(i = 0; ; i++){
			t = NV_Array_getByIndex(tokenizedList, i);
			if(NV_ID_isEqual(&t, &NODEID_NULL)) break;
			if(!NV_isTreeType(&t, &NODEID_TREE_TYPE_OP)) continue;
			opPrec = NV_getOpPrecAt(tokenizedList, i);
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
		if(lastOpPrec == -1) break;	// no more op
		NV_tryExecOpAt(tokenizedList, lastOpIndex);
	}
}
/*
void NV_evaluateSentence(const NV_ID *tokenizedList)
{
	int pIndex;
	int32_t tmpNum;
	NV_ID itemID, opID, opPrecID, lastOpID;
	NV_Node *item;
	int i;
	int lastOpPrec, opPrec, d;
	//
	for(;;){
		lastOpPrec = -1;
		lastOpID = NODEID_NULL;
		for(i = 0; ; i++){
			itemID = NV_Array_getByIndex(tokenizedList, i);
			if(NV_ID_isEqual(&itemID, &NODEID_NULL)) break;
			item = NV_Node_getByID(&itemID);
			// check operator
			if(!NV_isTreeType(&itemID, &NODEID_TREE_TYPE_OP)) continue;
			opPrecID = NV_Dict_get(&itemID, &RELID_OP_PRECEDENCE);
			opPrec = NV_Node_getInt32FromID(&opPrecID);
			if(lastOpPrec & 1 ? lastOpPrec <= opPrec : lastOpPrec < opPrec){
				lastOpPrec = opPrec;
				lastOpID = itemID;
				continue;
			}
			// found. lastOpID is target op.
			break;
		}
		if(NV_E_isNullPointer(last)){
			// no more op
			return;
		}
		NV_tryExecOp(excFlag, lang, last, vDict, root);
		if(*excFlag & NV_EXC_FLAG_FAILED){
			// op mismatched
			return;
		}
		if(*excFlag & NV_EXC_FLAG_EXIT){
			// end flag
			return;
		}
	}
}
*/
//
// main
//
int32_t NV_debugFlag;
int main(int argc, char *argv[])
{
	NV_ID cTypeList, opList;
	NV_Graph_init();
	//
	cTypeList = NV_createCharTypeList();
	NV_Node_retain(&cTypeList);
	//
	opList = NV_createOpList();
	NV_Node_retain(&opList);
	//
	NV_runInteractive(&cTypeList, &opList);
	//
	return 0;
}


