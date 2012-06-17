/* netif.h - PaulOS embedded operating system
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
 * $Id: netif.h,v 1.3 2003/08/01 11:58:01 psheer Exp $
 */
#ifndef __LWIP_NETIF_H__
#define __LWIP_NETIF_H__

#include "lwip/opt.h"

#include "lwip/err.h"

#include "lwip/ip_addr.h"

#include "lwip/inet.h"
#include "lwip/pbuf.h"


struct netif {
  struct netif *next;
  u8_t num;
#ifdef __PAULOS__
  u8_t pad1[3];
#endif
  struct ip_addr ip_addr;
  struct ip_addr netmask;  /* netmask in network byte order */
  struct ip_addr gw;
  char hwaddr[6];
#ifdef __PAULOS__
  u16_t mtu;
#endif
  int fd;		   /* paulos low level file descriptor */

  /* This function is called by the network device driver
     when it wants to pass a packet to the TCP/IP stack. */
  err_t (* input)(struct pbuf *p, struct netif *inp);

  /* The following two fields should be filled in by the
     initialization function for the device driver. */

  char name[2];
#ifdef __PAULOS__
  u8_t pad3[2];
#endif
  /* This function is called by the IP module when it wants
     to send a packet on the interface. */
  err_t (* output)(struct netif *netif, struct pbuf *p,
		   struct ip_addr *ipaddr);
#ifndef __PAULOS__
  err_t (* linkoutput)(struct netif *netif, struct pbuf *p);
#endif

  /* This field can be set bu the device driver and could point
     to state information for the device. */
  void *state;
} __attribute__ ((packed));

/* The list of network interfaces. */
extern struct netif *netif_list;
extern struct netif *netif_default;


/* netif_init() must be called first. */
void netif_init(void);

struct netif *netif_add(struct ip_addr *ipaddr, struct ip_addr *netmask,
			struct ip_addr *gw,
#ifdef __PAULOS__
			void (* init)(struct netif *netif, char *device_name),
			char *device_name,
#else
			void (* init)(struct netif *netif),
#endif
			err_t (* input)(struct pbuf *p, struct netif *netif));

/* Returns a network interface given its name. The name is of the form
   "et0", where the first two letters are the "name" field in the
   netif structure, and the digit is in the num field in the same
   structure. */
struct netif *netif_find(char *name);

void netif_set_default(struct netif *netif);

void netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr);
void netif_set_netmask(struct netif *netif, struct ip_addr *netmast);
void netif_set_gw(struct netif *netif, struct ip_addr *gw);

#endif /* __LWIP_NETIF_H__ */
