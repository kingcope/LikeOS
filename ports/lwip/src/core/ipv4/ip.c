/* ip.c - PaulOS embedded operating system
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
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ip.c,v 1.5 2003/09/10 09:51:25 psheer Exp $
 */


/*-----------------------------------------------------------------------------------*/
/* ip.c
 *
 * This is the code for the IP layer.
 *
 */   
/*-----------------------------------------------------------------------------------*/

#include <sequencer.h>
#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/icmp.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#ifdef HAVE_IPSEC
#include "lwip/ipsec.h"
#endif		/* HAVE_IPSEC */

#include "lwip/stats.h"

#include "arch/perf.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif /* LWIP_DHCP */
#include <sys/types.h>
#include <assert.h>

char *inet_ntoa (struct ip_addr in);

/*-----------------------------------------------------------------------------------*/
/* ip_init:
 *
 * Initializes the IP layer.
 */
/*-----------------------------------------------------------------------------------*/
void
ip_init(void)
{
}
/*-----------------------------------------------------------------------------------*/
/* ip_route:
 *
 * Finds the appropriate network interface for a given IP address. It
 * searches the list of network interfaces linearly. A match is found
 * if the masked IP address of the network interface equals the masked
 * IP address given to the function.
 */
#ifdef __PAULOS__
static struct netif *(*lwip_special_route) (struct ip_addr *, struct ip_addr *) = NULL;
void
lwip_set_special_route (struct netif *(*new_special_route)(struct ip_addr *, struct ip_addr *))
{
  lwip_special_route = new_special_route;
}

static int (*lwip_smartfilter) (unsigned char *, int) = NULL;
void
lwip_set_smartfilter (int (*new_smartfilter)(unsigned char *, int))
{
  lwip_smartfilter = new_smartfilter;
}
#endif
/*-----------------------------------------------------------------------------------*/
static struct netif *
ip_route_ipsec(struct ip_addr *dest, struct ip_addr *gw, u8_t ipsec)
{
  struct netif *netif;

#ifdef __PAULOS__
  assert (gw);
  gw->addr = 0;	/* default to the local network */
#endif

  for(netif = netif_list; netif != NULL; netif = netif->next)
    if(ip_addr_maskcmp(dest, &(netif->ip_addr), &(netif->netmask)))
      return netif;

#ifdef __PAULOS__
#ifdef HAVE_IPSEC
  if (ipsec)
    if ((netif = sa_lookup_by_dest (dest)))
      return netif;	/* gw is empty since these netif's do their own routing */
#endif

  if (lwip_special_route)
    netif = (*lwip_special_route) (dest, gw);
  if (!netif) {
    netif = netif_default;
    *gw = netif->gw;
  }
  if (gw->addr == netif->ip_addr.addr) {		/* sanity check */
printf ("gw address same as netif address, aborting route.\n");
    return NULL;
  }
  return netif;
#else
  return netif_default;
#endif
}

struct netif *
ip_route(struct ip_addr *dest, struct ip_addr *gw)
{
  return ip_route_ipsec (dest, gw, 1);
}

struct netif *
ip_route_no_ipsec(struct ip_addr *dest, struct ip_addr *gw)
{
  return ip_route_ipsec (dest, gw, 0);
}

#ifndef IP_FORWARD
#error
#endif
#if !IP_FORWARD
#error
#endif

