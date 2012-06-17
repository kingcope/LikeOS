#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "../os/os.h"

#include "support.h"


static FILE** __UNIX_filelist = NULL;
static unsigned int __UNIX_filelock = 0;
static int __UNIX_filemax = 0;
static int __UNIX_filecount = 0;



/** Initializes the library's internal array to be used
 * to manage and keep track of files. The number
 * of files that the library supports can change dynamically
 * when the array gets filled but it's not gauranteed
 * to work.
 *
 * \warning This function assumes that it's the only thread
 * accessing the file data - because it sets up locks 
 * and what not.
 *
 * \param count The number of files to support initially.
 * \return 0 if successful initializtion.
 */

int support_init_files( int count )
{
	int i;

	// Allocate a filelist.
	__UNIX_filelist = (FILE**)(malloc( count * sizeof(FILE*) ) );
	if ( __UNIX_filelist == NULL ) return -1;

	// Don't set the lock because this function is only called
	// from within a locked state. Trust that environment set
	// up the initialized data.
	__UNIX_filemax = count;
	__UNIX_filecount = 0;

	  for ( i = 0; i < __UNIX_filemax; i++ )
		__UNIX_filelist[i] = NULL;

	
	if ( atexit( support_shutdown_files ) != 0 )
	{
		perror("unable to install support_shutdown_files into the atexit handlers!");
		 abort();
	}
	  
	return 0;
}

/** This functions frees all open files and resources used by
 * the library to maintain the file lists internally. Note
 * that this function does not close files! It merely cleans
 * up on this side.  This function is embedded into the
 * atexit handlers by the support_init_files function if
 * it's ever invoked.
 *
 * \warning Does not close or flush files.
 * \warning This functions assumes that it's the only thread
 * running.
 */

void support_shutdown_files(void)
{
	int i;

	if ( __UNIX_filelist == NULL ) return;

	  for ( i = 0; i < __UNIX_filemax; i++ )
	  {
		if ( __UNIX_filelist[i] == NULL ) continue;

		free( __UNIX_filelist[i] );
		
		__UNIX_filelist[i] = NULL;
	  }
	
	free( __UNIX_filelist );
}



/** This is a static function which is supposed to be
 * inline to ensure quick, checking of init status
 * for the library.
 */

static inline void support_check_init()
{
	 // --- initialize file memory if required ----
	 if ( __UNIX_filelist == NULL )
	 {
		if ( support_init_files( FOPEN_MAX ) != 0 )
		{
		   perror("unable to allocate file management memory!");
		   os_release_spinlock( & __UNIX_filelock );
		   abort();
		}
	 }
}

/** This function returns the FILE* stream associated
 * with the particular fd - file descriptor.
 *
 * \param fd The file descriptor to search for.
 *
 * \return FILE* if file was found.
 * \return NULL if not found.
 */

FILE* support_retrieve_file( int fd )
{
	int i;
	FILE *answer = NULL;
	
	 os_acquire_spinlock( & __UNIX_filelock );

	 support_check_init();

	  for ( i = 0; i < __UNIX_filemax; i++ )
	  {
		if ( __UNIX_filelist[i] == NULL ) continue;

		if ( __UNIX_filelist[i]->fd == fd ) 
		{
			answer = __UNIX_filelist[i];
			break;
		}
	  }
	 
	 os_release_spinlock( & __UNIX_filelock );

	return answer;
}

/** This function returns the FILE* stream associated
 * with the particular fd - file descriptor - AND 
 * removes it from the library array.
 *
 * \param fd The file descriptor to search for.
 *
 * \return FILE* if file was found and removed.
 * \return NULL if not found.
 */

FILE* support_remove_file( int fd )
{
	int i;
	FILE *answer = NULL;
	
	 os_acquire_spinlock( & __UNIX_filelock );

	 support_check_init();

	  for ( i = 0; i < __UNIX_filemax; i++ )
	  {
		if ( __UNIX_filelist[i] == NULL ) continue;

		if ( __UNIX_filelist[i]->fd == fd ) 
		{
			answer = __UNIX_filelist[i];

			__UNIX_filelist[i] = NULL;
			__UNIX_filecount -= 1;
			break;
		}
	  }
	 
	 os_release_spinlock( & __UNIX_filelock );

	return answer;
}





/** This function inserts the FILE* structure into the
 * library's array for later use.
 *
 * \param stream The FILE* to insert and keep recorded.
 *
 * \return 0 on success.
 */
int support_insert_file( FILE* stream )
{
	int i;
	
	 os_acquire_spinlock( & __UNIX_filelock );

	 support_check_init();

	 // Is there space?
	 if ( __UNIX_filemax == __UNIX_filecount )
	 {
		// no.
	 	os_release_spinlock( & __UNIX_filelock );
		return -1;
	 } 

	 
	  for ( i = 0; i < __UNIX_filemax; i++ )
	  {
		if ( __UNIX_filelist[i] != NULL ) continue;

		__UNIX_filelist[i] = stream;
		__UNIX_filecount += 1;
		break;
	  }
	 
	 os_release_spinlock( & __UNIX_filelock );

	return 0;
}



/** This function takes the standard "wb+" and "r"
 * modes which are passed to functions like fopen
 * and converts them to fcntl.h equivalents like
 * O_CREAT|O_TRUNC, etc.
 * 
 * \param mode A character mode constant such as "wb+"
 *
 * \return Corresponding flags for open()
 */
unsigned int support_fmodes2flags( const char *mode )
{
	if ( (strcmp( mode, "r" ) == 0) || (strcmp( mode, "rb" ) == 0 ) ) return O_RDONLY; 
	if ( (strcmp( mode, "w" ) == 0) || (strcmp( mode, "wb" ) == 0 ) ) return O_WRONLY | O_CREAT | O_TRUNC; 
	if ( (strcmp( mode, "a" ) == 0) || (strcmp( mode, "ab" ) == 0 ) ) return O_WRONLY | O_CREAT | O_APPEND; 
	if ( (strcmp( mode, "r+" ) == 0) || (strcmp( mode, "rb+" ) == 0 ) 
		 || (strcmp( mode, "r+b") == 0) ) return O_RDWR ; 
	if ( (strcmp( mode, "w+" ) == 0) || (strcmp( mode, "wb+" ) == 0 ) 
		 || (strcmp( mode, "w+b") == 0) ) return O_WRONLY | O_TRUNC | O_CREAT; 

	if ( (strcmp( mode, "a+" ) == 0) || (strcmp( mode, "ab+" ) == 0 ) 
		 || (strcmp( mode, "a+b") == 0) ) return O_WRONLY | O_APPEND | O_CREAT; 
	
	return 0;
}


