/* etharp.c - PaulOS embedded operating system
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
 * $Id: etharp.c,v 1.6 2003/08/18 13:52:56 psheer Exp $
 *
 */

#ifdef HAVE_ETH

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/ip.h"
#include "lwip/stats.h"
#include <assert.h>

#if LWIP_DHCP
#  include "lwip/dhcp.h"
#endif

#define ARP_MAXAGE 12  /* 12 * 10 seconds = 2 minutes. */
#define ARP_MAXPENDING 2 /* 2 * 10 seconds = 20 seconds. */

#define HWTYPE_ETHERNET 1

#define ARP_REQUEST 1
#define ARP_REPLY 2

/* MUST be compiled with "pack structs" or equivalent! */
PACK_STRUCT_BEGIN
struct etharp_hdr {
  PACK_STRUCT_FIELD(struct eth_hdr ethhdr);
  PACK_STRUCT_FIELD(u16_t hwtype);
  PACK_STRUCT_FIELD(u16_t proto);
  PACK_STRUCT_FIELD(u16_t _hwlen_protolen);
  PACK_STRUCT_FIELD(u16_t opcode);
  PACK_STRUCT_FIELD(struct eth_addr shwaddr);
  PACK_STRUCT_FIELD(struct ip_addr sipaddr);
  PACK_STRUCT_FIELD(struct eth_addr dhwaddr);
  PACK_STRUCT_FIELD(struct ip_addr dipaddr);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

static void (*arp_callback) (void *, u8_t *, u32_t) = NULL;
static void *arp_callback_arg;

void etharp_set_callback(void (*cb) (void *, u8_t *, u32_t), void * arg)
{
  arp_callback_arg = arg;
  arp_callback = cb;
}

#define ARPH_HWLEN(hdr) (NTOHS((hdr)->_hwlen_protolen) >> 8)
#define ARPH_PROTOLEN(hdr) (NTOHS((hdr)->_hwlen_protolen) & 0xff)


#define ARPH_HWLEN_SET(hdr, len) (hdr)->_hwlen_protolen = HTONS(ARPH_PROTOLEN(hdr) | ((len) << 8))
#define ARPH_PROTOLEN_SET(hdr, len) (hdr)->_hwlen_protolen = HTONS((len) | (ARPH_HWLEN(hdr) << 8))

PACK_STRUCT_BEGIN
struct ethip_hdr {
  PACK_STRUCT_FIELD(struct eth_hdr eth);
  PACK_STRUCT_FIELD(struct ip_hdr ip);
};
PACK_STRUCT_END

enum etharp_state {
  ETHARP_STATE_EMPTY,
  ETHARP_STATE_PENDING,
  ETHARP_STATE_STABLE,
  ETHARP_STATE_STATIC
};

struct etharp_entry {
  struct ip_addr ipaddr;
  struct eth_addr ethaddr;
  u8_t ctime, pad0;
  struct pbuf *p;
  void *payload;
  u16_t len, tot_len;
  enum etharp_state state;
} __attribute__ ((packed));

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};
#if ARP_TABLE_SIZE >= 256
#error ARP_TABLE_SIZE must fit within unsigned char type
#endif
static struct etharp_entry arp_table[ARP_TABLE_SIZE];
static u8_t arp_table_last = 0;
static u8_t ctime;

static u8_t etharp_new_entry(struct pbuf *q, struct ip_addr *ipaddr, struct eth_addr *hwaddr, enum etharp_state s);

/*-----------------------------------------------------------------------------------*/
void
etharp_init(void)
{
  u8_t i;
  
  for(i = 0; i < ARP_TABLE_SIZE; ++i) {
    //bzero (&arp_table[i], sizeof (arp_table[i]));
    memset(&arp_table[i],0,sizeof (arp_table[i]));
    arp_table[i].state = ETHARP_STATE_EMPTY;
  }
#if 0
/* FIXME: remove static entries */
  arp_table[0].state = ETHARP_STATE_STATIC;
  IP4_ADDR(&arp_table[0].ipaddr, 192, 168, 254, 1);
  {
    u8_t d[6] = {0x00, 0x50, 0xBF, 0xD0, 0x05, 0x9C};
    for (i = 0; i < 6; ++i)
      arp_table[0].ethaddr.addr[i] = d[i];
  }
  arp_table[0].p = NULL;
  arp_table[0].payload = NULL;
  arp_table[0].len = arp_table[0].tot_len = 0;
#endif
}

