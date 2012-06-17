/* ipreass.c - PaulOS embedded operating system
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

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip.h"
#include "lwip/icmp.h"
#include "lwip/inet.h"
#include "lwip/netif.h"

#include <sys/types.h>

struct reassembly {
    struct pbuf *next;
    struct ip_hdr *iphdr;
    u8_t t;
    u8_t pad0;
    u8_t pad1;
    u8_t pad2;
};

u8_t reass_time = 0;

#define REASSEMBLY_STATES	10

static struct reassembly r[REASSEMBLY_STATES] =
    { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };

#define REASS_OFFSET(iphdr)		((ntohs (IPH_OFFSET (iphdr)) & IP_OFFMASK) << 3)
#define REASS_MORE(iphdr)		 (ntohs (IPH_OFFSET (iphdr)) & IP_MF)
#define REASS_HLEN(iphdr)		 (IPH_HL (iphdr) * 4)
#define REASS_FRAGLEN(iphdr)		 (ntohs (IPH_LEN (iphdr)) - REASS_HLEN (iphdr))
#define REASS_CMP(a,b)			((a)->dest.addr == (b)->dest.addr && \
					 (a)->src.addr  == (b)->src.addr  && IPH_ID (a) == IPH_ID (b))

static inline void reass_insert (struct reassembly *r, struct pbuf *p)
{
    u16_t offset;
    struct pbuf *q;
    offset = REASS_OFFSET ((struct ip_hdr *) p->payload);
    for (q = (struct pbuf *) r; q->next; q = q->next)
	if (offset < REASS_OFFSET ((struct ip_hdr *) q->next->payload)) {
	    p->next = q->next;
	    break;
	}
    q->next = p;
    if (!offset)		/* top packet is the header we copy */
	r->iphdr = (struct ip_hdr *) p->payload;
}

static inline u8_t reass_complete (struct pbuf *p)
{
    u16_t offset;
    for (offset = 0; p; p = p->next) {
	struct ip_hdr *h;
	h = (struct ip_hdr *) p->payload;
	if (offset != REASS_OFFSET (h))
	    return 0;
	if (!REASS_MORE (h))
	    return 1;
	offset += REASS_FRAGLEN (h);
    }
    return 0;
}

static inline u16_t reass_length (struct pbuf *p)
{
    u16_t offset;
    for (offset = 0; p; p = p->next)
	offset += REASS_FRAGLEN ((struct ip_hdr *) p->payload);
    return offset;
}

static inline void reass_copy (char *buf, struct pbuf *p)
{
    for (; p; p = p->next) {
	u16_t l;
	l = REASS_FRAGLEN ((struct ip_hdr *) p->payload);
	memcpy (buf, (char *) p->payload + REASS_HLEN ((struct ip_hdr *) p->payload), (size_t) l);
	buf += l;
    }
}

static inline void reass_free (struct reassembly *q)
{
    struct pbuf *p, *next;
    for (p = q->next; p; p = next) {
	next = p->next;
	p->next = NULL;
	pbuf_free (p);
    }
    q->iphdr = NULL;
    q->next = NULL;
}

struct pbuf *ip_reass (struct pbuf *p)
{
    u8_t i;
    if (p->next) {
/* PaulOS does not receive chained packets via ip_input: i.e.
the network devices always return monolithic packets: */
	printf ("***WARNING*** how did we get a chained packet?");
	pbuf_free (p);
	return NULL;
    }
    for (i = 0; i < REASSEMBLY_STATES; i++) {
	struct reassembly *q;
	q = &r[i];
	if (q->iphdr && REASS_CMP (q->iphdr, (struct ip_hdr *) p->payload)) {
	    reass_insert (q, p);
	    if (reass_complete (q->next)) {
		u16_t hlen, len;
		len = reass_length (q->next);
/* PaulOS pbuf_alloc always returns a monolithic packet: */
		p = pbuf_alloc (PBUF_TRANSPORT, len, PBUF_RAM);
		reass_copy (p->payload, q->next);
		hlen = REASS_HLEN (q->iphdr);
		if (pbuf_header (p, hlen)) {
		    pbuf_free (p);
		    reass_free (q);
		    return NULL;
		} else {
		    struct ip_hdr *iphdr;
		    iphdr = p->payload;
		    memcpy (iphdr, q->iphdr, (size_t) hlen);
		    reass_free (q);
		    IPH_ID_SET (iphdr, (u16_t) 0);
		    IPH_OFFSET_SET (iphdr, (u16_t) 0);
		    IPH_LEN_SET (iphdr, htons ((len + hlen)));
		    IPH_CHKSUM_SET (iphdr, 0);
		    IPH_CHKSUM_SET (iphdr, inet_chksum (iphdr, hlen));
		    return p;
		}
	    } else {
		return NULL;
	    }
	}
    }
/* new entry */
    for (i = 0; i < REASSEMBLY_STATES; i++) {
	struct reassembly *q;
	q = &r[i];
	if (!q->iphdr) {
	    q->iphdr = p->payload;
	    q->next = p;
	    q->t = reass_time;
	    return NULL;
	}
    }
/* no space - just drop it */
    pbuf_free (p);
    return NULL;
}

/* run from tcp_tmr() - every 500ms */
void reass_tmr (void)
{
    u8_t i;
    reass_time++;
    for (i = 0; i < REASSEMBLY_STATES; i++) {
	struct reassembly *q;
	q = &r[i];
	if (!q->iphdr)
	    continue;
	if ((u8_t) reass_time - q->t > (u8_t) 5 /* 2.5 seconds */ ) {
/* RFC-792: "If fragment zero is not available then no time exceeded need be sent at all.": */
	    if (REASS_OFFSET ((struct ip_hdr *) q->next->payload) == 0)
		icmp_time_exceeded (q->next, ICMP_TE_FRAG);
	    reass_free (q);
	}
    }
}


