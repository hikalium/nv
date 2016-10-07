#include "nv.h"
//
// NV_ElementID
//

NV_ElementID NV_ElementID_generateRandom()
{
	NV_ElementID id;
	id.d[0] = rand();
	id.d[1] = rand();
	id.d[2] = rand();
	id.d[3] = rand();
	return id;
}

int NV_ElementID_isEqual(const NV_ElementID *a, const NV_ElementID *b)
{
	int i;
	for(i = 0; i < 4; i++){
		if(a->d[i] != b->d[i]) return 0;
	}
	return 1;
}

