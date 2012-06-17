/* slipif.c - PaulOS embedded operating system
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

#ifdef HAVE_SLIP

#include <fcntl.h>
#include <sys/ioctl.h>
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
#include "netif/validate.h"

#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <socket.h>

#include <../../../../libc/local.h>

#define SLIP_DEVICE "/dev/ttyS1"

#define IFNAME0 's'
#define IFNAME1 'l'

#define SLIP_MTU	384
#define N_SEND_PACKETS	128
#define RAW_BUFFER	SLIP_MTU

struct slip_packet {
    int len, offset, used;
    unsigned char data[4];
};

struct slipif {
    int fd;
    int next_send_packet;
    int recv_len;
    int escape;
    long total_recieved;
    unsigned char found_CLIENT;
    unsigned char CLIENT_char;
    unsigned char pad0;
    unsigned char pad1;
    struct slip_packet *send_packet[N_SEND_PACKETS];
    unsigned char recv_decoded[SLIP_MTU + sizeof (int)];
    unsigned char recv_raw[RAW_BUFFER];
};

#define min(a,b) ((a) < (b) ? (a) : (b))

/* see rfc1055.txt */
#define END             0300
#define ESC             0333
#define ESC_END         0334
#define ESC_ESC         0335

static int slip_esc (struct pbuf *p, unsigned char *d)
{
    unsigned char *ptr = d;
    unsigned char c;
    *ptr++ = END;
    for (; p; p = p->next) {
	unsigned char *s;
#ifdef HAVE_MAGIC
	magic (p, PBUF_MAGIC);
#endif
	assert (p->len < 16384);
	for (s = p->payload; s < (unsigned char *) p->payload + p->len; s++) {
	    switch ((c = *s)) {
	    case END:
		*ptr++ = ESC;
		*ptr++ = ESC_END;
		break;
	    case ESC:
		*ptr++ = ESC;
		*ptr++ = ESC_ESC;
		break;
	    default:
		*ptr++ = c;
		break;
	    }
	}
    }
    *ptr++ = END;
    return (ptr - d);
}

/*-----------------------------------------------------------------------------------*/
static void slipif_write_poll (struct netif *netif)
{
    struct slipif *slipif = (struct slipif *) netif->state;
    struct slip_packet *s, **p;
    int i;
    for (i = 0; i < N_SEND_PACKETS; i++) {
	int l;
	p = &slipif->send_packet[(slipif->next_send_packet + i) % N_SEND_PACKETS];
	s = *p;
	if (!s)
	    continue;
	l = write (slipif->fd, s->data + s->offset, s->len - s->offset);
	s->offset += l > 0 ? l : 0;
	if (s->offset == s->len) {
	    mem_free (*p);
	    *p = NULL;
	}
	break;
    }
}

/*-----------------------------------------------------------------------------------*/
static err_t slipif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    struct slipif *slipif = (struct slipif *) netif->state;
    struct slip_packet *s;

    if (mem_usage_percent ((size_t) 0) > MEM_USAGE_HIGH)	/* suppress packet output */
	return ERR_OK;

    if (!p->len)
	return ERR_OK;

    if (slipif->send_packet[slipif->next_send_packet])
	return ERR_OK;		/* output queue full */

//    set_cpu_busy (2);

    s = slipif->send_packet[slipif->next_send_packet] =
	mem_malloc (sizeof (struct slip_packet) + p->tot_len * 2);
    slipif->next_send_packet = (slipif->next_send_packet + 1) % N_SEND_PACKETS;
    s->offset = 0;

    s->len = slip_esc (p, s->data);

//    set_cpu_asleep (2);

    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
static void slipif_poll (struct netif *netif)
{
    struct slipif *s;
    struct pbuf *p = 0;
    unsigned char *d;
    int len;

    slipif_write_poll (netif);

    s = netif->state;

    ioctl (s->fd, FIONREAD, &len);
    if (len <= 0)
	return;

//    set_cpu_busy (2);

/* read partial packet */
    len = read (s->fd, s->recv_raw, RAW_BUFFER);

    s->total_recieved += (len > 0) ? len : 0;

/* search in the first 1k of data for a windows "CLIENT" request string: */
    if (s->total_recieved < 1024 && s->found_CLIENT < 3) {
	int i;
	for (i = 0; i < len; i++) {
	    if (s->recv_raw[i] == "CLIENT"[s->CLIENT_char]) {
		s->CLIENT_char++;
		if (s->CLIENT_char >= 6) {
		    s->CLIENT_char = 0;
		    write (s->fd, "CLIENTSERVER", 12);	/* FIXME: and pray */
		    s->found_CLIENT++;
		}
	    } else {
		s->CLIENT_char = 0;
	    }
	}
    }

/* FIXME: cope with len < 0 */
    d = &s->recv_raw[0];
    while (len-- > 0) {
	switch (*d) {
/* escaped escape character */
	case ESC_ESC:
	    s->recv_decoded[s->recv_len++] = s->escape ? ESC : *d;
	    s->escape = 0;
	    break;
/* escaped escape end */
	case ESC_END:
	    s->recv_decoded[s->recv_len++] = s->escape ? END : *d;
	    s->escape = 0;
	    break;
/* escape character - go into escape mode */
	case ESC:
	    s->escape = 1;
	    break;
/* end character - scoop packet */
	case END:
	    if (mem_usage_percent ((size_t) 0) < MEM_USAGE_MEDIUM) {
		if (s->recv_len > 20 && s->recv_len <= SLIP_MTU) {
		    p = pbuf_alloc (PBUF_LINK, s->recv_len, PBUF_POOL);
		    memcpy (p->payload, s->recv_decoded, s->recv_len);

#if 0
{
int i;
for (i=0;i<s->recv_len;i++)
printf ("%02x%c", (unsigned int) s->recv_decoded[i], ((i + 1) % 16) ? ' ' : '\n');
}
#endif

		    if (packet_validate (p, netif, NULL)) {
			pbuf_free (p);
		    } else {
			netif->input (p, netif);
		    }
		}
	    }
	    s->recv_len = 0;
	    s->escape = 0;
	    break;
/* any other character */
	default:
	    if (s->recv_len < SLIP_MTU + 1)
		s->recv_decoded[s->recv_len++] = *d;
	    s->escape = 0;
	    break;
	}
	d++;
    }
//    set_cpu_asleep (2);
}

/*-----------------------------------------------------------------------------------*/
void slipif_init (struct netif *netif, char *device_name)
{
    struct termios t;
    struct slipif *s;
    int r;
    s = mem_malloc (sizeof (struct slipif));
    memset (s, 0, sizeof (struct slipif));
    netif->state = s;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = slipif_output;
    netif->mtu = SLIP_MTU;
    s->fd = open (device_name, O_RDWR);
    if (fcntl (s->fd, F_SETFL, O_NONBLOCK))
	die ("fcntl(%s,O_NONBLOCK)", device_name);
    r = tcgetattr (s->fd, &t);
    assert (!r);
    cfsetospeed (&t, B460800);
    cfmakeraw (&t);
    r = tcsetattr (s->fd, TCSANOW, &t);
    callmelater_register ((void (*)(void *)) slipif_poll, (void *) netif, 1);
}

/*-----------------------------------------------------------------------------------*/

#endif				/* HAVE_SLIP */
