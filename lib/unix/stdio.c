#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "support/support.h"

#include "os/os.h"



FILE *stdin = { 0 };
FILE *stdout = { 0 };
FILE *stderr = { 0 };



FILE* 	fopen(const char* filename, const char* mode)
{
	int fd;
	unsigned int flags = support_fmodes2flags(mode);
	
	fd = open( filename, flags );
	if ( fd < 0 ) return NULL;

	return support_retrieve_file( fd );
}

 
FILE* 	freopen(const char* filename, const char* mode, FILE* stream)
{
	int fd;
	int rc;
	void *data = stream->data;
	unsigned int flags = support_fmodes2flags(mode);
	

	if ( os_close( data, &rc ) == NOTIMPLEMENTED ) os_freakout();
	
	if ( rc != 0 ) 
	{
		free( stream );
		return NULL;
	}
	

	if ( os_open( filename, flags, &data, &fd ) == NOTIMPLEMENTED ) 
					os_freakout();
	if ( fd < 0 )
	{
		free( stream );
		return NULL;
	}

	stream->data = data;
	stream->fd = fd;
	stream->error = 0;
	stream->eof = 0;
	stream->mode = flags;
		
	return stream;
}

 
int 	fflush(FILE* stream)
{
	return 0;
}

 
int 	fclose(FILE* stream)
{
	return close( stream->fd );
}


FILE    *fdopen(int fd, const char *mode)
{
	/// \todo Honour mode changes.
	
	return support_retrieve_file( fd );
}
 
int 	remove(const char* filename)
{
	int rc;

	if ( os_delete( filename, &rc ) == NOTIMPLEMENTED )
			os_freakout();
		
	return rc;
}

 

 
FILE* 	tmpfile()
{
	char name[L_tmpnam];

	if ( tmpnam( name ) != name ) return NULL;
	
	/// \todo Ensure that this tmpfile gets deleted when the application closes or the file is closed.
	return fopen( name, "wb+" );
}

 
char* 	tmpnam(char s[L_tmpnam])
{
	char m[L_tmpnam];
	char *buf;
	int i;

	if ( s != NULL ) buf = s;
				else buf = m;
	

	for ( i = 0; i < L_tmpnam - 1; i++ )
	{
		char c = rand() % 52;

		if ( c < 26 ) buf[i] = 'a' + c;	
				else  buf[i] = 'A' + (c-26);	
	}
		
	buf[ L_tmpnam - 1] = 0;
	return buf;
}

 
 
int 	fprintf(FILE* stream, const char* format, ...)
{
	int rc;
	va_list ap;
	va_start(ap, format);
		rc = vfprintf( stream, format, ap );
	va_end(ap);
  return rc;
}


int 	printf(const char* format, ...)
{
	int rc;
	va_list ap;
	va_start(ap, format);
		rc = vfprintf( stdout, format, ap );
	va_end(ap);
  return rc;
}


int 	sprintf(char *buffer, const char* format, ...)
{
	int rc;
	va_list ap;
	va_start(ap, format);
		rc = vsprintf( buffer, format, ap );
	va_end(ap);
   return 0;

}


int 	vfprintf(FILE* stream, const char* format, va_list arg)
{
	return support_vfprintf( stream, format, arg );
}

 
int 	vprintf(const char* format, va_list arg)
{
	return vfprintf( stdout, format, arg );
}

 
int 	vsprintf(char* s, const char* format, va_list arg)
{
	return support_vsprintf( s, format, arg );
}


int 	fgetc(FILE* stream)
{
	unsigned char c;
	int rc;
	if ( feof( stream ) != 0 ) return EOF;

	if ( os_read( stream->data, &c, 1, &rc ) == NOTIMPLEMENTED )
			os_freakout();
	
	if ( rc == EOF )
	{
		stream->eof = 1;
		return EOF;
	}
	if ( rc < 0 )
	{
		stream->error = 1;
		return -1;
	}
	
	return (int)c;
}

 
 
int 	fputc(int c, FILE* stream)
{
	int rc;
	unsigned char ch = (unsigned char)c;
	
	if ( os_write( stream->data, &ch, 1, &rc ) == NOTIMPLEMENTED )
					os_freakout();
	
	if ( rc == EOF )
	{
		stream->eof = 1;
		return EOF;
	}
	if ( rc != 1 )
	{
		/// \todo confirm this action
		stream->error = 1;
		return EOF;
	}

	return c;
}

int      fputs(const char *str, FILE *stream)
{
	int rc;

	if ( os_write( stream->data, str, strlen(str), &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}

int 	getc(FILE* stream)
{
	return fgetc( stream );
}


int 	getchar(void)
{
	return getc( stdin );
}




size_t 	fread(void* /* restrict */ ptr, size_t size, size_t nmemb, FILE* /* restrict */ stream)
{
	unsigned char *buffer = (unsigned char*)ptr;
	int i,j,c;

	// C99 sanity check.
	if ( size == 0 ) return 0;
	if ( nmemb == 0 ) return 0;
	
	// For each member...
	for ( i = 0; i < nmemb; i++ )
	{
		for ( j = 0; j < size; j++ )
		{
			c = fgetc( stream );
			if ( (feof(stream) != 0) || ( ferror(stream) != 0 ) ) return i;

			*buffer = (unsigned char)c;
			buffer++;
		}
	}	


	// Apparently successful.
	return nmemb;	
}

size_t 	fwrite(const void* /* restrict */ ptr, size_t size, size_t nmemb, FILE* /* restrict */ stream)
{
	unsigned char *buffer = (unsigned char*)ptr;
	int i,j;

	// C99 sanity check.
	if ( size == 0 ) return 0;
	if ( nmemb == 0 ) return 0;
	
	// For each member...
	for ( i = 0; i < nmemb; i++ )
	{
		for ( j = 0; j < size; j++ )
		{
			if ( fputc( *buffer, stream ) != *buffer ) return i;
			buffer++;
		}
	}	


	// Apparently successful.
	return nmemb;	
}	

int 	fseek(FILE* stream, long offset, int origin)
{
	int rc;

	if ( os_seek( stream->data, offset, origin, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}


long int ftell(FILE* stream)
{
	long int rc;

	if ( os_tell( stream->data, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	return rc;
}
 
 
void 	rewind(FILE* stream)
{
	fseek(stream, 0L, SEEK_SET); 
	stream->error = 0;
}

 

void 	clearerr(FILE* stream)
{
	stream->error = 0;
	stream->eof = 0;
}


int		feof(FILE* stream)
{
	if ( stream->eof != 0 ) return 1;
	return 0;
}


int		ferror(FILE* stream)
{
	if ( stream->error != 0 ) return 1;
	return 0;
}

 
void 	perror(const char* s)
{
	fprintf(stderr, "%s: %s\n", (s != NULL ? s : ""), strerror(errno));
}

 




