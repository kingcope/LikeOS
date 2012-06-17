/* packet_dump.c - PaulOS embedded operating system
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


#include <stdio.h>
#include <string.h>

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

#include <sys/types.h>
#include <unistd.h>
#include <socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <../../../../libc/local.h>

struct etharp_hdr;

char *arp_format (struct etharp_hdr *hdr);

char *ethaddr_format (struct eth_addr *a)
{
    static int i = 0;
    static char r[8][40];
    i = (i + 1) % 8;
    snprintf (r[i], sizeof (r[0]), "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
	      (unsigned int) a->addr[0], (unsigned int) a->addr[1], (unsigned int) a->addr[2],
	      (unsigned int) a->addr[3], (unsigned int) a->addr[4], a->addr[5]);
    return r[i];
}

char *ip_format (struct ip_addr *a)
{
    union {
	struct ip_addr a;
	unsigned char n[4];
    } q;
    static int i = 0;
    static char r[8][40];
    memcpy (&q.a, a, sizeof (a));
    i = (i + 1) % 8;
    snprintf (r[i], sizeof (r[0]), "%u.%u.%u.%u", (unsigned int) q.n[0], (unsigned int) q.n[1],
	      (unsigned int) q.n[2], (unsigned int) q.n[3]);
    return r[i];
}

char *tcpflags_format (unsigned char flags)
{
    static char r[128];
    r[0] = '\0';
    if (flags & TCP_FIN)
	strcat (r, ",FIN");
    if (flags & TCP_SYN)
	strcat (r, ",SYN");
    if (flags & TCP_RST)
	strcat (r, ",RST");
    if (flags & TCP_PSH)
	strcat (r, ",PSH");
    if (flags & TCP_ACK)
	strcat (r, ",ACK");
    if (flags & TCP_URG)
	strcat (r, ",URG");
    return *r ? &r[1] : "";
}

char *icmp_format (unsigned char *p)
{
    static char r[128], *s;
    r[0] = '\0';
    switch (p[0]) {
    case ICMP_ER:
	strcat (r, "ER");
	break;
    case ICMP_DUR:
	strcat (r, "DUR");
	break;
    case ICMP_SQ:
	strcat (r, "SQ");
	break;
    case ICMP_RD:
	strcat (r, "RD");
	break;
    case ICMP_ECHO:
	strcat (r, "ECHO");
	break;
    case ICMP_TE:
	strcat (r, "TE");
	break;
    case ICMP_PP:
	strcat (r, "PP");
	break;
    case ICMP_TS:
	strcat (r, "TS");
	break;
    case ICMP_TSR:
	strcat (r, "TSR");
	break;
    case ICMP_IRQ:
	strcat (r, "IRQ");
	break;
    case ICMP_IR:
	strcat (r, "IR");
	break;
    default:
	sprintf (r, "%u ??? type?", (unsigned int) p[0]);
	break;
    }
    s = r + strlen (r);
    sprintf (s, " code=%u", (unsigned int) p[1]);
    return r;
}

const struct eth_type {
    unsigned short number;
    const char *description;
} eth_type[] = {
    {
    0x0060, "Ethernet Loopback packet"}, {
    0x0200, "Xerox PUP packet"}, {
    0x0201, "Xerox PUP Addr Trans packet"}, {
    0x0800, "Internet Protocol packet"}, {
    0x0805, "CCITT X.25"}, {
    0x0806, "Address Resolution packet"}, {
    0x08FF, "G8BPQ AX.25 Ethernet Packet"}, {
    0x0a00, "Xerox IEEE802.3 PUP packet"}, {
    0x0a01, "Xerox IEEE802.3 PUP Addr Trans packet"}, {
    0x6000, "DEC Assigned proto"}, {
    0x6001, "DEC DNA Dump/Load"}, {
    0x6002, "DEC DNA Remote Console"}, {
    0x6003, "DEC DNA Routing"}, {
    0x6004, "DEC LAT"}, {
    0x6005, "DEC Diagnostics"}, {
    0x6006, "DEC Customer use"}, {
    0x6007, "DEC Systems Comms Arch"}, {
    0x8035, "Reverse Addr Res packet"}, {
    0x809B, "Appletalk DDP"}, {
    0x80F3, "Appletalk AARP"}, {
    0x8100, "802.1Q VLAN Extended Header"}, {
    0x8137, "IPX over DIX"}, {
    0x86DD, "IPv6 over bluebook"}, {
    0x8863, "PPPoE discovery messages"}, {
    0x8864, "PPPoE session messages"}, {
    0x884c, "MultiProtocol Over ATM"}, {
    0x8884, "Frame-based ATM Transport"}, {
    0, NULL}
};

static void __packet_dump_ip (unsigned char *q, int len)
{
    struct ip_hdr *iphdr;
    struct tcp_hdr *tcphdr;
    struct icmp_echo_hdr *icmp;
    struct udp_hdr *udphdr;

/* IP header */
    iphdr = (struct ip_hdr *) q;
    len = ntohs (IPH_LEN (iphdr));
    printf ("s/d=%16.16s/%16.16s len=%4.0u ", ip_format (&iphdr->src), ip_format (&iphdr->dest), len);
    q += IPH_HL (iphdr) * 4;

/* Payload */
    switch (IPH_PROTO (iphdr)) {
    case IP_PROTO_UDP:
	printf ("UDP  ");
	udphdr = (struct udp_hdr *) q;
	printf ("s/d=%5.0u/%5.0u ", ntohs (udphdr->src), ntohs (udphdr->dest));
	break;
    case IP_PROTO_TCP:
	printf ("TCP  ");
	tcphdr = (struct tcp_hdr *) q;
	printf ("s/d=%5.0u/%5.0u s/a=%10.0lu/%10.0lu %s", (unsigned int) ntohs (tcphdr->src), (unsigned int) ntohs (tcphdr->dest), 
		(unsigned long) ntohl (tcphdr->seqno), (unsigned long) ntohl (tcphdr->ackno), tcpflags_format (TCPH_FLAGS (tcphdr)));
	break;
    case IP_PROTO_ICMP:
	printf ("ICMP ");
	icmp = (struct icmp_echo_hdr *) q;
	printf ("%s ", icmp_format (q));
	break;
    default:
	printf ("%4.0u ??? protocol? ", (unsigned int) IPH_PROTO (iphdr));
	break;
    }
    printf ("\n");
}

void packet_dump (unsigned char *q, int len, char *msg)
{
    int i;
    unsigned short type;
    struct eth_hdr *ethhdr;

/* Ethernet header */
    ethhdr = (struct eth_hdr *) q;
    printf ("%s: ", msg);
    printf ("src=%s dst=%s len=%4.0u ", ethaddr_format (&ethhdr->src), ethaddr_format (&ethhdr->dest), len);
    q += 14, len -= 14;

    switch ((type = ntohs (ethhdr->type))) {
    case ETHTYPE_IP:
	printf ("IP  ");
	__packet_dump_ip (q, len);
	return;
    case ETHTYPE_ARP:
	printf ("ARP %s\n", (char *) arp_format ((struct etharp_hdr *) ethhdr));
	return;
    default:
	for (i = 0; eth_type[i].description; i++) {
	    if (type == eth_type[i].number) {
		printf ("??? unknown? [%s]\n", eth_type[i].description);
		return;
	    }
	}
	printf ("??? unknown? [%hu]\n", type);
	return;
    }
}

void packet_dump_ip (unsigned char *q, int len, char *msg)
{
    printf ("%s:                                                      IP  ", msg);
    __packet_dump_ip (q, len);
}

