/* memp.c - PaulOS embedded operating system
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
 * $Id: memp.c,v 1.3 2003/08/01 11:58:01 psheer Exp $
 */

#ifdef __PAULOS__
#include "lwipopts-paulos.h"
#else
#include "lwipopts.h"
#endif

#include "lwip/memp.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/api_msg.h"
#include "lwip/tcpip.h"

#include "lwip/sys.h"
#include "lwip/stats.h"

struct memp {
  struct memp *next;
};



#ifndef __PAULOS__
static struct memp *memp_tab[MEMP_MAX];
#endif

static const u16_t memp_sizes[MEMP_MAX] = {
  sizeof(struct pbuf),
  sizeof(struct udp_pcb),
  sizeof(struct tcp_pcb),
#ifndef __PAULOS__
  sizeof(struct tcp_pcb_listen),
#endif
  sizeof(struct tcp_seg),
  sizeof(struct netbuf),
  sizeof(struct netconn),
#ifndef __PAULOS__
  sizeof(struct api_msg),
  sizeof(struct tcpip_msg),
  sizeof(struct sys_timeout)
#endif
};

#ifndef __PAULOS__

static const u16_t memp_num[MEMP_MAX] = {
  MEMP_NUM_PBUF,
  MEMP_NUM_UDP_PCB,
  MEMP_NUM_TCP_PCB,
  MEMP_NUM_TCP_PCB_LISTEN,
  MEMP_NUM_TCP_SEG,
  MEMP_NUM_NETBUF,
  MEMP_NUM_NETCONN,
  MEMP_NUM_API_MSG,
  MEMP_NUM_TCPIP_MSG,
  MEMP_NUM_SYS_TIMEOUT
};

#endif

int mem_type_size (int type)
{
    return memp_sizes[type];
}

#ifndef __PAULOS__

static u8_t memp_memory[(MEMP_NUM_PBUF *
			 MEM_ALIGN_SIZE(sizeof(struct pbuf) +
					sizeof(struct memp)) +
			MEMP_NUM_UDP_PCB *
			 MEM_ALIGN_SIZE(sizeof(struct udp_pcb) +
					sizeof(struct memp)) +
			MEMP_NUM_TCP_PCB *
			 MEM_ALIGN_SIZE(sizeof(struct tcp_pcb) +
					sizeof(struct memp)) +
			MEMP_NUM_TCP_PCB_LISTEN *
			 MEM_ALIGN_SIZE(sizeof(struct tcp_pcb_listen) +
					sizeof(struct memp)) +
			MEMP_NUM_TCP_SEG *
			 MEM_ALIGN_SIZE(sizeof(struct tcp_seg) +
					sizeof(struct memp)) +
			MEMP_NUM_NETBUF *
			 MEM_ALIGN_SIZE(sizeof(struct netbuf) +
					sizeof(struct memp)) +
			MEMP_NUM_NETCONN *
			 MEM_ALIGN_SIZE(sizeof(struct netconn) +
					sizeof(struct memp)) +
			MEMP_NUM_API_MSG *
			 MEM_ALIGN_SIZE(sizeof(struct api_msg) +
					sizeof(struct memp)) +
			MEMP_NUM_TCPIP_MSG *
			 MEM_ALIGN_SIZE(sizeof(struct tcpip_msg) +
					sizeof(struct memp)) +
			MEMP_NUM_SYS_TIMEOUT *
			 MEM_ALIGN_SIZE(sizeof(struct sys_timeout) +
					sizeof(struct memp)))];

