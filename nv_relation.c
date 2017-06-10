#include "nv.h"
//#include "nv_node.h"

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
NV_ID NV_NodeID_createRelation
(const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	NV_Relation *reld;
	int32_t size = sizeof(NV_Relation);
	//
	reld = NV_malloc(size);
	reld->from = *from;
	reld->rel = *rel;
	reld->to = *to;

	if(IS_DEBUG_MODE()){
		printf("Rel created: ");
		NV_NodeID_printForDebug(from);
		NV_NodeID_printForDebug(rel);
		NV_NodeID_printForDebug(to);
		printf("\n");
	}

	return NV_Node_createWithTypeAndData(kRelation, reld, size);
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
	// relがNV_Node_Internal_isEqualInValue()において等価であるような
	// relがすでに存在するならば、それのtoをupdateする。
	// 存在しないならば、新規作成する。
	return NV_NodeID_createRel_OnDupUpdate(
		from, rel, to, NV_NodeID_getEqRelationFrom);
}
/*
//void NV_Node_setRelation
//(const NV_ID *relnid, const NV_ID *from, const NV_ID *rel, const NV_ID *to)
{
	// retains to.
	// Relationはtoを保持し、
	// fromはRelationを保持する。
	NV_Node *n;
	NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	if(n){
		if(n->type != kNone){
			printf("Try to modify rel existed. abort.\n");
			exit(EXIT_FAILURE);
		}
		n->type = kRelation;
		n->size = sizeof(NV_Relation);
		n->data = NV_malloc(n->size);
		//:
		reld = n->data;
		reld->from = *from;
		reld->rel = *rel;
		reld->to = *to;
		//
	}
}
*/
NV_Node *NV_NodeID_Relation_getLinkFrom(const NV_ID *relnid)
{
	const NV_Relation *reld;
	//
	reld = NV_NodeID_getDataAsType(relnid, kRelation);
	if(!reld) return NULL;
	return NV_NodeID_getNode(&reld->from);
}

NV_ID NV_NodeID_Relation_getIDLinkTo(const NV_ID *relnid)
{
	const NV_Relation *reld;
	//
	reld = NV_NodeID_getDataAsType(relnid, kRelation);
	if(!reld) return NODEID_NOT_FOUND;
	return reld->to;
}

NV_Node *NV_NodeID_Relation_getLinkTo(const NV_ID *relnid)
{
	NV_ID id;
	id = NV_NodeID_Relation_getIDLinkTo(relnid);
	return NV_NodeID_getNode(&id);
}

NV_ID NV_NodeID_Relation_getIDLinkRel(const NV_ID *relnid)
{
	const NV_Relation *reld;
	//
	reld = NV_NodeID_getDataAsType(relnid, kRelation);
	return reld->rel;
}

NV_Node *NV_NodeID_Relation_getLinkRel(const NV_ID *relnid)
{
	NV_ID id;
	id = NV_NodeID_Relation_getIDLinkRel(relnid);
	return NV_NodeID_getNode(&id);
}

NV_ID NV_NodeID_updateRelationTo(const NV_ID *relnid, const NV_ID *to)
{
	// Updateとは言っているが、内容はImmutableにしたいので、
	// 既存のものを削除して、新しいノードを生成し、以前と同一の関係性を持たせる。
	NV_ID from, rel;
	NV_Node *n;
	const NV_Relation *reld;
	//
	n = NV_NodeID_getNode(relnid);
	reld = NV_Node_getDataAsType(n, kRelation);
	if(!reld || !to) return NODEID_NOT_FOUND;
	// 以前の関係性を取り出す
	from = reld->from;
	rel = reld->rel;
	// 古い関係を除去
	NV_NodeID_remove(relnid);
	// 新しい関係を作成して返す
	return NV_NodeID_createRelation(&from, &rel, to);
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
		reld = NV_Node_getDataAsType(n, kRelation);
		if(reld && NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
			// hit!
			return n;
		}
	}
	// check relCache
	n = NV_NodeID_getNode(from);
	if(n){
		n = NV_Node_getRelCache(n);
		reld = NV_Node_getDataAsType(n, kRelation);
		if(reld && NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
			return n;
		}
	}
	//
	for(n = NV_Node_getNextNode(&nodeRoot); n; n = NV_Node_getNextNode(n)){
		reld = NV_Node_getDataAsType(n, kRelation);
		if(!reld) continue;
		if(NV_NodeID_isEqual(&reld->from, from) && cmp(&reld->rel, rel)){
			//
			relCache[HASH_REL(from)][HASH_REL(rel)] = n;
			NV_Node *fn = NV_NodeID_getNode(from);
			if(fn){
				NV_Node_setRelCache(fn, n);
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
	if(n) return NV_Node_getID(n);
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqual);
	if(n){
		reld = NV_Node_getDataAsType(n, kRelation);
		return reld->to;
	}
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getEqRelationFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqualInValue);
	if(n) return NV_Node_getID(n);
	return NODEID_NOT_FOUND;
}

NV_ID NV_NodeID_getEqRelatedNodeFrom(const NV_ID *from, const NV_ID *rel)
{
	const NV_Node *n;
	const NV_Relation *reld;
	/*
	printf("\nNV_NodeID_getEqRelatedNodeFrom: find from #%08X, rel #%08X\n",
			from->d[0], rel->d[0]);
			*/
	//
	n = NV_NodeID_getRelNodeFromWithCmp(from, rel, NV_NodeID_isEqualInValue);
	if(n){
		reld = NV_Node_getDataAsType(n, kRelation);
		return reld->to;
	}	
	return NODEID_NOT_FOUND;
}

