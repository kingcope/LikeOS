/* tcp.c - PaulOS embedded operating system
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
 * $Id: tcp.c,v 1.10 2003/08/18 13:52:56 psheer Exp $
 */

/*-----------------------------------------------------------------------------------*/
/* tcp.c
 *
 * This file contains common functions for the TCP implementation, such as functinos
 * for manipulating the data structures and the TCP timer functions. TCP functions
 * related to input and output is found in tcp_input.c and tcp_output.c respectively.
 *
 */
/*-----------------------------------------------------------------------------------*/

#include <sequencer.h>
#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"

#include "lwip/tcp.h"
#include <assert.h>
//#include <paulos/tcpmss.h>

#define TCP_MSS_SMALL		336
#define TCP_MSS_MEDIUM		(TCP_MSS_SMALL*2)
#define TCP_MSS_LARGE		(TCP_MSS_SMALL*4)

/* Incremented every coarse grained timer shot
   (typically every 500 ms, determined by TCP_COARSE_TIMEOUT). */
u32_t tcp_ticks;
const u8_t tcp_backoff[13] =
    { 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7};

#ifndef BIND_CHECK
#error we *must* have bind port checking
#endif

/* The TCP PCB lists. */
#ifdef BIND_CHECK
#ifdef __PAULOS__
#define FNV_32_PRIME 0x01000193
static u32_t fnv_init;

#ifdef HAVE_LARGE_HASHTABLES
#define TCP_HASH_TABLE_SIZE		1024
#else
#define TCP_HASH_TABLE_SIZE		128
#endif
static struct tcp_pcb *tcp_bind_pcbs_hash[TCP_HASH_TABLE_SIZE];  /* List of all TCP PCBs in LISTEN state. */
//long random (void);
static inline unsigned int get_hash (unsigned short port)
{
    u32_t fnv = fnv_init;
    fnv ^= port;
    fnv *= FNV_32_PRIME;
    fnv ^= port >> 8;
    fnv *= FNV_32_PRIME;
    return fnv & (TCP_HASH_TABLE_SIZE - 1);
}
void tcp_hash_init (void)
{
    int i;
    for (i = 0; i < TCP_HASH_TABLE_SIZE; i++)
	tcp_bind_pcbs_hash[i] = NULL;
    //fnv_init = random ();
    fnv_init = 0x801;
}
#else
struct tcp_pcb *tcp_bind_pcbs;  /* List of all TCP PCBs in LISTEN state. */
#endif
#endif
struct tcp_pcb_listen *tcp_listen_pcbs;  /* List of all TCP PCBs in LISTEN state. */
struct tcp_pcb *tcp_active_pcbs;  /* List of all TCP PCBs that are in a
				 state in which they accept or send
				 data. */
struct tcp_pcb *tcp_tw_pcbs;      /* List of all TCP PCBs in TIME-WAIT. */

struct tcp_pcb *tcp_tmp_pcb;

#define MIN(x,y) (x) < (y)? (x): (y)

//extern int printf (char *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));

void print_pcb (struct tcp_pcb *pcb)
{
    int n, l;
    struct tcp_seg *p;
    char *state[] =
	{ "BIND", "CLOSED", "LISTEN", "SYN_SENT", "SYN_RCVD", "ESTABLISHED", "FIN_WAIT_1", "FIN_WAIT_2",
	"CLOSE_WAIT", "CLOSING", "LAST_ACK", "TIME_WAIT"
    };
    printf ("(%.30s) ", pcb->url);
    printf ("%s prio=%d %s:%hu-%s:%hu ", state[((unsigned) pcb->state) + 1], (unsigned) pcb->prio,
	    (char *) inet_ntoa (pcb->local_ip), pcb->local_port, (char *) inet_ntoa (pcb->remote_ip),
	    pcb->remote_port);
    printf ("rcv_nxt=%lu rcv_wnd=%hu ", (unsigned long) pcb->rcv_nxt, pcb->rcv_wnd);
    printf ("tmr=%lu rtime=%u die=%u mss=%hu ", (unsigned long) pcb->tmr, (unsigned) pcb->rtime,
	    (int) pcb->die_timeout, pcb->mss);
    printf ("%s%s%s%s%s%s ", (pcb->flags & TF_ACK_DELAY) ? "ACK_DELAY " : "",
	    (pcb->flags & TF_ACK_NOW) ? "ACK_NOW " : "", (pcb->flags & TF_INFR) ? "INFR " : "",
	    (pcb->flags & TF_RESET) ? "RESET " : "", (pcb->flags & TF_CLOSED) ? "CLOSED " : "",
	    (pcb->flags & TF_GOT_FIN) ? "GOT_FIN " : "");
    printf ("rttest=%hu rtseq=%lu sa=%ld sv=%ld nrtx=%u ", pcb->rttest, (unsigned long) pcb->rtseq,
	    (long) pcb->sa, (long) pcb->sv, (unsigned) pcb->nrtx);
    printf ("dupacks=%u rto=%hu lastack=%lu ", (unsigned) pcb->dupacks, pcb->rto,
	    (unsigned long) pcb->lastack);
    printf ("cwnd=%lu ssthresh=%lu ", (unsigned long) pcb->cwnd, (unsigned long) pcb->ssthresh);
    printf ("snd_nxt=%lu snd_max=%lu snd_wnd=%lu snd_wl1=%lu snd_wl2=%lu snd_lbb=%lu ",
	    (unsigned long) pcb->snd_nxt, (unsigned long) pcb->snd_max, (unsigned long) pcb->snd_wnd,
	    (unsigned long) pcb->snd_wl1, (unsigned long) pcb->snd_wl2, (unsigned long) pcb->snd_lbb);
    printf ("snd_queuelen=%u ", (unsigned) pcb->snd_queuelen);
    printf ("snd_buf=%hu ", (unsigned) pcb->snd_buf);
    printf ("acked=%hu ", (unsigned) pcb->acked);
    printf ("polltmr=%u pollinterval=%u ", (unsigned) pcb->polltmr, (unsigned) pcb->pollinterval);
    for (n = l = 0, p = pcb->unsent; p; p = p->next)
	n++, l += p->p->tot_len;
    printf ("unsent=%u,%ub ", n, l);
    for (n = l = 0, p = pcb->unacked; p; p = p->next)
	n++, l += p->p->tot_len;
    printf ("unacked=%u,%ub ", n, l);
    printf ("\n");
}

