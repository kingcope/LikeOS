/* tunif.c - PaulOS embedded operating system
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


#ifdef HAVE_TUN

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
#include <unistd.h>
#include <socket.h>

#include <../../../../libc/local.h>

#define IFNAME0 't'
#define IFNAME1 'p'

struct tunif {
    int fd[2];
};

#define min(a,b) ((a) < (b) ? (a) : (b))

/*-----------------------------------------------------------------------------------*/
static err_t tunif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    struct pbuf *q;
    struct tunif *tunif = (struct tunif *) netif->state;
    int len = 0;

    if (mem_usage_percent ((size_t) 0) > MEM_USAGE_HIGH)	/* suppress packet output */
	return ERR_OK;

    if (ioctl (tunif->fd[0], TIOCOUTQ, &len))
	die ("driver could not ioctl(TIOCOUTQ)");
/* to many competing thresholds create a thrashing
effect, so hard limit the queue here: */
    if (len > 32768)
	return ERR_OK;

    len = p->tot_len;
    if (ioctl (tunif->fd[0], FIONWRITE, &len))
	die ("driver could not allocate space for packet");
    if (p->tot_len == len) {
	for (q = p; q != NULL && len > 0; q = q->next) {
	    int r;
	    if ((r = write (tunif->fd[0], q->payload, q->len)) != q->len)
		die ("driver could not send packet returned %d/%d, [%s] ", r, q->len, strerror (errno));
	    len -= q->len;
	}

	if (len != 0)
	    die ("len != 0, p = %p", (void *) p);
    }

    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
static void tunif_input (struct netif *netif)
{
    struct tunif *tunif;
    struct pbuf *p = 0;
    int len, r;
    tunif = netif->state;
    ioctl (tunif->fd[0], FIONREAD, &len);
    if (len <= 0)
	return;
    if (mem_usage_percent ((size_t) 0) < MEM_USAGE_MEDIUM)	/* suppress packet input */
	p = pbuf_alloc (PBUF_LINK, len, PBUF_POOL);
    if (!p) {
	char buf[64];
	r = read (tunif->fd[0], buf, min (len, sizeof (buf)));
	assert (r == len || r == sizeof (buf));
	return;
    } else {
/* discard the packet */
	len = min (p->len, len);
	r = read (tunif->fd[0], p->payload, len);
	assert (r == len);
    }
    if (packet_validate (p, netif, NULL)) {
//	printf ("tunif - packet validation failed\n");
	pbuf_free (p);
	return;
    }
    netif->input (p, netif);
}

/*-----------------------------------------------------------------------------------*/
void tunif_init (struct netif *netif, char *device_name)
{
    struct tunif *tunif;
    int r;

    tunif = mem_malloc (sizeof (struct tunif));
    memset (tunif, 0, sizeof (struct tunif));
    netif->state = tunif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = tunif_output;
    r = socketpair (0, 0, 0, tunif->fd);
    assert (!r);
    callmelater_register ((void (*)(void *)) tunif_input, (void *) netif, 1);
}

int tunif_control (struct netif *netif)
{
    struct tunif *tunif = (struct tunif *) netif->state;
    return tunif->fd[1];
}

#endif		/* HAVE_TUN */

/*-----------------------------------------------------------------------------------*/
