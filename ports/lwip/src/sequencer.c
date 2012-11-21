/* sequencer.c - PaulOS embedded operating system
   Copyright (C) 2002, 2003  Paul Sheer

   Rights to copy and modify this program are restricted to strict copyright
   terms. These terms can be found in the file LICENSE distributed with this
   software.

   This software is provided "AS IS" and WITHOUT WARRANTY, either express or
   implied, including, without limitation, the warranties of NON-INFRINGEMENT,
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO
   THE QUALITY OF THIS SOFTWARE IS WITH YOU. Under no circumstances and under
   no legal theory, whether in tort (including negligence), contract, or
   otherwise, shall the copyright owners be liable for any direct, indirect,
   special, incidental, or consequential damages of any character arising as a
   result of the use of this software including, without limitation, damages
   for loss of goodwill, work stoppage, computer failure or malfunction, or any
   and all other commercial damages or losses. This limitation of liability
   shall not apply to liability for death or personal injury resulting from
   copyright owners' negligence to the extent applicable law prohibits such
   limitation. Some jurisdictions do not allow the exclusion or limitation of
   incidental or consequential damages, so this exclusion and limitation may
   not apply to You.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sequencer.h>

void print_factors (gen_t * f, int l);

typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;

static gen_t powmod (gen_t _t, gen_t u, gen_t n)
{
    if (n < 65536) {
	u_int32_t s = 1, t = _t;
	while (u) {
	    if (u & 1)
		s = (gen_t)(s * t) % n;
	    u >>= 1;
	    t = (gen_t)(t * t) % n;
	}
	return (gen_t) s;
    } else {
	u_int64_t s = 1, t = _t;
	while (u) {
	    if (u & 1)
		s = (gen_t)(s * t) % n;
	    u >>= 1;
	    t = (gen_t)(t * t) % n;
	}
	return (gen_t) s;
    }
}

static gen_t sqrt (gen_t n)
{
    gen_t r = 2;
    for (;;) {
	gen_t r_new;
	r_new = ((n / r) + r) / 2;
	switch ((int) (r_new - r)) {
	case -1:
	case 1:
	case 0:
	    return ((n / r_new) + r_new) / 2;	/* one more iteration just to be safe */
	}
	r = r_new;
    }
}

static int factorize (gen_t n, gen_t * factors)
{
    int factors_len = 0;
    gen_t limit, i;
    while (!(n % 2)) {
	factors[factors_len++] = 2;
	n >>= 1;
    }
    limit = sqrt (n + 1);
    i = 3;
    while (i <= limit) {
	if (!(n % i)) {
	    factors[factors_len++] = i;
	    n /= i;
	    limit = sqrt (n + 1);
	} else {
	    i = i + 2;
	}
    }
    if (n != 1)
	factors[factors_len++] = n;
    return factors_len;
}

static int get_generator (int bits, gen_t * prime)
{
    gen_t mask, limit;
    limit = (gen_t) 1L << (bits - 1);
    limit |= limit >> 1;
    mask = (gen_t) ~ 0 >> (sizeof (gen_t) * 8 - bits);
    for (;;) {
	gen_t p;
	int g, i, n_factors;
	gen_t factors[sizeof (gen_t) * 8 + 1];
      retry:
	p = 0;
/* pick a random generator: */
	while ((g = 0x803) < 2);
/* pick a random start for prime search: */
	do {
	    //p = ((p << 16) ^ 0x83) & mask;
	    p = limit + 100;
	} while (p < limit);
/* search for a prime: */
	while (factorize (p, factors) != 1)
	    if (++p >= mask)
		goto retry;
/* got a prime, put g in range: */
	g = g % p;
	if (g < 2)		/* rare */
	    goto retry;
/* do generator test: */
	n_factors = factorize (p - 1, factors);
	for (i = 0; i < n_factors; i++)
	    if (powmod (g, (p - 1) / factors[i], p) == 1)
		goto retry;
	*prime = p;
	return g;
    }
}


