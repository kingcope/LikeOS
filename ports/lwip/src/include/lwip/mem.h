/* mem.h - PaulOS embedded operating system
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
 * $Id: mem.h,v 1.3 2003/08/01 11:58:01 psheer Exp $
 */
#ifndef __LWIP_MEM_H__
#define __LWIP_MEM_H__

#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/arch.h"

#ifdef __PAULOS__

int mem_type_size (int type);

#ifdef HAVE_MAD

#define mem_malloc(x)		mad_alloc (x,__FILE__,__LINE__)
#define mem_realloc(x,y)	mad_realloc (x,y,__FILE__,__LINE__)
#define mem_free(x)		mad_free (x,__FILE__,__LINE__)
#define memp_malloc(x)		mad_alloc (mem_type_size ((int) x),__FILE__,__LINE__)
#define memp_mallocp(x)		mad_alloc (mem_type_size ((int) x),__FILE__,__LINE__)
#define memp_realloc(f,t,x)	mad_realloc (x,mem_type_size ((int) t),__FILE__,__LINE__)
#define memp_freep(t,x)		mad_free (x,__FILE__,__LINE__)
#define memp_free(t,x)		mad_free (x,__FILE__,__LINE__)

/* Signature for detecting overwrites */
#define MAD_SIGNATURE (('M'<<24)|('a'<<16)|('d'<<8)|('S'))
#define MAD_MARKER(x)	long mad_marker_ ## x = MAD_SIGNATURE

void *mad_alloc (int size, const char *file, int line);
void *mad_realloc (void *ptr, int size, const char *file, int line);
void mad_free (void *ptr, const char *file, int line);

#else

#define MAD_MARKER(x) 

#define mem_malloc(x)		malloc (x)
#define mem_realloc(x,y)	realloc (x,y)
#define mem_free(x)		free (x)
#define memp_malloc(x)		malloc (mem_type_size ((int) x))
#define memp_mallocp(x)		malloc (mem_type_size ((int) x))
#define memp_realloc(f,t,x)	realloc (x,mem_type_size ((int) t))
#define memp_free(t,x)		free (x)
#define memp_freep(t,x)		free (x)

void *malloc (long unsigned int size);
void *realloc (void *ptr, int size);
void free (void *p);


#endif

#else /* __PAULOS__ */

#if MEM_SIZE > 64000l
typedef u32_t mem_size_t;
#else
typedef u16_t mem_size_t;
#endif /* MEM_SIZE > 64000 */


void mem_init(void);

void *mem_malloc(mem_size_t size);
void mem_free(void *mem);
void *mem_realloc(void *mem, mem_size_t size);
void *mem_reallocm(void *mem, mem_size_t size);

#ifdef MEM_PERF
void mem_perf_start(void);
void mem_perf_init(char *fname);
#endif /* MEM_PERF */

#ifdef MEM_RECLAIM
typedef mem_size_t (*mem_reclaim_func)(void *arg, mem_size_t size);
void mem_register_reclaim(mem_reclaim_func f, void *arg);
void mem_reclaim(unsigned int size);
#else
#define mem_register_reclaim(f, arg)
#endif /* MEM_RECLAIM */

#endif /* !__PAULOS__ */

#define MEM_ALIGN_SIZE(size) (size + \
                             ((((size) % MEM_ALIGNMENT) == 0)? 0 : \
                             (MEM_ALIGNMENT - ((size) % MEM_ALIGNMENT))))

#define MEM_ALIGN(addr) (void *)MEM_ALIGN_SIZE((u32_t)addr)

#endif /* __LWIP_MEM_H__ */

