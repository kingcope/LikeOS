#include <stdio.h>
#include <sys/stat.h>

#include "../support/support.h"

#include "../os/os.h"


int    mkdir(const char *dirname, mode_t mode)
{
	int rc;
		
	if ( os_mkdir( dirname, mode, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}


int    fstat(int fd, struct stat *st)
{
	int rc;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	if ( os_stat( file->data, st, &rc ) == NOTIMPLEMENTED )
		   os_freakout();

	return rc;	
}

