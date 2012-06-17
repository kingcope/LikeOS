#ifndef _DIRENT_H
#define _DIRENT_H


#include <sys/types.h>

typedef struct 
{
} DIR;


struct dirent
{
	ino_t	d_ino;
	char	d_name[];
};

#ifdef __cplusplus
extern "C" {
#endif
		

int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *, struct dirent *, struct dirent **);
void           rewinddir(DIR *);
void           seekdir(DIR *, long int);
long int       telldir(DIR *);


#ifdef __cplusplus
}
#endif
		

    

#endif

