#ifndef _DLFCN_H
#define _DLFCN_H


#define	RTLD_LAZY		(1<<0)
#define	RTLD_NOW		(1<<1)
#define RTLD_GLOBAL		(1<<2)
#define RTLD_LOCAL		(1<<3)



#ifdef __cplusplus
extern "C" {
#endif
		

void  *dlopen(const char *, int);
void  *dlsym(void *, const char *);
int    dlclose(void *);
char  *dlerror(void);


#ifdef __cplusplus
}
#endif
		


#endif

