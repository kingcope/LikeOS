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

#ifdef HAVE_NET

struct in_addr;

int inet_aton (const char *cp, struct in_addr *inp);
void ident_init (void);

extern int tcpip_startup_done;

#include <paulos/config.h>
#include <../libc/local.h>

/* FIXME: some values of ARP_TMR_INTERVAL and TCP_TMR_INTERVAL will not work */
void tcp_timer_callback (void *d)
{
    static useconds_t last_t = 0;
    useconds_t t;
    if ((t = timer_future_mark (0, 0) / TCP_TMR_INTERVAL) != last_t) {
	last_t = t;
	tcp_tmr ();
#ifdef HAVE_ETH
	{
	    static int arp_count = 0;
	    if (arp_count++ == (ARP_TMR_INTERVAL / TCP_TMR_INTERVAL))	/* 10000 / 100 */
		etharp_tmr (), arp_count = 0;
	}
#endif
    }
}

/*-----------------------------------------------------------------------------------*/
void
paulos_tcpip_start(void)
{
  struct ip_addr ipaddr, netmask, gw;
  struct netif *n;

  printf("TCP/IP initializing.\n");

/* If there is no gateway for the default interface, just set it to the IP: */
#ifndef CONFIG_GATEWAY
#define CONFIG_GATEWAY CONFIG_IP
#endif
  inet_aton(CONFIG_GATEWAY, (void *) &gw);
  inet_aton(CONFIG_IP, (void *) &ipaddr);
  inet_aton(CONFIG_NETMASK, (void *) &netmask);

//
//  IP4_ADDR(&gw, 10, 0, 0, 1);
//  IP4_ADDR(&ipaddr, 10, 0, 0, 1);
//  IP4_ADDR(&netmask, 255, 0, 0, 0);
//
//  IP4_ADDR(&gw, 172, 16, 0, 1);
//  IP4_ADDR(&ipaddr, 172, 16, 4, 50);
//  IP4_ADDR(&netmask, 255, 254, 0, 0);
//
//  IP4_ADDR(&gw, 10, 10, 128, 5);
//  IP4_ADDR(&ipaddr, 10, 10, 128, 51);
//  IP4_ADDR(&netmask, 255, 255, 248, 0);
//
//  IP4_ADDR(&gw, 10, 10, 128, 5);
//  IP4_ADDR(&ipaddr, 10, 10, 132, 188);
//  IP4_ADDR(&netmask, 255, 255, 248, 0);
//
//  IP4_ADDR(&gw, 10, 10, 128, 5);
//  IP4_ADDR(&ipaddr, 10, 10, 130, 137);
//  IP4_ADDR(&netmask, 255, 255, 248, 0);
//
//  IP4_ADDR(&gw, 192, 168, 254, 1);
//  IP4_ADDR(&ipaddr, 192, 168, 254, 2);
//  IP4_ADDR(&netmask, 255, 255, 0, 0);
//

#ifdef HAVE_ETH
  n = netif_add(&ipaddr, &netmask, &gw, paulosif_init, "/dev/eth0",
		tcpip_input);
  netif_set_default(n);
#elif defined(HAVE_SLIP)
  n = netif_add(&ipaddr, &netmask, &gw, slipif_init, "/dev/ttyS1",
		tcpip_input);
  netif_set_default(n);
#endif

  IP4_ADDR(&gw, 127, 0, 0, 1);
  IP4_ADDR(&ipaddr, 127, 0, 0, 1);
  IP4_ADDR(&netmask, 255, 0, 0, 0);

  netif_add(&ipaddr, &netmask, &gw,
	    (void (*)(struct netif *, char *)) loopif_init, "ignored",
	    tcpip_input);

#ifdef HAVE_ETH
  etharp_init();
#endif
  tcpip_init(0, 0);
#ifdef HAVE_IDENT
  ident_init();
#endif

  callmelater_register((void (*) (void *)) tcp_timer_callback, NULL, 1);

#if 0
#warning test code to be removed
  IP4_ADDR(&netmask, 255, 255, 255, 255);
  IP4_ADDR(&ipaddr, 196, 15, 138, 4);
  IP4_ADDR(&gw, 196, 15, 138, 4);
  sa_add (1, "ABCDEFGHIJKLMNOPQRST", 16, 1000, 1001, &blfmd5_alg, &netmask, &ipaddr, &gw);
#endif

  printf("TCP/IP initialization done.\n");
}

#endif	/* HAVE_NET */

/*-----------------------------------------------------------------------------------*/
