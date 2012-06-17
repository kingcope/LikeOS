/* udp.c - PaulOS embedded operating system
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

/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Author: Adam Dunkels <adam@sics.se>, Heavily modified by Paul Sheer
 *
 */

#ifdef HAVE_UDP

#include <sequencer.h>
#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/icmp.h"

#include "lwip/stats.h"

#include "arch/perf.h"
#include <assert.h>

/* needed declarations: */
struct netif *lwip_get_internal (void);
long random (void);

/* we use the FNV fast hash with a random initializer: */
#define FNV_32_PRIME 0x01000193
static u32_t fnv_init;

/* hash table of fixed size: */
#ifdef HAVE_LARGE_HASHTABLES
#define UDP_HASH_TABLE_SIZE		1024
#else
#define UDP_HASH_TABLE_SIZE		128
#endif
static struct udp_pcb *udp_pcbs_hash[UDP_HASH_TABLE_SIZE];

/* hash calculation function - simply hashes the port number: */
static inline unsigned int get_hash (unsigned short port)
{
    u32_t fnv = fnv_init;
    fnv ^= port;
    fnv *= FNV_32_PRIME;
    fnv ^= port >> 8;
    fnv *= FNV_32_PRIME;
    return fnv & (UDP_HASH_TABLE_SIZE - 1);
}

/* initializes the hash table to NULL list heads */
void udp_init (void)
{
    int i;
    for (i = 0; i < UDP_HASH_TABLE_SIZE; i++)
	udp_pcbs_hash[i] = NULL;
    fnv_init = random ();
}

/* called from ip_input to insert a UDP packet into the UDP stack: */
void udp_input (struct pbuf *p, struct netif *inp)
{
    struct udp_hdr *udphdr;
    struct udp_pcb *pcb;
    struct udp_pcb *udp_pcbs;
    struct ip_hdr *iphdr;
    u16_t src, dest;

    iphdr = p->payload;
    pbuf_header (p, -(UDP_HLEN + IPH_HL (iphdr) * 4));
    udphdr = (struct udp_hdr *) ((u8_t *) p->payload - UDP_HLEN);

    src = NTOHS (udphdr->src);
    dest = NTOHS (udphdr->dest);

/* find the required list head for this port: */
    udp_pcbs = udp_pcbs_hash[get_hash (dest)];
/* demultiplex packet. perfect matches take priority
   over half-bound connections, so we try them first: */
    for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
	magic (pcb, UDPPCB_MAGIC);
/* FIXME: we don't check internal_only or secure_peer - ???? */
	if (pcb->remote_port == src &&
	    pcb->local_port == dest &&
	    (ip_addr_isany (&pcb->remote_ip) ||
	     ip_addr_cmp (&(pcb->remote_ip), &(iphdr->src))) && (ip_addr_isany (&pcb->local_ip) ||
								 ip_addr_cmp (&(pcb->local_ip),
									      &(iphdr->dest))))
	    break;
    }
/* if still not found, we try search half-bound ports: */
    if (pcb == NULL) {
	for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
	    DEBUGF (UDP_DEBUG, ("udp_input: pcb local port %d (dgram %d)\n", pcb->local_port, dest));
	    if (pcb->local_port == dest &&
		(ip_addr_isany (&pcb->remote_ip) ||
		 ip_addr_cmp (&(pcb->remote_ip), &(iphdr->src))) &&
		(!pcb->internal_only || (unsigned long) lwip_get_internal () == (unsigned long) inp
		 || (inp->name[0] == 'l' && inp->name[1] == 'o') || (pcb->secure_peer.addr
								     && pcb->secure_peer.addr ==
								     iphdr->src.addr))
		&& (ip_addr_isany (&pcb->local_ip) || ip_addr_cmp (&(pcb->local_ip), &(iphdr->dest)))) {
		break;
	    }
	}
    }

/* point ->payload past UDP header for checksum functions: */
    pbuf_header (p, UDP_HLEN);

/* perform checksum verification: */
#ifdef IPv6
    if (iphdr->nexthdr == IP_PROTO_UDPLITE)
#else
    if (IPH_PROTO (iphdr) == IP_PROTO_UDPLITE)
