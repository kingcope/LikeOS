#ifndef _FNMATCH_H
#define _FNMATCH_H



#define	FNM_NOMATCH		(1<<0)
#define FNM_PATHNAME	(1<<1)
#define FNM_PERIOD		(1<<2)
#define FNM_NOESCAPE	(1<<3)
#define FNM_NOSYS		(1<<4)

#ifdef __cplusplus
extern "C" {
#endif
		

int fnmatch(const char *, const char *, int);

		
#ifdef __cplusplus
}
#endif
		


#endif

