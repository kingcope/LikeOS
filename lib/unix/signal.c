#include <signal.h>

#include "os/os.h"


int raise(int sig)
{
	int rc;

	if ( os_signal( sig, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}


