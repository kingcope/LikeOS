/* mem.c - PaulOS embedded operating system
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
 * $Id: mem.c,v 1.2 2003/08/01 11:58:01 psheer Exp $
 */

#ifndef __PAULOS__

/*-----------------------------------------------------------------------------------*/
/* mem.c
 *
 * Memory manager.
 *
 */
/*-----------------------------------------------------------------------------------*/
#include "lwip/debug.h"

#include "lwip/arch.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"

#include "lwip/sys.h"

#include "lwip/stats.h"

struct mem {
  mem_size_t next, prev;
  u8_t used;
#if MEM_ALIGNMENT == 2
  u8_t dummy;
#endif /* MEM_ALIGNEMNT == 2 */
};

static struct mem *ram_end;
static u8_t ram[MEM_SIZE + sizeof(struct mem) + MEM_ALIGNMENT];

#define MIN_SIZE 12
#define SIZEOF_STRUCT_MEM MEM_ALIGN_SIZE(sizeof(struct mem))
/*#define SIZEOF_STRUCT_MEM (sizeof(struct mem) + \
                          (((sizeof(struct mem) % MEM_ALIGNMENT) == 0)? 0 : \
                          (4 - (sizeof(struct mem) % MEM_ALIGNMENT))))*/


static struct mem *lfree;   /* pointer to the lowest free block */

static sys_sem_t mem_sem;

