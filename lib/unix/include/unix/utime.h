#ifndef _UTIME_H
#define _UTIME_H


#include <sys/types.h>

struct utimbuf
{
	time_t    actime;
	time_t    modtime;
};

#ifdef __cplusplus
extern "C" {
#endif
		
int utime(const char *, const struct utimbuf *);

#ifdef __cplusplus
}
#endif
		


#endif

