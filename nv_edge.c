#include "nv.h"
//
// Edge
//

NV_Edge *NV_Edge_getByID(NV_ElementID id)
{
	NV_Edge *e;
	for(e = edgeRoot; e; e = e->next){
		if(NV_ElementID_isEqual(e->id, id)) return e;
	}
	return NULL;
}

NV_ElementID NV_Edge_add(NV_ElementID from, NV_ElementID rel, NV_ElementID to)
{
	NV_Edge *e = NV_malloc(sizeof(NV_Edge));
	if(!e) exit(EXIT_FAILURE);
	//
	e->id = NV_ElementID_generateRandom();
	e->from = from;
	e->rel = rel;
	e->to = to;
	//
	e->next = edgeRoot;
	edgeRoot = e;
	//
	return e->id;
}

void NV_Edge_update
(NV_ElementID eid, NV_ElementID from, NV_ElementID rel, NV_ElementID to)
{
	NV_Edge *e = NV_Edge_getByID(eid);
	if(e){
		e->from = from;
		e->rel = rel;
		e->to = to;
	}
}

NV_ElementID NV_Edge_getConnectedFrom(NV_ElementID from, NV_ElementID rel)
{
	NV_Edge *e;
	for(e = edgeRoot; e; e = e->next){
		if(	NV_ElementID_isEqual(e->from, from) && 
			NV_ElementID_isEqual(e->rel, rel))
			return e->id;
	}
	return NODEID_NULL;
}

void NV_Edge_dump(NV_Edge *e)
{
	if(!e){
		printf("(null)");
		return;
	}
	printf("%08X %08X -- %08X -> %08X", e->id.d[0], e->from.d[0], e->rel.d[0], e->to.d[0]);
}

