#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <float.h>
#include <math.h>

#include "os/os.h"

static void* __UNIX_exitfuncs[ ATEXIT_MAX ];
static int __UNIX_exitcount = 0;

extern unsigned long genrand();
extern void gensrand( unsigned int seed );


typedef void (*_libc_exit_func)(void);



int 	abs(int n)
{
	if ( n < 0 ) return (-n);
	return n;
}

 
long 	labs(long n)
{
	if ( n < 0 ) return (-n);
	return n;
}

 
div_t 	div(int num, int denom)
{
	div_t result;
	int tmp;
	int a_num;
	int a_denom;
	int sign;

	a_num = abs( num );
	a_denom = abs( denom );

	sign = -1;
	if ( (num < 0) && (denom < 0) ) sign = 1;
	if ( (num > 0) && (denom > 0) ) sign = 1;

	result.quot = 0;
	result.rem = 0;

	tmp = a_num;
	while ( tmp >= a_denom )
	{
		result.quot++;
		tmp = tmp - a_denom;
	}

	result.rem = tmp;

	if ( sign == -1 )
	{
		result.quot = (-result.quot);
		result.rem  = (-result.rem);
	}

	return result;
}

 
ldiv_t 	ldiv(long num, long denom)
{
	ldiv_t result;
	long tmp;
	long a_num;
	long a_denom;
	long sign;

	a_num = labs( num );
	a_denom = labs( denom );

	sign = -1;
	if ( (num < 0) && (denom < 0) ) sign = 1;
	if ( (num > 0) && (denom > 0) ) sign = 1;

	result.quot = 0;
	result.rem = 0;

	tmp = a_num;
	while ( tmp >= a_denom )
	{
		result.quot++;
		tmp = tmp - a_denom;
	}

	result.rem = tmp;

	if ( sign == -1 )
	{
		result.quot = (-result.quot);
		result.rem  = (-result.rem);
	}

	return result;
}

 
 
int 	atoi(const char* s)
{
	int result;
	int i;

	i = 0;
	result = 0;

	if ( s[i] == '-' ) i++;

	while ( isdigit(s[i]) != 0 )
	{
		result = result * 10 + (s[i] - '0');
		i++;
	}

	if (s[0] == '-') result = -result;

	return result;
}

 
 
// memory allocation routines may be found in alloc



void 	abort()
{
	raise(SIGABRT);
}

 
void 	exit(int status)
{
	int i;
	void (*fcm)(void);

	for ( i = (__UNIX_exitcount - 1); i >= 0; i -- )
	      {
	         fcm = __UNIX_exitfuncs[ i ];
			 fcm();
	      }
	

	int rc;
	if ( os_exit( status, &rc ) == NOTIMPLEMENTED )
			os_freakout();
}

 
int 	atexit(void (*fcm)(void))
{
   if ( __UNIX_exitcount == ATEXIT_MAX ) return -1;

   __UNIX_exitfuncs[ __UNIX_exitcount++ ] = fcm;

   return 0;
}

 
double strtod(const char *str, char **endptr)
{
	double number;
	int exponent;
	int negative;
	char *p = (char *) str;
	double p10;
	int n;
	int num_digits;
	int num_decimals;
	// Skip leading whitespace
	while (isspace(*p)) p++;
	// Handle optional sign
	negative = 0;
	switch (*p) 
	{             
		case '-': negative = 1; // Fall through to increment position
		case '+': p++;
	}
	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;
	// Process string of digits
	while (isdigit(*p))
	{
		number = number * 10. + (*p - '0');
		p++;
		num_digits++;
	}
	// Process decimal part
	if (*p == '.') 
	{
		p++;
		while (isdigit(*p))
		{
			number = number * 10. + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}
		exponent -= num_decimals;
	}
	if (num_digits == 0)
	{
		errno = ERANGE;
		return 0.0;
	}
	// Correct for sign
	if (negative) number = -number;
	// Process an exponent string
	if (*p == 'e' || *p == 'E') 
	{
		// Handle optional sign
		negative = 0;
		switch(*++p) 
		{   
			case '-': negative = 1;   // Fall through to increment pos
			case '+': p++;
		}
		// Process string of digits
		n = 0;
		while (isdigit(*p)) 
		{   
			n = n * 10 + (*p - '0');
			p++;
		}
		if (negative) exponent -= n;
				else exponent += n;
	}
	if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
	{
		errno = ERANGE;
		return HUGE_VAL;
	}
	// Scale the result
	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n) 
	{
		if (n & 1) 
		{
			if (exponent < 0)
				number /= p10;
			else
				number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}

	if (number == HUGE_VAL) errno = ERANGE;
	if (endptr) *endptr = p;

	return number;
}

double atof(const char *str)
{
	return strtod(str, NULL);
}



int 	system(const char* s)
{
	int rc;
	if ( s == NULL ) return -1;

	if ( os_system( s, &rc ) == NOTIMPLEMENTED )
			os_freakout();
	
	return rc;
}

 
char* 	getenv(const char* name)
{
	char* rc;
		
	if ( os_getenv( name, &rc ) == NOTIMPLEMENTED )
			os_freakout();
		
	return rc;
}

 
void* 	bsearch(const void* key, const void* base, size_t n, size_t size, int (*cmp)(const void* keyval, const void* datum))
{
	unsigned int pos = (unsigned int)base;
	void *a;
	int val;

	int middle;
	int top = 0;
	int bottom = (n-1);
	
	while ( top < bottom )
	{
		middle = (bottom + top) / 2;

		if ( (middle == top) || (middle == bottom) ) 
		{
			a = (void*)(pos + top * size);
			if ( cmp(key, a) == 0 ) return a;
			a = (void*)(pos + bottom * size);
			if ( cmp(key, a) == 0 ) return a;

			return NULL;
		}
		
		a = (void*)(pos + middle * size);

		val = cmp( key, a ); 

		if ( val == 0 ) return a;

		if ( val < 0 ) bottom = middle; // key is less than. move bottom up.
			 	  else top = middle;    // key is more than. move top down.
	}

	return NULL;
}

 
void 	qsort(void* base, size_t n, size_t size, int (*cmp)(const void*, const void*))
{
	void *a, *b, *swap;
	int i, j;
	unsigned int pos = (unsigned int)base;
	void *tmp = malloc( size );

	// This is actually a bubble sort. It should be a quick sort. 
	
	for ( i = 0; i < (n-1); i++ )
	{
		a = (void*)(pos + i * size);
		swap = a;

		 for ( j = i+1; j < n; j++ )
		 {
			b = (void*)(pos + j * size);
			if ( cmp( b, swap ) < 0 ) swap = b;
		 }

		 // Swap if we need to.
		if ( swap != a )
		{
			memcpy( tmp, a, size );
			memcpy( a, swap, size );
			memcpy( swap, tmp, size );
		}
	}

	free( tmp );
}

 
int 	rand(void)
{
	return genrand();
}

 
void 	srand(unsigned int seed)
{
	gensrand( seed );
}

 


