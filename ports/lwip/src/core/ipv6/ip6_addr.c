/* ip6_addr.c - PaulOS embedded operating system
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
 * $Id: ip6_addr.c,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */

#include "lwip/debug.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/*-----------------------------------------------------------------------------------*/
int
ip_addr_maskcmp(struct ip_addr *addr1, struct ip_addr *addr2,
                struct ip_addr *mask)
{
  return((addr1->addr[0] & mask->addr[0]) == (addr2->addr[0] & mask->addr[0]) &&
         (addr1->addr[1] & mask->addr[1]) == (addr2->addr[1] & mask->addr[1]) &&
         (addr1->addr[2] & mask->addr[2]) == (addr2->addr[2] & mask->addr[2]) &&
         (addr1->addr[3] & mask->addr[3]) == (addr2->addr[3] & mask->addr[3]));
        
}
/*-----------------------------------------------------------------------------------*/
int
ip_addr_cmp(struct ip_addr *addr1, struct ip_addr *addr2)
{
  return(addr1->addr[0] == addr2->addr[0] &&
         addr1->addr[1] == addr2->addr[1] &&
         addr1->addr[2] == addr2->addr[2] &&
         addr1->addr[3] == addr2->addr[3]);
}
/*-----------------------------------------------------------------------------------*/
void
ip_addr_set(struct ip_addr *dest, struct ip_addr *src)
{
  bcopy(src, dest, sizeof(struct ip_addr));
  /*  dest->addr[0] = src->addr[0];
  dest->addr[1] = src->addr[1];
  dest->addr[2] = src->addr[2];
  dest->addr[3] = src->addr[3];*/
}
/*-----------------------------------------------------------------------------------*/
int
ip_addr_isany(struct ip_addr *addr)
{
  if(addr == NULL) return 1;
  return((addr->addr[0] | addr->addr[1] | addr->addr[2] | addr->addr[3]) == 0);
}

/*-----------------------------------------------------------------------------------*/
/*#if IP_DEBUG*/
void
ip_addr_debug_print(struct ip_addr *addr)
{
  printf("%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx",
         ntohl(addr->addr[0]) >> 16 & 0xffff,
         ntohl(addr->addr[0]) & 0xffff,
         ntohl(addr->addr[1]) >> 16 & 0xffff,
         ntohl(addr->addr[1]) & 0xffff,
         ntohl(addr->addr[2]) >> 16 & 0xffff,
         ntohl(addr->addr[2]) & 0xffff,
         ntohl(addr->addr[3]) >> 16 & 0xffff,
         ntohl(addr->addr[3]) & 0xffff);
}
/*#endif*/ /* IP_DEBUG */
/*-----------------------------------------------------------------------------------*/
