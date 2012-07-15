#ifndef _COMMON_H_
#define _COMMON_H_

#include "config.h"

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else				
typedef enum {false, true} bool;
#endif /* HAVE_STDBOOL_H */

#define XSTR(str)	#str
#define STR(str)	XSTR(str)

#endif /* _COMMON_H_ */
