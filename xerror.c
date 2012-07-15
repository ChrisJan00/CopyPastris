#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void
xerror_info(const char *file, const char *func, int line)
{	
	if (func)
		fprintf(stderr, "Error in function: \"%s()\" in file: \"%s\" on"
		    " line: %d\n", func, file, line);
	else
		fprintf(stderr, "Error in file: \"%s\" on line: %d\n",
		    file, line);
}

void
xerror(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

#ifdef DEBUG
	abort();
#else
	exit(EXIT_FAILURE);
#endif
}
	    
