/* lwipopts-paulos.h - PaulOS embedded operating system
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

//#include <paulos/tcpmss.h>

#define MEM_ALIGNMENT           4
#undef MEMP_NUM_SYS_TIMEOUT
#define PBUF_LINK_HLEN          16
#define LWIP_TCP                1
#define TCP_TTL                 255
#define TCP_QUEUE_OOSEQ         1
#ifndef TCP_MSS
#define TCP_MSS                 TCP_MSS_LARGE
#endif
#define TCP_SND_QUEUELEN        128
#define TCP_WND_MAX             16384

#define TCP_SND_BUF		(TCP_MSS_LARGE*2)
#define TCP_WND			(TCP_MSS_LARGE*2)

#define TCP_MAXRTX              12
#define TCP_SYNMAXRTX           24
#define ARP_TABLE_SIZE		120
#define IP_FORWARD              1
#define IP_OPTIONS              1
#define ICMP_TTL                64
#define LWIP_DHCP               0
#define DHCP_DOES_ARP_CHECK     1
#define LWIP_UDP                1
#define UDP_TTL                 64
#undef STATS

#endif /* __LWIPOPTS_H__ */

