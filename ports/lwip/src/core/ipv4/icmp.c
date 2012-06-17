/* icmp.c - PaulOS embedded operating system
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
 * $Id: icmp.c,v 1.5 2003/08/18 13:52:56 psheer Exp $
 */

/* Some ICMP messages should be passed to the transport protocols. This
   is not implemented. */

#include "lwip/debug.h"

#include "lwip/icmp.h"
#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"

#include "lwip/stats.h"
#include <assert.h>

/*-----------------------------------------------------------------------------------*/
void
icmp_input(struct pbuf *p, struct netif *inp)
{
  unsigned char type;
  struct icmp_echo_hdr *iecho;
  struct ip_hdr *iphdr;
  struct ip_addr tmpaddr, gw;
  u16_t ip_id;
  u16_t hlen;

  magic (p, PBUF_MAGIC);

#ifdef ICMP_STATS
  ++stats.icmp.recv;
#endif /* ICMP_STATS */

  
  iphdr = p->payload;
  hlen = IPH_HL(iphdr) * 4/sizeof(u8_t);
  pbuf_header(p, -hlen);
  magic (p, PBUF_MAGIC);

  type = *((u8_t *)p->payload);

  switch(type) {
  case ICMP_ECHO:
    if(ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) ||
       ip_addr_ismulticast(&iphdr->dest)) {
      DEBUGF(ICMP_DEBUG, ("Smurf.\n"));
#ifdef ICMP_STATS
      ++stats.icmp.err;
#endif /* ICMP_STATS */
      pbuf_free(p);
      return;
    }
    DEBUGF(ICMP_DEBUG, ("icmp_input: ping\n"));
#ifndef __PAULOS__
    DEBUGF(DEMO_DEBUG, ("Pong!\n"));
#endif
    if(p->tot_len < sizeof(struct icmp_echo_hdr)) {
      DEBUGF(ICMP_DEBUG, ("icmp_input: bad ICMP echo received\n"));
      pbuf_free(p);
#ifdef ICMP_STATS
      ++stats.icmp.lenerr;
#endif /* ICMP_STATS */
      return;      
    }
    iecho = p->payload;    

    if(inet_chksum_pbuf(p) != 0) {
      DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed for received ICMP echo\n"));
      pbuf_free(p);
#ifdef ICMP_STATS
      ++stats.icmp.chkerr;
#endif /* ICMP_STATS */
      return;
    }
    tmpaddr.addr = iphdr->src.addr;
    iphdr->src.addr = iphdr->dest.addr;
    iphdr->dest.addr = tmpaddr.addr;
    ICMPH_TYPE_SET(iecho, ICMP_ER);

    IPH_TTL_SET(iphdr, ICMP_TTL);
    IPH_OFFSET_SET (iphdr, (u16_t) 0);
    //ip_id = random ();
    ip_id=0x800;
    IPH_ID_SET (iphdr, ip_id);

    /* adjust the checksum */
    if(iecho->chksum >= htons(0xffff - (ICMP_ECHO << 8))) {
      iecho->chksum += htons(ICMP_ECHO << 8) + 1;
    } else {
      iecho->chksum += htons(ICMP_ECHO << 8);
    }

    /* calculate the IP checksum */
    IPH_CHKSUM_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, IP_HLEN));
#ifdef ICMP_STATS
    ++stats.icmp.xmit;
#endif /* ICMP_STATS */

    pbuf_header(p, hlen);
    magic (p, PBUF_MAGIC);

    if((inp = ip_route(&iphdr->dest, &gw)) == NULL) {
printf ("ICMP unroutable???\n");
      break;
    }

    ip_output_if(p, &(iphdr->src), IP_HDRINCL,
		 ICMP_TTL, IP_PROTO_ICMP, inp, &gw);
    magic (p, PBUF_MAGIC);

    break; 
  default:
    magic (p, PBUF_MAGIC);
    DEBUGF(ICMP_DEBUG, ("icmp_input: ICMP type not supported.\n"));
#ifdef ICMP_STATS
    ++stats.icmp.proterr;
    ++stats.icmp.drop;