char *inet_ntoa ();

void arp_dump (void)
{
    /*u8_t i;
    for (i = 0; i < arp_table_last; ++i) {
	printf ("%s - %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x  %d  %p p->ref=%d len=%d\n",
		(char *) inet_ntoa (arp_table[i].ipaddr),
		(int) arp_table[i].ethaddr.addr[0],
		(int) arp_table[i].ethaddr.addr[1],
		(int) arp_table[i].ethaddr.addr[2],
		(int) arp_table[i].ethaddr.addr[3],
		(int) arp_table[i].ethaddr.addr[4],
		(int) arp_table[i].ethaddr.addr[5],
		(int) arp_table[i].state,
		(void *) arp_table[i].p, (int) (arp_table[i].p ? arp_table[i].p->ref : 0),
		(int) (arp_table[i].p ? arp_table[i].p->tot_len : 0));
    }*/
}
/*-----------------------------------------------------------------------------------*/
void
etharp_tmr(void)
{
  u8_t i;
  
  ++ctime;
  for(i = 0; i < arp_table_last; ++i) {
    if(arp_table[i].state == ETHARP_STATE_STABLE &&       
       ctime - arp_table[i].ctime >= ARP_MAXAGE) {
      DEBUGF(ETHARP_DEBUG, ("etharp_timer: expired stable entry %d.\n", i));
      arp_table[i].state = ETHARP_STATE_EMPTY;
    } else if(arp_table[i].state == ETHARP_STATE_PENDING &&
	      ctime - arp_table[i].ctime >= ARP_MAXPENDING) {
      struct pbuf *q;
      DEBUGF(ETHARP_DEBUG, ("etharp_timer: expired pending entry %d - dequeueing %p.\n", i, arp_table[i].p));

      arp_table[i].state = ETHARP_STATE_EMPTY;
      q = arp_table[i].p;
      arp_table[i].p = NULL;
      pbuf_free (q);
    }
  }  
  if (arp_table_last > 0)
    if (arp_table[arp_table_last - 1].state == ETHARP_STATE_EMPTY)
      arp_table_last--;
}
/*-----------------------------------------------------------------------------------*/
static struct pbuf *
update_arp_entry(struct ip_addr *_ipaddr, struct eth_addr *ethaddr, int create_new)
{
  u8_t i, k;
  struct pbuf *p;
  struct eth_hdr *ethhdr;
  struct ip_addr ipaddr;

  memcpy (&ipaddr, _ipaddr, sizeof (ipaddr));

  /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
  for(i = 0; i < arp_table_last; ++i) {
    /* Check if the source IP address of the incoming packet matches
       the IP address in this ARP table entry. */
    if(ip_addr_cmp (&ipaddr, &arp_table[i].ipaddr)) {
      /* First, check those entries that are already in use. */
      if(arp_table[i].state == ETHARP_STATE_STABLE) {
	/* An old entry found, update this and return. */
	for(k = 0; k < 6; ++k) {
	  arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
	}
	arp_table[i].ctime = ctime;
	return NULL;
      }
      if(arp_table[i].state == ETHARP_STATE_PENDING) {
	/* A pending entry was found, so we fill this in and return
	   the queued packet (if any). */
	for(k = 0; k < 6; ++k) {
	  arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
	}
	arp_table[i].ctime = ctime;
	if (arp_callback)
	    (*arp_callback)(arp_callback_arg, arp_table[i].ethaddr.addr, arp_table[i].ipaddr.addr);
	arp_table[i].state = ETHARP_STATE_STABLE;
	if (!arp_table[i].p) {
	  return NULL;
	} else {
	  p = arp_table[i].p;
	  arp_table[i].p = NULL;
	  p->payload = arp_table[i].payload;	
	  p->len = arp_table[i].len;
	  p->tot_len = arp_table[i].tot_len;      
	  
	  ethhdr = p->payload;
	  
	  for(k = 0; k < 6; ++k) {
	    ethhdr->dest.addr[k] = ethaddr->addr[k];
	  }
	  
	  ethhdr->type = htons(ETHTYPE_IP);	  	 	  
	  return p;
	}
      }
    }
  }
  if (create_new)
    (void) etharp_new_entry(NULL, &ipaddr, ethaddr, ETHARP_STATE_STABLE);
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
struct pbuf *
add_arp_entry(struct ip_addr *_ipaddr, struct eth_addr *ethaddr)
{
  u8_t i, k;
  struct pbuf *p;
  struct eth_hdr *ethhdr;
  struct ip_addr ipaddr;

  memcpy (&ipaddr, _ipaddr, sizeof (ipaddr));

  for(i = 0; i < arp_table_last; ++i) {
    
    /* Check entries that are pending. */
    if(arp_table[i].state == ETHARP_STATE_PENDING &&
       ip_addr_cmp(&ipaddr, &arp_table[i].ipaddr)) {

      for(k = 0; k < 6; ++k) {
	arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
      }
      arp_table[i].ctime = ctime;
      arp_table[i].state = ETHARP_STATE_STABLE;
      if (arp_callback)
	(*arp_callback)(arp_callback_arg, arp_table[i].ethaddr.addr, arp_table[i].ipaddr.addr);
      if (!arp_table[i].p) {
        return NULL;
      } else {	
	p = arp_table[i].p;
	arp_table[i].p = NULL;
	p->payload = arp_table[i].payload;	
	p->len = arp_table[i].len;
	p->tot_len = arp_table[i].tot_len;      

	ethhdr = p->payload;
	assert (ethhdr);
      
	for(k = 0; k < 6; ++k) {
	  ethhdr->dest.addr[k] = ethaddr->addr[k];
	}
      
	ethhdr->type = htons(ETHTYPE_IP);     
	return p;
      }
    }
  }

  /* If we get here, no existing ARP table entry was found, so we bail
     out. */

  return NULL;

}
/*-----------------------------------------------------------------------------------*/
struct pbuf *
etharp_ip_input(struct netif *netif, struct pbuf *p)
{
  struct ethip_hdr *hdr;
  
  hdr = p->payload;
  
  /* Only insert/update an entry if the source IP address of the
     incoming IP packet comes from a host on the local network. */
  if(!ip_addr_maskcmp(&(hdr->ip.src), &(netif->ip_addr), &(netif->netmask))) {
    return NULL;
  }
  if(ip_addr_isbroadcast(&(hdr->ip.src), &(netif->netmask))) {
    return NULL;
  }
  DEBUGF(ETHARP_DEBUG, ("etharp_ip_input: updating ETHARP table.\n"));
  if (ip_addr_isbroadcast(&(hdr->ip.dest), &(netif->netmask)))
    return update_arp_entry(&(hdr->ip.src), &(hdr->eth.src), 0);
  else
    return update_arp_entry(&(hdr->ip.src), &(hdr->eth.src), 1);
}

char *ethaddr_format (struct eth_addr *a);
char *ip_format (struct ip_addr *a);

char *
arp_format(struct etharp_hdr *hdr)
{
	/*
  static char r[256];
  r[0] = '\0';
  switch(htons(hdr->opcode)) {
  case ARP_REQUEST:
    sprintf (r, "REQUE src=%15.15s/%s dst=%15.15s/%s ", 
	ip_format (&hdr->sipaddr), ethaddr_format (&hdr->shwaddr),
	ip_format (&hdr->dipaddr), ethaddr_format (&hdr->dhwaddr));
    break;
  case ARP_REPLY:
    sprintf (r, "REPLY src=%15.15s/%s dst=%15.15s/%s ", 
	ip_format (&hdr->sipaddr), ethaddr_format (&hdr->shwaddr),
	ip_format (&hdr->dipaddr), ethaddr_format (&hdr->dhwaddr));
    break;
  default:
    sprintf (r, "%u ??? unknown?", (unsigned int) hdr->opcode);
    break;
  }
  return r;*/
}
/*-----------------------------------------------------------------------------------*/
struct pbuf *
etharp_arp_input(struct netif *netif, struct eth_addr *ethaddr, struct pbuf *p, struct pbuf **queued)
{
  struct etharp_hdr *hdr;
  u8_t i;
  
  if(p->tot_len < sizeof(struct etharp_hdr)) {
    DEBUGF(ETHARP_DEBUG, ("etharp_etharp_input: packet too short (%d/%d)\n", (int) p->tot_len, (int) sizeof(struct etharp_hdr)));
    return NULL;
  }

  hdr = p->payload;

  switch(htons(hdr->opcode)) {
  case ARP_REQUEST:
    *queued = update_arp_entry(&(hdr->sipaddr), &(hdr->shwaddr), 0);
    /* ARP request. If it asked for our address, we send out a
       reply. */
    DEBUGF(ETHARP_DEBUG, ("etharp_arp_input: ARP request\n"));
    if(!memcmp(&(hdr->dipaddr), &(netif->ip_addr), sizeof (hdr->dipaddr))) {
      pbuf_ref(p);
      hdr->opcode = htons(ARP_REPLY);

      memcpy (&(hdr->dipaddr), &(hdr->sipaddr), sizeof (hdr->dipaddr));
      memcpy (&(hdr->sipaddr), &(netif->ip_addr), sizeof (hdr->sipaddr));

      for(i = 0; i < 6; ++i) {
	hdr->dhwaddr.addr[i] = hdr->shwaddr.addr[i];
	hdr->shwaddr.addr[i] = ethaddr->addr[i];
	hdr->ethhdr.dest.addr[i] = hdr->dhwaddr.addr[i];
	hdr->ethhdr.src.addr[i] = ethaddr->addr[i];
      }

      hdr->hwtype = htons(HWTYPE_ETHERNET);
      ARPH_HWLEN_SET(hdr, 6);
      
      hdr->proto = htons(ETHTYPE_IP);
      ARPH_PROTOLEN_SET(hdr, sizeof(struct ip_addr));
      
      hdr->ethhdr.type = htons(ETHTYPE_ARP);      
      return p;
    }
    break;
  case ARP_REPLY:    
    /* ARP reply. We insert or update the ARP table. */
    DEBUGF(ETHARP_DEBUG, ("etharp_arp_input: ARP reply\n"));
    if(!memcmp (&(hdr->dipaddr), &(netif->ip_addr), sizeof (hdr->dipaddr))) {     
#if (LWIP_DHCP && DHCP_DOES_ARP_CHECK)
      dhcp_arp_reply(&hdr->sipaddr);
#endif
      /* add_arp_entry() will return a pbuf that has previously been
	 queued waiting for an ARP reply. */
    }
/* whether its destined for us or not, we update the arp table */
      return add_arp_entry(&(hdr->sipaddr), &(hdr->shwaddr));
    break;
  default:
    DEBUGF(ETHARP_DEBUG, ("etharp_arp_input: unknown type %d\n", htons(hdr->opcode)));
    break;
  }

  return NULL;
}

/* can be called from outside if we happen to find an arp mapping
through special means (for example, DHCP) */
void
etharp_add_mapping(struct ip_addr *ipaddr, struct eth_addr *ethaddr)
{
  u8_t i;
/* FIXME: should we allow just anyone to add an ARP entry? */
  for(i = 0; i < arp_table_last; ++i) {
    if(ip_addr_cmp(ipaddr, &arp_table[i].ipaddr)) {
#if 0
      u8_t k;
      for(k = 0; k < 6; ++k) {
	arp_table[i].ethaddr.addr[k] = ethaddr->addr[k];
      }
      arp_table[i].ctime = ctime;
      arp_table[i].state = ETHARP_STATE_STABLE;
#endif
      return;
    }
  }
  (void) etharp_new_entry(NULL, ipaddr, ethaddr, ETHARP_STATE_STABLE);
}

/* returns ARP_TABLE_SIZE or the index of the new entry */
static u8_t
etharp_new_entry(struct pbuf *q, struct ip_addr *ipaddr, struct eth_addr *hwaddr, enum etharp_state s)
{
    u8_t i;

    /* We now try to find an unused entry in the ARP table that we
       will setup and queue the outgoing packet. */
    for(i = 0; i < arp_table_last; ++i) {
      if(arp_table[i].state == ETHARP_STATE_EMPTY) {
	break;
      }
    }

    if(i == arp_table_last && arp_table_last < ARP_TABLE_SIZE)
      arp_table_last++;

    /* If no unused entry is found, we try to find the oldest entry and
       throw it away. */
    if(i == arp_table_last) {
      u8_t j, maxtime;
      maxtime = 0;
      j = arp_table_last;
      for(i = 0; i < arp_table_last; ++i) {
	if((arp_table[i].state == ETHARP_STATE_STABLE || arp_table[i].state == ETHARP_STATE_STATIC) &&
	   ctime - arp_table[i].ctime > maxtime) {
	  maxtime = ctime - arp_table[i].ctime;
	  j = i;
	}
      }
      i = j;
    }
    
    /* If all table entries were in pending state, we won't send out any
       more ARP requests. We'll just give up. */
    if(i == arp_table_last) {
      DEBUGF(ETHARP_DEBUG, ("etharp_output: no more ARP table entries avaliable.\n"));
      return ARP_TABLE_SIZE;
    }
  
    /* Now, i is the ARP table entry which we will fill with the new
       information. */
    memcpy (&arp_table[i].ipaddr, ipaddr, sizeof (arp_table[i].ipaddr));
    arp_table[i].ctime = ctime;
    arp_table[i].state = s;
    if (q) {
    /* Because the pbuf will be queued, we'll increase the refernce
       count. */
      arp_table[i].p = q;
      pbuf_ref (q);
      arp_table[i].payload = q->payload;
      arp_table[i].len = q->len;
      arp_table[i].tot_len = q->tot_len;
    } else {
      arp_table[i].p = NULL;
      arp_table[i].payload = NULL;
      arp_table[i].len = 0;
      arp_table[i].tot_len = 0;
    }
    if (hwaddr) {
      u8_t k;
      for(k = 0; k < 6; ++k) {
        arp_table[i].ethaddr.addr[k] = hwaddr->addr[k];
      }
      if (s == ETHARP_STATE_STABLE)
	if (arp_callback)
	  (*arp_callback)(arp_callback_arg, arp_table[i].ethaddr.addr, arp_table[i].ipaddr.addr);
    }

    DEBUGF(ETHARP_DEBUG, ("etharp_output: queueing %p\n", q));
    return i;
}

u8_t *
etharp_find(struct ip_addr *ipaddr)
{
  int i;
  for(i = 0; i < arp_table_last; ++i)
    if((arp_table[i].state == ETHARP_STATE_STABLE || arp_table[i].state == ETHARP_STATE_STATIC) &&
	                                      ip_addr_cmp(ipaddr, &arp_table[i].ipaddr))
      return arp_table[i].ethaddr.addr;
  return NULL;
}

/*-----------------------------------------------------------------------------------*/
struct pbuf *
etharp_output(struct netif *netif, struct ip_addr *ipaddr, struct pbuf *q)
{
  struct eth_addr *dest, *srcaddr, mcastaddr;
  struct eth_hdr *ethhdr;
  struct etharp_hdr *hdr;
  struct pbuf *p;
  u8_t i;

  srcaddr = (struct eth_addr *)netif->hwaddr;

  /* Make room for Ethernet header. */
  if(pbuf_header(q, sizeof(struct eth_hdr)) != 0) {    
    /* The pbuf_header() call shouldn't fail, and we'll just bail
       out if it does.. */
    DEBUGF(ETHARP_DEBUG, ("etharp_output: could not allocate room for header.\n"));
#ifdef LINK_STATS
    ++stats.link.lenerr;
#endif /* LINK_STATS */
    return NULL;
  }


  dest = NULL;
  /* Construct Ethernet header. Start with looking up deciding which
     MAC address to use as a destination address. Broadcasts and
     multicasts are special, all other addresses are looked up in the
     ARP table. */
  if(ip_addr_isany(ipaddr) ||
     ip_addr_isbroadcast(ipaddr, &(netif->netmask))) {
    dest = (struct eth_addr *)&ethbroadcast;
  } else if(ip_addr_ismulticast(ipaddr)) {
    /* Hash IP multicast address to MAC address. */
    mcastaddr.addr[0] = 0x01;
    mcastaddr.addr[1] = 0x0;
    mcastaddr.addr[2] = 0x5e;
    mcastaddr.addr[3] = ip4_addr2(ipaddr) & 0x7f;
    mcastaddr.addr[4] = ip4_addr3(ipaddr);
    mcastaddr.addr[5] = ip4_addr4(ipaddr);
    dest = &mcastaddr;
  } else {
#ifdef __PAULOS__
/* abort on insane conditions */
    if (!ip_addr_maskcmp(ipaddr, &(netif->ip_addr), &(netif->netmask)))
	return NULL;
    if (ipaddr->addr == netif->ip_addr.addr)
	return NULL;
#else
    if(!ip_addr_maskcmp(ipaddr, &(netif->ip_addr), &(netif->netmask))) {
      /* Use the IP address of the default gateway if the destination
         is NOT on the same subnet as we are. ("NOT" added 20021113 psheer@) */      
      ipaddr = &(netif->gw);
    }
#endif

    /* We try to find a stable mapping. */
    for(i = 0; i < arp_table_last; ++i) {    
      if((arp_table[i].state == ETHARP_STATE_STABLE || arp_table[i].state == ETHARP_STATE_STATIC) &&
	 ip_addr_cmp(ipaddr, &arp_table[i].ipaddr)) {
	dest = &arp_table[i].ethaddr;

#if 0
// FIXME: remove this test code
	if (!((int) rand() % 2)) {
	  dest = NULL;
	  arp_table[i].state = ETHARP_STATE_EMPTY;
	  if (arp_table[i].p)
	    pbuf_free (arp_table[i].p);
	  arp_table[i].p = NULL;
          arp_table[i].payload = NULL;
          arp_table[i].len = arp_table[0].tot_len = 0;
	}
#endif

	break;
      }
    }
  }
  
  if(dest == NULL) {
    /* No destination address has been found, so we'll have to send
       out an ARP request for the IP address. The outgoing packet is
       queued unless the queue is full. */
    
    /* We check if we are already querying for this address. If so,
       we'll bail out. */
    for(i = 0; i < arp_table_last; ++i) {
      if(arp_table[i].state == ETHARP_STATE_PENDING &&
	 ip_addr_cmp(ipaddr, &arp_table[i].ipaddr)) {
	DEBUGF(ETHARP_DEBUG, ("etharp_output: already queued\n"));
	return NULL;
      }
    }

    hdr = q->payload;
    for(i = 0; i < 6; ++i)
      hdr->ethhdr.src.addr[i] = srcaddr->addr[i];
    hdr->ethhdr.type = htons(ETHTYPE_IP);

    i = etharp_new_entry(q, ipaddr, NULL, ETHARP_STATE_PENDING);

    /* We allocate a pbuf for the outgoing ARP request packet. */
    p = pbuf_alloc(PBUF_RAW, sizeof(struct etharp_hdr) + 2, PBUF_RAM);
    if(p == NULL) {
      /* No ARP request packet could be allocated, so we forget about
	 the ARP table entry. */
      if(i != ARP_TABLE_SIZE) {
	arp_table[i].state = ETHARP_STATE_EMPTY;
	/* We decrease the reference count of the queued pbuf (which now
	   is dequeued). */
	DEBUGF(ETHARP_DEBUG, ("etharp_output: couldn't alloc pbuf for query, dequeueing %p\n", q));
      }      
      return NULL;
    }
    pbuf_header (p, (s16_t) -2);
    
    hdr = p->payload;
    
    hdr->opcode = htons(ARP_REQUEST);
    
    for(i = 0; i < 6; ++i) {
      hdr->dhwaddr.addr[i] = 0x00;
      hdr->shwaddr.addr[i] = srcaddr->addr[i];
    }
    
    memcpy (&(hdr->dipaddr), ipaddr, sizeof (hdr->dipaddr));
    memcpy (&(hdr->sipaddr), &(netif->ip_addr), sizeof (hdr->sipaddr));
    
    hdr->hwtype = htons(HWTYPE_ETHERNET);
    ARPH_HWLEN_SET(hdr, 6);
    
    hdr->proto = htons(ETHTYPE_IP);
    ARPH_PROTOLEN_SET(hdr, sizeof(struct ip_addr));
    
    for(i = 0; i < 6; ++i) {
      hdr->ethhdr.dest.addr[i] = 0xff;
      hdr->ethhdr.src.addr[i] = srcaddr->addr[i];
    }
    
    hdr->ethhdr.type = htons(ETHTYPE_ARP);      
    return p;	/* (1) */
  } else {
    /* A valid IP->MAC address mapping was found, so we construct the
       Ethernet header for the outgoing packet. */

    ethhdr = q->payload;
    
    for(i = 0; i < 6; i++) {
      ethhdr->dest.addr[i] = dest->addr[i];
      ethhdr->src.addr[i] = srcaddr->addr[i];
    }
    
    ethhdr->type = htons(ETHTYPE_IP);
  
    pbuf_ref (q);  /* <--- this is important, because the reference
must parallel that when returning over here (1). Callers must then
ALWAYS do a pbuf_free on the return value of etharp_output(). */
    return q;
  }
  

}
/*-----------------------------------------------------------------------------------*/

#endif		/* HAVE_ETH */
