#ifndef _LOCALE_H
#define _LOCALE_H


#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

struct lconv
{
	char    *currency_symbol;
	char    *decimal_point;
	char     frac_digits;
	char    *grouping;
	char    *int_curr_symbol;
	char     int_frac_digits;
	char    *mon_decimal_point;
	char    *mon_grouping;
	char    *mon_thousands_sep;
	char    *negative_sign;
	char     n_cs_precedes;
	char     n_sep_by_space;
	char     n_sign_posn;
	char    *positive_sign;
	char     p_cs_precedes;
	char     p_sep_by_space;
	char     p_sign_posn;
	char    *thousands_sep;
};


#define		LC_ALL				1
#define		LC_COLLATE			2
#define		LC_CTYPE			3
#define		LC_MESSAGES			4
#define		LC_MONETARY			5
#define		LC_NUMERIC			6
#define		LC_TIME				7



struct  lconv *localeconv(void);
char    setlocale(int, const char *);




#endif