#endif				/* IPv4 */
    {
	/* Do the UDP Lite checksum */
	if (inet_chksum_pseudo (p, (struct ip_addr *) &(iphdr->src),
				(struct ip_addr *) &(iphdr->dest),
				IP_PROTO_UDPLITE, ntohs (udphdr->len)) != 0) {
	    pbuf_free (p);
	    return;
	}
    } else {
	if (udphdr->chksum != 0) {
	    if (inet_chksum_pseudo (p, (struct ip_addr *) &(iphdr->src),
				    (struct ip_addr *) &(iphdr->dest), IP_PROTO_UDP, p->tot_len) != 0) {
		pbuf_free (p);
		return;
	    }
	}
    }

/* rewind back to UDP header: */
    pbuf_header (p, -UDP_HLEN);
    if (pcb != NULL) {
	assert (pcb->recv != NULL);
/* pass the packet to the application: */

//printf ("udp recv: src=%s:%hu dst=%s:%hu\n", (char *) inet_ntoa (iphdr->src), ntohs (src),
//	(char *) inet_ntoa (iphdr->dest), ntohs (dest));

	pcb->recv (pcb->recv_arg, pcb, p, &(iphdr->src), src);
    } else {
/* no match was found, send ICMP destination port
unreachable unless destination address was
broadcast/multicast: */
	if (!ip_addr_isbroadcast (&iphdr->dest, &inp->netmask) && !ip_addr_ismulticast (&iphdr->dest)) {
/* restore payload offset in order to pass the packet back within
the ICMP destination-unreachable packet: */
	    pbuf_header (p, ((unsigned long) p->payload - (unsigned long) iphdr));
	    assert ((unsigned long) p->payload == (unsigned long) iphdr);
	    icmp_dest_unreach (p, ICMP_DUR_PORT, 0);
	}
	pbuf_free (p);
    }
}

void udp_set_default (struct udp_pcb *pcb, struct netif *netif)
{
    pcb->netif = netif;
}

err_t udp_send (struct udp_pcb *pcb, struct pbuf *p)
{
    struct udp_hdr *udphdr;
    struct netif *netif;
    struct ip_addr *src_ip;
    err_t err;
    struct ip_addr gw;
    struct pbuf *q = 0;

    magic (pcb, UDPPCB_MAGIC);

    if (pbuf_header (p, UDP_HLEN)) {
	q = pbuf_alloc (PBUF_IP, UDP_HLEN, PBUF_RAM);
	if (q == NULL) {
	    err = ERR_MEM;
	    goto done;
	}
	pbuf_chain (q, p);
	p = q;
    }

    udphdr = p->payload;
    udphdr->src = htons (pcb->local_port);
    udphdr->dest = htons (pcb->remote_port);
    udphdr->chksum = 0x0000;

    if (pcb->netif && (ip_addr_isany (&pcb->remote_ip) || ip_addr_isallcast (&pcb->remote_ip))) {	/* force null-dest packets through pcb->netif */
	netif = pcb->netif;
    } else if ((netif = ip_route (&(pcb->remote_ip), &gw)) == NULL) {
	DEBUGF (UDP_DEBUG, ("udp_send: No route to 0x%lx\n", pcb->remote_ip.addr));
	err = ERR_RTE;
	goto done;
    }

    if (ip_addr_isany (&pcb->local_ip)) {
	src_ip = &(netif->ip_addr);
    } else {
	src_ip = &(pcb->local_ip);
    }

    if (pcb->flags & UDP_FLAGS_UDPLITE) {
	udphdr->len = htons (pcb->chksum_len);
	/* calculate checksum */
	udphdr->chksum = inet_chksum_pseudo (p, src_ip, &(pcb->remote_ip), IP_PROTO_UDP, pcb->chksum_len);
	if (udphdr->chksum == 0x0000) {
	    udphdr->chksum = 0xffff;
	}
	err = ip_output_if (p, src_ip, &pcb->remote_ip, UDP_TTL, IP_PROTO_UDPLITE, netif, &gw);
    } else {
	udphdr->len = htons (p->tot_len);
	/* calculate checksum */
	if ((pcb->flags & UDP_FLAGS_NOCHKSUM) == 0) {
	    udphdr->chksum = inet_chksum_pseudo (p, src_ip, &pcb->remote_ip, IP_PROTO_UDP, p->tot_len);
	    if (udphdr->chksum == 0x0000) {
		udphdr->chksum = 0xffff;
	    }
	}
	err = ip_output_if (p, src_ip, &pcb->remote_ip, UDP_TTL, IP_PROTO_UDP, netif, &gw);
    }

  done:
    if (q) {
	pbuf_dechain (q);
	pbuf_free (q);
    }
    return err;
}

