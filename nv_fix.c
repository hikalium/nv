#include "nv.h"

#undef malloc
#undef free

char *NV_strncpy(char *dst, const char *src, size_t dst_size, size_t copy_size)
{
	int real_size = MIN(dst_size, copy_size + 1);
	char *retv = strncpy(dst, src, real_size);
	dst[real_size - 1] = 0;
	return retv;
}

long NV_strtolSeq(const char **restrict p, int base)
{
	char *q;
	long n;
	n = strtol(*p, &q, base);
	*p = q;
	return n;
}

int mallocCount = 0;
int NV_getMallocCount()
{
	return mallocCount;
}

void *NV_malloc(size_t size)
{
	void *retv = malloc(size);
	if(!retv){
		fputs("NV_malloc: malloc error!\n", stderr);
		exit(EXIT_FAILURE);
	}
	mallocCount++;
	//NV_DbgInfo("%s", "malloc!");
	return retv;
}

void NV_free(void *p)
{
	if(!p){
		fputs("NV_malloc: free error!\n", stderr);
		exit(EXIT_FAILURE);
	}
	free(p);
	mallocCount--;
	//NV_DbgInfo("%s", "free!");
	return;
}
