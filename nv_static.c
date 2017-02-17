#include "nv.h"

const NV_ID NODEID_NV_STATIC_ROOT
	= {{0xB257ACBF, 0x5D434C81, 0x8D79C638, 0xA2BF94B3}};
const NV_ID NODEID_NULL
	= {{0x00000000, 0, 0, 0}};
const NV_ID NODEID_NOT_FOUND
	= {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};
const NV_ID NODEID_TERM_TYPE_ARRAY
	= {{0xBA7C82D7, 0, 0, 0}};
const NV_ID NODEID_TERM_TYPE_VARIABLE
	= {{0x67DEB167, 0, 0, 0}};
const NV_ID NODEID_TERM_TYPE_OP
	= {{0x83E75537, 0x03CB43A8, 0x9F0B84ED, 0xEA0C635A}};
//
const NV_ID RELID_ARRAY_NEXT
	= {{0xA71CE915, 0, 0, 0}};
const NV_ID RELID_VARIABLE_DATA
	= {{0x40776C61, 0, 0, 0}};
const NV_ID RELID_POINTER_TARGET
	= {{0xCD64EF96, 0, 0, 0}};
const NV_ID RELID_OP_PRECEDENCE
	= {{0x46452917, 0x15084D12, 0xAB03F07E, 0xA0D58BC7}};
const NV_ID RELID_OP_FUNC
	= {{0x803C7EBA, 0xFAAF4DA3, 0x9D8486BB, 0xEF6DC077}};
const NV_ID RELID_TERM_TYPE
	= {{0xE804DE81, 0, 0, 0}};
const NV_ID RELID_EVAL_STACK
	= {{0x62ABDADF, 0xA2E24E00, 0x96702BE8, 0x764D2801}};
const NV_ID RELID_CURRENT_TERM_INDEX
	= {{0x40D94FBD, 0x20C44119, 0x8C0D2F54, 0xF86FF8FC}};
const NV_ID RELID_CURRENT_TERM_PHASE
	= {{0xBE4102F0, 0x36D94B9C, 0xB32D2C77, 0xC0BB0B22}};

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
