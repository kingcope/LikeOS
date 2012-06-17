#include <sched.h>
#include <errno.h>
#include "os/os.h"


int    sched_yield(void)
{
	int rc;
		
	if ( os_sched_yield(&rc) == NOTIMPLEMENTED )
			os_freakout();

	if ( rc != 0 )
	{
		errno = rc;
		return -1;
	}

	return 0;
}


