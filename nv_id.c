#include "nv.h"
//
// NV_ID
//

NV_ID NV_ID_generateRandom()
{
	NV_ID id;
	id.d[0] = rand();
	id.d[1] = rand();
	id.d[2] = rand();
	id.d[3] = rand();
	return id;
}

int NV_ID_isEqual(const NV_ID *a, const NV_ID *b)
{
	int i;
	for(i = 0; i < 4; i++){
		if(a->d[i] != b->d[i]) return 0;
	}
	return 1;
}

int NV_ID_isEqualInValue(const NV_ID *a, const NV_ID *b)
{
	NV_Node *na, *nb;
	na = NV_Node_getByID(a);
	nb = NV_Node_getByID(b);
	return NV_Node_isEqualInValue(na, nb);
}

void NV_ID_printPrimVal(const NV_ID *id)
{
	NV_Node *n;
	n = NV_Node_getByID(id);
	NV_Node_printPrimVal(n);
}

