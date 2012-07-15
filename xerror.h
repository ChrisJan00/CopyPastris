#ifndef _XERROR_H_
#define _XERROR_H_

void xerror(const char *, ...);
void xerror_info(const char *, const char *, int);

#if __STDC_VERSION__ >= 199901L
#define ERROR xerror_info(__FILE__, __func__, __LINE__),xerror
#else
#define ERROR xerror_info(__FILE__, 0, __LINE__),xerror
#endif

#endif
