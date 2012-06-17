#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "os/os.h"

#include "support/support.h"



int          close(int fd)
{
	// Exist?
	int rc;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	// Can close?
	if ( os_close( file->data, &rc ) == NOTIMPLEMENTED )
		   os_freakout();

	if ( rc != 0 ) return -1;
	support_remove_file( fd );
	free( file );
	return 0;
}



off_t        lseek(int fd, off_t position, int whence)
{
	int ans;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	if ( os_seek( file->data, position, whence, &ans ) == NOTIMPLEMENTED )
		   	os_freakout();

	return ans;	
}


ssize_t      read(int fd, void *buffer, size_t num)
{
	ssize_t rc;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;
	if ( os_read( file->data, buffer, num, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}

int          rmdir(const char *directory)
{
	int rc;
	if ( os_rmdir( directory, &rc ) == NOTIMPLEMENTED )
				os_freakout();
	return rc;
}


unsigned int sleep(unsigned int seconds)
{
	unsigned int rc;

	if ( os_sleep( seconds, &rc ) == NOTIMPLEMENTED )
			os_freakout();
		
	return rc;
}



ssize_t      write(int fd, const void * buffer, size_t num)
{
	ssize_t rc;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	if ( os_write( file->data, buffer, num, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}