/*-----------------------------------------------------------------------------------*/
static err_t
lwip_decrement_ttl(struct pbuf *p, struct ip_hdr *iphdr)
{
//printf ("(%d) TTL %d -> %d\n", *((u8_t *)p->payload + IPH_HL(iphdr) * 4/sizeof(u8_t)),
//	(int) IPH_TTL(iphdr), (int) IPH_TTL(iphdr) - 1);

  /* Decrement TTL and send ICMP if ttl == 0. */
  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);

  /* Incremental update of the IP checksum. */
  if(IPH_CHKSUM(iphdr) >= htons(0xffff - 0x100)) {
    IPH_CHKSUM_SET(iphdr, IPH_CHKSUM(iphdr) + htons(0x100) + 1);
  } else {
    IPH_CHKSUM_SET(iphdr, IPH_CHKSUM(iphdr) + htons(0x100));
  }

  if(IPH_TTL(iphdr) == 0) {
    /* Special case: ICMP echo requests can be used for traceroute's: */
    if(IPH_PROTO(iphdr) == IP_PROTO_ICMP &&
	*((u8_t *)p->payload + IPH_HL(iphdr) * 4/sizeof(u8_t)) == ICMP_ECHO) {
      icmp_time_exceeded(p, ICMP_TE_TTL);
/* Don't send ICMP messages in response to ICMP messages: */
    } else if(IPH_PROTO(iphdr) != IP_PROTO_ICMP) {
      icmp_time_exceeded(p, ICMP_TE_TTL);
    }
    return 1;
  }
  return 0;
}

#if IP_FORWARD
/*-----------------------------------------------------------------------------------*/
/* ip_forward:
 *
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 */
/*-----------------------------------------------------------------------------------*/
#ifdef __PAULOS__
err_t
#else
static err_t
#endif
ip_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp)
{
#ifdef __PAULOS__
  struct netif *netif;
  static int nest = 0;
  struct ip_addr gw;
#else
  static struct netif *netif;
#endif

  if (nest) {
    printf ("WARNING: nested call to ip_forward");
    return ERR_RTE;
  }

  nest++;
  
  PERF_START;
  
  if((netif = ip_route((struct ip_addr *)&(iphdr->dest), &gw)) == NULL) {
    DEBUGF(IP_DEBUG, ("ip_forward: no forwarding route for 0x%lx found\n",
		      iphdr->dest.addr));
printf ("ip_forward: no route\n");
    nest--;
    return ERR_RTE;
  }

  /* Don't forward packets onto the same network interface on which
     they arrived. */
  if(netif == inp) {
//printf("input iface = output iface, so checking...\n");
/* a packet going to a different network (like through a
alternate gateway) is allowed: */
    if (gw.addr && (netif->name[0] == 'e' && netif->name[1] == 't')) {

/* FIXME: what if a gateway is forwarding to the wrong
place. We need to send the *gateway* the ICMP packet, NOT
the src host. */

//printf ("ICMP redirect: packet(src=%s dst=%s) in_iface %c%c, out_iface %c%c m=%s a=%s gw=%s\n",
//inet_ntoa (iphdr->src), inet_ntoa (iphdr->dest), inp->name[0], inp->name[1], netif->name[0], netif->name[1],
//inet_ntoa (netif->netmask), (char *) inet_ntoa (netif->ip_addr), (char *) inet_ntoa (gw));

      icmp_redirect (iphdr, netif, &gw);
    } else {
      DEBUGF(IP_DEBUG, ("ip_forward: not forward packets back on incoming interface.\n"));
      nest--;
      return ERR_RTE;
    }
  }

  if (lwip_decrement_ttl (p, iphdr)) {
    nest--;
    return ERR_RTE;       
  }

  DEBUGF(IP_DEBUG, ("ip_forward: forwarding packet to 0x%lx\n",
		    iphdr->dest.addr));

#ifdef IP_STATS
  ++stats.ip.fw;
  ++stats.ip.xmit;
#endif /* IP_STATS */

  PERF_STOP("ip_forward");

#ifdef __PAULOS__
  if (lwip_smartfilter)
    if ((*lwip_smartfilter) ((unsigned char *) iphdr, (int) ntohs(IPH_LEN(iphdr)))) {
      nest--;
      return ERR_RTE;
    }
#endif

#ifndef __PAULOS__
  netif->output(netif, p, gw.addr ? &gw : (struct ip_addr *)&(iphdr->dest));
#else
  ip_output_if(p, NULL, IP_HDRINCL, 0, 0, netif, &gw);
#endif

  nest--;
  return ERR_OK;
}
#endif /* IP_FORWARD */

