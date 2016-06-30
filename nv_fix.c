#include "nv.h"

char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size)
{
	int real_size = MIN(dst_size, copy_size + 1);
	char *retv = strncpy(dst, src, real_size);
	dst[real_size - 1] = 0;
	return retv;
}

void *NV_malloc(size_t size)
{
	void *retv = malloc(size);
	if(!retv){
		fputs("NV_malloc: malloc error!\n", stderr);
		exit(EXIT_FAILURE);
	}
	return retv;
}
