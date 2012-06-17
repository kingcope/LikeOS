#ifndef _NDBM_H
#define _NDBM_H


#include <stddef.h>

#ifndef _HAVE_DATUM;
#define _HAVE_DATUM;
typedef struct 
{
	void *dptr;
	size_t dsize;
} datum;
#endif


#ifndef _HAVE_DBM
#define _HAVE_DBM
typedef struct
{
	int dunno;	// TODO: find out
} DBM;
#endif


#define	DBM_INSERT		0
#define	DBM_REPLACE		1

#ifdef __cplusplus
extern "C" {
#endif
		


int     dbm_clearerr(DBM *);
void    dbm_close(DBM *);
int     dbm_delete(DBM *, datum);
int     dbm_error(DBM *);
datum   dbm_fetch(DBM *, datum);
datum   dbm_firstkey(DBM *);
datum   dbm_nextkey(DBM *);
DBM    *dbm_open(const char *, int, mode_t);
int     dbm_store(DBM *, datum, datum, int);


#ifdef __cplusplus
}
#endif
		

#endif

