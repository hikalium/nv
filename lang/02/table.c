#include "../../nv.h"

const char *NV_Lang02_charTypeList[NV_LANG_CHAR_TYPE_LIST_LEN] = {
    " \t\r\n",
    "#!%&-=^~|+*:.<>/$",
    "(){}[],;\"`\\",
};

NV_Lang_OpTag NV_Lang02_opList[] = {
    {";", 0, "nothing"},
    //
    {"print", 10, "prefix"},
    {"printel", 10, "prefix"},
    {"println", 10, "prefix"},
    {"ls", 10, "prefix"},
    {"lsdep", 10, "prefix"},
    {"dump", 10, "prefix"},
    {"clean", 10, "prefix"},
    /*
    {"}",		10,		"NV_Op_codeBlockClose"},
    {"ls2",		10,		"NV_Op_ls2"},
    {"lsctx",	10,		"NV_Op_lsctx"},
    {"swctx",	10,		"NV_Op_swctx"},
    {"last",	10,		"NV_Op_last"},
    {"save",	10,		"NV_Op_save"},
    {"restore",	10,		"NV_Op_restore"},
    {"out",		10,		"NV_Op_out"},
    {"fmt",		10,		"NV_Op_fmt"},
    {"info",	10,		"NV_Op_info"},
    {"clean",	10,		"NV_Op_clean"},
    {"push",	10,		"NV_Op_push"},
    */
    {",", 40, "infix"},
    {":", 50, "infix"},
    //
    {"=", 101, "infix"},
    //
    {"<", 500, "infix"},
    {">=", 500, "infix"},
    {"<=", 500, "infix"},
    {">", 500, "infix"},
    {"==", 500, "infix"},
    {"!=", 500, "infix"},
    //
    {"+", 1000, "infix"},
    {"-", 1000, "infix"},
    {"*", 2000, "infix"},
    {"/", 2000, "infix"},
    {"%", 2000, "infix"},
    //
    {"+", 5001, "prefix"},
    {"-", 5001, "prefix"},
    //
    {"++", 6000, "postfix"},
    {"--", 6000, "postfix"},
    //
    {"if", 10000, "if"},
    {"for", 10000, "for"},
    //
    //{"#",       14000,  "NV_Op_unbox"},
    //
    {"(", 15000, "parentheses"},
    //{"[",		15000,	"NV_Op_arrayAccessor"},
    {".", 15000, "infix"},
    //
    {" ", 20000, "nothing"},
    {"\t", 20000, "nothing"},
    {"\n", 20000, "nothing"},
    //
    {"{", 30000, "codeblock"},
    {"\"", 40000, "strliteral"},
    //
    //{"\"",		100000,	"NV_Op_strLiteral"},
    //
    {"", -1, ""}  // terminate tag
};

NV_Lang_ParseTag NV_Lang02_funcList[] = {
    {"nothing", NV_parseToCodeGraph_nothing},
    {"infix", NV_parseToCodeGraph_infixOp},
    {"prefix", NV_parseToCodeGraph_prefixOp},
    {"postfix", NV_parseToCodeGraph_postfixOp},
    {"codeblock", NV_parseToCodeGraph_codeblock},
    {"strliteral", NV_parseToCodeGraph_strLiteral},
    {"parentheses", NV_parseToCodeGraph_parentheses},
    {"if", NV_parseToCodeGraph_if},
    {"for", NV_parseToCodeGraph_for},
    {NULL, NULL}  // terminate tag
};

NV_Lang_EvalTag NV_Lang02_evalList[] = {
    {"infixOp", NV_Lang02_OpFunc_infixOp},
    {"prefixOp", NV_Lang02_OpFunc_prefixOp},
    {"postfixOp", NV_Lang02_OpFunc_postfixOp},
    {"cond", NV_Lang02_OpFunc_cond},
    {"do", NV_Lang02_OpFunc_do},
    {"updt", NV_Lang02_OpFunc_do},
    {"init", NV_Lang02_OpFunc_do},
    {"endfor", NV_Lang02_OpFunc_do},
    {"endif", NV_Lang02_OpFunc_doNothing},
    {"()", NV_Lang02_OpFunc_parentheses},
    {NULL, NULL}};

