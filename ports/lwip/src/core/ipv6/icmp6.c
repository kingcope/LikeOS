/* icmp6.c - PaulOS embedded operating system
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
 * $Id: icmp6.c,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */

/* Some ICMP messages should be passed to the transport protocols. This
   is not implemented. */

#include "lwip/debug.h"

#include "lwip/icmp.h"
#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/def.h"

#include "lwip/stats.h"

/*-----------------------------------------------------------------------------------*/
void
icmp_input(struct pbuf *p, struct netif *inp)
{
  unsigned char type;
  struct icmp_echo_hdr *iecho;
  struct ip_hdr *iphdr;
  struct ip_addr tmpaddr;
  
 
#ifdef ICMP_STATS
  ++stats.icmp.recv;
#endif /* ICMP_STATS */

  type = ((char *)p->payload)[0];

  switch(type) {
  case ICMP6_ECHO:
    DEBUGF(ICMP_DEBUG, ("icmp_input: ping\n"));

    if(p->tot_len < sizeof(struct icmp_echo_hdr)) {
      DEBUGF(ICMP_DEBUG, ("icmp_input: bad ICMP echo received\n"));

      pbuf_free(p);
#ifdef ICMP_STATS
      ++stats.icmp.lenerr;
#endif /* ICMP_STATS */

      return;      
    }
    iecho = p->payload;
    iphdr = (struct ip_hdr *)((char *)p->payload - IP_HLEN);
    if(inet_chksum_pbuf(p) != 0) {
      DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed for received ICMP echo (%x)\n", inet_chksum_pseudo(p, &(iphdr->src), &(iphdr->dest), IP_PROTO_ICMP, p->tot_len)));

#ifdef ICMP_STATS
      ++stats.icmp.chkerr;
#endif /* ICMP_STATS */
    /*      return;*/
    }
    DEBUGF(ICMP_DEBUG, ("icmp: p->len %d p->tot_len %d\n", p->len, p->tot_len));
    ip_addr_set(&tmpaddr, &(iphdr->src));
    ip_addr_set(&(iphdr->src), &(iphdr->dest));
    ip_addr_set(&(iphdr->dest), &tmpaddr);
    iecho->type = ICMP6_ER;
    /* adjust the checksum */
    if(iecho->chksum >= htons(0xffff - (ICMP6_ECHO << 8))) {
      iecho->chksum += htons(ICMP6_ECHO << 8) + 1;
    } else {
      iecho->chksum += htons(ICMP6_ECHO << 8);
    }
    DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed for received ICMP echo (%x)\n", inet_chksum_pseudo(p, &(iphdr->src), &(iphdr->dest), IP_PROTO_ICMP, p->tot_len)));
#ifdef ICMP_STATS
    ++stats.icmp.xmit;
#endif /* ICMP_STATS */

    /*    DEBUGF("icmp: p->len %d p->tot_len %d\n", p->len, p->tot_len);*/
    ip_output_if(p, &(iphdr->src), IP_HDRINCL,
		 iphdr->hoplim, IP_PROTO_ICMP, inp);
    break; 
  default:
    DEBUGF(ICMP_DEBUG, ("icmp_input: ICMP type not supported.\n"));

#ifdef ICMP_STATS
    ++stats.icmp.proterr;
    ++stats.icmp.drop;
#endif /* ICMP_STATS */
  }

  pbuf_free(p);
}
/*-----------------------------------------------------------------------------------*/
void
icmp_dest_unreach(struct pbuf *p, enum icmp_dur_type t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_dur_hdr *idur;
  
  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RAM);
  /* ICMP header + IP header + 8 bytes of data */

  iphdr = p->payload;
  
  idur = q->payload;
  idur->type = (char)ICMP6_DUR;
  idur->icode = (char)t;

  bcopy(p->payload, (char *)q->payload + 8, IP_HLEN + 8);
  
  /* calculate checksum */
  idur->chksum = 0;
  idur->chksum = inet_chksum(idur, q->len);
#ifdef ICMP_STATS
  ++stats.icmp.xmit;
#endif /* ICMP_STATS */

  ip_output(q, NULL,
	    (struct ip_addr *)&(iphdr->src), ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}
/*-----------------------------------------------------------------------------------*/
void
icmp_time_exceeded(struct pbuf *p, enum icmp_te_type t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_te_hdr *tehdr;

  DEBUGF(ICMP_DEBUG, ("icmp_time_exceeded\n"));
  
  q = pbuf_alloc(PBUF_TRANSPORT, 8 + IP_HLEN + 8, PBUF_RAM);

  iphdr = p->payload;
  
  tehdr = q->payload;
  tehdr->type = (char)ICMP6_TE;
  tehdr->icode = (char)t;

  /* copy fields from original packet */
  bcopy((char *)p->payload, (char *)q->payload + 8, IP_HLEN + 8);
  
  /* calculate checksum */
  tehdr->chksum = 0;
  tehdr->chksum = inet_chksum(tehdr, q->len);
#ifdef ICMP_STATS
  ++stats.icmp.xmit;
#endif /* ICMP_STATS */
  ip_output(q, NULL, 
	    (struct ip_addr *)&(iphdr->src), ICMP_TTL, IP_PROTO_ICMP);
  pbuf_free(q);
}








