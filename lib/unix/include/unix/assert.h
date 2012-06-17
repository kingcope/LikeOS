#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>

#ifdef NDEBUG

#define assert(ignore)((void) 0)

#else

#define assert(check) \
	if ((check)==0) 								\
	{												\
		fprintf( stderr, "%s: %s : %s,%s(%i)\n",	\
						 "assert failed: ",			\
					 	 "check",					\
						 __FUNCTION__,				\
						 __FILE__, __LINE__ );  	\
													\
		abort();									\
	}				


#endif

#ifdef __cplusplus
extern "C" {
#endif

void __assert_fail(const char *assertion,
				   const char *file,
				   unsigned int line,
				   const char *function);

#ifdef __cplusplus
}
#endif
		
		
#endif

