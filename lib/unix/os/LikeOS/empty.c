#include "../os.h"
#define __x86__

int 	os_mem_alloc( int pages, void** ptr )
{
	*ptr = (void*)kalloc(pages);
	return 0;
}

 
int		os_mem_free( void* mem )
{
	kfree(mem);
	return 0;
}


int		os_acquire_spinlock( unsigned int *lock )
{/*
#ifdef __x86__
  while ( 1==1 )
  {
  	int gotit = 0;
	asm volatile ( "lock btsl $0x0, (%1)\n"
              	   "setnc %0\n "
			  		: "=g" (gotit)
			  		: "p" (lock)
				);

	if ( gotit != 0 ) break;
	asm ( "pause" );

	os_sched_yield();
  }
  return 0;
#else*/
  return NOTIMPLEMENTED;
//#endif
}

int		os_release_spinlock( unsigned int *lock )
{/*
#ifdef __x86__
	asm volatile ( "lock btrl $0x0, (%0)\n" : : "p" (lock) );
	return 0;
#else*/
  return NOTIMPLEMENTED;
//#endif
}

int		os_rand( int *rand )
{
	return NOTIMPLEMENTED;
}
#define UINT unsigned int
#define WHITE_TXT 0x07

UINT k_printf(char *message, UINT line) // the message and then the line #
{
	char *vidmem = (char *) 0xb8000;
	UINT i=0;

	i=(line*80*2);

	while(*message!=0)
	{
		if(*message=='\n') // check for a new line
		{
			line++;
			i=(line*80*2);
			*message++;
		} else {
			vidmem[i]=*message;
			*message++;
			i++;
			vidmem[i]=WHITE_TXT;
			i++;
		};
	};

	return(1);
};

int		os_puts( const char *str )
{
	static char buffer[5000];
	
	strcat(buffer, str);
	k_printf(buffer, 0);
	return 0;
}


int		os_open( const char *filename, unsigned int mode, void **data, int *fd )
{
	return NOTIMPLEMENTED;
}


int		os_read( void *data, void *buffer, int len, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_write( void *data, const void *buffer, int len, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_seek( void *data, long offset, int origin, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_tell( void *data, long *pos )
{
	return NOTIMPLEMENTED;
}


int		os_close( void *data, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_stat( void *data, struct stat *st, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_delete( const char *filename, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_mkdir( const char *filename, unsigned int mode, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_rmdir( const char *filename, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_clock( clock_t *clock )
{
	return NOTIMPLEMENTED;
}


int		os_system_time( time_t *seconds, time_t *milliseconds, int *rc )
{
	return NOTIMPLEMENTED;
}

	
int 	os_sleep( unsigned int seconds, unsigned int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_system( const char *s, int *rc )
{
	return NOTIMPLEMENTED;
}


int 	os_exit( int status, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_signal( int sig, int *rc)
{
	return NOTIMPLEMENTED;
}


int 	os_getenv(const char* name, char **rc )
{
	return NOTIMPLEMENTED;
}

int 	os_sched_yield( int *rc )
{
	return NOTIMPLEMENTED;
}

void    os_freakout()
{
}


