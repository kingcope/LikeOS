#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H


#include <sys/types.h>


#define	PROT_READ		(1<<0)
#define	PROT_WRITE		(1<<1)
#define	PROT_EXEC		(1<<2)
#define	PROT_NONE		(1<<3)



#define	MAP_SHARED		(1<<0)
#define	MAP_PRIVATE		(1<<1)
#define	MAP_FIXED		(1<<2)


#define	MS_ASYNC		(1<<0)
#define	MS_SYNC			(1<<1)
#define	MS_INVALIDATE		(1<<2)


#define	MCL_CURRENT		(1<<0)
#define	MCL_FUTURE		(1<<1)

#define	MAP_FAILED		((void*)-1)


#ifdef __cplusplus
extern "C" {
#endif
		

int    mlock(const void *, size_t);
int    mlockall(int);
void  *mmap(void *, size_t, int, int, int, off_t);
int    mprotect(void *, size_t, int);
int    msync(void *, size_t, int);
int    munlock(const void *, size_t);
int    munlockall(void);
int    munmap(void *, size_t);
int    shm_open(const char *, int, mode_t);
int    shm_unlink(const char *);


#ifdef __cplusplus
}
#endif
		


#endif