/* it seems OpenBSD does a similar thing to me, see
    http://www.usenix.org/publications/library/proceedings/usenix99/full_papers/deraadt/deraadt_html/node17.html
*/
void sequencer_init (struct sequencer *s, int bits)
{
    memset (s, 0, sizeof (*s));
    s->bits = bits;
    s->g = 100;//get_generator (s->bits - 1, &s->prime);
    s->range = (s->prime * 2) / 3;
    s->counter = 0;
    s->field = 1;
    s->mask = (gen_t) ~ 0 >> (sizeof (gen_t) * 8 - bits);
    s->init = 1;
}

/* generates a sequence of numbers. the returned number will
never be returned again within (3 / 8) * (2 / 3) * (1 << bits) calls
to sequencer_next(). The sequence is cryptographically strongly
random so long as random() is strong. Hence use a strong random()
function, and not std libc's. */
gen_t sequencer_next (struct sequencer *s)
{
    gen_t r;
    r = powmod ((gen_t) s->g, s->field, s->prime) + s->counter;
    if (++s->field >= s->range) {
	s->counter += s->prime;	/* move beyond the range of the field */
	s->last_prime = s->prime;
	s->g = get_generator (s->bits - 1, &s->prime);	/* create a new field */
	s->field = 1;
    }
    return r & s->mask;
}

gen_t sequencer_powmod (struct sequencer *s, gen_t u)
{
    return powmod ((gen_t) s->g, u, s->prime);
}

/* Tests follow */
#ifdef STANDALONE
#if 1

void print_factors (gen_t * f, int l)
{
    int i;
    fprintf (stderr, "[");
    for (i = 0; i < l; i++)
	fprintf (stderr, "%lu%s", (unsigned long) f[i], i < l - 1 ? ", " : "");
    fprintf (stderr, "]\n");
}

#define BITS		11

/* acid test - print the minimum spacing beween repeated numbers:
*/
int main (int argc, char **argv)
{
    int n = 2 << BITS;
    int i = 0, j, min = 1 << 30, min_;
    unsigned long *a;
    struct sequencer s;
    a = malloc (n * sizeof (long));
    memset (a, '\0', n * sizeof (long));
    sequencer_init (&s, BITS);
    for (;;) {
	a[i] = sequencer_next (&s);
	j = (i + (n / 2) + 1) % n;
	do {
	    if (a[i] == a[j] && a[i]) {
		min_ = i - j;
		if (min_ < 0)
		    min_ = i - j + n;
		if (min_ < min) {
		    min = min_;
		    printf ("%d:  d = %d, i = %d, j = %d  a[i] = %lu, field = %lu, g%d,p%lu,p'%lu,c%lu\n", 1 << BITS,
			    min, i, j, a[i], (unsigned long) s.field, s.g, (unsigned long) s.prime, (unsigned long) s.last_prime, (unsigned long) s.counter);
		}
	    }
	    j = (j + 1) % n;
	} while (j != i);
	i = (i + 1) % n;
    }
    return 0;
}

#else

void dump_field (int g, gen_t p)
{
    int i;
    for (i = 0; i < p - 1; i++) {
	printf ("%lu\n", powmod (g, i, p));
    }
}

int main (int argc, char **argv)
{
    int g;
    unsigned long *a;
    gen_t prime;
    unsigned long f[100];

    for (;;) {
	int i;
	g = get_generator (11, &prime);
	fprintf (stderr, "g=%d prime=%lu\n", g, prime);
	a = malloc (prime * sizeof (unsigned long));
	for (i = 0; i < prime - 1; i++) {
	    int j;
	    a[i] = powmod (g, i, prime);
	    for (j = 0; j < i; j++)
		if (a[j] == a[i]) {
		    fprintf (stderr, "error %lu(a[%d]) = %lu(a[%d])\n", a[j], j, a[i], i);
		    dump_field (g, prime);
		    exit (1);
		}
	}
	free (a);
    }
    return 0;
}

#endif
#endif

