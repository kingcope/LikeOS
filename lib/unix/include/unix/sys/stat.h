#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

struct stat
{
	dev_t     st_dev;
	ino_t     st_ino;
	mode_t    st_mode;
	nlink_t   st_nlink;
	uid_t     st_uid;
	gid_t     st_gid;
	dev_t     st_rdev;
	off_t     st_size;
	time_t    st_atime;
	time_t    st_mtime;
	time_t    st_ctime; 
	blksize_t st_blksize;
	blkcnt_t  st_blocks; 
};



#define	S_IFMT				(1<<0)
#define	S_IFBLK				(1<<1)
#define	S_IFCHR				(1<<2)
#define	S_IFIFO				(1<<3)
#define	S_IFREG				(1<<4)
#define	S_IFDIR				(1<<5)
#define	S_IFLNK				(1<<6)


#define S_IREAD		400
#define S_IWRITE	200
#define S_IEXEC		100

#define	S_ISUID   04000
#define	S_ISGID   02000
#define	S_ISVTX   01000
#define	S_IRUSR (S_IREAD)
#define S_IWUSR (S_IWRITE)
#define	S_IXUSR (S_IEXEC)
#define	S_IRGRP   00040
#define	S_IWGRP   00020
#define	S_IXGRP   00010
#define	S_IROTH   00004
#define	S_IWOTH   00002
#define	S_IXOTH   00001
																													



#define	S_IRWXU		( S_IRUSR | S_IWUSR | S_IXUSR )
#define	S_IRWXO		( S_IRGRP | S_IWGRP | S_IXGRP )
#define	S_IRWXG		( S_IROTH | S_IWOTH | S_IXOTH )

#define S_ISBLK(m)	( (m==S_IFBLK) )
#define S_ISCHR(m)	( (m==S_IFCHR) )
#define S_ISDIR(m)	( (m==S_IFDIR) )
#define S_ISFIFO(m)	( (m==S_IFFIFO) )
#define S_ISREG(m)	( (m==S_IFREG) )
#define S_ISLNK(m)	( (m==S_IFLNK) )


// TODO: complete

#define	S_TYPEISMQ(buf)			0
#define	S_TYPEISSEM(buf)		0
#define	S_TYPEISSHM(buf)		0

#ifdef __cplusplus
extern "C" {
#endif

int    chmod(const char *, mode_t);
int    fchmod(int, mode_t);
int    fstat(int, struct stat *);
int    lstat(const char *, struct stat *);
int    mkdir(const char *, mode_t);
int    mkfifo(const char *, mode_t);
int    mknod(const char *, mode_t, dev_t);
int    stat(const char *, struct stat *);
mode_t umask(mode_t);
																				      
#ifdef __cplusplus
}
#endif

#endif

