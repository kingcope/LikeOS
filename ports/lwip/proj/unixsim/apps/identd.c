/* identd.c - PaulOS embedded operating system
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

#ifdef __PAULOS__
#ifdef HAVE_IDENT

#include <string.h>
#include <assert.h>

#include "lwip/debug.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include <paulos/config.h>

#define MAX_REQUEST 19

struct ident_state {
  char request[MAX_REQUEST + 1];
  int done_read;
  int timer;
  struct in_addr remote;
};

/*-----------------------------------------------------------------------------------*/
static void
conn_err(void *arg, err_t err)
{
  struct ident_state *hs;

  hs = arg;
  if (hs)
    mem_free(hs);
}
/*-----------------------------------------------------------------------------------*/
static void
close_conn(struct tcp_pcb *pcb, struct ident_state *hs)
{
  if (!pcb)
    return;
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  if (hs)
    mem_free(hs);
  tcp_close(pcb);
}
/*-----------------------------------------------------------------------------------*/
static err_t
ident_poll(void *arg, struct tcp_pcb *pcb)
{
  struct ident_state *hs;
  hs = arg;

  if(!arg) {
    tcp_close(pcb);
    return ERR_OK;
  }

  if (hs->timer++ > 2)
    tcp_close(pcb);

  return ERR_OK;
}

static char *(*ident_callback) (struct sockaddr_in *local, struct sockaddr_in *remote) = 0;

void
ident_set_callback (char *(*cb) (struct sockaddr_in *, struct sockaddr_in *))
{
  ident_callback = cb;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

/*-----------------------------------------------------------------------------------*/
static err_t
ident_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  char response[80], *r = 0;
  struct sockaddr_in local = { 0, 0, 0 }, remote = {
  0, 0, 0};
  int local_port = 0, remote_port = 0, l;
  struct ident_state *hs;

  if (err != ERR_OK)
    die("err != ERR_OK is not supposed to happen");

  hs = arg;

  if (!p) {
    close_conn(pcb, hs);
    return ERR_OK;
  }

  /* Inform TCP that we have taken the data. */
  tcp_recved(pcb, p->tot_len);

  memcpy(hs->request + hs->done_read, p->payload,
	 min(p->len, MAX_REQUEST - hs->done_read));
  hs->done_read += p->len;
  pbuf_free(p);
  if (hs->done_read >= MAX_REQUEST)
    goto error;
  if (!strchr(hs->request, '\n'))	/* partial message */
    return ERR_OK;
  if (sscanf(hs->request, "%d , %d", &local_port, &remote_port) != 2)
    goto error;
  local.sin_port = htons((unsigned short) local_port);
  remote.sin_port = htons((unsigned short) remote_port);
  remote.sin_addr = hs->remote;
  if (ident_callback) {
    if (!(r = (*ident_callback) (&local, &remote)))
      goto error;
  } else {
    r = "root";
  }
  l = snprintf(response, 80, "%hu , %hu : USERID : OTHER :%s\r\n",
	       htons(local.sin_port), htons(remote.sin_port), r);
  if (l > tcp_write_space(pcb, MEM_USAGE_MEDIUM_HIGH)) {
    close_conn(pcb, hs);
    return ERR_OK;
  }
  tcp_write(pcb, response, l, 1);
/* reset everything: */
  memset(hs, 0, sizeof(*hs));
  hs->remote = remote.sin_addr;
  return ERR_OK;

error:
  l = snprintf(response, 80, "%hu , %hu : ERROR : UNKNOWN-ERROR\r\n",
	       htons(local.sin_port), htons(remote.sin_port));
  if (l <= tcp_write_space(pcb, MEM_USAGE_MEDIUM_HIGH))
    tcp_write(pcb, response, l, 1);
  close_conn(pcb, hs);

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
ident_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct ident_state *hs;

  /* Allocate memory for the structure that holds the state of the
     connection. */
  hs = mem_malloc(sizeof(struct ident_state));

  if(hs == NULL) {
    printf("ident_accept: Out of memory\n");
    return ERR_MEM;
  }
  
  /* Initialize the structure. */
  memset (hs, 0, sizeof (*hs));
  if (sizeof (hs->remote) != sizeof (pcb->remote_ip))
    die ("sizeof (hs.remote) != sizeof (pcb->remote_ip)");
  memcpy (&hs->remote, &pcb->remote_ip, sizeof (hs->remote));

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the ident_recv() function. */
  tcp_recv(pcb, ident_recv);

  tcp_err(pcb, conn_err);
  
  tcp_poll(pcb, ident_poll, 10);
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
ident_init(void)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 113);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, ident_accept);
}
/*-----------------------------------------------------------------------------------*/

#endif	/* HAVE_IDENT */
#endif	/* __PAULOS__ */

