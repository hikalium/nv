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
/*
//int idnum = 1;
//NV_ID NV_ID_generateRandom()
{
	NV_ID id;
	id.d[0] = idnum++;
	id.d[1] = 0;
	id.d[2] = 0;
	id.d[3] = 0;
	return id;
}
*/


int NV_ID_setFromString(NV_ID *id, const char *s)
{
	int i, k, c;
	uint32_t buf[4];
	if(!s || !id) return 1; 
	for(i = 0; i < 4; i++){
		buf[i] = 0;
		for(k = 0; k < 8; k++){
			buf[i] <<= 4;
			c = toupper(s[i * 8 + k]);
			if('0' <= c && c <= '9'){
				c -= '0';
			} else if('A' <= c && c <= 'F'){
				c -= 'A' - 0x0A;
			} else{
				return 1;
			}
			buf[i] |= c;
		}
	}
	for(i = 0; i < 4; i++){
		id->d[i] = buf[i];
	}
	return 0;
}

void NV_ID_dumpIDToFile(const NV_ID *id, FILE *fp)
{
	int i;
	for(i = 0; i < 4; i++){
		fprintf(fp, "%08X", id->d[i]);
	}
}

