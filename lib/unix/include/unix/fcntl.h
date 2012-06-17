#ifndef _FCNTL_H
#define _FCNTL_H


#include <sys/types.h>


#define	F_DUPFD					1
#define	F_GETFD					2
#define	F_SETFD					3
#define	F_GETFL					4
#define	F_SETFL					5
#define	F_GETLK					6
#define	F_SETLK					7
#define	F_SETLKW				8


#define	FD_CLOEXEC				(1<<0)


#define	F_RDLCK			(1<<0)
#define	F_UNLCK			(1<<1)
#define	F_WRLCK			(1<<2)


#define	O_CREAT			(1<<0)
#define	O_EXCL			(1<<1)
#define	O_NOCTTY		(1<<2)
#define	O_TRUNC			(1<<3)


#define	O_APPEND		(1<<4)
#define	O_DSYNC			(1<<5)
#define	O_NONBLOCK		(1<<6)
#define	O_RSYNC			(1<<7)
#define	O_SYNC			(1<<8)


#define	O_ACCMODE	(O_RDONLY|O_RDWR|O_WRONLY)

#define	O_RDONLY		(1<<9)
#define	O_RDWR			(1<<10)
#define	O_WRONLY		(1<<11)



struct flock
{
	short l_type;
	short l_whence;
	off_t l_start;
	off_t l_len; 
	pid_t l_pid;
};

#ifdef __cplusplus
extern "C" {
#endif

int  creat(const char *, mode_t);
int  fcntl(int, int, ...);
int  open(const char *, int, ...);


#ifdef __cplusplus
}
#endif


#endif

