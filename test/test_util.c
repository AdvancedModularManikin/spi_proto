#include <stdlib.h>
#include <stdio.h>

void
print_bytes(void *buf, size_t n, size_t wrap)
{
	//TODO if ended on a newline don't print another
	for (unsigned int i = 0; i < n;i++) {
		printf("%02x ", ((unsigned char *) buf)[i]);
		if (i % wrap == (wrap-1) && i > 0) puts("");
	}
	printf("\n");
}
