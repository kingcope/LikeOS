#include <kernel.h>
#include <stdio.h>
#include <fcntl.h>
#include <spoon/storage/File.h>
#include <spoon/storage/Directory.h>

#include "../os.h"

/* C++ file to add support for OS file operations.  */




// File operations








extern "C" int     os_open( const char *filename, unsigned int mode, void **data, int *rc )
{
	
	bool bCreate = ((mode & O_CREAT) != 0 );
	bool bTrunc  = ((mode & O_TRUNC) != 0 );
	bool exists  = File::Exists( filename );

	
	if ( (bCreate == false) && (exists == false) )
	{
		printf("%s\n","not create and not exist");
		*data = NULL;
		return -1;
	}
			
	if ( bTrunc == true )
	{
		// Doesn't matter what was there...
		if ( exists ) File::Delete( (char*)filename );
		bCreate = true;
		exists = false;
	}

	if ( (bCreate == true) && (exists == false) )
	{
		if ( File::Create( (char*)filename ) != 0 )
		{
			printf("%s\n","create failed");
			*data = NULL;
			return -1;
		}
	}
	
	
	// Now open.. should be there.

	File *file = new File( (char*)filename );

	if ( file->Open() < 0 )
	{
		delete file;
		*data = NULL;
		return -1;
	}
	

	*data = file;
	*rc = file->fd();
	return 0;
}



extern "C" int     os_read( void *data, void *buffer, int len, int *rc )
{
	File *file = (File*)data;
	*rc = file->Read( buffer, len );
	return 0;
}

extern "C" int     os_write( void *data, const void *buffer, int len )
{
	File *file = (File*)data;
	*rc = file->Write( (void*)buffer, len );
	return 0;
}

extern "C" int     os_seek( void *data, long offset, int origin, int *rc )
{
	File *file = (File*)data;

	switch (origin)
	{
		case SEEK_SET:
			if ( file->Seek( offset ) == offset ) 
					*rc = 0;
					return 0;
		case SEEK_CUR:
		case SEEK_END:
			break;
	}
	
	return NOTIMPLEMENTED;
}

extern "C" int    os_tell( void *data, long *rc )
{
	return NOTIMPLEMENTED;
}

extern "C" int     os_close( void *data, int *rc )
{
	File *file = (File*)data;
		  file->Close();
	delete file;
	*rc = 0;
	return 0;
}




extern "C" int		os_stat( void *data, struct stat *st, int *rc )
{
	File *file = (File*)data;
	*rc = file->Stat( st );
	return 0;
}



extern "C" int		os_delete( const char *filename, int *rc )
{
	*rc = ( File::Delete( (char*)filename ) );
	return 0;
}


extern "C" int		os_mkdir( const char *filename, unsigned int mode, int *rc )
{
	*rc = Directory::Create( filename );
	return 0;
}

extern "C" int		os_rmdir( const char *filename, int *rc )
{
	*rc = Directory::Delete( filename );
	return 0;
}




