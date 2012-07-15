#ifndef _XMALLOC_H_
#define _XMALLOC_H_

#define xmalloc(size)	malloc_wrap(size, __FILE__, __LINE__)

void *malloc_wrap(size_t, const char *, int);
void xfree(void **);

#endif