/* utility function used from outside of LwIP to
determine if an address is routable through an interface */
int
lwip_check_route (struct ip_addr *dest, struct netif *netif)
{
  assert (netif);
  if ((dest->addr & netif->netmask.addr) == (netif->ip_addr.addr & netif->netmask.addr))
    return 1;
  else
    return 0;
}

/* utility function used from outside of LwIP to
determine if an address is local */
int
lwip_is_local_ip (struct ip_addr *dest)
{
  struct netif *netif;
  for(netif = netif_list; netif != NULL; netif = netif->next) {
    if(ip_addr_isany(&(netif->ip_addr)) ||
       ip_addr_cmp(dest, &(netif->ip_addr)) ||
        (ip_addr_isbroadcast(dest, &(netif->netmask)) &&
	ip_addr_maskcmp(dest, &(netif->ip_addr), &(netif->netmask))) ||
       ip_addr_cmp(dest, IP_ADDR_BROADCAST)) {
      return 1;
    }
  }
  return 0;
}

/* utility function used from outside of LwIP to
determine if an address is local to the segment */
int
lwip_is_on_subnet (struct netif *netif, struct ip_addr *dest)
{
  return ip_addr_maskcmp(dest, &(netif->ip_addr), &(netif->netmask));
}

/* utility function used from outside of LwIP to
determine if an address will route through to the internal LAN */
int
lwip_is_internal_ip (struct ip_addr *dest)
{
  struct netif *netif;
  struct ip_addr gw;
  if (lwip_is_local_ip (dest))
    return 0;
  gw.addr = 0;
  if (!(netif = ip_route_no_ipsec (dest, &gw)))
    return 0;
/* if all other routes failed to match, netif_default would
have been returned by ip_route. in this case, dest must be
an arbitrary (i.e. non-local) Internet address: */
  if ((unsigned long) netif == (unsigned long) netif_default)
    return 0;
/* if a proper gw has been returned, and that gw matches the
gateway of the default route, then a packet of IP "dest"
would be forwarded via the same route as an arbitrary
Internet address. In this case, again, we presume dest to be
an arbitrary (i.e. non-local) Internet address: */
  if (gw.addr && gw.addr == netif_default->gw.addr)
    return 0;
  return 1;
}

/* utility function used from outside of LwIP to
get the internal interface */
struct netif *
lwip_get_internal (void)
{
  struct netif *netif;
  if (!netif_default)	/* an interface is not "internal" unless there is a default interface - by definition */
    return NULL;
  for(netif = netif_list; netif != NULL; netif = netif->next)
    if ((unsigned long) netif != (unsigned long) netif_default &&
     !(netif->name[0] == 'l' && netif->name[1] == 'o'))
      return netif;
  return NULL;
}

/* utility function used from outside of LwIP to
get a named interface */
struct netif *
lwip_get_iface (char *name)
{
  struct netif *netif;
  for(netif = netif_list; netif != NULL; netif = netif->next)
    if (netif->name[0] == name[0] && netif->name[1] == name[1])
      return netif;
  return NULL;
}

/* utility function used from outside of LwIP to get the low level driver device descriptor */
int
lwip_get_iface_fd (char *name, unsigned char *hwaddr)
{
  struct netif *netif;
  for(netif = netif_list; netif != NULL; netif = netif->next)
    if (netif->name[0] == name[0] && netif->name[1] == name[1]) {
      if (hwaddr)
        memcpy (hwaddr, netif->hwaddr, (size_t) 6);
      return netif->fd;
    }
  return -1;
}

/* utility function used from outside of LwIP to
get the ip address of the internal interface */
struct ip_addr *
lwip_get_internal_ip (void)
{
  struct netif *netif;
  netif = lwip_get_internal ();
  return netif ? &netif->ip_addr : NULL;
}

