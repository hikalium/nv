#include "nv.h"
//
// Variable
//
/*
変数は名前とスコープを持つ。
スコープがNODEID_NOT_FOUNDの場合は、無効な変数だ。
スコープに代入するとは、そのスコープから名前をrelとするようなrelationを張って、
その指す先に代入したノードが存在するようにすることである。
*/

typedef struct {
	NV_ID scope, name;
	int isAssignable;
} NV_Variable_Info;

NV_Variable_Info NV_Variable_Internal_getInfo(const NV_ID *v)
{
	NV_Variable_Info info;
	info.isAssignable = 0;
	//
	info.scope = NV_NodeID_getRelatedNodeFrom(v, &RELID_VARIABLE_SCOPE);
	info.name = NV_NodeID_getRelatedNodeFrom(v, &RELID_VARIABLE_NAME);
	//
	if(NV_NodeID_isEqual(&info.name, &NODEID_NOT_FOUND)){
		if(IS_DEBUG_MODE()){
			printf("Attempt to access invalid name.\n");
		}
		return info;
	}
	if(NV_NodeID_isEqual(&info.scope, &NODEID_NOT_FOUND)){
		if(IS_DEBUG_MODE()){
			printf("Attempt to access invalid scope. name = %s\n",
					NV_NodeID_getCStr(&info.name));
		}
		return info;
	}
/*	
	printf("Valid variable. %s@%s\n",
			NV_NodeID_getCStr(&info.name), NV_NodeID_getCStr(&info.scope));
*/			
	info.isAssignable = 1;
	return info;
}

int NV_Variable_isVariable(const NV_ID *id)
{
	return NV_isTermType(id, &NODEID_TERM_TYPE_VARIABLE);
}

NV_ID NV_Variable_create()
{
	NV_ID v;
	//
	v = NV_Node_createWithString("Var");
	//
	NV_NodeID_createRelation(&v, &RELID_TERM_TYPE, &NODEID_TERM_TYPE_VARIABLE);
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_SCOPE, &v);
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_NAME, &RELID_VARIABLE_ANONYMOUS);
	return v;
}

NV_ID NV_Variable_createSubScopeOf(const NV_ID *parentNode)
{
	NV_ID subScope = 
		NV_Node_createWithStringFormat("%s/scope", NV_NodeID_getCStr(parentNode));
	NV_ID v = NV_Variable_createWithName(&subScope, &RELID_PARENT_SCOPE);
	NV_Variable_assign(&v, parentNode);
	return subScope;
}

NV_ID NV_Variable_createWithName(const NV_ID *parentNode, const NV_ID *nameNode)
{
	if(!parentNode || !nameNode) return NODEID_NULL;
	NV_ID v = NV_Variable_create();
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_SCOPE, parentNode);
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_NAME, nameNode);
	/*
	printf("var created. %s@%08X(%s)\n" ,
			NV_NodeID_getCStr(nameNode), parentNode->d[0],
			NV_NodeID_getCStr(parentNode));
			*/
	return v;
}

NV_ID NV_Variable_createWithNameCStr(const NV_ID *parentNode, const char *name)
{
	if(!parentNode || !name) return NODEID_NULL;
	NV_ID nameNode = NV_Node_createWithString(name);
	NV_ID v = NV_Variable_create();
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_SCOPE, parentNode);
	NV_NodeID_createUniqueIDRelation(&v, &RELID_VARIABLE_NAME, &nameNode);
	return v;
}

void NV_Variable_assign(const NV_ID *v, const NV_ID *data)
{
	NV_Variable_Info info = NV_Variable_Internal_getInfo(v);
	if(info.isAssignable){
		NV_NodeID_createUniqueEqRelation(&info.scope, &info.name, data);
		/*
		NV_Dict_print(&info.scope);
		printf("Assigned: #%08X(name: %08X) = #%08X\n",
				v->d[0], info.name.d[0], data->d[0]);
				*/
	}
}

int NV_Variable_statByName(const NV_ID *parentNode, const NV_ID *nameNode)
{
	NV_ID data;
	data = NV_Dict_get(parentNode, nameNode);
	if(NV_NodeID_isEqual(&data, &NODEID_NOT_FOUND)){
		/*
		printf("stat: not found %s@%08X(%s)\n", 
				NV_NodeID_getCStr(nameNode), parentNode->d[0],
				NV_NodeID_getCStr(parentNode));
				*/
		return 0;
	}
	return 1;
}

NV_ID NV_Variable_getData(const NV_ID *v)
{
	NV_Variable_Info info = NV_Variable_Internal_getInfo(v);
	/*
	printf("Find: #%08X(name: %08X)\n",
			v->d[0], info.name.d[0]);
			*/
	NV_ID data = NV_NodeID_getEqRelatedNodeFrom(&info.scope, &info.name);
	return data;
}

NV_ID NV_Variable_findByNameCStr(const char *name, const NV_ID *ctx)
{
	NV_ID nameNode = NV_Node_createWithString(name);
	return NV_Variable_findByName(&nameNode, ctx);
}

NV_ID NV_Variable_findByName(const NV_ID *nameNode, const NV_ID *ctx)
{
	// TODO: Impl recursive find
	NV_ID data = NV_NodeID_getEqRelatedNodeFrom(ctx, nameNode);
	return data;
}

/*
//NV_ID NV_Variable_getData(const NV_ID *vid)
{
	return NV_NodeID_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
}
*/
void NV_Variable_print(const NV_ID *v)
{
	NV_ID data;
	if(!NV_isTermType(v, &NODEID_TERM_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", v->d[0]);
		return;
	}
	NV_Variable_Info info = NV_Variable_Internal_getInfo(v);
	data = NV_Variable_getData(v);
	printf("(Var ");
	NV_Term_print(&info.name);
	printf("@%08X = ", info.scope.d[0]);
	NV_Term_print(&data);
	printf(")");
}

void NV_Variable_snprintf(char *s, size_t size, const NV_ID *v)
{
	NV_ID data;
	if(!NV_isTermType(v, &NODEID_TERM_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", v->d[0]);
		return;
	}
	data = NV_Variable_getData(v);
	if(NV_NodeID_isInteger(&data)){
		snprintf(s, size, "(Var = %d)", NV_NodeID_getInt32(&data));
	} else{
		snprintf(s, size, "(Var = )");
	}
}
/*
//void NV_Variable_printPrimVal(const NV_ID *vid)
{
	NV_ID targetID;
	if(!NV_isTermType(vid, &NODEID_TERM_TYPE_VARIABLE)){
		printf("id: %08X is not Variable.", vid->d[0]);
		return;
	}
	targetID = NV_NodeID_getRelatedNodeFrom(vid, &RELID_VARIABLE_DATA);
	NV_Term_print(&targetID);
}
*/
