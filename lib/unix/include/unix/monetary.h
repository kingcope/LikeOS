#ifndef _MONETARY_H
#define _MONETARY_H


#include <inttypes.h>


typedef	uint32_t	size_t;
typedef	int32_t		ssize_t;


#ifdef __cplusplus
extern "C" {
#endif
		

ssize_t    strfmon(char *, size_t, const char *, ...);

#ifdef __cplusplus
}
#endif
		
	

#endif