#endif /* ICMP_STATS */
  }

  pbuf_free(p);
}
/*-----------------------------------------------------------------------------------*/
static inline void memcpyfill (char *dst, char *src, int src_len, int fill)
{

#ifdef PAULOS_ARCH_LINUX
    {
	int i;
	volatile int p = 0;
	for (i = 0; i < src_len; i++)
	    if (src[i])
		p++;
    }
#endif

    if (src_len < fill) {
	memcpy (dst, src, src_len);
	memset (dst + src_len, '\0', fill - src_len);
    } else {
	memcpy (dst, src, fill);
    }
}
/*-----------------------------------------------------------------------------------*/
void
icmp_dest_unreach(struct pbuf *p, enum icmp_dur_type t, u16_t mtu)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_dur_hdr *idur;
  
  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RAM);
  /* ICMP header + IP header + 8 bytes of data */

  iphdr = p->payload;
  
  idur = q->payload;
  ICMPH_TYPE_SET(idur, ICMP_DUR);
  ICMPH_CODE_SET(idur, t);
  idur->unused = htons (0x0000);
  idur->mtu = htons (mtu);

  memcpyfill ((char *)q->payload + 8, p->payload, p->len, IP_HLEN + 8);
  
  /* calculate checksum */
  idur->chksum = 0;
  idur->chksum = inet_chksum(idur, q->len);
#ifdef ICMP_STATS
  ++stats.icmp.xmit;
#endif /* ICMP_STATS */
  ip_output(q, NULL, &(iphdr->src),
	    ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}
/*-----------------------------------------------------------------------------------*/
#if IP_FORWARDING > 0
void
icmp_time_exceeded(struct pbuf *p, enum icmp_te_type t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_te_hdr *tehdr;

  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RAM);

  iphdr = p->payload;
#if ICMP_DEBUG
  DEBUGF(ICMP_DEBUG, ("icmp_time_exceeded from "));
  ip_addr_debug_print(&(iphdr->src));
  DEBUGF(ICMP_DEBUG, (" to "));
  ip_addr_debug_print(&(iphdr->dest));
  DEBUGF(ICMP_DEBUG, ("\n"));
#endif /* ICMP_DEBNUG */

  tehdr = q->payload;
  ICMPH_TYPE_SET(tehdr, ICMP_TE);
  ICMPH_CODE_SET(tehdr, t);

  /* copy fields from original packet */
  memcpyfill ((char *)q->payload + 8, (char *)p->payload, p->len, IP_HLEN + 8);
  
  /* calculate checksum */
  tehdr->chksum = 0;
  tehdr->chksum = inet_chksum(tehdr, q->len);
#ifdef ICMP_STATS
  ++stats.icmp.xmit;
#endif /* ICMP_STATS */

  ip_output(q, NULL, &(iphdr->src),
	    ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}

void
icmp_redirect(struct ip_hdr *iphdr, struct netif *netif, struct ip_addr *new_hop)
{
  struct pbuf *q;
  struct ip_addr gw;
  struct icmp_redir_hdr *rehdr;

  q = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct icmp_redir_hdr), PBUF_RAM);

#if ICMP_DEBUG
  DEBUGF(ICMP_DEBUG, ("sending redirect: packet (from "));
  ip_addr_debug_print(&(iphdr->src));
  DEBUGF(ICMP_DEBUG, (" to "));
  ip_addr_debug_print(&(iphdr->dest));
  DEBUGF(ICMP_DEBUG, ("), new gw "));
  ip_addr_debug_print(gw);
  DEBUGF(ICMP_DEBUG, ("\n"));
#endif /* ICMP_DEBNUG */

  rehdr = q->payload;
  ICMPH_TYPE_SET(rehdr, ICMP_RD);
  ICMPH_CODE_SET(rehdr, 1 /* => redirect host */);

  rehdr->gw = new_hop->addr;
  /* copy fields from original packet */
  bcopy((char *)iphdr, (char *)rehdr->was_packet, sizeof (rehdr->was_packet));
  
  /* calculate checksum */
  rehdr->chksum = 0;
  rehdr->chksum = inet_chksum(rehdr, q->len);
#ifdef ICMP_STATS
  ++stats.icmp.xmit;
#endif /* ICMP_STATS */
  gw.addr = 0;
  ip_output_if(q, &(netif->ip_addr), &(iphdr->src), 2, IP_PROTO_ICMP, netif, &gw);
  pbuf_free(q);
}

#endif /* IP_FORWARDING > 0 */







