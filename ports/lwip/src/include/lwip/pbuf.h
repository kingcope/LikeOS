/* pbuf.h - PaulOS embedded operating system
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
 * $Id: pbuf.h,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */
/*-----------------------------------------------------------------------------------*/
#ifndef __LWIP_PBUF_H__
#define __LWIP_PBUF_H__

#include "lwip/debug.h"
#include "lwip/arch.h"


#define PBUF_TRANSPORT_HLEN 20
#define PBUF_IP_HLEN        20

typedef enum {
  PBUF_TRANSPORT,
  PBUF_IP,
  PBUF_LINK,
  PBUF_RAW
} pbuf_layer;

typedef enum {
  PBUF_RAM,
  PBUF_ROM,
  PBUF_POOL
} pbuf_flag;

/* Definitions for the pbuf flag field (these are not the flags that
   are passed to pbuf_alloc()). */
#define PBUF_FLAG_RAM   0x00    /* Flags that pbuf data is stored in RAM. */
#define PBUF_FLAG_ROM   0x01    /* Flags that pbuf data is stored in ROM. */
#define PBUF_FLAG_POOL  0x02    /* Flags that the pbuf comes from the
				   pbuf pool. */

struct pbuf {
  struct pbuf *next;
#ifdef HAVE_MAGIC
#define PBUF_MAGIC 0xF45E0139
  u32_t magic;
  unsigned long trace[16];
#endif
  /* Pointer to the actual data in the buffer. */
  void *payload;
  
  /* Total length of buffer + additionally chained buffers. */
  u16_t tot_len;
  
  /* Length of this buffer. */
  u16_t len;  
  
  /* Flags and reference count. */
  u8_t flags, ref;
  
  u8_t pad1, pad2;
} __attribute__ ((packed));

/* pbuf_init():

   Initializes the pbuf module. The num parameter determines how many
   pbufs that should be allocated to the pbuf pool, and the size
   parameter specifies the size of the data allocated to those.  */
void pbuf_init(void);

/* pbuf_alloc():
   
   Allocates a pbuf at protocol layer l. The actual memory allocated
   for the pbuf is determined by the layer at which the pbuf is
   allocated and the requested size (from the size parameter). The
   flag parameter decides how and where the pbuf should be allocated
   as follows:
 
   * PBUF_RAM: buffer memory for pbuf is allocated as one large
               chunk. This includesprotocol headers as well.
   
   * RBUF_ROM: no buffer memory is allocated for the pbuf, even for
                protocol headers.  Additional headers must be
                prepended by allocating another pbuf and chain in to
                the front of the ROM pbuf.

   * PBUF_ROOL: the pbuf is allocated as a pbuf chain, with pbufs from
                the pbuf pool that is allocated during pbuf_init().  */
#ifdef HAVE_MAD
struct pbuf *mad_pbuf_alloc(pbuf_layer l, u16_t size, pbuf_flag flag, char *file, int line);
#define pbuf_alloc(l,s,f) mad_pbuf_alloc(l,s,f,__FILE__,__LINE__)
#else
struct pbuf *pbuf_alloc(pbuf_layer l, u16_t size, pbuf_flag flag);
#endif

/* pbuf_realloc():

   Shrinks the pbuf to the size given by the size parameter. 
 */
#ifdef HAVE_MAD
void mad_pbuf_realloc(struct pbuf *p, u16_t size, char *file, int line); 
#define pbuf_realloc(p,s) mad_pbuf_realloc(p,s,__FILE__,__LINE__)
#else
void pbuf_realloc(struct pbuf *p, u16_t size); 
#endif

/* pbuf_header():

   Tries to move the p->payload pointer header_size number of bytes
   upward within the pbuf. The return value is non-zero if it
   fails. If so, an additional pbuf should be allocated for the header
   and it should be chained to the front. */
u8_t pbuf_header(struct pbuf *p, s16_t header_size);

/* pbuf_ref():

   Increments the reference count of the pbuf p.
 */
void pbuf_ref(struct pbuf *p);

/* pbuf_free():

   Decrements the reference count and deallocates the pbuf if the
   reference count is zero. If the pbuf is a chain all pbufs in the
   chain are deallocated.  */
// u8_t pbuf_free(struct pbuf *p);
#ifdef HAVE_MAD
u8_t mad_pbuf_free(struct pbuf *p, char *file, int line);
#define pbuf_free(x) mad_pbuf_free(x,__FILE__,__LINE__)
#else
u8_t pbuf_free(struct pbuf *p);
#endif

/* pbuf_clen():

   Returns the length of the pbuf chain. */
u8_t pbuf_clen(struct pbuf *p);  

/* pbuf_chain():

   Chains pbuf t on the end of pbuf h. Pbuf h will have it's tot_len
   field adjusted accordingly. Pbuf t should no be used any more after
   a call to this function, since pbuf t is now a part of pbuf h.  */
void pbuf_chain(struct pbuf *h, struct pbuf *t);

/* pbuf_dechain():

   Picks off the first pbuf from the pbuf chain p. Returns the tail of
   the pbuf chain or NULL if the pbuf p was not chained. */
struct pbuf *pbuf_dechain(struct pbuf *p);

u16_t pbuf_read (struct pbuf *p, u8_t * buf, u16_t start, u16_t _len);

#endif /* __LWIP_PBUF_H__ */
