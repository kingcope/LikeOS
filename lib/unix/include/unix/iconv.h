#ifndef _ICONV_H
#define _ICONV_H

#include <inttypes.h>

typedef uint32_t iconv_t;


#ifdef __cplusplus
extern "C" {
#endif
		
iconv_t iconv_open(const char *, const char *);
size_t  iconv(iconv_t, char **, size_t *, char **, size_t *);
int     iconv_close(iconv_t);


#ifdef __cplusplus
}
#endif
		


#endif

