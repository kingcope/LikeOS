#ifndef _LIBGEN_H
#define _LIBGEN_H


#ifdef __cplusplus
extern "C" {
#endif


extern char* __loc1;		// LEGACY

char  *basename(char *);
char  *dirname(char *);
char  *regcmp(const char *, ...);		// LEGACY 
char  *regex(const char *, const char *, ...); // LEGACY


#ifdef __cplusplus
}
#endif
		

#endif

