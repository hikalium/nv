#include "nv.h"

//
//	Type checks
//

int NV_isTermType(const NV_ID *node, const NV_ID *tType)
{
	NV_ID typeID = NV_NodeID_getRelatedNodeFrom(node, &RELID_TERM_TYPE);
	return NV_ID_isEqual(&typeID, tType);
}

NV_ID NV_Term_tryReadAsVariable(const NV_ID *id, const NV_ID *ctx)
{
	// 変数としてidが解釈できるなら、その変数の値に相当するidを返す
	// 変数として解釈できる、とは
	//   - リテラルでない文字列で、コンテキストに存在する変数名と等しい
	//   - TermType === Variable
	// 無理ならば、もとのidを返す
	NV_ID vid;
	//
	vid = *id;
	if(NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)){
		vid = NV_Variable_getData(id);
	} else if(NV_NodeID_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		if(!NV_ID_isEqual(&vid, &NODEID_NOT_FOUND)){
			vid = NV_Variable_getData(&vid);
		} else{
			vid = *id;
		}
	}
	return vid;
}

int NV_Term_f_OpPrec_Dec(const void *n1, const void *n2)
{
	const NV_ID *e1 = n1, *e2 = n2;
	return NV_getOpPrec(e2) - NV_getOpPrec(e1);
}

NV_ID NV_Term_tryReadAsOperator(const NV_ID *id, const NV_ID *ctx)
{
	// <id>: String であることを想定
	// <id>/triedPrec が設定されているならば、それ未満のPrecのものの中で
	// 最大のものを返す。
	// なければ、NOT_FOUND
	NV_ID opID, opList;
	int i;
	//
	opList = NV_Dict_getAll(ctx, id);
	opList = NV_Array_getSorted(&opList, NV_Term_f_OpPrec_Dec);
	for(i = 0; ; i++){
		opID = NV_Array_getByIndex(&opList, i);
		break;
	}

	if(!NV_ID_isEqual(&opID, &NODEID_NOT_FOUND)){
		return opID;
	}
	return *id;
}


int NV_Term_isStrLiteral(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid = NV_Term_tryReadAsVariable(id, ctx);
	return NV_NodeID_isInteger(&vid);
}

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

int NV_Term_isInteger(const NV_ID *id, const NV_ID *ctx)
{
	if(NV_Term_isIntegerNotVal(id)){
		return 1;
	}
	// ダメだったら変数として解釈を試みる
	if(NV_Term_isAssignable(id, ctx)){
		NV_ID vid = NV_Term_tryReadAsVariable(id, ctx);
		return NV_Term_isIntegerNotVal(&vid);
	}
	// 変数でもないので整数とは解釈できない
	return 0;
}

int NV_Term_isAssignable(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_NodeID_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		id = &vid;
	}

	return NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE);
}

int NV_Term_isArray(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid = NV_Term_tryReadAsVariable(id, ctx);
	return NV_isTermType(&vid, &NODEID_TERM_TYPE_ARRAY);
}

//
// Read term data
//

int32_t NV_Term_getInt32NotVal(const NV_ID *id)
{
	// 整数ならそれを返す
	if(NV_NodeID_isInteger(id)){
		return NV_NodeID_getInt32(id);
	}
	// リテラルでない文字列ならば解釈を試みる
	if(NV_NodeID_isString(id)){
		// idはstringである。数値として解釈することを試みる。
		int endi;
		int32_t v;
		v = NV_NodeID_String_strtol(id, &endi, 0);
		if((size_t)endi == NV_NodeID_String_strlen(id)){
			// 文字列全体が整数として解釈できたのでこれは整数
			return v;
		}
	}
	// どうやっても数値とは解釈できない
	return -1;
}

int32_t NV_Term_getInt32(const NV_ID *id, const NV_ID *ctx)
{
	if(NV_Term_isIntegerNotVal(id)){
		return NV_Term_getInt32NotVal(id);
	}
	// ダメだったら変数として解釈を試みる
	if(NV_Term_isAssignable(id, ctx)){
		NV_ID vid = NV_Term_tryReadAsVariable(id, ctx);
		return NV_Term_getInt32NotVal(&vid);
	}
	// 変数でもないので整数とは解釈できない
	return -1;
}

NV_ID NV_Term_getAssignableNode(const NV_ID *id, const NV_ID *ctx)
{
	NV_ID vid;
	if(NV_NodeID_isString(id)){
		vid = NV_Variable_getNamed(ctx, id);
		id = &vid;
	}
	if(!NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE)) return NODEID_NOT_FOUND;
	return *id;
}

//
// print
//

void NV_printNodeByID(const NV_ID *id)
{
	NV_Node *n = NV_NodeID_getNode(id);
	NV_printNode(n);
}

void NV_printNode(const NV_Node *n)
{
	if(!n){
		printf("(null pointer node)");
		return;
	}
	if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_ARRAY)){
		NV_Array_print(&n->id);
	} else if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_VARIABLE)){
		NV_Variable_print(&n->id);
	} else if(NV_isTermType(&n->id, &NODEID_TERM_TYPE_OP)){
		NV_printOp(&n->id);
	} else{
		NV_Node_printPrimVal(n);
	}
}
