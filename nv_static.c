#include "nv.h"

const NV_ID NODEID_NULL
	= {{0x00000000, 0, 0, 0}};
const NV_ID NODEID_REL_MASTERLINK
	= {{0xffffffff, 0, 0, 0}};

const NV_ID NODEID_TREE_TYPE_ARRAY
	= {{0xBA7C82D7, 0, 0, 0}};
const NV_ID NODEID_TREE_TYPE_VARIABLE
	= {{0x67DEB167, 0, 0, 0}};
const NV_ID RELID_ARRAY_NEXT
	= {{0xA71CE915, 0, 0, 0}};
const NV_ID RELID_VARIABLE_DATA
	= {{0x40776C61, 0, 0, 0}};
const NV_ID RELID_POINTER_TARGET
	= {{0xCD64EF96, 0, 0, 0}};
const NV_ID RELID_TREE_TYPE
	= {{0xE804DE81, 0, 0, 0}};

const NV_ID NODEID_NV_STATIC_ROOT
	= {{0xB257ACBF, 0x5D434C81, 0x8D79C638, 0xA2BF94B3}};

const char *NV_NodeTypeList[kNodeTypeCount] = {
	"None",
	"Relation",
	"WeakRelation",
	"String",
	"Integer",
};