/* insert UDP PCB into the list of active UDP PCBs: */
static err_t udp_insert (struct udp_pcb *pcb, u16_t local_port)
{
    struct udp_pcb *ipcb;
    struct udp_pcb **udp_pcbs;
/* get the correct list head: */
    udp_pcbs = &udp_pcbs_hash[get_hash (local_port)];
    for (ipcb = *udp_pcbs; ipcb != NULL; ipcb = ipcb->next)
	if (pcb == ipcb)
	    return ERR_OK;	/* already on the list */
/* search if the port is already bound: */
    for (ipcb = *udp_pcbs; ipcb != NULL; ipcb = ipcb->next) {
	if (ipcb->local_port == local_port) {
	    if (ip_addr_isany (&(ipcb->local_ip)) ||
		ip_addr_isany (&(pcb->local_ip)) || ip_addr_cmp (&(ipcb->local_ip), &(pcb->local_ip))) {
		return ERR_USE;
	    }
	}
    }
/* insert onto the list: */
    pcb->next = *udp_pcbs;
    *udp_pcbs = pcb;
/* (1) this is the only place where the local_port member
is set, hence if ->local_port is non-zero, the pcb is sure
to be in the list: */
    assert (!pcb->local_port);
    pcb->local_port = local_port;
    return ERR_OK;
}

/* wrapper to call udp_insert */
err_t udp_bind (struct udp_pcb * pcb, struct ip_addr * ipaddr, u16_t port)
{
    magic (pcb, UDPPCB_MAGIC);

    ip_addr_set (&pcb->local_ip, ipaddr);

    DEBUGF (UDP_DEBUG, ("udp_bind: bound to port %d\n", port));
    return udp_insert (pcb, port);
}

/* binds to both local port and a remote port */
err_t udp_connect (struct udp_pcb * pcb, struct ip_addr * ipaddr, u16_t port)
{
    static struct sequencer s = { 0 };
    u16_t sport;
    err_t r;

    if (!s.init)
	sequencer_init (&s, 16 /* 0 - 65535 */ );

    magic (pcb, UDPPCB_MAGIC);

    ip_addr_set (&pcb->remote_ip, ipaddr);
    pcb->remote_port = port;

    if (pcb->local_port)	/* already in the list - see (1) */
	return ERR_OK;

    do {
	sport = (u16_t) sequencer_next (&s);
    } while ((r = udp_insert (pcb, sport)) == ERR_USE);

    return r;
}

/* set the callback used by udp_input */
void
udp_recv (struct udp_pcb *pcb,
	  void (*recv) (void *arg, struct udp_pcb * upcb, struct pbuf * p,
			struct ip_addr * addr, u16_t port), void *recv_arg)
{
    magic (pcb, UDPPCB_MAGIC);
    pcb->recv = recv;
    pcb->recv_arg = recv_arg;
}

/* unlink and free the UDP struct */
void udp_remove (struct udp_pcb *pcb)
{
    struct udp_pcb **udp_pcbs;

    magic (pcb, UDPPCB_MAGIC);

    udp_pcbs = &udp_pcbs_hash[get_hash (pcb->local_port)];
    if (*udp_pcbs == pcb) {
	*udp_pcbs = (*udp_pcbs)->next;
    } else {
	struct udp_pcb *pcb2;
	for (pcb2 = *udp_pcbs; pcb2 != NULL; pcb2 = pcb2->next)
	    if (pcb2->next == pcb)
		pcb2->next = pcb2->next->next;
    }

#ifdef HAVE_MAGIC
    pcb->magic = 0;
#endif
    memp_free (MEMP_UDP_PCB, pcb);
}

/* allocate a new UDP structure */
struct udp_pcb *udp_new (void)
{
    struct udp_pcb *pcb;
    pcb = memp_malloc (MEMP_UDP_PCB);
    if (pcb != NULL) {
	bzero (pcb, sizeof (struct udp_pcb));
#ifdef HAVE_MAGIC
	pcb->magic = UDPPCB_MAGIC;
#endif
	return pcb;
    }
    return NULL;
}

int udp_debug_print (struct udp_hdr *udphdr)
{
    return 0;
}

#endif				/* HAVE_UDP */

