#include "nv.h"
#include "nv_node.h"

//
// Internal
//

NV_ID NV_NodeID_createRel_OnDupUpdate
(const NV_ID *from, const NV_ID *rel, const NV_ID *to, 
NV_ID (*find)(const NV_ID *from, const NV_ID *rel))
{
	NV_ID r;
	r = find(from, rel);
	if(NV_NodeID_isEqual(&r, &NODEID_NOT_FOUND)){
		// 新規
		return NV_NodeID_createRelation(from, rel, to);
	}
	// 既存
	NV_NodeID_updateRelationTo(&r, to);
	return r;
} 

//
// Relation
//
/*
int NV_Node_isLiveRelation(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(n){
		reld = n->data;
	}
	
}
*/
NV_ID NV_NodeID_createRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	NV_ID r = NV_Node_create();
	NV_Node_setRelation(&r, from, rel, to);
	return r;
}

NV_ID NV_NodeID_createUniqueIDRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	// fromが同一のIDで、
	// re1.id === rel2.idとなるような
	// relがすでに存在するならば、それのtoをupdateする。
	// 存在しないならば、新規作成する。
	return NV_NodeID_createRel_OnDupUpdate(
		from, rel, to, NV_NodeID_getRelationFrom);
}

NV_ID NV_NodeID_createUniqueEqRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	// fromが同一のIDで、
	// relがNV_Node_isEqualInValue()において等価であるような
	// relがすでに存在するならば、それのtoをupdateする。
	// 存在しないならば、新規作成する。
	return NV_NodeID_createRel_OnDupUpdate(
		from, rel, to, NV_NodeID_getEqRelationFrom);
}

void NV_Node_setRelation
(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	// retains to.
	// Relationはtoを保持し、
	// fromはRelationを保持する。
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(n){
		if(n->type != kNone) NV_Node_Internal_resetData(n);
		n->type = kRelation;
		n->size = sizeof(NV_Relation);
		n->data = NV_malloc(n->size);
		//:
		reld = n->data;
		reld->from = *from;
		reld->rel = *rel;
		reld->to = *to;
		//
		NV_NodeID_retain(to);
		//
		n->refCount++;
	}
}

NV_Node *NV_NodeID_Relation_getLinkFrom(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(!n || n->type != kRelation) return NULL;
	reld = n->data;
	return NV_NodeID_getNode(&reld->from);
}

NV_ID NV_NodeID_Relation_getIDLinkTo(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(!n || n->type != kRelation) return NODEID_NOT_FOUND;
	reld = n->data;
	return reld->to;
}

NV_Node *NV_NodeID_Relation_getLinkTo(const NV_ID *relnid)
{
	NV_ID id;
	id = NV_NodeID_Relation_getIDLinkTo(relnid);
	return NV_NodeID_getNode(&id);
}

NV_Node *NV_NodeID_Relation_getLinkRel(const NV_ID *relnid)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(!n || n->type != kRelation) return NULL;
	reld = n->data;
	return NV_NodeID_getNode(&reld->rel);
}

void NV_NodeID_updateRelationTo(const NV_ID *relnid, const NV_ID *to)
{
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(n){
		if(n->type != kRelation) return;
		reld = n->data;
		//
		NV_NodeID_release(&reld->to);
		//
		reld->to = *to;
		NV_NodeID_retain(&reld->to);
	}
}

#define REL_CACHE_MASK 0xFF
#define HASH_REL(id)	(id->d[0] & REL_CACHE_MASK)

const NV_Node *relCache[REL_CACHE_MASK + 1][REL_CACHE_MASK + 1]; // [from][rel] -> to

const NV_Node *NV_NodeID_getRelNodeFromWithCmp
(const NV_ID *from, const NV_ID *rel, int (*cmp)(const NV_ID *p, const NV_ID *q))
{
	const NV_Node *n;
	const NV_Relation *reld;
	if(!from || !rel || !cmp) return NULL;
	// check cache
	n = relCache[HASH_REL(from)][HASH_REL(rel)];
	if(n){
		reld = n->data;
		if(n->type == kRelation && reld &&
			NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
			// hit!
			return n;
		}
	}
	// check relCache
	n = NV_NodeID_getNode(from);
	if(n){
		n = n->relCache;
		if(n && n->type == kRelation){
			reld = n->data;
			if(NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
				return n;
			}
		}
	}
	//
	for(n = nodeRoot.next; n; n = n->next){
		if(n->type != kRelation) continue;
		reld = n->data;
		if(NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
			//
			relCache[HASH_REL(from)][HASH_REL(rel)] = n;
			NV_Node *fn = NV_NodeID_getNode(from);
			if(fn){
				fn->relCache = n;
			}
			//
			return n;
		}
	}
	return NULL;
}

NV_ID NV_NodeID_getRelationFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqual);
	if(n) return n->id;
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqual);
	if(n){
		reld = n->data;
		return reld->to;
	}
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getEqRelationFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqualInValue);
	if(n) return n->id;
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getEqRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqualInValue);
	if(n){
		reld = n->data;
		return reld->to;
	}	
	return NODEID_NOT_FOUND;
}

