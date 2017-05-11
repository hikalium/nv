#include "nv.h"

//
//	Type checks
//

int NV_Term_isNotFound(const NV_ID *id)
{
	return NV_NodeID_isEqual(id, &NODEID_NOT_FOUND);
}

int NV_Term_isNull(const NV_ID *id)
{
	return NV_NodeID_isEqual(id, &NODEID_NULL);
}

int NV_Term_isOperator(const NV_ID *id)
{
	return NV_isTermType(id, &NODEID_TERM_TYPE_OP);
}

int NV_Term_canBeOperator(const NV_ID *id, const NV_ID *opDict)
{
	NV_ID op = NV_Term_tryReadAsOperator(id, opDict);
	return NV_Term_isOperator(&op);
}

int NV_isTermType(const NV_ID *node, const NV_ID *tType)
{
	NV_ID typeID = NV_NodeID_getRelatedNodeFrom(node, &RELID_TERM_TYPE);
	return NV_NodeID_isEqual(&typeID, tType);
}

NV_ID NV_Term_tryReadAsVariableData(const NV_ID *id, const NV_ID *scope)
{
	// 変数としてidが解釈できるなら、その変数の値に相当するidを返す
	// 変数として解釈できる、とは
	//   - リテラルでない文字列で、コンテキストに存在する変数名と等しい
	//    - コンテキストはネストされている。元までたどる。
	//   - TermType === Variable
	// 無理ならば、もとのidを返す
	NV_ID vid;
	//
	vid = NV_Term_tryReadAsVariable(id, scope);
	if(NV_isTermType(&vid, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(&vid);
		/*
		printf("NV_Term_tryReadAsVariableData: variable!\n");
		NV_Term_print(&vid);
		printf("\n");
		*/
		if(!NV_NodeID_isEqual(&vid, &NODEID_NOT_FOUND)){
			return vid;
		}
	}
	return *id;
}

NV_ID NV_Term_tryReadAsVariable(const NV_ID *id, const NV_ID *scope)
{
	// 変数としてidが解釈できるなら、その変数に相当するidを返す
	// 変数として解釈できる、とは
	//   - リテラルでない文字列で、コンテキストに存在する変数名と等しい
	//    - コンテキストはネストされている。元までたどる。
	//   - TermType === Variable
	// 無理ならば、もとのidを返す
	//

	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		// もともと変数オブジェクトだった
		printf("Already val obj\n");
		return *id;
	}
	if(NV_NodeID_isString(id) && NV_Variable_statByName(scope, id)){
		// 文字列でかつ現在のscopeに存在するのでその変数を返す
		/*
		printf("name \"%s\" found in context.\n", 
				NV_NodeID_getCStr(id));
				*/
		return NV_Variable_createWithName(scope, id);
	}

	// このコンテキスト階層では見つからなかったので、親があればたどる
	NV_ID pCtx = NV_Dict_getEqID(scope, &RELID_PARENT_SCOPE);
	if(!NV_NodeID_isEqual(&pCtx, &NODEID_NOT_FOUND)){
		// 親の検索結果を返す
		/*
		if(IS_DEBUG_MODE()){
			printf("Var NOT found. Search parent.\n");
		}
		*/
		return NV_Term_tryReadAsVariable(id, &pCtx);
	}
	// あきらめる
	/*
	if(IS_DEBUG_MODE()){
		printf("Var NOT found and has no parent.\n");
	}
	*/
	return *id;
}

NV_ID NV_Term_getPrimNodeID(const NV_ID *id, const NV_ID *scope)
{
	// 変数がネストされている場合は、一番外側の変数のみが展開される。
	NV_ID n;
	if(!id) return NODEID_NOT_FOUND;
	n = *id;
	if(1/* TODO: Add literal string check. */){
		n = NV_Term_tryReadAsVariableData(&n, scope);
		if(IS_DEBUG_MODE()){
			printf("Var check result: ");
			NV_Term_print(id);
			printf(" -> ");
			NV_Term_print(&n);
			putchar('\n');
		}
	}
	return n;
}

int NV_Term_f_OpPrec_Dec(const void *n1, const void *n2)
{
	const NV_ID *e1 = n1, *e2 = n2;
	return NV_getOpPrec(e2) - NV_getOpPrec(e1);
}

