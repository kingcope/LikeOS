/* inet.h - PaulOS embedded operating system
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
 * $Id: inet.h,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */
#ifndef __LWIP_INET_H__
#define __LWIP_INET_H__

#include "lwip/arch.h"

#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"

u16_t inet_chksum(void *dataptr, u16_t len);
u16_t inet_chksum_pbuf(struct pbuf *p);
u16_t inet_chksum_pseudo(struct pbuf *p,
			 struct ip_addr *src, struct ip_addr *dest,
			 u8_t proto, u16_t proto_len);

#ifdef HTONS
#undef HTONS
#endif /* HTONS */
#ifdef NTOHS
#undef NTOHS
#endif /* NTOHS */
#ifdef HTONL
#undef HTONL
#endif /* HTONL */
#ifdef NTOHL
#undef NTOHL
#endif /* NTOHL */

#ifdef htons
#undef htons
#endif /* htons */
#ifdef htonl
#undef htonl
#endif /* htonl */
#ifdef ntohs
#undef ntohs
#endif /* ntohs */
#ifdef ntohl
#undef ntohl
#endif /* ntohl */



#ifndef HTONS
#   if BYTE_ORDER == BIG_ENDIAN
#      define HTONS(n) (n)
#   else /* BYTE_ORDER == BIG_ENDIAN */
#      define HTONS(n) (((((u16_t)(n) & 0xff)) << 8) | (((u16_t)(n) & 0xff00) >> 8))
#   endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* HTONS */

#define htons HTONS
#define NTOHS HTONS
#define ntohs htons


#ifndef HTONL
#   if BYTE_ORDER == BIG_ENDIAN
#      define HTONL(n) (n)
#   else /* BYTE_ORDER == BIG_ENDIAN */
#      define HTONL(n) (((((u32_t)(n) & 0xff)) << 24) | \
                        ((((u32_t)(n) & 0xff00)) << 8) | \
                        ((((u32_t)(n) & 0xff0000)) >> 8) | \
                        ((((u32_t)(n) & 0xff000000)) >> 24))
#   endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* HTONL */

#define htonl HTONL
#define NTOHL HTONL
#define ntohl htonl

#endif /* __LWIP_INET_H__ */

