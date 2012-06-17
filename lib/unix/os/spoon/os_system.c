#include <kernel.h> 
#include <stdio.h>
#include <setjmp.h>

#include "../os.h"


int		os_system( const char *s, int *rc )
{
	return NOTIMPLEMENTED;
}


int 	os_exit( int status, int *rc)
{
	smk_exit( status );
	*rc = -1;
	return 0; 
}


int		os_signal( int sig, int *rc )
{
	return NOTIMPLEMENTED;
}


int 	os_getenv(const char* name, char **env )
{
	return NOTIMPLEMENTED;
}

