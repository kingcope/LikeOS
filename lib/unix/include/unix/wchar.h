#ifndef _WCHAR_H
#define _WCHAR_H

#include <stdio.h>
#include <stddef.h>
#include <time.h>


#ifndef _HAVE_WINT_T
#define _HAVE_WINT_T
typedef unsigned int wint_t;
#endif

#ifndef _HAVE_WCTYPE_T
#define _HAVE_WCTYPE_T
typedef unsigned long int wctype_t;
#endif
		
#ifndef _HAVE_MBSTATE_T
#define _HAVE_MBSTATE_T
typedef struct
{
	int count;
	wint_t temp;
} mbstate_t;
#endif


#ifdef __cplusplus
extern "C" {
#endif
				

wint_t            btowc(int);
int               fwprintf(FILE *, const wchar_t *, ...);
int               fwscanf(FILE *, const wchar_t *, ...);
int               iswalnum(wint_t);
int               iswalpha(wint_t);
int               iswcntrl(wint_t);
int               iswdigit(wint_t);
int               iswgraph(wint_t);
int               iswlower(wint_t);
int               iswprint(wint_t);
int               iswpunct(wint_t);
int               iswspace(wint_t);
int               iswupper(wint_t);
int               iswxdigit(wint_t);
int               iswctype(wint_t, wctype_t);
wint_t            fgetwc(FILE *);
wchar_t          *fgetws(wchar_t *, int, FILE *);
wint_t            fputwc(wchar_t, FILE *);
int               fputws(const wchar_t *, FILE *);
int               fwide(FILE *, int);
wint_t            getwc(FILE *);
wint_t            getwchar(void);
int               mbsinit(const mbstate_t *);
size_t            mbrlen(const char *, size_t, mbstate_t *);
size_t            mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t            mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);
wint_t            putwc(wchar_t, FILE *);
wint_t            putwchar(wchar_t);
int               swprintf(wchar_t *, size_t, const wchar_t *, ...);
int               swscanf(const wchar_t *, const wchar_t *, ...);
wint_t            towlower(wint_t);
wint_t            towupper(wint_t);
wint_t            ungetwc(wint_t, FILE *);
int               vfwprintf(FILE *, const wchar_t *, va_list);
int               vwprintf(const wchar_t *, va_list);
int               vswprintf(wchar_t *, size_t, const wchar_t *, va_list);
size_t            wcrtomb(char *, wchar_t, mbstate_t *);
wchar_t          *wcscat(wchar_t *, const wchar_t *);
wchar_t          *wcschr(const wchar_t *, wchar_t);
int               wcscmp(const wchar_t *, const wchar_t *);
int               wcscoll(const wchar_t *, const wchar_t *);
wchar_t          *wcscpy(wchar_t *, const wchar_t *);
size_t            wcscspn(const wchar_t *, const wchar_t *);
size_t            wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *);
size_t            wcslen(const wchar_t *);
wchar_t          *wcsncat(wchar_t *, const wchar_t *, size_t);
int               wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t          *wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t          *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t          *wcsrchr(const wchar_t *, wchar_t);
size_t            wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
size_t            wcsspn(const wchar_t *, const wchar_t *);
wchar_t          *wcsstr(const wchar_t *, const wchar_t *);
double            wcstod(const wchar_t *, wchar_t **);
wchar_t          *wcstok(wchar_t *, const wchar_t *, wchar_t **);
long int          wcstol(const wchar_t *, wchar_t **, int);
unsigned long int wcstoul(const wchar_t *, wchar_t **, int);
wchar_t          *wcswcs(const wchar_t *, const wchar_t *);
int               wcswidth(const wchar_t *, size_t);
size_t            wcsxfrm(wchar_t *, const wchar_t *, size_t);
int               wctob(wint_t);
wctype_t          wctype(const char *);
int               wcwidth(wchar_t);
wchar_t          *wmemchr(const wchar_t *, wchar_t, size_t);
int               wmemcmp(const wchar_t *, const wchar_t *, size_t);
wchar_t          *wmemcpy(wchar_t *, const wchar_t *, size_t);
wchar_t          *wmemmove(wchar_t *, const wchar_t *, size_t);
wchar_t          *wmemset(wchar_t *, wchar_t, size_t);
int               wprintf(const wchar_t *, ...);
int               wscanf(const wchar_t *, ...);


#ifdef __cplusplus
}
#endif


#define WCHAR_MIN (-2147483648l)
#define WCHAR_MAX (2147483647l)
#define	WEOF		0xffffffffu



#endif

