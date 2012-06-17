#ifndef _ULIMIT_H
#define _ULIMIT_H



#define	UL_GETFSIZE		1
#define	UL_SETFSIZE		2

#ifdef __cplusplus
extern "C" {
#endif
		

long int ulimit(int, ...);


#ifdef __cplusplus
}
#endif
		


#endif

