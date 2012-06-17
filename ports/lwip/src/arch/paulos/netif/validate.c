/* validate.c - PaulOS embedded operating system
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


#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/icmp.h"
#include "lwip/tcp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "netif/paulosif.h"
#include "netif/validate.h"

#include <sys/types.h>
#include <unistd.h>
#include <socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <../../../../libc/local.h>

// #define ERRORIF(c)   do { if (c) { if (!eth_addr_match(&ethhdr->dest, &ethbroadcast)) printf ("%s:%d: \"%s\"\n", __FUNCTION__, __LINE__, #c); return 1; } } while (0)
#define ERRORIF(c)	do { if (c) return 1; } while (0)

static const struct eth_addr ethbroadcast = { {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} };

static inline int eth_addr_match (const struct eth_addr *a, const struct eth_addr *b)
{
    if (a->addr[0] == b->addr[0] &&
	a->addr[1] == b->addr[1] &&
	a->addr[2] == b->addr[2] &&
	a->addr[3] == b->addr[3] && a->addr[4] == b->addr[4] && a->addr[5] == b->addr[5])
	return 1;
    return 0;
}

/* returns 1 on fault */
int packet_validate_buf (unsigned char *q, int len, struct netif *inp, struct genericif *genericif)
{
    int l, hl;
    struct ip_hdr *iphdr;
    struct tcp_hdr *tcphdr;
    struct icmp_echo_hdr *icmp;
#ifdef HAVE_UDP
    struct udp_hdr *udphdr;
#endif
    struct eth_hdr *ethhdr = 0;

    if (genericif) {
/* initial length must be at least Ethernet */
	ERRORIF (len < 14);
/* now advance over Ethernet */
	ethhdr = (struct eth_hdr *) q;
#ifndef HAVE_MULTICAST		/* FIXME: support multicast */
	ERRORIF (!eth_addr_match (&ethhdr->dest, &ethbroadcast)
		 && !eth_addr_match (&ethhdr->dest, genericif->ethaddr));
#endif
	q += 14;
	len -= 14;
	switch (htons (ethhdr->type)) {
	case ETHTYPE_IP:
	    break;
	case ETHTYPE_ARP:
	    return 0;
	default:
	    ERRORIF (1);
	}
    }

    ERRORIF (len < 20);
    iphdr = (struct ip_hdr *) q;
/* and check IP header: */
    hl = IPH_HL (iphdr) * 4;
    ERRORIF (hl < 20);
/* FIXME: the check pbuf_alloc(PBUF_TRANSPORT) in the packet forwarding code: if we have iphdr > 40 bytes, it breaks: */
    ERRORIF (hl > 40);
    ERRORIF (len < hl);
    ERRORIF (IPH_V (iphdr) != 4);
    ERRORIF (IPH_TTL (iphdr) == 0);
    l = ntohs (IPH_LEN (iphdr));
    ERRORIF (len < l);

    ERRORIF (inet_chksum (iphdr, hl) != 0);

/* latter parts of a fragmented packet */
    if ((IPH_OFFSET (iphdr) & htons (IP_OFFMASK | IP_MF)) != 0)
	return 0;

/* now advance over IP */
    q += IPH_HL (iphdr) * 4;
    len -= IPH_HL (iphdr) * 4;

    switch (IPH_PROTO (iphdr)) {
    case IP_PROTO_ESP:
	ERRORIF (len < 32);	/* ESP header is 8 bytes, TCP + trailer is at least 24 bytes, and IP header might be missing. */
	break;
    case IP_PROTO_UDP:
#ifdef HAVE_UDP
	udphdr = (struct udp_hdr *) q;
	ERRORIF (ntohs (udphdr->len) < 8);
	ERRORIF (len < ntohs (udphdr->len));
#else
	ERRORIF (1);
#endif
	break;
    case IP_PROTO_TCP:
	tcphdr = (struct tcp_hdr *) q;
//printf ("#%04x\n", (unsigned int) tcphdr->chksum);
	ERRORIF (TCPH_OFFSET (tcphdr) < 5);
	ERRORIF (len < (TCPH_OFFSET (tcphdr) >> 4) * 4);
/* no such thing as TCP to or from a multicast address */
	if (inp) {
	    ERRORIF (ip_addr_isbroadcast (&iphdr->dest, &inp->netmask) || ip_addr_ismulticast (&iphdr->dest));
	    ERRORIF (ip_addr_isbroadcast (&iphdr->src, &inp->netmask) || ip_addr_ismulticast (&iphdr->src));
	}
	if (ethhdr) {
	    ERRORIF (ethhdr->src.addr[0] == 0x01 && ethhdr->src.addr[1] == 0x00
		     && ethhdr->src.addr[2] == 0x5e);
	    ERRORIF (ethhdr->src.addr[0] == 0xff && ethhdr->src.addr[1] == 0xff
		     && ethhdr->src.addr[2] == 0xff && ethhdr->src.addr[3] == 0xff
		     && ethhdr->src.addr[4] == 0xff && ethhdr->src.addr[5] == 0xff);
	    ERRORIF (ethhdr->dest.addr[0] == 0x01 && ethhdr->dest.addr[1] == 0x00
		     && ethhdr->dest.addr[2] == 0x5e);
	    ERRORIF (ethhdr->dest.addr[0] == 0xff && ethhdr->dest.addr[1] == 0xff
		     && ethhdr->dest.addr[2] == 0xff && ethhdr->dest.addr[3] == 0xff
		     && ethhdr->dest.addr[4] == 0xff && ethhdr->dest.addr[5] == 0xff);
	}
	break;
    case IP_PROTO_ICMP:
	icmp = (struct icmp_echo_hdr *) q;
	ERRORIF (len < sizeof (struct icmp_echo_hdr));
	ERRORIF (ICMPH_TYPE (icmp) > NR_ICMP_TYPES);
	break;
    default:
	ERRORIF (1);
    }
    return 0;
}

/* returns 1 on fault */
int packet_validate (struct pbuf *p, struct netif *inp, struct genericif *genericif)
{
    return packet_validate_buf (p->payload, p->tot_len, inp, genericif);
}

