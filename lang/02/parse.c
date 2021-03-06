#include "../../nv.h"

NV_ID NV_parseToCodeGraph_nothing(const NV_ID *tokenList, NV_ID *lastNode,
                                  NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(lastNode);
  PARAM_UNUSED(ident);
  //
  NV_Array_removeIndex(tokenList, p->index);
  return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_infixOp(const NV_ID *tokenList, NV_ID *lastNode,
                                  NV_OpPointer *p, const char *ident) {
  NV_ID funcNode = NV_Node_createWithString("infixOp");
  NV_ID op = NV_Node_createWithString(ident);
  NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
  NV_ID opR = NV_Array_getByIndex(tokenList, p->index + 1);
  NV_ID result = NV_Variable_create();
  //
  if (NV_Term_isNotFound(&opL)) {
    return NV_Node_createWithString("Unexpected opL == NOT_FOUND");
  }
  if (NV_Lang_canBeOperator(&opL, &p->dict)) {
    return NV_Node_createWithString("Expected opL is a value");
  }
  if (NV_Lang_canBeOperator(&opR, &p->dict)) {
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

NV_ID NV_parseToCodeGraph_prefixOp(const NV_ID *tokenList, NV_ID *lastNode,
                                   NV_OpPointer *p, const char *ident) {
  // puts("parse: prefix: begin");
  NV_ID funcNode = NV_Node_createWithString("prefixOp");
  NV_ID op = NV_Node_createWithString(ident);
  NV_ID opR = NV_Array_getByIndex(tokenList, p->index + 1);
  NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
  NV_ID result = NV_Variable_create();
  //
  if (!NV_Term_isNotFound(&opL) && !NV_Lang_canBeOperator(&opL, &p->dict)) {
    return NV_Node_createWithString("Expected opL is not a value");
  }
  if (NV_Lang_canBeOperator(&opR, &p->dict)) {
    return NV_Node_createWithString("Expected opR is NOT an operator");
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

NV_ID NV_parseToCodeGraph_postfixOp(const NV_ID *tokenList, NV_ID *lastNode,
                                    NV_OpPointer *p, const char *ident) {
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

NV_ID NV_parseToCodeGraph_codeblock(const NV_ID *tokenList, NV_ID *lastNode,
                                    NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(lastNode);
  PARAM_UNUSED(ident);
  NV_Lang_parseCodeBlock(tokenList, p->index, "{", "}");
  return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_strLiteral(const NV_ID *tokenList, NV_ID *lastNode,
                                     NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(lastNode);
  PARAM_UNUSED(ident);
  NV_Lang_parseStrLiteral(tokenList, p->index);
  return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_parentheses(const NV_ID *tokenList, NV_ID *lastNode,
                                      NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(ident);
  NV_ID *opDict = &p->dict;
  NV_Lang_parseCodeBlock(tokenList, p->index, "(", ")");
  NV_ID funcNode = NV_Node_createWithString("()");
  NV_ID opL = NV_Array_getByIndex(tokenList, p->index - 1);
  NV_ID inner = NV_Array_getByIndex(tokenList, p->index);
  NV_ID result = NV_Variable_create();
  //
  inner = NV_Lang_parseToCodeGraph(&inner, opDict, NV_Lang02_funcList);
  //
  NV_Dict_addUniqueEqKeyByCStr(&funcNode, "opL", &opL);
  NV_Dict_addUniqueEqKeyByCStr(&funcNode, "inner", &inner);
  NV_Dict_addUniqueEqKeyByCStr(&funcNode, "result", &result);
  //
  NV_Array_writeToIndex(tokenList, p->index, &result);
  if (!NV_Term_isNotFound(&opL) && !NV_Lang_canBeOperator(&opL, &p->dict)) {
    NV_Array_removeIndex(tokenList, p->index - 1);
  }
  //
  NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &funcNode);
  *lastNode = funcNode;
  return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_if(const NV_ID *tokenList, NV_ID *lastNode,
                             NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(ident);
  // if {cond} {do} [{cond} {do}] [{else}]
  int i, count;
  NV_ID t;
  NV_ID condT = NODEID_NOT_FOUND, doT;
  NV_ID terminateNode = NV_Node_createWithString("endif");
  NV_ID *opDict = &p->dict;

  for (i = 1;; i++) {
    t = NV_Array_getByIndex(tokenList, p->index + i);
    if (i & 1) {
      // 奇数: 条件節、もしくはelse節
      if (!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)) {
        // 終了
        break;
      }
      condT = t;
    } else {
      // 偶数：実行部分
      if (!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)) {
        // もうブロックがない
        if (!NV_NodeID_isEqual(&condT, &NODEID_NOT_FOUND)) {
          // 直前の節(condT)はelseだった。
          condT = NV_Lang_parseToCodeGraph(&condT, opDict, NV_Lang02_funcList);
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
      condT = NV_Lang_parseToCodeGraph(&condT, opDict, NV_Lang02_funcList);
      doT = NV_Lang_parseToCodeGraph(&doT, opDict, NV_Lang02_funcList);
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
  for (i = 0; i < count; i++) {
    NV_Array_removeIndex(tokenList, p->index);
  }
  NV_Dict_addUniqueEqKeyByCStr(lastNode, "next", &terminateNode);
  *lastNode = terminateNode;
  return NODEID_NULL;
}

NV_ID NV_parseToCodeGraph_for(const NV_ID *tokenList, NV_ID *lastNode,
                              NV_OpPointer *p, const char *ident) {
  PARAM_UNUSED(ident);
  // for {init block}{conditional block}{update block}[{statement}]
  NV_ID initT, condT, updtT, doT;
  NV_ID initF, condF, updtF, doF;
  NV_ID terminateF, t;
  NV_ID *opDict = &p->dict;
  int i;
  for (i = 1; i <= 3; i++) {
    t = NV_Array_getByIndex(tokenList, p->index + i);
    if (!NV_isTermType(&t, &NODEID_TERM_TYPE_ARRAY)) {
      fprintf(stderr, "too few codeblocks for for.\n");
      return NODEID_NULL;
    }
  }
  initT = NV_Array_getByIndex(tokenList, p->index + 1);
  condT = NV_Array_getByIndex(tokenList, p->index + 2);
  updtT = NV_Array_getByIndex(tokenList, p->index + 3);
  doT = NV_Array_getByIndex(tokenList, p->index + 4);
  for (i = 0; i < 4; i++) {
    NV_Array_removeIndex(tokenList, p->index);
  }
  if (!NV_isTermType(&doT, &NODEID_TERM_TYPE_ARRAY)) {
    doT = NV_Array_create();
  } else {
    NV_Array_removeIndex(tokenList, p->index);
  }
  //
  initT = NV_Lang_parseToCodeGraph(&initT, opDict, NV_Lang02_funcList);
  if (NV_NodeID_isEqual(&initT, &NODEID_NULL)) {
    return NODEID_NULL;
  }
  condT = NV_Lang_parseToCodeGraph(&condT, opDict, NV_Lang02_funcList);
  if (NV_NodeID_isEqual(&condT, &NODEID_NULL)) {
    return NODEID_NULL;
  }
  updtT = NV_Lang_parseToCodeGraph(&updtT, opDict, NV_Lang02_funcList);
  if (NV_NodeID_isEqual(&updtT, &NODEID_NULL)) {
    return NODEID_NULL;
  }
  doT = NV_Lang_parseToCodeGraph(&doT, opDict, NV_Lang02_funcList);
  if (NV_NodeID_isEqual(&doT, &NODEID_NULL)) {
    return NODEID_NULL;
  }
  //
  initF = NV_Node_createWithString("init");
  NV_Dict_addUniqueEqKeyByCStr(&initF, "call", &initT);
  condF = NV_Node_createWithString("cond");
  NV_Dict_addUniqueEqKeyByCStr(&condF, "flag", &condT);
  updtF = NV_Node_createWithString("updt");
  NV_Dict_addUniqueEqKeyByCStr(&updtF, "call", &updtT);
  doF = NV_Node_createWithString("do");
  NV_Dict_addUniqueEqKeyByCStr(&doF, "call", &doT);
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

