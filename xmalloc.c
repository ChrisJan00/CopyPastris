#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"

void *
malloc_wrap(size_t size, const char *fname, int line)
{
	void *tmp;

	tmp = malloc(size);
	/* braklo pamieci */
	if (tmp == NULL) {
		fprintf(stderr, "xmalloc: %s\n%s: %d\nExiting.\n",
		    strerror(errno), fname, line);

		exit(EXIT_FAILURE);
	}

	return tmp;
}

void
xfree(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}
	