/* utility function used from outside of LwIP to
get the ip netmask of the internal interface */
struct ip_addr *
lwip_get_internal_netmask (void)
{
  struct netif *netif;
  netif = lwip_get_internal ();
  return netif ? &netif->netmask : NULL;
}

/*-----------------------------------------------------------------------------------*/
/* ip_input:
 *
 * This function is called by the network interface device driver when
 * an IP packet is received. The function does the basic checks of the
 * IP header such as packet size being at least larger than the header
 * size etc. If the packet was not destined for us, the packet is
 * forwarded (using ip_forward). The IP checksum is always checked.
 *
 * Finally, the packet is sent to the upper layer protocol input function.
 */

/* This function is re-written for PaulOS: */
/*-----------------------------------------------------------------------------------*/
err_t
ip_input(struct pbuf *p, struct netif *inp) {
  static int nest = 0;
  struct ip_hdr *iphdr;
  struct netif *netif;
  u8_t hl;

  if (nest) {
    //__print_trace_back (__FILE__, __FUNCTION__, __LINE__);
    return ERR_RTE;
  }
  nest++;

#if LWIP_DHCP
#error LWIP_DHCP
#endif

#if IP_OPTIONS == 0
#error IP_OPTIONS
#endif

/* IPSEC re-input of a tunneled IP datagram begins here: */
#ifdef HAVE_IPSEC
restart:
#endif		/* HAVE_IPSEC */

  /* identify the IP header */
  iphdr = p->payload;
  assert (!((unsigned long) iphdr & 3));

  hl = IPH_HL(iphdr);
  
  /* Trim pbuf. This should have been done at the netif layer,
     but we'll do it anyway just to be sure that its done. */
  pbuf_realloc(p, ntohs(IPH_LEN(iphdr)));

  /* is this packet for us? */
  for(netif = netif_list; netif != NULL; netif = netif->next) {
    if(ip_addr_isany(&(netif->ip_addr)) ||
       ip_addr_cmp(&(iphdr->dest), &(netif->ip_addr)) ||
        (ip_addr_isbroadcast(&(iphdr->dest), &(netif->netmask)) &&
	ip_addr_maskcmp(&(iphdr->dest), &(netif->ip_addr), &(netif->netmask))) ||
       ip_addr_cmp(&(iphdr->dest), IP_ADDR_BROADCAST)) {
      break;
    }
  }

/* packet not for us, route or discard */
  if(netif == NULL) {
    err_t err = ERR_RTE;

/* check if this is an ICMP redirect - we don't forward these: */
    if (p) {
      if (IPH_PROTO(iphdr) == IP_PROTO_ICMP) {
	if (*((u8_t *)p->payload + hl * 4) == ICMP_RD) {
	  pbuf_free(p);
	  p = NULL;
	}
      }
    }

/* check if this is a broadcast - we don't forward these either: */
    if (p) {
      if(ip_addr_isbroadcast(&(iphdr->dest), &(inp->netmask))) {
        pbuf_free(p);
	p = NULL;
      }
    }

/* try forward */
    if (p) {
      err = ip_forward(p, iphdr, inp);
      pbuf_free(p);
      p = NULL;
    }

/* <--- add additional rules here before this last block */
    if (p) {
      pbuf_free(p);
      p = NULL;
    }
    nest--;
    return err;
  }

/* reassemble fragmented packets: */
  if((IPH_OFFSET(iphdr) & htons(IP_OFFMASK | IP_MF)) != 0) {
    p = (struct pbuf *) ip_reass(p);
    if(p == NULL) {
      nest--;
      return ERR_OK;
    }
    iphdr = p->payload;
  }

  switch(IPH_PROTO(iphdr)) {
#ifdef HAVE_IPSEC
  case IP_PROTO_ESP:
/* IPSEC packets - decrypt and extract tunneled packet, then
reinsert into from top of this function: */
    p = ipsec_tunnel_decrypt(p, &inp);
    if(!p)
      break;
    goto restart;
#endif
#ifdef HAVE_UDP
#if LWIP_UDP > 0
  case IP_PROTO_UDP:
    udp_input(p, inp);
    break;
#endif /* LWIP_UDP */
#endif
#if LWIP_TCP > 0    
  case IP_PROTO_TCP:
    tcp_input(p, inp);
    break;
#endif /* LWIP_TCP */
  case IP_PROTO_ICMP:
    icmp_input(p, inp);
    break;
  default:
    pbuf_free(p);
    break;
  }
  nest--;
  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/* ip_output_if:
 *
 * Sends an IP packet on a network interface. This function constructs
 * the IP header and calculates the IP header checksum. If the source
 * IP address is NULL, the IP address of the outgoing network
 * interface is filled in as source address.
 */
/*-----------------------------------------------------------------------------------*/
err_t
ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
	     u8_t ttl,
	     u8_t proto, struct netif *netif, struct ip_addr *gw)
{
  struct ip_hdr *iphdr;
  static struct sequencer s = {0};
  if (!s.init)
    sequencer_init (&s, 16 /* 0 - 65535 */);

  magic (p, PBUF_MAGIC);

/* null src means we lend some help: */
  if (!src || ip_addr_isany (src))
    src = &(netif->ip_addr);

/* is the header included? it would be if we are called from a
forwarder or an echo: */
  if (dest == IP_HDRINCL) {
    iphdr = p->payload;
    dest = &iphdr->dest;

/* condition for fragmenting a packet: */
    if (p->len > netif->mtu) {
      struct pbuf *q;
      s16_t max_payload, offset = 0, total, hlen, skip;
      u16_t frag, ip_id;
      ip_id = (u16_t) sequencer_next (&s);
      frag = ntohs(IPH_OFFSET (iphdr));
/* completely ignore 2nd and later fragments that are to large: */
      if ((frag & IP_OFFMASK)) {
//printf ("cannot fragment a fragment - dropping\n");
	return ERR_OK;
      }
/* for Don't-Fragment packets and already-fragmented packets,
inform the sender of our MTU with Datagram-To-Big message */
      if ((frag & (IP_DF | IP_MF))) {
//printf ("datagram too large - sending Datagram-To-Big ICMP\n");
	icmp_dest_unreach(p, ICMP_DUR_FRAG, netif->mtu);
	return ERR_OK;
      }
/* skip over the header - i.e. split only the payload.
hlen is the length the first fragment's header: */
      skip = hlen = IPH_HL (iphdr) * 4;
      total = p->tot_len - skip;
//printf ("fragmenting %d + %d into parts of %d + %d\n", total, skip, max_payload, 20);
/* make a multiple of 8 bytes: */
      max_payload = ((netif->mtu - hlen) & ~7);
/* allocate a temporary pbuf for transmission (we use
PBUF_TRANSPORT to give 20 extra bytes for options - its
not actually enough - see FIXME: in
validate.c:packet_validate()): */
      q = pbuf_alloc (PBUF_TRANSPORT, max_payload, PBUF_RAM);
/* loop over payload parts: */
      while (offset < total) {
	u8_t *save;

/* calculate fragment bits field, setting IP_MF if its not
the last fragment: */
	frag = ((offset + max_payload < total) ? IP_MF : 0) | ((offset >> 3) & IP_OFFMASK);
/* read a payload part: */
	q->len = q->tot_len = pbuf_read (p, q->payload, offset + skip, max_payload);
	
/* add IP header: */
	save = q->payload;
	pbuf_header (q, hlen);
	iphdr = q->payload;

/* fill in IP header - we copy all of it with its options first: */
	memcpy (iphdr, p->payload, (size_t) hlen);
/* according to rfc 791, the following fields may be
effected: (1) options field (2) more fragments flag (3)
fragment offset (4) internet header length field (5) total
length field (6) header checksum: */
	IPH_LEN_SET (iphdr, htons (q->tot_len));
	IPH_OFFSET_SET (iphdr, htons (frag));
	IPH_VHLTOS_SET (iphdr, 4, hlen / 4, IPH_TOS ((struct ip_hdr *) p->payload));
	IPH_ID_SET (iphdr, ip_id);
	IPH_CHKSUM_SET (iphdr, 0);
	IPH_CHKSUM_SET (iphdr, inet_chksum (iphdr, hlen));

/* send the packet, and reset the pbuf: */
	magic (q, PBUF_MAGIC);
	
	netif->output (netif, q, (gw && gw->addr) ? gw : dest);
	pbuf_header (q, -((u8_t *) save - (u8_t *) q->payload));

/* move to next block */
	offset += max_payload;
/* now, for the rest of the fragments, we leave out the
options (FIXME: leaving out *all* options is not strictly
correct): */
	hlen = IP_HLEN;
        max_payload = ((netif->mtu - IP_HLEN) & ~7);
      }
/* free the temporary pbuf */
      pbuf_free (q);
      return ERR_OK;
    } else {
/* simplest case: included header with no fragmentation: */
      return netif->output (netif, p, (gw && gw->addr) ? gw : dest);
    }
  } else {
/* condition for fragmenting a packet: */
    if (p->len + IP_HLEN > netif->mtu) {
      struct pbuf *q;
      u16_t ip_id;
      s16_t max_payload, offset = 0;
      ip_id = (u16_t) sequencer_next (&s);
/* make a multiple of 8 bytes: */
      max_payload = ((netif->mtu - IP_HLEN) & ~7);
/* allocate a temporary pbuf for transmission. this is our
own packet, so its never going to have IP options that
would make the header length more than 20 bytes: */
      q = pbuf_alloc (PBUF_IP, max_payload, PBUF_RAM);
/* loop over payload parts: */
      while (offset < p->tot_len) {
	u16_t frag;
	u8_t *save;
/* calculate fragment bits field, setting IP_MF if its not
the last fragment: */
	frag = ((offset + max_payload < p->tot_len) ? IP_MF : 0) | ((offset >> 3) & IP_OFFMASK);
/* read a payload part: */
	q->len = q->tot_len = pbuf_read (p, q->payload, offset, max_payload);
/* add IP header: */
	save = q->payload;
	pbuf_header (q, IP_HLEN);
	iphdr = q->payload;
/* fill in IP header: */
	IPH_TTL_SET (iphdr, ttl);
	IPH_PROTO_SET (iphdr, proto);
	IPH_VHLTOS_SET (iphdr, 4, IP_HLEN / 4, 0);
	IPH_LEN_SET (iphdr, htons (q->tot_len));
	IPH_OFFSET_SET (iphdr, htons (frag));
	IPH_ID_SET (iphdr, ip_id);

	ip_addr_set (&(iphdr->dest), dest);
	ip_addr_set (&(iphdr->src), src);

	IPH_CHKSUM_SET (iphdr, 0);
	IPH_CHKSUM_SET (iphdr, inet_chksum (iphdr, IP_HLEN));

/* send the packet, and reset the pbuf: */
	magic (q, PBUF_MAGIC);
	netif->output (netif, q, (gw && gw->addr) ? gw : dest);
	pbuf_header (q, -((u8_t *) save - (u8_t *) q->payload));

/* goto next block */
	offset += max_payload;
      }
/* free the temporary pbuf */
      pbuf_free (q);
      return ERR_OK;
    } else {
/* normal no-to-be-fragmented packet: */
      u8_t r;
      u16_t ip_id;
      //ip_id = random ();	/* no fragmentation, don't need a non-repeating ID. secure random is ok*/
      ip_id = 0x800;
/* create ip header */
      r = pbuf_header (p, IP_HLEN);
      assert (!r);
      iphdr = p->payload;
/* fill in IP header: */
      IPH_TTL_SET (iphdr, ttl);
      IPH_PROTO_SET (iphdr, proto);
      IPH_VHLTOS_SET (iphdr, 4, IP_HLEN / 4, 0);
      IPH_LEN_SET (iphdr, htons (p->tot_len));
      IPH_OFFSET_SET (iphdr, htons (0));
      IPH_ID_SET (iphdr, ip_id);

      ip_addr_set (&(iphdr->dest), dest);
      ip_addr_set (&(iphdr->src), src);

      IPH_CHKSUM_SET (iphdr, 0);
      IPH_CHKSUM_SET (iphdr, inet_chksum (iphdr, IP_HLEN));

/* send the packet: */
      magic (p, PBUF_MAGIC);
      netif->output (netif, p, (gw && gw->addr) ? gw : dest);
      return ERR_OK;
    }
  }
}

