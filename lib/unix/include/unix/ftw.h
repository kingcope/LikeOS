#ifndef _FTW_H
#define _FTW_H

#include <sys/stat.h>

struct FTW
{
	int base;
	int level;
};


#define		FTW_F			(1<<0)	/* File.  */
#define		FTW_D			(1<<1)	/* Directory.  */
#define		FTW_DNR			(1<<2)	/* Directory without read permission.  */
#define		FTW_DP			(1<<3)	/* Directory with subdirectories visited.  */
#define		FTW_NS			(1<<4)	/* Unknown type, stat() failed.  */
#define		FTW_SL			(1<<5)	/* Symbolic link.  */
#define		FTW_SLN			(1<<6)	/* Symbolic link that names a non-existent file. */


#define		FTW_PHYS		(1<<0)	/* Physical walk, does not follow symbolic links. Otherwise, nftw() will follow links but will not walk down any path that crosses itself.  */
#define		FTW_MOUNT		(1<<1)	/* The walk will not cross a mount point.  */
#define		FTW_DEPTH		(1<<2)	/* All subdirectories will be visited before the directory itself.  */
#define		FTW_CHDIR		(1<<3)	/* The walk will change to each directory before reading it. */



#ifdef __cplusplus
extern "C" {
#endif
		

int ftw(const char *, int (*)(const char *, const struct stat *, int), int);
int nftw(const char *, int (*) (const char *, const struct stat *, int, struct FTW*), int, int);


#ifdef __cplusplus
}
#endif
		



#endif

