#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "os/os.h"

#include "support/support.h"


int  creat(const char *filename, mode_t mode)
{
	return open( filename, O_CREAT|O_WRONLY|O_TRUNC );
}


int  open(const char *filename, int flags, ...)
{
	void *data;
	FILE* file;
	int fd;
	
	if ( os_open( filename, flags, &data, &fd ) == NOTIMPLEMENTED ) 
										os_freakout();
	if ( fd < 0 ) return -1;

	file = (FILE*)malloc( sizeof(FILE) );
	file->data = data;
	file->fd = fd;
	file->eof = 0;
	file->error = 0;
	file->mode = flags;

	if ( support_insert_file( file ) != 0 )
	{
		// Only allow files that we can keep track of.
		close( fd );	// close the stream
		return -1;
	}
	
	return fd;
}



