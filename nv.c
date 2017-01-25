#include "nv.h"

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
		NV_Array_print(tokenizedList);
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


