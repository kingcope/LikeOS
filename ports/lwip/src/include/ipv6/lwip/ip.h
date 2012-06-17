/* ip.h - PaulOS embedded operating system
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
 * $Id: ip.h,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */
#ifndef __LWIP_IP_H__
#define __LWIP_IP_H__

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"

#include "lwip/err.h"

#define IP_HLEN 40

#define IP_PROTO_ICMP 58
#define IP_PROTO_UDP 17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP 6

/* This is passed as the destination address to ip_output_if (not
   to ip_output), meaning that an IP header already is constructed
   in the pbuf. This is used when TCP retransmits. */
#ifdef IP_HDRINCL
#undef IP_HDRINCL
#endif /* IP_HDRINCL */
#define IP_HDRINCL  NULL


/* The IPv6 header. */
struct ip_hdr {
#if BYTE_ORDER == LITTLE_ENDIAN
  u8_t tclass1:4, v:4;
  u8_t flow1:4, tclass2:4;  
#else
  u8_t v:4, tclass1:4;
  u8_t tclass2:8, flow1:4;
#endif
  u16_t flow2;
  u16_t len;                /* payload length */
  u8_t nexthdr;             /* next header */
  u8_t hoplim;              /* hop limit (TTL) */
  struct ip_addr src, dest;          /* source and destination IP addresses */
} __attribute__ ((packed));

void ip_init(void);

#include "lwip/netif.h"

struct netif *ip_route(struct ip_addr *dest, struct ip_addr *gw);

void ip_input(struct pbuf *p, struct netif *inp);

/* source and destination addresses in network byte order, please */
err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
	       unsigned char ttl, unsigned char proto);

err_t ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
		  unsigned char ttl, unsigned char proto,
		  struct netif *netif, struct ip_addr *gw);

#if IP_DEBUG
void ip_debug_print(struct pbuf *p);
#endif /* IP_DEBUG */

#endif /* __LWIP_IP_H__ */


