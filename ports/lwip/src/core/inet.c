/* inet.c - PaulOS embedded operating system
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

#include "lwip/debug.h"
#include "lwip/arch.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"

#define REDUCE(s)					\
    do {						\
	(s) = ((s) >> 16) + ((s) & 0xffff);		\
	if ((s) > 0xffff)				\
	    (s) -= 0xffff;				\
    } while (0)

u16_t inet_chksum_pbuf (struct pbuf *p)
{
    return inet_chksum_pseudo (p, NULL, NULL, 0, 0);
}

u16_t inet_chksum (void *dataptr, u16_t len)
{
    struct pbuf p;
    p.len = p.tot_len = len;
    p.payload = dataptr;
    p.next = NULL;
    return inet_chksum_pseudo (&p, NULL, NULL, 0, 0);
}

u16_t inet_chksum_pseudo (struct pbuf * m, struct ip_addr * src, struct ip_addr * dest, u8_t proto,
			  u16_t proto_len)
{
    register u16_t *w;
    register u32_t sum = 0;
    register int mlen = 0;
    u8_t byte_swapped = 0;
    union {
	u8_t c[2];
	u16_t s;
    } s_util;

    for (; m != NULL; m = m->next) {
	if (m->len == 0)
	    continue;

	w = (u16_t *) m->payload;
	if (mlen == -1) {
	    /*
	     * The first byte of this mbuf is the continuation
	     * of a word spanning between this mbuf and the
	     * last mbuf.
	     *
	     * s_util.c[0] is already saved when scanning previous 
	     * mbuf.
	     */
	    s_util.c[1] = *(u8_t *) w;
	    sum += s_util.s;
	    w = (u16_t *) ((u8_t *) w + 1);
	    mlen = m->len - 1;
	} else
	    mlen = m->len;
	/*
	 * Force to even boundary.
	 */
	if ((1 & (long) w) && (mlen > 0)) {
	    REDUCE (sum);
	    sum <<= 8;
	    s_util.c[0] = *(u8_t *) w;
	    w = (u16_t *) ((u8_t *) w + 1);
	    mlen--;
	    byte_swapped = 1;
	}
	/*
	 * Unroll the loop to make overhead from
	 * branches &c small.
	 */
#if defined(PAULOS_ARCH_ATMEL) || defined(PAULOS_ARCH_SAMSUNG)

	if ((2 & (long) w) && (mlen >= 2)) {
	    sum += *w++;
	    mlen -= 2;
	}

	asm volatile ("\n\
	mov	r5, #0xFF00\n\
	orr	r5, r5, #0x00FF\n\
	sub	%2, %2, #32\n\
	cmp	%2, #0\n\
	blt	.skip\n\
\n\
.loop:\n\
	ldmia	%1!, {r6, r7, r8, r9}\n\
	add	%0, %0, r6, lsr #16\n\
	add	%0, %0, r7, lsr #16\n\
	add	%0, %0, r8, lsr #16\n\
	add	%0, %0, r9, lsr #16\n\
	and	r6, r6, r5\n\
	and	r7, r7, r5\n\
	and	r8, r8, r5\n\
	and	r9, r9, r5\n\
	add	%0, %0, r6\n\
	add	%0, %0, r7\n\
	add	%0, %0, r8\n\
	add	%0, %0, r9\n\
\n\
	sub	%2, %2, #32\n\
\n\
	ldmia	%1!, {r6, r7, r8, r9}\n\
	add	%0, %0, r6, lsr #16\n\
	add	%0, %0, r7, lsr #16\n\
	add	%0, %0, r8, lsr #16\n\
	add	%0, %0, r9, lsr #16\n\
	and	r6, r6, r5\n\
	and	r7, r7, r5\n\
	and	r8, r8, r5\n\
	and	r9, r9, r5\n\
	add	%0, %0, r6\n\
	add	%0, %0, r7\n\
	add	%0, %0, r8\n\
	add	%0, %0, r9\n\
\n\
	cmp	%2, #0\n\
	bge	.loop\n\
\n\
.skip:\n\
":"=r" (sum), "=r" (w), "=r" (mlen):"0" (sum), "1" (w), "2" (mlen):"r5", "r6", "r7", "r8", "r9");

#else

	while ((mlen -= 32) >= 0) {
	    sum += w[0];
	    sum += w[1];
	    sum += w[2];
	    sum += w[3];
	    sum += w[4];
	    sum += w[5];
	    sum += w[6];
	    sum += w[7];
	    sum += w[8];
	    sum += w[9];
	    sum += w[10];
	    sum += w[11];
	    sum += w[12];
	    sum += w[13];
	    sum += w[14];
	    sum += w[15];
	    w += 16;
	}

#endif
	mlen += 32;
	while ((mlen -= 8) >= 0) {
	    sum += w[0];
	    sum += w[1];
	    sum += w[2];
	    sum += w[3];
	    w += 4;
	}
	mlen += 8;
	if (mlen == 0 && byte_swapped == 0)
	    continue;
	REDUCE (sum);
	while ((mlen -= 2) >= 0) {
	    sum += *w++;
	}
	if (byte_swapped) {
	    REDUCE (sum);
	    sum <<= 8;
	    byte_swapped = 0;
	    if (mlen == -1) {
		s_util.c[1] = *(u8_t *) w;
		sum += s_util.s;
		mlen = 0;
	    } else
		mlen = -1;
	} else if (mlen == -1)
	    s_util.c[0] = *(u8_t *) w;
    }
    if (mlen == -1) {
	/* The last mbuf has odd # of bytes. Follow the
	   standard (the odd byte may be shifted left by 8 bits
	   or not as determined by endian-ness of the machine) */
	s_util.c[1] = 0;
	sum += s_util.s;
    }

    if (src) {
	sum += (src->addr & 0xffff);
	sum += ((src->addr >> 16) & 0xffff);
	sum += (dest->addr & 0xffff);
	sum += ((dest->addr >> 16) & 0xffff);
	sum += (u32_t) htons ((u16_t) proto);
	sum += (u32_t) htons (proto_len);
    }

    REDUCE (sum);

    return (u16_t) (~sum & 0xffff);
}


