#include "nv.h"

int NV_Op_isOperator(const NV_ID *id)
{
	return NV_isTermType(id, &NODEID_TERM_TYPE_OP);
}

int NV_Op_canBeOperator(const NV_ID *ident, const NV_ID *opDict)
{
	NV_ID op = NV_Op_findOpNamed(ident, opDict);
	return NV_Op_isOperator(&op);
}

int NV_Op_f_OpPrec_Dec(const void *n1, const void *n2)
{
	const NV_ID *e1 = n1, *e2 = n2;
	return NV_Lang_getOpPrec(e2) - NV_Lang_getOpPrec(e1);
}

NV_ID NV_Op_findOpNamed(const NV_ID *id, const NV_ID *opDict)
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
	opList = NV_Array_getSorted(&opList, NV_Op_f_OpPrec_Dec);
	triedPrecNode = NV_Dict_getByStringKey(id, "triedPrec");
	triedPrec = NV_NodeID_getInt32(&triedPrecNode);
	//
	for(i = 0; ; i++){
		opID = NV_Array_getByIndex(&opList, i);
		if(triedPrec == -1 || NV_Lang_getOpPrec(&opID) < triedPrec) break;
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