NV_ID NV_Term_tryReadAsOperator(const NV_ID *id, const NV_ID *opDict)
{
	// <id>: String であることを想定
	// <id>/triedPrec が設定されているならば、それ未満のPrecのものの中で
	// 最大のものを返す。
	// なければもとのidを返す。
	NV_ID opID, opList, triedPrecNode;
	int i;
	int32_t triedPrec;
	//
	opList = NV_Dict_get(opDict, id);
	if(NV_NodeID_isEqual(&opList, &NODEID_NOT_FOUND)){
		return *id;
	}
	opList = NV_Array_getSorted(&opList, NV_Term_f_OpPrec_Dec);
	triedPrecNode = NV_Dict_getByStringKey(id, "triedPrec");
	triedPrec = NV_NodeID_getInt32(&triedPrecNode);
	//
	for(i = 0; ; i++){
		opID = NV_Array_getByIndex(&opList, i);
		if(triedPrec == -1 || NV_getOpPrec(&opID) < triedPrec) break;
	}
	if(!NV_NodeID_isEqual(&opID, &NODEID_NOT_FOUND)){
		/*
		if(IS_DEBUG_MODE()){
			printf("op found at index: %d\n", i);
		}
		*/
		return opID;
	}
	return *id;
}
/*
int NV_Term_isIntegerNotVal(const NV_ID *id)
{
	// 整数ならそれを返す
	if(NV_NodeID_isInteger(id)){
		return 1;
	}
	// リテラルでない文字列ならば解釈を試みる
	if(NV_NodeID_isString(id)){
		// idはstringである。数値として解釈することを試みる。
		int endi;
		NV_NodeID_String_strtol(id, &endi, 0);
		if((size_t)endi == NV_NodeID_String_strlen(id)){
			// 文字列全体が整数として解釈できたのでこれは整数
			return 1;
		}
	}
	// どうやっても数値とは解釈できない
	return 0;
}
*/
int NV_Term_isInteger(const NV_ID *id, const NV_ID *scope)
{
	NV_ID n;
	n = NV_Term_getPrimNodeID(id, scope);
	if(NV_NodeID_isInteger(&n)) return 1;
	if(NV_NodeID_isString(&n)){
		int endi;
		NV_Node_String_strtol(&n, &endi, 0);
		if((size_t)endi == NV_Node_String_strlen(&n)){
			// 文字列全体が整数として解釈できたのでこれは整数
			return 1;
		}
	}
	// 整数とは解釈できない
	return 0;
}

int NV_Term_isAssignable(const NV_ID *id, const NV_ID *scope)
{
	NV_ID vid;
	vid = NV_Term_tryReadAsVariable(id, scope);

	return NV_isTermType(&vid, &NODEID_TERM_TYPE_VARIABLE);
}

int NV_Term_isArray(const NV_ID *id, const NV_ID *scope)
{
	NV_ID vid = NV_Term_tryReadAsVariableData(id, scope);
	return NV_isTermType(&vid, &NODEID_TERM_TYPE_ARRAY);
}

//
// Read term data
//

int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *scope)
{
	NV_ID n;
	n = NV_Term_getPrimNodeID(id, scope);
	if(NV_NodeID_isInteger(&n)) return NV_NodeID_getInt32(&n);
	if(NV_NodeID_isString(&n)){
		int endi;
		int v;
		v = NV_Node_String_strtol(&n, &endi, 0);
		if((size_t)endi == NV_Node_String_strlen(&n)){
			// 文字列全体が整数として解釈できたのでこれは整数
			return v;
		}
	}
	// 整数とは解釈できない
	return -1;
}

NV_ID NV_Term_getAssignableNode(const NV_ID *id, const NV_ID *scope)
{
	NV_ID vid;
	if(NV_NodeID_isString(id) && NV_Variable_statByName(scope, id)){
		// idが文字列でしかもそれに相当する変数が存在するなら変換
		vid = NV_Variable_createWithName(scope, id);
		id = &vid;
	}
	if(!NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)) return NODEID_NOT_FOUND;
	return *id;
}

//
// Assign to term
//
NV_ID NV_Term_assign(const NV_ID *v, const NV_ID *data)
{
	// v: Path or Variable
	/*
	if(NV_isTermType(v, &NODEID_TERM_TYPE_PATH)){
		NV_Path_assign(v, data);
	} else 
	*/
	if(NV_isTermType(v, &NODEID_TERM_TYPE_VARIABLE)){
		NV_Variable_assign(v, data);
	} else{
		return NV_Node_createWithString("NV_Term_assign:Not assignable");
	}
	return NODEID_NULL;
}

//
// print
//

void NV_Term_print(const NV_ID *id)
{
	if(NV_isTermType(id, &NODEID_TERM_TYPE_ARRAY)){
		NV_Array_print(id);
	} else if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		NV_Variable_print(id);
	} else if(NV_isTermType(id, &NODEID_TERM_TYPE_OP)){
		NV_printOp(id);
		/*
	} else if(NV_isTermType(id, &NODEID_TERM_TYPE_PATH)){
		NV_Path_print(id);
		*/
	} else{
		NV_Node_printPrimVal(id);
	}
}
