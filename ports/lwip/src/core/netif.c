/* netif.c - PaulOS embedded operating system
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
 * $Id: netif.c,v 1.3 2003/08/31 21:31:56 psheer Exp $
 */

#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/netif.h"

#include <assert.h>

struct netif *netif_list = NULL;
struct netif *netif_default = NULL;

/*-----------------------------------------------------------------------------------*/
struct netif *
netif_add(struct ip_addr *ipaddr, struct ip_addr *netmask,
	  struct ip_addr *gw,
#ifdef __PAULOS__
	  void (* init)(struct netif *netif, char *device_name),
	  char *device_name,
#else
	  void (* init)(struct netif *netif),
#endif
	  err_t (* input)(struct pbuf *p, struct netif *netif))
{
  struct netif *netif;
  static int netifnum = 0;
  
  netif = mem_malloc(sizeof(struct netif));

  if(netif == NULL) {
    return NULL;
  }

  memset (netif, 0, sizeof (struct netif));
  
  netif->num = netifnum++;
  netif->mtu = 1500;
  netif->input = input;
  ip_addr_set(&(netif->ip_addr), ipaddr);
  ip_addr_set(&(netif->netmask), netmask);
  ip_addr_set(&(netif->gw), gw);

#ifdef __PAULOS__
  init(netif, device_name);
#else
  init(netif);
#endif
  
  netif->next = netif_list;
  netif_list = netif;
#if NETIF_DEBUG
  DEBUGF(NETIF_DEBUG, ("netif: added interface %c%c IP addr ",
		       netif->name[0], netif->name[1]));
  ip_addr_debug_print(ipaddr);
  DEBUGF(NETIF_DEBUG, (" netmask "));
  ip_addr_debug_print(netmask);
  DEBUGF(NETIF_DEBUG, (" gw "));  
  ip_addr_debug_print(gw);
  DEBUGF(NETIF_DEBUG, ("\n"));
#endif /* NETIF_DEBUG */
  return netif;
}
/*-----------------------------------------------------------------------------------*/
struct netif *
netif_find(char *name)
{
  struct netif *netif;
#ifndef __PAULOS__
  u8_t num;
#endif
  
  if(name == NULL) {
    return NULL;
  }

#ifndef __PAULOS__
  num = name[2] - '0';
#endif
 
  for(netif = netif_list; netif != NULL; netif = netif->next) {
#ifndef __PAULOS__
    if(num == netif->num &&
#else
    if(
#endif
       name[0] == netif->name[0] &&
       name[1] == netif->name[1]) {
#ifndef __PAULOS__
      DEBUGF(NETIF_DEBUG, ("netif_find: found %s\n", name));
#endif
      return netif;
    }    
  }
#ifndef __PAULOS__
  DEBUGF(NETIF_DEBUG, ("netif_find: didn't find %s\n", name));
#endif
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
void
netif_set_iface_mtu (struct netif *netif, u16_t mtu)
{
  assert (netif);
  netif->mtu = mtu;
}
/*-----------------------------------------------------------------------------------*/
void
netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr)
{
  ip_addr_set(&(netif->ip_addr), ipaddr);
  DEBUGF(NETIF_DEBUG, ("netif: setting IP address of interface %c%c to %d.%d.%d.%d\n",
		       netif->name[0], netif->name[1],
		       (u8_t)(ntohl(ipaddr->addr) >> 24 & 0xff),
		       (u8_t)(ntohl(ipaddr->addr) >> 16 & 0xff),
		       (u8_t)(ntohl(ipaddr->addr) >> 8 & 0xff),
		       (u8_t)(ntohl(ipaddr->addr) & 0xff)));
}
/*-----------------------------------------------------------------------------------*/
void
netif_get_ipaddr(struct netif *netif, struct ip_addr *ipaddr)
{
  ip_addr_set(ipaddr, &(netif->ip_addr));
}
/*-----------------------------------------------------------------------------------*/
void
netif_get_netmask(struct netif *netif, struct ip_addr *ipaddr)
{
  ip_addr_set(ipaddr, &(netif->netmask));
}
/*-----------------------------------------------------------------------------------*/
char *
netif_get_name(struct netif *netif)
{
  return netif->name;
}
/*-----------------------------------------------------------------------------------*/
void
netif_set_gw(struct netif *netif, struct ip_addr *gw)
{
  ip_addr_set(&(netif->gw), gw);
}
/*-----------------------------------------------------------------------------------*/
void
netif_set_netmask(struct netif *netif, struct ip_addr *netmask)
{
  ip_addr_set(&(netif->netmask), netmask);
}
/*-----------------------------------------------------------------------------------*/
void
netif_set_default(struct netif *netif)
{
  netif_default = netif;
  if (netif_default)
    DEBUGF(NETIF_DEBUG, ("netif: setting default interface %c%c\n",
		       netif->name[0], netif->name[1]));
  else
    DEBUGF(NETIF_DEBUG, ("netif: setting default interface NONE\n"));
}
/*-----------------------------------------------------------------------------------*/
void
netif_init(void)
{
  netif_list = netif_default = NULL;
}
/*-----------------------------------------------------------------------------------*/