/*-----------------------------------------------------------------------------------*/
static void
plug_holes(struct mem *mem)
{
  struct mem *nmem;
  struct mem *pmem;

  ASSERT("plug_holes: mem >= ram", (u8_t *)mem >= ram);
  ASSERT("plug_holes: mem < ram_end", (u8_t *)mem < (u8_t *)ram_end);
  ASSERT("plug_holes: mem->used == 0", mem->used == 0);
  
  /* plug hole forward */
  ASSERT("plug_holes: mem->next <= MEM_SIZE", mem->next <= MEM_SIZE);
  
  nmem = (struct mem *)&ram[mem->next];
  if(mem != nmem && nmem->used == 0 && (u8_t *)nmem != (u8_t *)ram_end) {
    if(lfree == nmem) {
      lfree = mem;
    }
    mem->next = nmem->next;
    ((struct mem *)&ram[nmem->next])->prev = (u8_t *)mem - ram;
  }

  /* plug hole backward */
  pmem = (struct mem *)&ram[mem->prev];
  if(pmem != mem && pmem->used == 0) {
    if(lfree == mem) {
      lfree = pmem;
    }
    pmem->next = mem->next;
    ((struct mem *)&ram[mem->next])->prev = (u8_t *)pmem - ram;
  }

}
/*-----------------------------------------------------------------------------------*/
void
mem_init(void)
{
  struct mem *mem;

  bzero(ram, MEM_SIZE);
  mem = (struct mem *)ram;
  mem->next = MEM_SIZE;
  mem->prev = 0;
  mem->used = 0;
  ram_end = (struct mem *)&ram[MEM_SIZE];
  ram_end->used = 1;
  ram_end->next = MEM_SIZE;
  ram_end->prev = MEM_SIZE;

  mem_sem = sys_sem_new(1);

  lfree = (struct mem *)ram;

#ifdef MEM_STATS
  stats.mem.avail = MEM_SIZE;
#endif /* MEM_STATS */
}
/*-----------------------------------------------------------------------------------*/
void *
mem_malloc(mem_size_t size)
{
  mem_size_t ptr, ptr2;
  struct mem *mem, *mem2;

  if(size == 0) {
    return NULL;
  }

  /* Expand the size of the allocated memory region so that we can
     adjust for alignment. */
  if((size % MEM_ALIGNMENT) != 0) {
    size += MEM_ALIGNMENT - ((size + SIZEOF_STRUCT_MEM) % MEM_ALIGNMENT);
  }
  
  if(size > MEM_SIZE) {
    return NULL;
  }
  
  sys_sem_wait(mem_sem);

  for(ptr = (u8_t *)lfree - ram; ptr < MEM_SIZE; ptr = ((struct mem *)&ram[ptr])->next) {
    mem = (struct mem *)&ram[ptr];
    if(!mem->used &&
       mem->next - (ptr + SIZEOF_STRUCT_MEM) >= size + SIZEOF_STRUCT_MEM) {
      ptr2 = ptr + SIZEOF_STRUCT_MEM + size;
      mem2 = (struct mem *)&ram[ptr2];

      mem2->prev = ptr;      
      mem2->next = mem->next;
      mem->next = ptr2;      
      if(mem2->next != MEM_SIZE) {
        ((struct mem *)&ram[mem2->next])->prev = ptr2;
      }
      
      mem2->used = 0;      
      mem->used = 1;
#ifdef MEM_STATS
      stats.mem.used += size;
      /*      if(stats.mem.max < stats.mem.used) {
        stats.mem.max = stats.mem.used;
	} */
      if(stats.mem.max < ptr2) {
        stats.mem.max = ptr2;
      }      
#endif /* MEM_STATS */

      if(mem == lfree) {
	/* Find next free block after mem */
        while(lfree->used && lfree != ram_end) {
	  lfree = (struct mem *)&ram[lfree->next];
        }
        ASSERT("mem_malloc: !lfree->used", !lfree->used);
      }
      sys_sem_signal(mem_sem);
      ASSERT("mem_malloc: allocated memory not above ram_end.",
	     (u32_t)mem + SIZEOF_STRUCT_MEM + size <= (u32_t)ram_end);
      ASSERT("mem_malloc: allocated memory properly aligned.",
	     (unsigned long)((u8_t *)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGNMENT == 0);
      return (u8_t *)mem + SIZEOF_STRUCT_MEM;
    }    
  }
  DEBUGF(MEM_DEBUG, ("mem_malloc: could not allocate %d bytes\n", (int)size));
#ifdef MEM_STATS
  ++stats.mem.err;
#endif /* MEM_STATS */  
  sys_sem_signal(mem_sem);
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
void
mem_free(void *rmem)
{
  struct mem *mem;

  if(rmem == NULL) {
    return;
  }
  
  sys_sem_wait(mem_sem);

  ASSERT("mem_free: legal memory", (u8_t *)rmem >= (u8_t *)ram &&
	 (u8_t *)rmem < (u8_t *)ram_end);
  
  
  if((u8_t *)rmem < (u8_t *)ram || (u8_t *)rmem >= (u8_t *)ram_end) {
    DEBUGF(MEM_DEBUG, ("mem_free: illegal memory\n"));
#ifdef MEM_STATS
    ++stats.mem.err;
#endif /* MEM_STATS */
    return;
  }
  mem = (struct mem *)((u8_t *)rmem - SIZEOF_STRUCT_MEM);

  ASSERT("mem_free: mem->used", mem->used);
  
  mem->used = 0;

  if(mem < lfree) {
    lfree = mem;
  }
  
#ifdef MEM_STATS
  stats.mem.used -= mem->next - ((u8_t *)mem - ram) - SIZEOF_STRUCT_MEM;
  
#endif /* MEM_STATS */
  plug_holes(mem);
  sys_sem_signal(mem_sem);
}
/*-----------------------------------------------------------------------------------*/
void *
mem_reallocm(void *rmem, mem_size_t newsize)
{
  void *nmem;
  nmem = mem_malloc(newsize);
  if(nmem == NULL) {
    return mem_realloc(rmem, newsize);
  }
  bcopy(rmem, nmem, newsize);
  mem_free(rmem);
  return nmem;
}
/*-----------------------------------------------------------------------------------*/
void *
mem_realloc(void *rmem, mem_size_t newsize)
{
  mem_size_t size;
  mem_size_t ptr, ptr2;
  struct mem *mem, *mem2;
  
  sys_sem_wait(mem_sem);
  
  ASSERT("mem_realloc: legal memory", (u8_t *)rmem >= (u8_t *)ram &&
	 (u8_t *)rmem < (u8_t *)ram_end);
  
  if((u8_t *)rmem < (u8_t *)ram || (u8_t *)rmem >= (u8_t *)ram_end) {
    DEBUGF(MEM_DEBUG, ("mem_free: illegal memory\n"));
    return rmem;
  }
  mem = (struct mem *)((u8_t *)rmem - SIZEOF_STRUCT_MEM);

  ptr = (u8_t *)mem - ram;

  size = mem->next - ptr - SIZEOF_STRUCT_MEM;
#ifdef MEM_STATS
  stats.mem.used -= (size - newsize);
#endif /* MEM_STATS */
  
  if(newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size) {
    ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
    mem2 = (struct mem *)&ram[ptr2];
    mem2->used = 0;
    mem2->next = mem->next;
    mem2->prev = ptr;
    mem->next = ptr2;
    if(mem2->next != MEM_SIZE) {
      ((struct mem *)&ram[mem2->next])->prev = ptr2;
    }

    plug_holes(mem2);
  }
  sys_sem_signal(mem_sem);  
  return rmem;
}
/*-----------------------------------------------------------------------------------*/
#endif /* !__PAULOS__ */
