#ifndef _WCTYPE_H
#define _WCTYPE_H

#include <wchar.h>


#ifndef _HAVE_WCTRANS_T
#define _HAVE_WCTRANS_T
typedef unsigned long int wctrans_t;
#endif
	

#ifdef __cplusplus
extern "C" {
#endif

int       iswalnum(wint_t);
int       iswalpha(wint_t);
int       iswcntrl(wint_t);
int       iswdigit(wint_t);
int       iswgraph(wint_t);
int       iswlower(wint_t);
int       iswprint(wint_t);
int       iswpunct(wint_t);
int       iswspace(wint_t);
int       iswupper(wint_t);
int       iswxdigit(wint_t);
int       iswctype(wint_t, wctype_t);
wint_t    towctrans(wint_t, wctrans_t);
wint_t    towlower(wint_t);
wint_t    towupper(wint_t);
wctrans_t wctrans(const char *);
wctype_t  wctype(const char *);

#ifdef __cplusplus
}
#endif



#endif