/*-----------------------------------------------------------------------------------*/
static sys_sem_t mutex;
/*-----------------------------------------------------------------------------------*/
#ifdef LWIP_DEBUG
static int
memp_sanity(void)
{
  int i, c;
  struct memp *m, *n;
  
  for(i = 0; i < MEMP_MAX; i++) {
    for(m = memp_tab[i]; m != NULL; m = m->next) {
      c = 1;
      for(n = memp_tab[i]; n != NULL; n = n->next) {
	if(n == m) {
	  --c;
	}
	if(c < 0)
	  abort();
      }
    }
  }
  return 1;
}
#endif /* LWIP_DEBUG */
/*-----------------------------------------------------------------------------------*/
void
memp_init(void)
{
  struct memp *m, *memp;
  u16_t i, j;
  u16_t size;
      
#ifdef MEMP_STATS
  for(i = 0; i < MEMP_MAX; ++i) {
    stats.memp[i].used = stats.memp[i].max =
      stats.memp[i].err = stats.memp[i].reclaimed = 0;
    stats.memp[i].avail = memp_num[i];
  }
#endif /* MEMP_STATS */

  memp = (struct memp *)&memp_memory[0];
  for(i = 0; i < MEMP_MAX; ++i) {
    size = MEM_ALIGN_SIZE(memp_sizes[i] + sizeof(struct memp));
    if(memp_num[i] > 0) {
      memp_tab[i] = memp;
      m = memp;
      
      for(j = 0; j < memp_num[i]; ++j) {
	m->next = (struct memp *)MEM_ALIGN((u8_t *)m + size);
	memp = m;
	m = m->next;
      }
      memp->next = NULL;
      memp = m;
    } else {
      memp_tab[i] = NULL;
    }
  }

  mutex = sys_sem_new(1);

  
}
/*-----------------------------------------------------------------------------------*/
void *
memp_malloc(memp_t type)
{
  struct memp *memp;

  ASSERT("memp_malloc: type < MEMP_MAX", type < MEMP_MAX);

  memp = memp_tab[type];
  
  if(memp != NULL) {    
    memp_tab[type] = memp->next;    
    memp->next = NULL;
#ifdef MEMP_STATS
    ++stats.memp[type].used;
    if(stats.memp[type].used > stats.memp[type].max) {
      stats.memp[type].max = stats.memp[type].used;
    }
#endif /* MEMP_STATS */
    ASSERT("memp_malloc: memp properly aligned",
	   ((u32_t)MEM_ALIGN((u8_t *)memp + sizeof(struct memp)) % MEM_ALIGNMENT) == 0);

    return MEM_ALIGN((u8_t *)memp + sizeof(struct memp));
  } else {
    DEBUGF(MEMP_DEBUG, ("memp_malloc: out of memory in pool %d\n", type));
#ifdef MEMP_STATS
    ++stats.memp[type].err;
#endif /* MEMP_STATS */
    return NULL;
  }
}
/*-----------------------------------------------------------------------------------*/
void *
memp_mallocp(memp_t type)
{
  void *mem;
  sys_sem_wait(mutex);
  mem = memp_malloc(type);
  sys_sem_signal(mutex);
  return mem;
}
/*-----------------------------------------------------------------------------------*/
#if 0
void *
memp_realloc(memp_t fromtype, memp_t totype, void *mem)
{
  void *rmem;
  u16_t size;
  
  if(mem == NULL) {
    return NULL;
  }
  
  rmem = memp_malloc(totype);
  if(rmem != NULL) { 
    size = memp_sizes[totype];
    if(memp_sizes[fromtype] < size) {
      size = memp_sizes[fromtype];
    }
    bcopy(mem, rmem, size);
    memp_free(fromtype, mem);
  }
  return rmem;
}
#endif /* 0 */
/*-----------------------------------------------------------------------------------*/
void
memp_free(memp_t type, void *mem)
{
  struct memp *memp;

  if(mem == NULL) {
    return;
  }
  memp = (struct memp *)((u8_t *)mem - sizeof(struct memp));

#ifdef MEMP_STATS
  stats.memp[type].used--; 
#endif /* MEMP_STATS */
  
  memp->next = memp_tab[type]; 
  memp_tab[type] = memp;

  ASSERT("memp sanity", memp_sanity());
  return;
}
/*-----------------------------------------------------------------------------------*/
void 
memp_freep(memp_t type, void *mem)
{
  sys_sem_wait(mutex);
  memp_free(type, mem);
  sys_sem_signal(mutex);
}
/*-----------------------------------------------------------------------------------*/
#endif /* !__PAULOS__ */
/*-----------------------------------------------------------------------------------*/