#define TR(pcb)								\
    do {								\
	if ((pcb)->debug) {						\
	    print_pcb (pcb);						\
	}								\
    } while (0)

static u8_t tcp_timer;

/*-----------------------------------------------------------------------------------*/
/*
 * tcp_init():
 *
 * Initializes the TCP layer.
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_init(void)
{
  /* Clear globals. */
#ifdef BIND_CHECK
#ifdef __PAULOS__
  tcp_hash_init ();
#else
  tcp_bind_pcbs = NULL;
#endif
#endif
  tcp_listen_pcbs = NULL;
  tcp_active_pcbs = NULL;
  tcp_tw_pcbs = NULL;
  tcp_tmp_pcb = NULL;
  
  /* initialize timer */
  //tcp_ticks = random ();
  tcp_ticks = 0x803;
  tcp_timer = 0;

}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_tmr():
 *
 * Called periodically to dispatch TCP timers.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_tmr(void)
{
  ++tcp_timer;
  if(tcp_timer == 10) {
    tcp_timer = 0;
  }
  
  if(tcp_timer & 1) {
    /* Call tcp_fasttmr() every 200 ms, i.e., every other timer
       tcp_tmr() is called. */
    tcp_fasttmr();
  }
  if(tcp_timer == 0 || tcp_timer == 5) {
    /* Call tcp_slowtmr() every 500 ms, i.e., every fifth timer
       tcp_tmr() is called. */
    tcp_slowtmr();
    reass_tmr();
#ifdef HAVE_IPSEC
    ipsec_tmr();
#endif
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_close():
 *
 * Closes the connection held by the PCB.
 *
 */
/*-----------------------------------------------------------------------------------*/
err_t
tcp_close(struct tcp_pcb *pcb)
{
  err_t err;
  magic (pcb, TCPPCB_MAGIC);

  TR (pcb);

#if TCP_DEBUG
  DEBUGF(TCP_DEBUG, ("tcp_close: closing in state "));
  tcp_debug_print_state(pcb->state);
  DEBUGF(TCP_DEBUG, ("\n"));
#endif /* TCP_DEBUG */
  switch(pcb->state) {
#ifdef BIND_CHECK
  case BIND:
    err = ERR_OK;
#ifdef __PAULOS__
    tcp_pcb_remove(&tcp_bind_pcbs_hash[get_hash (pcb->local_port)], pcb);
#else
    tcp_pcb_remove(&tcp_bind_pcbs, pcb);
#endif
#ifdef HAVE_MAGIC
    pcb->magic = 0;
    get_stack_trace (pcb->backtrace, 32);
#endif
    memp_free(MEMP_TCP_PCB, pcb);
    pcb = NULL;
    break;
#endif
  case LISTEN:
    err = ERR_OK;
    tcp_pcb_remove((struct tcp_pcb **)&tcp_listen_pcbs, pcb);
#ifdef HAVE_MAGIC
    pcb->magic = 0;
    get_stack_trace (pcb->backtrace, 32);
#endif
#ifdef __PAULOS__
    memp_free(MEMP_TCP_PCB, pcb);
#else
    memp_free(MEMP_TCP_PCB_LISTEN, pcb);
#endif
    pcb = NULL;
    break;
  case SYN_SENT:
    err = ERR_OK;
    tcp_pcb_remove(&tcp_active_pcbs, pcb);
#ifdef HAVE_MAGIC
    pcb->magic = 0;
    get_stack_trace (pcb->backtrace, 32);
#endif
    memp_free(MEMP_TCP_PCB, pcb);
    pcb = NULL;
    break;
  case SYN_RCVD:
    err = tcp_send_ctrl(pcb, TCP_FIN);
    if(err == ERR_OK) {
      pcb->state = FIN_WAIT_1;
    }
    break;
  case ESTABLISHED:
    err = tcp_send_ctrl(pcb, TCP_FIN);
    if(err == ERR_OK) {
      pcb->state = FIN_WAIT_1;
    }
    break;
  case CLOSE_WAIT:
    err = tcp_send_ctrl(pcb, TCP_FIN);
    if(err == ERR_OK) {
      pcb->state = LAST_ACK;
    }
    break;
  default:
    /* Has already been closed, do nothing. */
    err = ERR_OK;
    pcb = NULL;
    break;
  }

  if(pcb != NULL && err == ERR_OK) {
    err = tcp_output(pcb);
  }
  return err;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_abort()
 *
 * Aborts a connection by sending a RST to the remote host and deletes
 * the local protocol control block. This is done when a connection is
 * killed because of shortage of memory.
 *
 * This function free's the pcb struct. Do NOT use the pcb struct after
 * calling this function.
 *
 */
/*-----------------------------------------------------------------------------------*/
int
tcp_abortable(struct tcp_pcb *pcb)
{
  struct tcp_pcb *cpcb;
  for(cpcb = (struct tcp_pcb *)tcp_active_pcbs;
      cpcb != NULL; cpcb = cpcb->next)
    if (cpcb == pcb)
      return 1;
  for(cpcb = (struct tcp_pcb *)tcp_tw_pcbs;
      cpcb != NULL; cpcb = cpcb->next)
    if (cpcb == pcb)
      return 1;
  return 0;
}

void
tcp_abort(struct tcp_pcb *pcb)
{
  u32_t seqno, ackno;
  u16_t remote_port, local_port;
  struct ip_addr remote_ip, local_ip;
#if LWIP_CALLBACK_API  
  void (* errf)(void *arg, err_t err);
#endif /* LWIP_CALLBACK_API */
  void *errf_arg;

  magic (pcb, TCPPCB_MAGIC);

  TR (pcb);
  
  /* Figure out on which TCP PCB list we are, and remove us. If we
     are in an active state, call the receive function associated with
     the PCB with a NULL argument, and send an RST to the remote end. */
  if(pcb->state == TIME_WAIT) {
    tcp_pcb_remove(&tcp_tw_pcbs, pcb);
#ifdef HAVE_MAGIC
    pcb->magic = 0;
    get_stack_trace (pcb->backtrace, 32);
#endif
    memp_free(MEMP_TCP_PCB, pcb);
    pcb = NULL;
  } else {
    seqno = pcb->snd_nxt;
    ackno = pcb->rcv_nxt;
    ip_addr_set(&local_ip, &(pcb->local_ip));
    ip_addr_set(&remote_ip, &(pcb->remote_ip));
    local_port = pcb->local_port;
    remote_port = pcb->remote_port;
#if LWIP_CALLBACK_API
    errf = pcb->errf;
#endif /* LWIP_CALLBACK_API */
    errf_arg = pcb->callback_arg;
    tcp_pcb_remove(&tcp_active_pcbs, pcb);
    if(pcb->unacked != NULL) {
      tcp_segs_free(pcb->unacked);
      pcb->unacked = NULL;
    }
    if(pcb->unsent != NULL) {
      tcp_segs_free(pcb->unsent);
      pcb->unsent = NULL;
    }
#if TCP_QUEUE_OOSEQ    
    if(pcb->ooseq != NULL) {
      tcp_segs_free(pcb->ooseq);
      pcb->ooseq = NULL;
    }
#endif /* TCP_QUEUE_OOSEQ */
#ifdef HAVE_MAGIC
    pcb->magic = 0;
    get_stack_trace (pcb->backtrace, 32);
#endif
    memp_free(MEMP_TCP_PCB, pcb);
    pcb = 0;

 //print_trace_back();

    TCP_EVENT_ERR(errf, errf_arg, ERR_ABRT);
    DEBUGF(TCP_RST_DEBUG, ("tcp_abort: sending RST\n"));
    tcp_rst(seqno, ackno, &local_ip, &remote_ip, local_port, remote_port);
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_bind():
 *
 * Binds the connection to a local portnumber and IP address. If the
 * IP address is not given (i.e., ipaddr == NULL), the IP address of
 * the outgoing network interface is used instead.
 *
 */

/*-----------------------------------------------------------------------------------*/
err_t
tcp_bind(struct tcp_pcb *pcb, struct ip_addr *ipaddr, u16_t port)
{
  struct tcp_pcb *cpcb;
#ifdef __PAULOS__
  struct tcp_pcb **tcp_bind_pcbs;
#endif

  magic (pcb, TCPPCB_MAGIC);

  /* Check if the address already is in use. */
  for(cpcb = (struct tcp_pcb *)tcp_listen_pcbs;
      cpcb != NULL; cpcb = cpcb->next) {
    magic (cpcb, TCPPCB_MAGIC);
    if(cpcb->local_port == port) {
      if(ip_addr_isany(&(cpcb->local_ip)) ||
	 ip_addr_isany(ipaddr) ||
	 ip_addr_cmp(&(cpcb->local_ip), ipaddr)) {
	return ERR_USE;
      }
    }
  }
#ifdef BIND_CHECK

#ifdef __PAULOS__
  tcp_bind_pcbs = &tcp_bind_pcbs_hash[get_hash (port)];
  for(cpcb = *tcp_bind_pcbs;
      cpcb != NULL; cpcb = cpcb->next) 
#else
  for(cpcb = tcp_bind_pcbs;
      cpcb != NULL; cpcb = cpcb->next) 
#endif
    {
    magic (cpcb, TCPPCB_MAGIC);
    if(cpcb->local_port == port) {
      if(ip_addr_isany(&(cpcb->local_ip)) ||
	 ip_addr_isany(ipaddr) ||
	 ip_addr_cmp(&(cpcb->local_ip), ipaddr)) {
	return ERR_USE;
      }
    }
  }
#endif
  for(cpcb = tcp_active_pcbs;
      cpcb != NULL; cpcb = cpcb->next) {
    if(cpcb->local_port == port) {
      if(ip_addr_isany(&(cpcb->local_ip)) ||
	 ip_addr_isany(ipaddr) ||
	 ip_addr_cmp(&(cpcb->local_ip), ipaddr)) {
	return ERR_USE;
      }
    }
  }
  if(!ip_addr_isany(ipaddr)) {
    pcb->local_ip = *ipaddr;
  }
  pcb->local_port = port;
#ifdef BIND_CHECK
  pcb->state = BIND;
#ifdef __PAULOS__
  TCP_REG(tcp_bind_pcbs, pcb);
#else
  TCP_REG(&tcp_bind_pcbs, pcb);
#endif
#endif
  DEBUGF(TCP_DEBUG, ("tcp_bind: bind to port %d\n", port));
  return ERR_OK;
}
#if LWIP_CALLBACK_API
static err_t
tcp_accept_null(void *arg, struct tcp_pcb *pcb, err_t err)
{

 //print_trace_back();

  return ERR_ABRT;
}
#endif /* LWIP_CALLBACK_API */
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_listen():
 *
 * Set the state of the connection to be LISTEN, which means that it
 * is able to accept incoming connections. The protocol control block
 * is reallocated in order to consume less memory. Setting the
 * connection to LISTEN is an irreversible process.
 *
 */
/*-----------------------------------------------------------------------------------*/
struct tcp_pcb *
tcp_listen(struct tcp_pcb *pcb)
{
  struct tcp_pcb_listen *lpcb;

#ifdef BIND_CHECK
  ASSERT("tcp_listen: pcb->state == BIND", pcb->state == BIND);
#endif

  lpcb = memp_malloc(MEMP_TCP_PCB);
  if(lpcb == NULL) {
    return NULL;
  }
  memset (lpcb, 0, sizeof (struct tcp_pcb_listen));
#ifdef HAVE_MAGIC
  lpcb->magic = TCPPCB_MAGIC;
#endif
  lpcb->callback_arg = pcb->callback_arg;
  lpcb->local_port = pcb->local_port;
#ifdef __PAULOS__
  lpcb->internal_only = pcb->internal_only;
  lpcb->secure_peer = pcb->secure_peer;
  lpcb->mss = pcb->mss;
#endif
  ip_addr_set(&lpcb->local_ip, &pcb->local_ip);
#ifdef __PAULOS__
  TCP_RMV(&tcp_bind_pcbs_hash[get_hash (pcb->local_port)], pcb);
#else
  TCP_RMV((&tcp_bind_pcbs), pcb);
#endif
#ifdef HAVE_MAGIC
  pcb->magic = 0;
  get_stack_trace (pcb->backtrace, 32);
#endif
  memp_free(MEMP_TCP_PCB, pcb);
#if LWIP_CALLBACK_API
  lpcb->accept = tcp_accept_null;
#endif /* LWIP_CALLBACK_API */
  lpcb->state = LISTEN;

  
  // COMPILE ERROR XXX nr
  //TCP_REG((struct tcp_pcb **)&tcp_listen_pcbs, (struct tcp_pcb *)lpcb);
  return (struct tcp_pcb *)lpcb;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_recved():
 *
 * This function should be called by the application when it has
 * processed the data. The purpose is to advertise a larger window
 * when the data has been processed.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_recved(struct tcp_pcb *pcb, u16_t len)
{
  magic (pcb, TCPPCB_MAGIC);

  pcb->rcv_wnd += len;
  if(pcb->rcv_wnd > TCP_WND_MAX) {
    pcb->rcv_wnd = TCP_WND_MAX;
  }
  if(!(pcb->flags & TF_ACK_DELAY) &&
     !(pcb->flags & TF_ACK_NOW)) {
    tcp_ack(pcb);
  }
  DEBUGF(TCP_DEBUG, ("tcp_recved: recveived %d bytes, wnd %u (%u).\n",
		     len, pcb->rcv_wnd, TCP_WND - pcb->rcv_wnd));
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_new_port():
 *
 * A nastly hack featuring 'goto' statements that allocates a
 * new TCP local port.
 */
/*-----------------------------------------------------------------------------------*/
static u16_t
tcp_new_port(void)
{
#ifdef __PAULOS__
  static struct sequencer s = {0};
  struct tcp_pcb *pcb;
  u16_t port;
  struct tcp_pcb *tcp_bind_pcbs;

  if (!s.init)
    sequencer_init (&s, 16 /* 0 - 65535 */);
#else
  struct tcp_pcb *pcb;
  static u16_t port = 4096;
#endif

 again:
#ifdef __PAULOS__
  port = (u16_t) sequencer_next (&s);
#else
  if(++port > 0x7fff) {
    port = 4096;
  }
#endif
  
/* FIXME: write README about the port problem - same port on each startup, etc. */  
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->local_port == port) {
      goto again;
    }
  }
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->local_port == port) {
      goto again;
    }
  }
  for(pcb = (struct tcp_pcb *)tcp_listen_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->local_port == port) {
      goto again;
    }
  }
#ifdef BIND_CHECK
#ifdef __PAULOS__
  tcp_bind_pcbs = tcp_bind_pcbs_hash[get_hash (port)];
#endif
  for(pcb = (struct tcp_pcb *)tcp_bind_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->local_port == port) {
      goto again;
    }
  }
#endif
  return port;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_connect():
 *
 * Connects to another host. The function given as the "connected"
 * argument will be called when the connection has been established.
 *
 */
/*-----------------------------------------------------------------------------------*/
err_t
tcp_connect(struct tcp_pcb *pcb, struct ip_addr *ipaddr, u16_t port,
	    err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err))
{
  u32_t optdata;
  err_t ret;
  u32_t iss;

  magic (pcb, TCPPCB_MAGIC);

  DEBUGF(TCP_DEBUG, ("tcp_connect to port %d\n", port));
  if(ipaddr != NULL) {
    pcb->remote_ip = *ipaddr;
  } else {
    return ERR_VAL;
  }

  pcb->remote_port = port;

 if(pcb->local_port == 0) {
    pcb->local_port = tcp_new_port();
 }
 
#ifdef BIND_CHECK
  if (pcb->state == BIND) {
#ifdef __PAULOS__
    TCP_RMV((&tcp_bind_pcbs_hash[get_hash (pcb->local_port)]), pcb);
#else
    TCP_RMV((&tcp_bind_pcbs), pcb);
#endif
  }
#endif

  	  //->
  iss = tcp_next_iss();
  pcb->rcv_nxt = 0;
  pcb->snd_nxt = iss;
  pcb->lastack = iss - 1;
  pcb->snd_lbb = iss - 1;
  pcb->rcv_wnd = TCP_WND;
  pcb->snd_wnd = TCP_WND;
#ifdef __PAULOS__
  assert(pcb->mss == TCP_MSS_SMALL || pcb->mss == TCP_MSS_MEDIUM || pcb->mss == TCP_MSS_LARGE);
#else
  pcb->mss = TCP_MSS;
#endif
  pcb->cwnd = 1;
  pcb->ssthresh = pcb->mss * 10;
  pcb->state = SYN_SENT;
#if LWIP_CALLBACK_API  
  pcb->connected = connected;
#endif /* LWIP_CALLBACK_API */  
  TCP_REG(&tcp_active_pcbs, pcb);
  /* Build an MSS option */
  optdata = HTONL(((u32_t)2 << 24) | 
		  ((u32_t)4 << 16) | 
		  (((u32_t)pcb->mss / 256) << 8) |
		  (pcb->mss & 255));

  ret = tcp_enqueue(pcb, NULL, 0, TCP_SYN, 0, (u8_t *)&optdata, 4);
  if(ret == ERR_OK) { 
    tcp_output(pcb);
  }
  return ret;
} 
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_slowtmr():
 *
 * Called every 500 ms and implements the retransmission timer and the timer that
 * removes PCBs that have been in TIME-WAIT for enough time. It also increments
 * various timers such as the inactivity timer in each PCB.
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_slowtmr(void)
{
  struct tcp_pcb *pcb, *pcb2, *prev;
  u32_t eff_wnd;
  u8_t pcb_remove;      /* flag if a PCB should be removed */
  err_t err = ERR_OK;

  ++tcp_ticks;
  
  /* Steps through all of the active PCBs. */
  prev = NULL;
  pcb = tcp_active_pcbs;
  while(pcb != NULL) {
    ASSERT("tcp_timer_coarse: active pcb->state != CLOSED", pcb->state != CLOSED);
    ASSERT("tcp_timer_coarse: active pcb->state != LISTEN", pcb->state != LISTEN);
#ifdef BIND_CHECK
    ASSERT("tcp_timer_coarse: active pcb->state != BIND", pcb->state != BIND);
#endif
    ASSERT("tcp_timer_coarse: active pcb->state != TIME-WAIT", pcb->state != TIME_WAIT);

    pcb_remove = 0;

    if(pcb->state == SYN_SENT && pcb->nrtx == TCP_SYNMAXRTX) {
      ++pcb_remove;
    } else if(pcb->state != SYN_SENT && pcb->nrtx == TCP_MAXRTX) { /* psheer: Added pcb->state != SYN_SENT because
							   I like connect to try much harder than anything else */
      ++pcb_remove;
    } else {
      ++pcb->rtime;
      if(pcb->unacked != NULL && pcb->rtime >= pcb->rto) {
        
	/* Time for a retransmission. */
        DEBUGF(TCP_RTO_DEBUG, ("tcp_timer_coarse: rtime %ld pcb->rto %d\n",
                               tcp_ticks - pcb->rtime, pcb->rto));

	/* Double retransmission time-out unless we are trying to
         * connect to somebody (i.e., we are in SYN_SENT). */
	if(pcb->state != SYN_SENT) {
	  pcb->rto = ((pcb->sa >> 3) + pcb->sv) << tcp_backoff[pcb->nrtx];
	}
	tcp_rexmit(pcb);
        /* Reduce congestion window and ssthresh. */
        eff_wnd = MIN(pcb->cwnd, pcb->snd_wnd);
        pcb->ssthresh = eff_wnd >> 1;
        if(pcb->ssthresh < pcb->mss) {
          pcb->ssthresh = pcb->mss * 2;
        }
        pcb->cwnd = pcb->mss;

        DEBUGF(TCP_CWND_DEBUG, ("tcp_rexmit_seg: cwnd %u ssthresh %u\n",
                                (unsigned int) pcb->cwnd, (unsigned int) pcb->ssthresh));
      }
    }
	  
    /* Check if this PCB has stayed too long in FIN-WAIT-2 */
    if(pcb->state >= FIN_WAIT_1) {  /* psheer: was "if(pcb->state == FIN_WAIT_2)" --> I don't like my TCP
                                       sessions to hang around for ever. I don't care what the RFC says. */
      if((u32_t)(tcp_ticks - pcb->tmr) >
	 TCP_FIN_WAIT_TIMEOUT / TCP_SLOW_INTERVAL) {
 //print_trace_back();
        ++pcb_remove;
      }
    }

#ifdef __PAULOS__
    if(pcb->die_timeout) {
      if((u32_t)(tcp_ticks - pcb->tmr) >
	 (u32_t)pcb->die_timeout * 1000 / TCP_SLOW_INTERVAL) {
 //print_trace_back();
        ++pcb_remove;
      }
    }
#endif

    /* If this PCB has queued out of sequence data, but has been
       inactive for too long, will drop the data (it will eventually
       be retransmitted). */
#if TCP_QUEUE_OOSEQ    
    if(pcb->ooseq != NULL &&
       (u32_t)tcp_ticks - pcb->tmr >=
       pcb->rto * TCP_OOSEQ_TIMEOUT) {
      tcp_segs_free(pcb->ooseq);
      pcb->ooseq = NULL;
    }
#endif /* TCP_QUEUE_OOSEQ */

    /* Check if this PCB has stayed too long in SYN-RCVD */
    if(pcb->state == SYN_RCVD) {
      if((u32_t)(tcp_ticks - pcb->tmr) >
	 TCP_SYN_RCVD_TIMEOUT / TCP_SLOW_INTERVAL) {
        ++pcb_remove;
      }
    }


    /* If the PCB should be removed, do it. */
    if(pcb_remove) {
      tcp_pcb_purge(pcb);      
      /* Remove PCB from tcp_active_pcbs list. */
      if(prev != NULL) {
	ASSERT("tcp_timer_coarse: middle tcp != tcp_active_pcbs", pcb != tcp_active_pcbs);
        prev->next = pcb->next;
      } else {
        /* This PCB was the first. */
        ASSERT("tcp_timer_coarse: first pcb == tcp_active_pcbs", tcp_active_pcbs == pcb);
        tcp_active_pcbs = pcb->next;
      }

      TCP_EVENT_ERR(pcb->errf, pcb->callback_arg, ERR_ABRT);
      /*      if(pcb->errf != NULL) {
	pcb->errf(pcb->callback_arg, ERR_ABRT);
	}*/

      pcb2 = pcb->next;
#ifdef HAVE_MAGIC
      pcb->magic = 0;
      get_stack_trace (pcb->backtrace, 32);
#endif
      memp_free(MEMP_TCP_PCB, pcb);
      pcb = pcb2;
    } else {

      /* We check if we should poll the connection. */
      ++pcb->polltmr;
      if(pcb->polltmr >= pcb->pollinterval) {
	pcb->polltmr = 0;
	TCP_EVENT_POLL(pcb, err);
	/*	pcb->poll(pcb->callback_arg, pcb);*/
	if(err == ERR_OK) {
	tcp_output(pcb);
      }
      }
      
      prev = pcb;
      pcb = pcb->next;
    }
  }

  
  /* Steps through all of the TIME-WAIT PCBs. */
  prev = NULL;    
  pcb = tcp_tw_pcbs;
  while(pcb != NULL) {
    ASSERT("tcp_timer_coarse: TIME-WAIT pcb->state == TIME-WAIT", pcb->state == TIME_WAIT);
    pcb_remove = 0;

    /* Check if this PCB has stayed long enough in TIME-WAIT */
#ifdef __PAULOS__
/* we limit this to 10 seconds (NOT 2 full minutes as in
   default lwIP) to reduce number of current connections */
    if((u32_t)(tcp_ticks - pcb->tmr) > 10000 / TCP_SLOW_INTERVAL) {
      ++pcb_remove;
    }
#else
    if((u32_t)(tcp_ticks - pcb->tmr) > 2 * TCP_MSL / TCP_SLOW_INTERVAL) {
      ++pcb_remove;
    }
#endif
    


    /* If the PCB should be removed, do it. */
    if(pcb_remove) {
      tcp_pcb_purge(pcb);      
      /* Remove PCB from tcp_tw_pcbs list. */
      if(prev != NULL) {
	ASSERT("tcp_timer_coarse: middle tcp != tcp_tw_pcbs", pcb != tcp_tw_pcbs);
        prev->next = pcb->next;
      } else {
        /* This PCB was the first. */
        ASSERT("tcp_timer_coarse: first pcb == tcp_tw_pcbs", tcp_tw_pcbs == pcb);
        tcp_tw_pcbs = pcb->next;
      }
      pcb2 = pcb->next;
#ifdef HAVE_MAGIC
      pcb->magic = 0;
      get_stack_trace (pcb->backtrace, 32);
#endif
      memp_free(MEMP_TCP_PCB, pcb);
      pcb = pcb2;
    } else {
      prev = pcb;
      pcb = pcb->next;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_fasttmr():
 *
 * Is called every TCP_FINE_TIMEOUT (100 ms) and sends delayed ACKs.
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_fasttmr(void)
{
  struct tcp_pcb *pcb;

  /* send delayed ACKs */  
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->flags & TF_ACK_DELAY) {
      DEBUGF(TCP_DEBUG, ("tcp_timer_fine: delayed ACK\n"));
      tcp_ack_now(pcb);
      pcb->flags &= ~(TF_ACK_DELAY | TF_ACK_NOW);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_segs_free():
 *
 * Deallocates a list of TCP segments (tcp_seg structures).
 *
 */
/*-----------------------------------------------------------------------------------*/
u8_t
tcp_segs_free(struct tcp_seg *seg)
{
  u8_t count = 0;
  struct tcp_seg *next;
 again:  
  if(seg != NULL) {
    next = seg->next;
    count += tcp_seg_free(seg);
    seg = next;
    goto again;
  }
  return count;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_seg_free():
 *
 * Frees a TCP segment.
 *
 */
/*-----------------------------------------------------------------------------------*/
u8_t
tcp_seg_free(struct tcp_seg *seg)
{
  u8_t count = 0;
  
  if(seg != NULL) {
    if(seg->p == NULL) {
      memp_free(MEMP_TCP_SEG, seg);
    } else {
      count = pbuf_free(seg->p);
#if TCP_DEBUG
      seg->p = NULL;
#endif /* TCP_DEBUG */
      memp_free(MEMP_TCP_SEG, seg);
    }
  }
  return count;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_setprio():
 *
 * Sets the priority of a connection.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_setprio(struct tcp_pcb *pcb, u8_t prio)
{
  pcb->prio = prio;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_seg_copy():
 *
 * Returns a copy of the given TCP segment.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
struct tcp_seg *
tcp_seg_copy(struct tcp_seg *seg)
{
  struct tcp_seg *cseg;

  cseg = memp_malloc(MEMP_TCP_SEG);
  if(cseg == NULL) {
    return NULL;
  }
  bcopy(seg, cseg, sizeof(struct tcp_seg));
  pbuf_ref(cseg->p);
  return cseg;
}
/*-----------------------------------------------------------------------------------*/
#if LWIP_CALLBACK_API
static err_t
tcp_recv_null(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  arg = arg;
  if(p != NULL) {
    pbuf_free(p);
  } else if(err == ERR_OK) {
    return tcp_close(pcb);
  }
  return ERR_OK;
}
#endif /* LWIP_CALLBACK_API */
#ifndef __PAULOS__
/*-----------------------------------------------------------------------------------*/
static void
tcp_kill_prio(u8_t prio)
{
  struct tcp_pcb *pcb, *inactive;
  u32_t inactivity;
  u8_t mprio;


  mprio = TCP_PRIO_MAX;
  
  /* We kill the oldest active connection that has lower priority than
     prio. */
  inactivity = 0;
  inactive = NULL;
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
    if(pcb->prio <= prio &&
       pcb->prio <= mprio &&
       (u32_t)(tcp_ticks - pcb->tmr) >= inactivity) {
      inactivity = tcp_ticks - pcb->tmr;
      inactive = pcb;
      mprio = pcb->prio;
    }
  }
  if(inactive != NULL) {
    DEBUGF(TCP_DEBUG, ("tcp_mem_reclaim: killing oldest PCB 0x%p (%ld)\n",
		       inactive, inactivity));
    tcp_abort(inactive);
  }      
}
#endif

#ifndef __PAULOS__
/*-----------------------------------------------------------------------------------*/
static void
tcp_kill_timewait(void)
{
  struct tcp_pcb *pcb, *inactive;
  u32_t inactivity;

  inactivity = 0;
  inactive = NULL;
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
    if((u32_t)(tcp_ticks - pcb->tmr) >= inactivity) {
      inactivity = tcp_ticks - pcb->tmr;
      inactive = pcb;
    }
  }
  if(inactive != NULL) {
    DEBUGF(TCP_DEBUG, ("tcp_mem_reclaim: killing oldest TIME-WAIT PCB 0x%p (%ld)\n",
		       inactive, inactivity));
    tcp_abort(inactive);
  }      
}
#endif

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
struct tcp_pcb *
tcp_alloc(u8_t prio)
{
  struct tcp_pcb *pcb;
  u32_t iss;
  
  pcb = memp_malloc(MEMP_TCP_PCB);
#ifndef __PAULOS__
  if(pcb == NULL) {
    /* Try killing oldest connection in TIME-WAIT. */
    DEBUGF(TCP_DEBUG, ("tcp_new: killing off oldest TIME-WAIT connection\n"));
    tcp_kill_timewait();
    pcb = memp_malloc(MEMP_TCP_PCB);
    if(pcb == NULL) {
      tcp_kill_prio(prio);    
      pcb = memp_malloc(MEMP_TCP_PCB);
    }
  }
#endif
  if(pcb != NULL) {
//    bzero(pcb, sizeof(struct tcp_pcb));
memset(pcb, 0, sizeof(struct tcp_pcb));
#ifdef HAVE_MAGIC
    pcb->magic = TCPPCB_MAGIC;
#endif
    pcb->prio = TCP_PRIO_NORMAL;
    pcb->snd_buf = TCP_SND_BUF;
    pcb->snd_queuelen = 0;
    pcb->rcv_wnd = TCP_WND;
    pcb->mss = TCP_MSS;
    pcb->rto = 3000 / TCP_SLOW_INTERVAL;
    pcb->sa = 0;
    pcb->sv = 3000 / TCP_SLOW_INTERVAL;
    pcb->rtime = 0;
    pcb->cwnd = 1;
    iss = tcp_next_iss();
    pcb->snd_wl2 = iss;
    pcb->snd_nxt = iss;
    pcb->snd_max = iss;
    pcb->lastack = iss;
    pcb->snd_lbb = iss;   
    pcb->tmr = tcp_ticks;

    pcb->polltmr = 0;

#if LWIP_CALLBACK_API
    pcb->recv = tcp_recv_null;
#endif /* LWIP_CALLBACK_API */  
  }
  return pcb;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_new():
 *
 * Creates a new TCP protocol control block but doesn't place it on
 * any of the TCP PCB lists.
 *
 */
/*-----------------------------------------------------------------------------------*/
struct tcp_pcb *
tcp_new(void)
{
  return tcp_alloc(TCP_PRIO_NORMAL);
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_arg():
 *
 * Used to specify the argument that should be passed callback
 * functions.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
void
tcp_arg(struct tcp_pcb *pcb, void *arg)
{  
  magic (pcb, TCPPCB_MAGIC);

  pcb->callback_arg = arg;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_recv():
 *
 * Used to specify the function that should be called when a TCP
 * connection receives data.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
#if LWIP_CALLBACK_API
void
tcp_recv(struct tcp_pcb *pcb,
	 err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err))
{
  magic (pcb, TCPPCB_MAGIC);

  pcb->recv = recv;
}
#endif /* LWIP_CALLBACK_API */
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_sent():
 *
 * Used to specify the function that should be called when TCP data
 * has been successfully delivered to the remote host.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
#if LWIP_CALLBACK_API
void
tcp_sent(struct tcp_pcb *pcb,
	 err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len))
{
  magic (pcb, TCPPCB_MAGIC);

  pcb->sent = sent;
}
#endif /* LWIP_CALLBACK_API */
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_err():
 *
 * Used to specify the function that should be called when a fatal error
 * has occured on the connection.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
#if LWIP_CALLBACK_API
void
tcp_err(struct tcp_pcb *pcb,
	 void (* errf)(void *arg, err_t err))
{
  magic (pcb, TCPPCB_MAGIC);

  pcb->errf = errf;
}
#endif /* LWIP_CALLBACK_API */
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_poll():
 *
 * Used to specify the function that should be called periodically
 * from TCP. The interval is specified in terms of the TCP coarse
 * timer interval, which is called twice a second.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
void
tcp_poll(struct tcp_pcb *pcb,
	 err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval)
{
  magic (pcb, TCPPCB_MAGIC);

#if LWIP_CALLBACK_API
  pcb->poll = poll;
#endif /* LWIP_CALLBACK_API */  
  pcb->pollinterval = interval;
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_accept():
 *
 * Used for specifying the function that should be called when a
 * LISTENing connection has been connected to another host.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
#if LWIP_CALLBACK_API
void
tcp_accept(struct tcp_pcb *pcb,
	   err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err))
{
  magic (pcb, TCPPCB_MAGIC);

  ((struct tcp_pcb_listen *)pcb)->accept = accept;
}
#endif /* LWIP_CALLBACK_API */
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_pcb_purge():
 *
 * Purges a TCP PCB. Removes any buffered data and frees the buffer memory.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_pcb_purge(struct tcp_pcb *pcb)
{
  magic (pcb, TCPPCB_MAGIC);

  if(pcb->state != CLOSED &&
     pcb->state != TIME_WAIT &&
#ifdef BIND_CHECK
     pcb->state != BIND &&
#endif
     pcb->state != LISTEN) {

    DEBUGF(TCP_DEBUG, ("tcp_pcb_purge\n"));
    
#if TCP_DEBUG
    if(pcb->unsent != NULL) {    
      DEBUGF(TCP_DEBUG, ("tcp_pcb_purge: not all data sent\n"));
    }
    if(pcb->unacked != NULL) {    
      DEBUGF(TCP_DEBUG, ("tcp_pcb_purge: data left on ->unacked\n"));
    }
    if(pcb->ooseq != NULL) {    
      DEBUGF(TCP_DEBUG, ("tcp_pcb_purge: data left on ->ooseq\n"));
    }
#endif /* TCP_DEBUG */
    tcp_segs_free(pcb->unsent);
#if TCP_QUEUE_OOSEQ
    tcp_segs_free(pcb->ooseq);
#endif /* TCP_QUEUE_OOSEQ */
    tcp_segs_free(pcb->unacked);
    pcb->unacked = pcb->unsent =
#if TCP_QUEUE_OOSEQ
      pcb->ooseq =
#endif /* TCP_QUEUE_OOSEQ */
      NULL;
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_pcb_remove():
 *
 * Purges the PCB and removes it from a PCB list. Any delayed ACKs are sent first.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tcp_pcb_remove(struct tcp_pcb **pcblist, struct tcp_pcb *pcb)
{
  TCP_RMV(pcblist, pcb);

  magic (pcb, TCPPCB_MAGIC);

  tcp_pcb_purge(pcb);
  
  /* if there is an outstanding delayed ACKs, send it */
  if(pcb->state != TIME_WAIT &&
#ifdef BIND_CHECK
     pcb->state != BIND &&
#endif
     pcb->state != LISTEN &&
     pcb->flags & TF_ACK_DELAY) {
    pcb->flags |= TF_ACK_NOW;
    tcp_output(pcb);
  }  
  pcb->state = CLOSED;

  ASSERT("tcp_pcb_remove: tcp_pcbs_sane()", tcp_pcbs_sane());
}
/*-----------------------------------------------------------------------------------*/
/*
 * tcp_next_iss():
 *
 * Calculates a new initial sequence number for new connections.
 *
 */
/*-----------------------------------------------------------------------------------*/

u32_t
tcp_next_iss(void)
{
  static u32_t iss = 6510;
  
  iss += tcp_ticks;       /* XXX */
  return iss;
}
/*-----------------------------------------------------------------------------------*/
#if TCP_DEBUG || TCP_INPUT_DEBUG || TCP_OUTPUT_DEBUG
void
tcp_debug_print(struct tcp_hdr *tcphdr)
{
  DEBUGF(TCP_DEBUG, ("TCP header:\n"));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(TCP_DEBUG, ("|      %04x     |      %04x     | (src port, dest port)\n",
		     tcphdr->src, tcphdr->dest));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(TCP_DEBUG, ("|            %08lu           | (seq no)\n",
			    tcphdr->seqno));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(TCP_DEBUG, ("|            %08lu           | (ack no)\n",
		     tcphdr->ackno));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(TCP_DEBUG, ("| %2d |    |%d%d%d%d%d|    %5d      | (offset, flags (",
		     TCPH_FLAGS(tcphdr) >> 4 & 1,
		     TCPH_FLAGS(tcphdr) >> 4 & 1,
		     TCPH_FLAGS(tcphdr) >> 3 & 1,
		     TCPH_FLAGS(tcphdr) >> 2 & 1,
		     TCPH_FLAGS(tcphdr) >> 1 & 1,
		     TCPH_FLAGS(tcphdr) & 1,
		     tcphdr->wnd));
  tcp_debug_print_flags(TCPH_FLAGS(tcphdr));
  DEBUGF(TCP_DEBUG, ("), win)\n"));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
  DEBUGF(TCP_DEBUG, ("|    0x%04x     |     %5d     | (chksum, urgp)\n",
		     ntohs(tcphdr->chksum), ntohs(tcphdr->urgp)));
  DEBUGF(TCP_DEBUG, ("+-------------------------------+\n"));
}
/*-----------------------------------------------------------------------------------*/
void
tcp_debug_print_state(enum tcp_state s)
{
  DEBUGF(TCP_DEBUG, ("State: "));
  switch(s) {
  case CLOSED:
    DEBUGF(TCP_DEBUG, ("CLOSED\n"));
    break;
#ifdef BIND_CHECK
 case BIND:
   DEBUGF(TCP_DEBUG, ("BIND\n"));
   break;
#endif
 case LISTEN:
   DEBUGF(TCP_DEBUG, ("LISTEN\n"));
   break;
  case SYN_SENT:
    DEBUGF(TCP_DEBUG, ("SYN_SENT\n"));
    break;
  case SYN_RCVD:
    DEBUGF(TCP_DEBUG, ("SYN_RCVD\n"));
    break;
  case ESTABLISHED:
    DEBUGF(TCP_DEBUG, ("ESTABLISHED\n"));
    break;
  case FIN_WAIT_1:
    DEBUGF(TCP_DEBUG, ("FIN_WAIT_1\n"));
    break;
  case FIN_WAIT_2:
    DEBUGF(TCP_DEBUG, ("FIN_WAIT_2\n"));
    break;
  case CLOSE_WAIT:
    DEBUGF(TCP_DEBUG, ("CLOSE_WAIT\n"));
    break;
  case CLOSING:
    DEBUGF(TCP_DEBUG, ("CLOSING\n"));
    break;
  case LAST_ACK:
    DEBUGF(TCP_DEBUG, ("LAST_ACK\n"));
    break;
  case TIME_WAIT:
    DEBUGF(TCP_DEBUG, ("TIME_WAIT\n"));
   break;
  }
}
/*-----------------------------------------------------------------------------------*/
void
tcp_debug_print_flags(u8_t flags)
{
  if(flags & TCP_FIN) {
    DEBUGF(TCP_DEBUG, ("FIN "));
  }
  if(flags & TCP_SYN) {
    DEBUGF(TCP_DEBUG, ("SYN "));
  }
  if(flags & TCP_RST) {
    DEBUGF(TCP_DEBUG, ("RST "));
  }
  if(flags & TCP_PSH) {
    DEBUGF(TCP_DEBUG, ("PSH "));
  }
  if(flags & TCP_ACK) {
    DEBUGF(TCP_DEBUG, ("ACK "));
  }
  if(flags & TCP_URG) {
    DEBUGF(TCP_DEBUG, ("URG "));
  }
}
/*-----------------------------------------------------------------------------------*/
void
tcp_debug_print_pcbs(void)
{
  struct tcp_pcb *pcb;
  DEBUGF(TCP_DEBUG, ("Active PCB states:\n"));
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
    DEBUGF(TCP_DEBUG, ("Local port %d, foreign port %d snd_nxt %lu rcv_nxt %lu ",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt));
    tcp_debug_print_state(pcb->state);
  }    
#ifdef BIND_CHECK
  DEBUGF(TCP_DEBUG, ("Bind PCB states:\n"));
#ifdef __PAULOS__
  {
    for (i = 0; i < TCP_HASH_TABLE_SIZE; i++) {
      struct tcp_pcb *tcp_bind_pcbs;
      tcp_bind_pcbs = tcp_bind_pcbs_hash[i];
#endif

  for(pcb = (struct tcp_pcb *)tcp_bind_pcbs; pcb != NULL; pcb = pcb->next) {
    DEBUGF(TCP_DEBUG, ("Local port %d, foreign port %d snd_nxt %lu rcv_nxt %lu ",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt));
    tcp_debug_print_state(pcb->state);
  }    
#endif
#ifdef __PAULOS__
    }
  }
#endif

  DEBUGF(TCP_DEBUG, ("Listen PCB states:\n"));
  for(pcb = (struct tcp_pcb *)tcp_listen_pcbs; pcb != NULL; pcb = pcb->next) {
    DEBUGF(TCP_DEBUG, ("Local port %d, foreign port %d snd_nxt %lu rcv_nxt %lu ",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt));
    tcp_debug_print_state(pcb->state);
  }    
  DEBUGF(TCP_DEBUG, ("TIME-WAIT PCB states:\n"));
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
    DEBUGF(TCP_DEBUG, ("Local port %d, foreign port %d snd_nxt %lu rcv_nxt %lu ",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt));
    tcp_debug_print_state(pcb->state);
  }    
}
/*-----------------------------------------------------------------------------------*/
int
tcp_pcbs_sane(void)
{
  struct tcp_pcb *pcb;
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
    ASSERT("tcp_pcbs_sane: active pcb->state != CLOSED", pcb->state != CLOSED);
    ASSERT("tcp_pcbs_sane: active pcb->state != LISTEN", pcb->state != LISTEN);
#ifdef BIND_CHECK
    ASSERT("tcp_pcbs_sane: active pcb->state != BIND", pcb->state != BIND);
#endif
    ASSERT("tcp_pcbs_sane: active pcb->state != TIME-WAIT", pcb->state != TIME_WAIT);
  }
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
    ASSERT("tcp_pcbs_sane: tw pcb->state == TIME-WAIT", pcb->state == TIME_WAIT);
  }
  return 1;
}
#endif /* TCP_DEBUG */
/*-----------------------------------------------------------------------------------*/









