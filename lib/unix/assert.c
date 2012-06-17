#include <stdio.h>
#include <stdlib.h>


/** Put in here for libm. Seems to want it. */
void __assert_fail(const char *assertion,
				   const char *file,
				   unsigned int line,
				   const char *function)
{
	fprintf( stderr, "%s:%i: %s: Assertion %s failed.",
					file,
					line,
					function,
					assertion );
	abort();
}

