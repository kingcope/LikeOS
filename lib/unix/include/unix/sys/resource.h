#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <sys/types.h>
#include <sys/time.h>


#define	PRIO_PROCESS			1
#define	PRIO_PGRP			2
#define	PRIO_USER			3


#ifndef _HAVE_RLIM_T
#define _HAVE_RLIM_T
typedef		uint32_t	rlim_t;
#endif


// TODO: check these constants

#define	RLIM_INFINITY			0xfffffffful	
#define	RLIM_SAVED_MAX			0xfffffffful	
#define	RLIM_SAVED_CUR			0xfffffffful	



#define		RUSAGE_SELF		1
#define		RUSAGE_CHILDREN		2

#ifndef _HAVE_RLIMIT
#define _HAVE_RLIMIT
struct	rlimit
{
	rlim_t rlim_cur;
	rlim_t rlim_max;
};
#endif



#ifndef _HAVE_RUSAGE
#define _HAVE_RUSAGE
struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
};
#endif



#define	RLIMIT_CORE			1
#define	RLIMIT_CPU			2
#define	RLIMIT_DATA			3
#define	RLIMIT_FSIZE			4
#define	RLIMIT_NOFILE			5
#define	RLIMIT_STACK			6
#define	RLIMIT_AS			7


#ifdef __cplusplus
extern "C" {
#endif

int  getpriority(int, id_t);
int  getrlimit(int, struct rlimit *);
int  getrusage(int, struct rusage *);
int  setpriority(int, id_t, int);
int  setrlimit(int, const struct rlimit *);

#ifdef __cplusplus
}
#endif
		
 

#endif


