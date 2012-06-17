/* paulos.c - PaulOS embedded operating system
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

#include "netif/etharp.h"
#include "netif/paulosif.h"
#include "netif/slipif.h"
#include "netif/loopif.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ipsec.h"
#include "sys/un.h"
#include "netinet/in.h"
#include "sys/types.h"
#include "etherboot.h"
#include "nic.h"

const char *inet_ntop (int af, const void *src, char *r, size_t cnt)
{
    struct in_addr *in = (struct in_addr *) src;
    unsigned long t;
    
    t = ntohl (in->s_addr);
    sprintf (r, "%d.%d.%d.%d",
	      (unsigned int) (t >> 24) & 0xFF,
	      (unsigned int) (t >> 16) & 0xFF,
	      (unsigned int) (t >> 8) & 0xFF, (unsigned int) (t >> 0) & 0xFF);
    return r;
}

char *inet_ntoa (struct in_addr in)
{
    static char s[4][16];
    static int i = 0;
    return (char *) inet_ntop (0, &in, s[i = (i + 1) % 4], 20);
}

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
inet_aton(const char *cp, struct in_addr *addr)
{
	register unsigned int val;
	register int base, n;
	register char c;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}

//extern int tcpip_startup_done;

/*#include <paulos/config.h>
#include <../libc/local.h>*/

/* FIXME: some values of ARP_TMR_INTERVAL and TCP_TMR_INTERVAL will not work */
void tcp_timer_callback()
{
/*    static useconds_t last_t = 0;
    useconds_t t;
    if ((t = timer_future_mark (0, 0) / TCP_TMR_INTERVAL) != last_t) {
	last_t = t;*/
	tcp_tmr ();
#ifdef HAVE_ETH
	{
	    //static int arp_count = 0;
	    //if (arp_count++ == (ARP_TMR_INTERVAL / TCP_TMR_INTERVAL))
		etharp_tmr ();//, arp_count = 0;
	}
#endif
    //}
}

/*-----------------------------------------------------------------------------------*/
#define CONFIG_GATEWAY "192.168.2.1"
#define CONFIG_IP "192.168.2.23"
#define CONFIG_NETMASK "255.255.255.0"

extern void likeif_init (struct netif *netif, char *device_name);
extern void paulosif_input (struct netif *netif);

struct netif *n;

void
like_tcpip_start(void)
{
  struct ip_addr ipaddr, netmask, gw;

  printf("TCP/IP initializing.\n");

/* If there is no gateway for the default interface, just set it to the IP: */
#ifndef CONFIG_GATEWAY
#define CONFIG_GATEWAY CONFIG_IP
#endif
  inet_aton(CONFIG_GATEWAY, (void *) &gw);
  inet_aton(CONFIG_IP, (void *) &ipaddr);
  inet_aton(CONFIG_NETMASK, (void *) &netmask);

  n = netif_add(&ipaddr, &netmask, &gw, likeif_init, "ignored",
		tcpip_input);
  netif_set_default(n);

  IP4_ADDR(&gw, 127, 0, 0, 1);
  IP4_ADDR(&ipaddr, 127, 0, 0, 1);
  IP4_ADDR(&netmask, 255, 0, 0, 0);

  netif_add(&ipaddr, &netmask, &gw,
	    (void (*)(struct netif *, char *)) loopif_init, "ignored",
	    tcpip_input);

  etharp_init();
  tcpip_init(0, 0);

//  callmelater_register((void (*) (void *)) tcp_timer_callback, NULL, 1);

  printf("TCP/IP initialization done.\n");
}

void tcpip_mainloop() {	
  for(;;) {
	if (like_eth_poll(1)) {
		paulosif_input(n);
	}
  }	
}

/*-----------------------------------------------------------------------------------*/
