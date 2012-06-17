#ifndef _OS_H
#define _OS_H

#include <time.h>
#include <setjmp.h>
#include <sys/stat.h>

/** \defgroup OSFUNCS OS Dependant Functions 
 *
 * These are the OS specific functions which need to 
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{
 */


#define 		NOTIMPLEMENTED			-1
#define			OK						0


#ifdef __cplusplus
extern "C" {
#endif

/** You will want to implement these stub functions in your
 * own OS subdirectory.
 *
 *
 * They should all take the form of:
 *
 *       int function( ... params ... );
 *
 * If the function is not implemented, it should return NOTIMPLEMENTED
 * or OK if it is implemented. The actual return values will be
 * returned in the parameters.
 * 
 */



/** Given the number of pages required, will return some memory
 * available for reading and writing. A page size is expected
 * to be 4096 bytes and to be aligned on a 4K bounday. The memory 
 * does not need to be cleaned but it is expected to be
 * writable.
 * 
 * \param pages The number of 4K-aligned 4096 byte pages required.
 * \return NULL if failure.
 * \return The address of the newly allocated memory.
 */
int 	os_mem_alloc( int pages, void** ptr ); 

/** This function is expected to free the memory previously
 * allocated using os_mem_alloc. As you can see the size
 * and pages used by the memory is not returned so it's up 
 * to you or your OS to remember that kind of information
 * if it is required.
 *
 * \param mem A pointer to memory previously allocated by
 *            os_mem_alloc.
 */
int		os_mem_free( void* mem );


/** os_acquire_spinlock should be a platform specific way 
 * of implementing spinlocks. According to the library, a 
 * spinlock is an unsigned int which can either be 
 * unlocked (0) or locked ( not 0 ). 
 *
 * This function should be atomic and the library should
 * be guaranteed that there is 1 and only 1 thread holding
 * a spinlock at any one time.
 *
 * The default implementation defined in the empty platform
 * should work on any x86 machine. 
 *
 * \note The default implementation is not quite a spinlock
 * in that it calls sched_yield() on each spin, to ensure
 * faster turnover time.
 *  
 * \param lock A pointer to an unsigned int.
 */
int 	os_acquire_spinlock( unsigned int *lock  );

/** os_release_spinlock should be a platform specific way 
 * of implementing spinlocks. According to the library, a 
 * spinlock is an unsigned int which can either be 
 * unlocked (0) or locked ( not 0 ). 
 *
 * This function should be atomic and the library should
 * be guaranteed that there is 1 and only 1 thread holding
 * a spinlock at any one time.
 *  
 * The default implementation defined in the empty platform
 * should work on any x86 machine.
 *  
 * \param lock A pointer to an unsigned int.
 */

int 	os_release_spinlock( unsigned int *lock  );



/** This function should return a random number, any number
 * between 0 and MAX_INT. 
 *
 * \return A number such that 0 gte number lte MAX_INT
 */
int		os_rand( int *rand );


/** This function should put a NULL terminated string onto
 * the currently active console/screen. It is up to the
 * OS to handle the screen scrolling, cursor manipulation, etc
 * ... but really the library just cares that the string has
 * been displayed.
 *
 * \param str A NULL-terminated string.
 */
int		os_puts( const char *str );


/** This is the file system open command, very much like the
 * standard UNIX open. Your function can open the file, create
 * a data structure in which to maintain it's information about
 * it and then it must return this structure in *data. This
 * value will be passed back to the OS functions in all
 * later file operations to maintain state, etc.
 * 
 *
 * \param filename  A fully qualified filename.
 * \param mode The mode in which the file should be opened. These are
 *             the same flags specified in the fcntl.h file - 
 *             O_RDWR, O_CREATE, etc.  
 *             
 * \param data The os_open function must return a data structure
 *             in *data.  It doesn't matter what the structure looks
 *             like or what's in it, as long as there is something.
 *             It's only for your use in later file operations.
 * 
 * \return -1 for failure.
 * \return >= 0, the file descriptor of the newly open file.
 * 
 */
int		os_open( const char *filename, unsigned int mode, void **data, int *fd );

/** Typical read function which copies at least len amount of
 * bytes into the given buffer. It is not supposed to return 
 * more than "len" specified amount of bytes.
 *
 * \param data The original data returned in the os_open function.
 * \param buffer A buffer into which the data should be copied.
 * \param len The maximum amount of bytes requested.
 * 
 * \return -1 if failure to read.
 * \return >= 0, the number of bytes read.
 */
int		os_read( void *data, void *buffer, int len, int *rc );


/** Typical write function.
 *
 * \param data The original data returned in the os_open function.
 * \param buffer A buffer which contains the data to be written.
 * \param len The number of bytes to write.
 * 
 * \return -1 on failure to write any bytes
 * \return >= 0, the number of bytes written.
 */
int		os_write( void *data, const void *buffer, int len, int *rc );

/** This is very much the standard file seek operation
 * and the parameters are exactly the same: SEEK_SET, etc. 
 *
 * \return 0 on successful seek.
 */
int		os_seek( void *data, long offset, int origin, int *rc );

/** This is the standard unix version of tell, which
 * should return the current position in the given
 * file. The current position is in relation to the 
 * beginning of the file - considered to be byte 0.
 *
 */
int		os_tell( void *data, long *pos );

/** Closes the open file. The data value which your os_open
 * function returned should be released and free here. All
 * resources should be returned to the system. After
 * calling this function, the file is completely forgotten
 * by the library.
 *
 * \return 0 on successful close of the file.
 */
int		os_close( void *data, int *rc );

/** This should return as much information as possible
 * with regards to the struct stat parameter.
 *
 * \param data The original data returned by os_open.
 * \param st A pointer to a struct stat which must
 *           hold the gathered data.
 *
 * \return 0 on successful stat.
 */
int		os_stat( void *data, struct stat *st, int *rc );


/** File operation which will delete the given filename.
 *
 * \return 0 on successful deletion of the given filename.
 */
int		os_delete( const char *filename, int *rc );

/** File operation which will create the directory with
 * the filename given. 
 *
 * \return 0 on successful creation.
 */
int		os_mkdir( const char *filename, unsigned int mode, int *rc );

/** File operation which will remove a directory with the
 * given filename.
 *
 * \return 0 on successful deletion.
 */
int		os_rmdir( const char *filename, int *rc );

/** Just like the standard unix command, this function must
 * return the amount of clock ticks used by the current 
 * application.  To get the number of seconds used by the 
 * application, divide the return value by CLOCKS_PER_SEC
 *
 * \return the number of clocks used by the processor.
 */ 
int		os_clock( clock_t *clock );

/** This returns the system time. It returns seconds and
 * milliseconds. This time is presumed to be the amount of
 * seconds and milliseconds since the standard UNIX epoch.
 *
 * (00:00:00 GMT, January 1, 1970)
 *
 * \return 0 if the time was successfully retrieved.
 */
int		os_system_time( time_t *seconds, time_t *milliseconds, int *rc );	

/** This function should put the calling thread to sleep for the
 * given amount of seconds.
 *
 * \return the actual number of seconds that the thread
 * was asleep for.
 */
int 	os_sleep( unsigned int seconds, unsigned int *rc );

/** This will attempt to execute the command line given. The
 * command line is a full command with command line parameters. It's
 * up to your system to parse it, split parameters if necessary,
 * pre-launch, execute the application, etc.
 *
 * \return PID of spawned process on successful execution of the
 * command.
 * \return -1 if command failed.
 */
int		os_system( const char *s, int *rc );

/** OS specific exit routine. This should, theoretically, return
 * the status back to the parent application which launched this
 * one. When this function is called, all threads stop and the
 * application terminated.
 *
 * \return -1 if failure to exit ocurred.
 */
int 	os_exit( int status, int *rc );


/** If your OS supports signalling, this is the signalling
 * hook into your OS. It's supposed to trigger a signal.
 *
 * \return 0 on successful signal.
 */
int		os_signal( int sig, int *rc);

/** Returns the environment variable setting identified
 * by the name parameter or NULL if the requested environment
 * variable does not exist.
 *
 * \return NULL if the environment variable does not exist.
 * \return The data of the environment variable.
 */
int 	os_getenv(const char* name, char **env );

/** This function should release control of the CPU and
 * timeslice so that any other process or thread may run.
 *
 * \param rc This should be set to 0 if the yield was successful. 
 * Otherwise, this should be the error code (err) that needs to
 * be set.
 */
int 	os_sched_yield( int *rc );


/** This is the only operation that needs to be implemented. 
 * It should know how to freak out on your system when an
 * operation that is required to me implemented is not
 * implemented.
 *
 *
 */
void	os_freakout();

#ifdef __cplusplus
}
#endif


/** @} 
 */

#endif

