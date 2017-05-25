#include "nv.h"

const NV_ID NODEID_NV_STATIC_ROOT
	= {{0xB257ACBF, 0x5D434C81, 0x8D79C638, 0xA2BF94B3}};
const NV_ID NODEID_NULL
	= {{0x00000000, 0, 0, 0}};
const NV_ID NODEID_NOT_FOUND
	= {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};
const NV_ID NODEID_TERM_TYPE_ARRAY
	= {{0x22E57D41, 0x92C74F8A, 0xA1724F15, 0x8AA67792}};
const NV_ID NODEID_TERM_TYPE_VARIABLE
	= {{0xACF77B34, 0x8D844B54, 0xBF55E4A5, 0x037A6523}};
const NV_ID NODEID_TERM_TYPE_OP
	= {{0x83E75537, 0x03CB43A8, 0x9F0B84ED, 0xEA0C635A}};
const NV_ID NODEID_TERM_TYPE_PATH
	= {{0x9F32D54F, 0xE09B4862, 0x82CC1ABF, 0xD75A12A9}};	
//
const NV_ID RELID_ARRAY_NEXT
	= {{0x73EEE19B, 0x5086494E, 0xAACE7CD4, 0x80D9FA78}};
const NV_ID RELID_ARRAY_COUNT
	= {{0x50CBE2C3, 0x27A74DCE, 0xAAFFA276, 0xA4EA9D87}};
const NV_ID RELID_ARRAY_DATA
	= {{0x1EF14960, 0x0131475D, 0xBE36707A, 0x52876DE3}};
const NV_ID RELID_VARIABLE_NAME
	= {{0x6260CAF9, 0x3671493E, 0xB33FEE3D, 0x8AE44718}};
const NV_ID RELID_VARIABLE_SCOPE
	= {{0xEDB792C8, 0x94F04565, 0x8D43FDB1, 0xB086C655}};
const NV_ID RELID_VARIABLE_ANONYMOUS
	= {{0xED5B6B24, 0x1F924A57, 0xBB756E64, 0xDC92684E}};
const NV_ID RELID_OP_PRECEDENCE
	= {{0x46452917, 0x15084D12, 0xAB03F07E, 0xA0D58BC7}};
const NV_ID RELID_OP_FUNC
	= {{0x803C7EBA, 0xFAAF4DA3, 0x9D8486BB, 0xEF6DC077}};
const NV_ID RELID_TERM_TYPE
	= {{0xAC1CFA32, 0x80C14C4B, 0xBD2F38A4, 0xD148B9D5}};
const NV_ID RELID_EVAL_STACK
	= {{0x62ABDADF, 0xA2E24E00, 0x96702BE8, 0x764D2801}};
const NV_ID RELID_LAST_RESULT
	= {{0x97555D43, 0xB7F54EE1, 0xB386C0D0, 0x6268AF98}};
const NV_ID RELID_CURRENT_TERM_INDEX
	= {{0x40D94FBD, 0x20C44119, 0x8C0D2F54, 0xF86FF8FC}};
const NV_ID RELID_CURRENT_TERM_PHASE
	= {{0xBE4102F0, 0x36D94B9C, 0xB32D2C77, 0xC0BB0B22}};
const NV_ID RELID_CURRENT_SCOPE
	= {{0x7B1FD5B1, 0x54354214, 0x8B3CE220, 0x42599125}};
const NV_ID RELID_PARENT_SCOPE
	= {{0x9302D59B, 0xF92A48A4, 0x8B839165, 0xBEE5A1F4}};
const NV_ID RELID_CONTEXT_LIST
	= {{0xC3CC02EE, 0x585B428A, 0xA965B56B, 0x57D98193}};	
const NV_ID RELID_NEXT_CONTEXT
	= {{0x694D5C33, 0x1D0849C0, 0x84B49D4F, 0xF10770E4}};	
const NV_ID RELID_OP_DICT
	= {{0xC0F19B8B, 0x6FFF4613, 0xA238A172, 0x767CF0B6}};	

const char *NV_NodeTypeList[kNodeTypeCount] = {
	"None",
	"Relation",
	"BLOB",
	"String",
	"Integer",
};

const char c2hexTable[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//
// Internal
//
typedef struct NV_STATIC_NODE_NAME_TAG NV_StaticNodeNameTag;
struct NV_STATIC_NODE_NAME_TAG {
	const NV_ID *id;
	const char *s;
};


NV_StaticNodeNameTag staticNodeNameList[] = {
	{&NODEID_NULL, "NullElement"},
	//
	{&NODEID_TERM_TYPE_ARRAY, "Type(Array)"},
	{&NODEID_TERM_TYPE_VARIABLE, "Type(Variable)"},
	{&NODEID_TERM_TYPE_OP, "Type(Op)"},
	//
	{&RELID_TERM_TYPE, "type"},
	{&RELID_ARRAY_NEXT, "next"},
	{&RELID_ARRAY_DATA, "data"},
	//{&RELID_VARIABLE_NAME, "name"},
	{&RELID_OP_PRECEDENCE, "precedence"},
	{&RELID_OP_FUNC, "func"},
	{&RELID_EVAL_STACK, "evalStack"},
	{&RELID_LAST_RESULT, "lastResult"},
	{&RELID_CURRENT_TERM_INDEX, "currentTermIndex"},
	{&RELID_CURRENT_TERM_PHASE, "currentTermPhase"},
	{&RELID_CURRENT_SCOPE, "currentScope"},
	{&RELID_PARENT_SCOPE, "parentScope"},
	{&RELID_CONTEXT_LIST, "contextList"},
	{&RELID_VARIABLE_ANONYMOUS, "anonymous"},
	//
	{NULL, NULL}
};

void NV_Graph_addStaticNode(const NV_ID *id, const char *s)
{
	int f = NV_NodeID_exists(id);
	//
	NV_NodeID_createAndString(id, s);
	if(!f){
		NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
	}
}

void NV_Graph_initStaticNodes()
{
	NV_NodeID_createAndString(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_NodeID_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_StaticNodeNameTag *t;
	int i;
	for(i = 0; ; i++){
		t = &staticNodeNameList[i];
		if(!t->s) break;
		NV_Graph_addStaticNode(t->id, t->s); 
	}
}

//
// Public
//

void NV_insertInitialNode()
{
	NV_Graph_initStaticNodes();
	// contextListが存在しなければ作成する
	NV_ID contextList;
	contextList = NV_getContextList();
	if(NV_NodeID_isEqual(&contextList, &NODEID_NOT_FOUND)){
		contextList = NV_Array_create();
		NV_NodeID_createRelation(&NODEID_NV_STATIC_ROOT, 
			&RELID_CONTEXT_LIST, &contextList);
	}
}


