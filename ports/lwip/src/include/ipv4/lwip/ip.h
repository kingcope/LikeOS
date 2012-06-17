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
 * $Id: ip.h,v 1.3 2003/08/01 11:58:01 psheer Exp $
 */
#ifndef __LWIP_IP_H__
#define __LWIP_IP_H__

#include "lwip/arch.h"

#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"

#include "lwip/err.h"

void ip_init(void);
u8_t ip_lookup(void *header, struct netif *inp);
struct netif *ip_route(struct ip_addr *dest, struct ip_addr *gw);
struct netif *ip_route_no_ipsec(struct ip_addr *dest, struct ip_addr *gw);
err_t ip_input(struct pbuf *p, struct netif *inp);
#ifdef __PAULOS__
void ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
		u8_t ttl, u8_t proto);
#else
err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
		u8_t ttl, u8_t proto);
#endif
err_t ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest,
		   u8_t ttl, u8_t proto,
		   struct netif *netif, struct ip_addr *gw);

struct pbuf *ip_reass(struct pbuf *p);

#define IP_HLEN 20

#define IP_PROTO_ICMP 1
#define IP_PROTO_IPIP 4
#define IP_PROTO_UDP 17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP 6
#define IP_PROTO_ESP 50

/* This is passed as the destination address to ip_output_if (not
   to ip_output), meaning that an IP header already is constructed
   in the pbuf. This is used when TCP retransmits. */
#ifdef IP_HDRINCL
#undef IP_HDRINCL
#endif /* IP_HDRINCL */
#define IP_HDRINCL  NULL

PACK_STRUCT_BEGIN
struct ip_hdr {
  /* version / header length / type of service */
  PACK_STRUCT_FIELD(u16_t _v_hl_tos);
  /* total length */
  PACK_STRUCT_FIELD(u16_t _len);
  /* identification */
  PACK_STRUCT_FIELD(u16_t _id);
  /* fragment offset field */
  PACK_STRUCT_FIELD(u16_t _offset);
#define IP_RF 0x8000        /* reserved fragment flag */
#define IP_DF 0x4000        /* dont fragment flag */
#define IP_MF 0x2000        /* more fragments flag */
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
  /* time to live / protocol*/
  PACK_STRUCT_FIELD(u16_t _ttl_proto);
  /* checksum */
  PACK_STRUCT_FIELD(u16_t _chksum);
  /* source and destination IP addresses */
  PACK_STRUCT_FIELD(struct ip_addr src);
  PACK_STRUCT_FIELD(struct ip_addr dest); 
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

#define IPH_V(hdr)  (NTOHS((hdr)->_v_hl_tos) >> 12)
#define IPH_HL(hdr) ((NTOHS((hdr)->_v_hl_tos) >> 8) & 0x0f)
#define IPH_TOS(hdr) HTONS((NTOHS((hdr)->_v_hl_tos) & 0xff))
#define IPH_LEN(hdr) ((hdr)->_len)
#define IPH_ID(hdr) ((hdr)->_id)
#define IPH_OFFSET(hdr) ((hdr)->_offset)
#define IPH_TTL(hdr) (NTOHS((hdr)->_ttl_proto) >> 8)
#define IPH_PROTO(hdr) (NTOHS((hdr)->_ttl_proto) & 0xff)
#define IPH_CHKSUM(hdr) ((hdr)->_chksum)

#define IPH_VHLTOS_SET(hdr, v, hl, tos) (hdr)->_v_hl_tos = HTONS(((v) << 12) | ((hl) << 8) | (tos))
#define IPH_LEN_SET(hdr, len) (hdr)->_len = (len)
#define IPH_ID_SET(hdr, id) (hdr)->_id = (id)
#define IPH_OFFSET_SET(hdr, off) (hdr)->_offset = (off)
#define IPH_TTL_SET(hdr, ttl) (hdr)->_ttl_proto = HTONS(IPH_PROTO(hdr) | ((ttl) << 8))
#define IPH_PROTO_SET(hdr, proto) (hdr)->_ttl_proto = HTONS((proto) | (IPH_TTL(hdr) << 8))
#define IPH_CHKSUM_SET(hdr, chksum) (hdr)->_chksum = (chksum)



#if IP_DEBUG
void ip_debug_print(struct pbuf *p);
#endif /* IP_DEBUG */

#endif /* __LWIP_IP_H__ */


