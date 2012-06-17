/* lwipopts2.h - PaulOS embedded operating system
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

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
#define MEM_ALIGNMENT           1
#define MEM_SIZE                1600
#define MEMP_NUM_PBUF           16
#define MEMP_NUM_UDP_PCB        4
#define MEMP_NUM_TCP_PCB        5
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_TCP_SEG        16
#define MEMP_NUM_SYS_TIMEOUT    3
#define MEMP_NUM_NETBUF         2
#define MEMP_NUM_NETCONN        4
#define MEMP_NUM_API_MSG        8
#define MEMP_NUM_TCPIP_MSG      8
#define MEM_RECLAIM             1
#define MEMP_RECLAIM            1
#define PBUF_POOL_SIZE          6
#define PBUF_POOL_BUFSIZE       128
#define PBUF_LINK_HLEN          16
#define LWIP_TCP                1
#define TCP_TTL                 255
#define TCP_QUEUE_OOSEQ         1
#define TCP_MSS                 128
#define TCP_SND_BUF             256
#define TCP_SND_QUEUELEN        4 * TCP_SND_BUF/TCP_MSS
#define TCP_WND                 1024
#define TCP_MAXRTX              12
#define TCP_SYNMAXRTX           4
#define ARP_TABLE_SIZE 10
#define IP_FORWARD              1
#define IP_OPTIONS              1
#define ICMP_TTL                255
#define LWIP_DHCP               0
#define DHCP_DOES_ARP_CHECK     1
#define LWIP_UDP                1
#define UDP_TTL                 255
#define STATS
#ifdef STATS
#define LINK_STATS
#define IP_STATS
#define ICMP_STATS
#define UDP_STATS
#define TCP_STATS
#define MEM_STATS
#define MEMP_STATS
#define PBUF_STATS
#define SYS_STATS
#endif /* STATS */
#endif /* __LWIPOPTS_H__ */
