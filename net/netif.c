/* paulosif.c - PaulOS embedded operating system
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


#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/icmp.h"
#include "lwip/tcp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "netif/paulosif.h"
#include "netif/validate.h"
#include "etherboot.h"
#include "nic.h"

#include <sys/types.h>

#define IFNAME0 'e'
#define IFNAME1 't'

void paulosif_input (struct netif *netif);
err_t paulosif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);

void low_level_init (struct netif *netif)
{
    struct genericif *genericif;
    char buf[100];

    genericif = netif->state;

    memcpy(genericif->ethaddr->addr, nic.node_addr, 6);
    
    /* Do whatever else is needed to initialize interface. */
/*    netif->fd = genericif->fd = open (genericif->device_name, 0, 0);
    if (genericif->fd < 0)
	die ("can't open %s", genericif->device_name);

    ioctl (genericif->fd, ETHGETHWADDR, genericif->ethaddr->addr);

    printf ("HW Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
	    genericif->ethaddr->addr[0], genericif->ethaddr->addr[1],
	    genericif->ethaddr->addr[2], genericif->ethaddr->addr[3],
	    genericif->ethaddr->addr[4], genericif->ethaddr->addr[5]);

    snprintf (buf, sizeof (buf), "ifconfig tap0 inet %d.%d.%d.%d",
	      ip4_addr1 (&(netif->gw)),
	      ip4_addr2 (&(netif->gw)), ip4_addr3 (&(netif->gw)), ip4_addr4 (&(netif->gw)));

    DEBUGF (TAPIF_DEBUG, ("paulosif_init: system(\"%s\");\n", buf));

    callmelater_register ((void (*)(void *)) paulosif_input, (void *) netif, 1);*/
}

#define min(a,b) ((a) < (b) ? (a) : (b))

err_t low_level_output (struct genericif *genericif, struct pbuf *p)
{
    struct pbuf *q;
    int len = p->tot_len;

    magic (p, PBUF_MAGIC);
/*    if (mem_usage_percent ((size_t) 0) > MEM_USAGE_HIGH) {
	printf ("low_level_output: memory low - dropping packet\n");
	return ERR_OK;
    }*/

/*    if (enable_packet_dump)
	packet_dump (p->payload, (int) p->len, "OUT ");*/

/*    if (ioctl (genericif->fd, FIONWRITE, &len))
	die ("driver could not allocate space for packet");*/
    if (p->tot_len == len) {
	for (q = p; q != NULL && len > 0; q = q->next) {
	    /*int r;
	    if ((r = write (genericif->fd, q->payload, q->len)) != q->len)
		die ("driver could not send packet returned %d/%d, [%s] ", r, q->len, strerror (errno));
	    len -= q->len;*/
	    
	    like_eth_transmit(0,0,q->len,q->payload);
	}
	}
/*
	if (len != 0)
	    die ("len != 0, p = %p", (void *) p);
    }*/
    return ERR_OK;
}

struct pbuf *low_level_input (struct genericif *genericif)
{
    struct pbuf *p = 0;
    int len, r;
    
    len = nic.packetlen;
    //ioctl (genericif->fd, FIONREAD, &len);
    //if (mem_usage_percent ((size_t) 0) < MEM_USAGE_MEDIUM) {	/* suppress packet input */
	p = pbuf_alloc (PBUF_RAW, len + 2, PBUF_RAM);
	pbuf_header (p, (s16_t) - 2);
    //}
/*    if (!p) {
	char buf[64];
	r = read (genericif->fd, buf + 2, min (len, sizeof (buf) - 2));
	assert (r == len || r == sizeof (buf) - 2);
	return NULL;
    } else {*/
	len = min (p->len, len);
	//r = read (genericif->fd, p->payload, len);
	memcpy(p->payload, nic.packet, len);
	
	
	//assert (r == len);
    //}
    /*if (enable_packet_dump)
	packet_dump (p->payload, (int) p->len, "IN  ");*/
    return p;
}


err_t paulosif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    struct pbuf *q;
    struct genericif *genericif;
    genericif = netif->state;

    q = etharp_output (netif, ipaddr, p);

    if (q != NULL) {
	low_level_output (genericif, q);
	pbuf_free (q);
    }
    return ERR_OK;
}

/*static void *pppoe_user_data;
static void (*pppoe_input_callback) (void *pppoe_user_data, struct pbuf * p, u16_t ether_type) = NULL;

void set_pppoe_callback (void (*c) (void *, struct pbuf *, u16_t ether_type), void *user_data)
{
    pppoe_input_callback = c;
    pppoe_user_data = user_data;
}

static void *packet_filter_data;
static int (*packet_filter_callback) (void *, unsigned char *, int) = NULL;

void set_packet_filter_callback (int (*c) (void *, unsigned char *, int), void *user_data)
{
    packet_filter_callback = c;
    packet_filter_data = user_data;
}
*/
void paulosif_input (struct netif *netif)
{
    struct genericif *genericif;
    struct eth_hdr *ethhdr;
    struct pbuf *p, *q = NULL, *r = NULL;
    int len;
    u16_t ether_type;

    genericif = netif->state;
    //ioctl (genericif->fd, FIONREAD, &len);

//    if (len <= 0)
//	return;

    p = low_level_input (genericif);

    if (!p) {
	//DEBUGF (TAPIF_DEBUG, ("paulosif_input: low_level_input returned NULL\n"));
	return;
    }
    
    ethhdr = p->payload;
    ether_type = ntohs (ethhdr->type);

  /*  if (pppoe_input_callback && (ether_type == 0x8863 || ether_type == 0x8864)) {
//printf ("paulosif_input: got PPPoE packet, %d bytes\n", (int) p->len);
	(*pppoe_input_callback) (pppoe_user_data, p, ether_type);
	pbuf_free (p);
	return;
    }
*/
  /*  if (packet_validate (p, netif, genericif)) {
	pbuf_free (p);
	return;
    }
*/
  /*  if (packet_filter_callback) {
	if ((*packet_filter_callback) (packet_filter_data, (unsigned char *) p->payload, (int) p->len)) {
	    pbuf_free (p);
	    return;
	}
    }
*/
    switch (ether_type) {
    case ETHTYPE_IP:
	//DEBUGF (TAPIF_DEBUG, ("paulosif_input: IP packet\n"));
	q = etharp_ip_input (netif, p);
	pbuf_header (p, -14);
	//netif->input (p, netif);	/* segfault */
	ip_input(p, netif);
	break;
    case ETHTYPE_ARP:
	//DEBUGF (TAPIF_DEBUG, ("paulosif_input: ARP packet\n"));
	q = etharp_arp_input (netif, genericif->ethaddr, p, &r);
	pbuf_free (p);
	break;
    default:
	pbuf_free (p);
	break;
    }
    if (q != NULL) {
	magic (q, PBUF_MAGIC);
	low_level_output (genericif, q);
	pbuf_free (q);		/* free'd */
    }
    if (r != NULL) {
	magic (r, PBUF_MAGIC);
	low_level_output (genericif, r);
	pbuf_free (r);		/* free'd */
    }
}


void likeif_input(struct netif *nif) {
	paulosif_input(nif);
}

void likeif_init (struct netif *netif, char *device_name)
{
    struct genericif *genericif;

    genericif = mem_malloc (sizeof (struct genericif));
    memset (genericif, 0, sizeof (struct genericif));
    netif->state = genericif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = paulosif_output;

    genericif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);
    genericif->device_name = device_name;

    low_level_init (netif);
}
