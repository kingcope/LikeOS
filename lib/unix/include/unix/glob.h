#ifndef _GLOB_H
#define _GLOB_H

#include <stddef.h>


#ifndef _HAVE_GLOB_T
#define _HAVE_GLOB_T
typedef struct 
{
	size_t   gl_pathc;
	char   **gl_pathv;
	size_t   gl_offs;
} glob_t;
#endif



#define	GLOB_APPEND			(1<<0)
#define	GLOB_DOOFFS			(1<<1)
#define	GLOB_ERR			(1<<2)
#define	GLOB_MARK			(1<<3)
#define	GLOB_NOCHECK		(1<<4)
#define	GLOB_NOESCAPE		(1<<5)
#define	GLOB_NOSORT			(1<<6)


#define	GLOB_ABORTED			-1
#define	GLOB_NOMATCH			-2
#define	GLOB_NOSPACE			-3
#define	GLOB_NOSYS				-4


#ifdef __cplusplus
extern "C" {
#endif

int  glob(const char *, int, int (*)(const char *, int), glob_t *);
void globfree (glob_t *);


#ifdef __cplusplus
}
#endif


#endif

