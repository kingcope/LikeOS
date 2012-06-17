#ifndef _WORDEXP_H
#define _WORDEXP_H


#include <stddef.h>

typedef struct 
{
	size_t   we_wordc;  //count of words matched by words
	char   **we_wordv;  //pointer to list of expanded words 
	size_t   we_offs;   //slots to reserve at the beginning of we_wordv
} wordexp_t;


#define	WRDE_APPEND					(1<<0)
#define WRDE_DOOFFS					(1<<1)
#define WRDE_NOCMD					(1<<2)
#define WRDE_REUSE					(1<<3)
#define WRDE_SHOWERR				(1<<4)
#define WRDE_UNDEF					(1<<5)


#define WRDE_NOSYS					-1
#define WRDE_BADCHAR				1
#define WRDE_BADVAL					2
#define WRDE_CMDSUB					3
#define WRDE_NOSPACE				4
#define WRDE_SYNTAX					5


#ifdef __cplusplus
extern "C" {
#endif

int  wordexp(const char *, wordexp_t *, int);
void wordfree(wordexp_t *);


#ifdef __cplusplus
}
#endif

#endif