/*-----------------------------------------------------------------------------------*/
/* ip_output:
 *
 * Simple interface to ip_output_if. It finds the outgoing network
 * interface and calls upon ip_output_if to do the actual work.
 */
/*-----------------------------------------------------------------------------------*/
#ifdef __PAULOS__
void
#else
err_t
#endif
ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
	  u8_t ttl, u8_t proto)
{
#ifdef __PAULOS__
  struct netif *netif;
  struct ip_addr gw;
#else
  static struct netif *netif;
#endif

  if (proto == IP_PROTO_ESP)
    netif = ip_route_no_ipsec(dest, &gw);
  else
    netif = ip_route(dest, &gw);
  
  if(netif == NULL) {
    DEBUGF(IP_DEBUG, ("ip_output: No route to 0x%lx\n", dest->addr));

#ifdef IP_STATS
    ++stats.ip.rterr;
#endif /* IP_STATS */
//printf ("ip_output: no route\n");

/* This is a major bug: this line should NOT be here. -paul */
#if 0
    pbuf_free(p);
#endif
    return;
  }

#ifdef __PAULOS__
  ip_output_if(p, src ? src : &netif->ip_addr, dest, ttl, proto, netif, &gw);
#else
  return ip_output_if(p, src, dest, ttl, proto, netif);
#endif
}
/*-----------------------------------------------------------------------------------*/
#if IP_DEBUG
void
ip_debug_print(struct pbuf *p)
{
  struct ip_hdr *iphdr = p->payload;
  u8_t *payload;

  payload = (u8_t *)iphdr + IP_HLEN/sizeof(u8_t);
  
  DEBUGF(IP_DEBUG, ("IP header:\n"));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(IP_DEBUG, ("|%2d |%2d |   %2d  |      %4d     | (v, hl, tos, len)\n",
		    IPH_V(iphdr),
		    IPH_HL(iphdr),
		    IPH_TOS(iphdr),
		    ntohs(IPH_LEN(iphdr))));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(IP_DEBUG, ("|    %5d      |%d%d%d|    %4d   | (id, flags, offset)\n",
		    ntohs(IPH_ID(iphdr)),
		    ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
		    ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
		    ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
		    ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(IP_DEBUG, ("|   %2d  |   %2d  |    0x%04x     | (ttl, proto, chksum)\n",
		    IPH_TTL(iphdr),
		    IPH_PROTO(iphdr),
		    ntohs(IPH_CHKSUM(iphdr))));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(IP_DEBUG, ("|  %3ld  |  %3ld  |  %3ld  |  %3ld  | (src)\n",
		    ntohl(iphdr->src.addr) >> 24 & 0xff,
		    ntohl(iphdr->src.addr) >> 16 & 0xff,
		    ntohl(iphdr->src.addr) >> 8 & 0xff,
		    ntohl(iphdr->src.addr) & 0xff));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(IP_DEBUG, ("|  %3ld  |  %3ld  |  %3ld  |  %3ld  | (dest)\n",
		    ntohl(iphdr->dest.addr) >> 24 & 0xff,
		    ntohl(iphdr->dest.addr) >> 16 & 0xff,
		    ntohl(iphdr->dest.addr) >> 8 & 0xff,
		    ntohl(iphdr->dest.addr) & 0xff));
  DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
}
#endif /* IP_DEBUG */
/*-----------------------------------------------------------------------------------*/





