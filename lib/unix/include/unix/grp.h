#ifndef _GRP_H
#define _GRP_H


#include <sys/types.h>



struct group
{
	char   *gr_name;
	gid_t   gr_gid; 
	char  **gr_mem; 
};



#ifdef __cplusplus
extern "C" {
#endif

struct group  *getgrgid(gid_t);
struct group  *getgrnam(const char *);
int            getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
int            getgrnam_r(const char *, struct group *, char *, size_t , struct group **);
struct group  *getgrent(void);
void           endgrent(void);
void           setgrent(void);


#ifdef __cplusplus
}
#endif


#endif

