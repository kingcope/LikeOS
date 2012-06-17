/* pbuf.c - PaulOS embedded operating system
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


#include "lwip/debug.h"

#include "lwip/stats.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"

#include "lwip/sys.h"

#include "arch/perf.h"
#include <assert.h>

#ifdef HAVE_MAD
#undef malloc
#undef free
#undef pbuf_free
#define malloc(x)	mad_alloc(x,file,line)
#define free(x)		mad_free(x,file,line)
#define pbuf_free(x)	mad_pbuf_free(x,file,line)
#endif

void get_stack_trace (unsigned long *results, int max)
{
}

#ifdef HAVE_MAGIC
#define clear_magic(p)	do { (p)->magic = 0; get_stack_trace ((p)->trace, 16); } while (0)
#define set_magic(p)	do { (p)->magic = PBUF_MAGIC; } while (0)
#define check_magic(p)										\
	do {											\
	    if ((p)->flags != PBUF_POOL && (p)->flags != PBUF_ROM && (p)->flags != PBUF_RAM) {	\
		__die (__FILE__, __FUNCTION__, __LINE__, 					\
		    "magic: pbuf flags bad value for pointer %p", (void *) (p));		\
	    }											\
	    magic(p, PBUF_MAGIC);								\
	} while (0)
#else
#define clear_magic(p)	do {  } while (0)
#define set_magic(p)	do {  } while (0)
#define check_magic(p)	do {  } while (0)
#endif


#ifdef HAVE_MAD
struct pbuf *mad_pbuf_alloc (pbuf_layer l, u16_t size, pbuf_flag flag, char *file, int line)
#else
struct pbuf *pbuf_alloc (pbuf_layer l, u16_t size, pbuf_flag flag)
#endif
{
    struct pbuf *p = NULL;
    u16_t offset;

    offset = 0;
    switch (l) {
    case PBUF_TRANSPORT:
	offset += PBUF_TRANSPORT_HLEN;
	/* FALLTHROUGH */
    case PBUF_IP:
	offset += PBUF_IP_HLEN;
	/* FALLTHROUGH */
    case PBUF_LINK:
	offset += PBUF_LINK_HLEN;
	break;
    case PBUF_RAW:
	break;
    default:
	die ("unknown pbuf type");
    }

    switch (flag) {
    case PBUF_POOL:
    case PBUF_RAM:
	p = malloc (sizeof (struct pbuf) + size + offset);
	assert (!((unsigned long) p & 3));
	p->payload = ((u8_t *) p + sizeof (struct pbuf) + offset);
	p->len = p->tot_len = size;
	p->next = NULL;
	p->flags = PBUF_FLAG_RAM;
	set_magic (p);
	break;
    case PBUF_ROM:
	/* If the pbuf should point to ROM, we only need to allocate
	   memory for the pbuf structure. */
	p = malloc (mem_type_size (MEMP_PBUF));
	p->payload = NULL;
	p->len = p->tot_len = size;
	p->next = NULL;
	p->flags = PBUF_FLAG_ROM;
	set_magic (p);
	break;
    default:
	die ("bad flag");
    }
    p->ref = 1;
    check_magic (p);
    return p;
}

void pbuf_refresh (void)
{
}

/*-----------------------------------------------------------------------------------*/
#ifdef HAVE_MAD
void mad_pbuf_realloc (struct pbuf *p, u16_t size, char *file, int line)
#else
void pbuf_realloc (struct pbuf *p, u16_t size)
#endif
{
    struct pbuf *q;
    u16_t rsize;

    check_magic (p);

    if (p->tot_len <= size)
	return;

    switch (p->flags) {
    case PBUF_FLAG_POOL:
    case PBUF_FLAG_RAM:
	for (q = p, rsize = size; rsize > q->len; q = q->next) {
	    check_magic (q);
	    rsize -= q->len;
	}
	q->len = rsize;
	if (q->next) {
	    pbuf_free (q->next);
	    q->next = NULL;
	}
	break;
    case PBUF_FLAG_ROM:
	p->len = size;
	break;
    default:
	die ("bad pbuf flags");
    }
    p->tot_len = size;
    check_magic (p);
}

/*-----------------------------------------------------------------------------------*/
u8_t pbuf_header (struct pbuf *p, s16_t header_size)
{
    void *save;
    check_magic (p);
    if (p->flags == PBUF_FLAG_ROM)
	return -1;
    save = p->payload;
    p->payload = (u8_t *) p->payload - header_size;
    if ((u8_t *) p->payload < (u8_t *) p + sizeof (struct pbuf)) {
	p->payload = save;
	return -1;
    }
    p->len += header_size;
    p->tot_len += header_size;
    return 0;
}

/*-----------------------------------------------------------------------------------*/
#ifdef HAVE_MAD
u8_t mad_pbuf_free (struct pbuf *p, char *file, int line)
#else
u8_t pbuf_free (struct pbuf *p)
#endif
{
    struct pbuf *q;
    u8_t count = 0;
    if (p) {
	check_magic (p);
	assert (p->ref > 0);
	if (!--p->ref) {
	    do {
		q = p->next;
		check_magic (p);
		p->flags = 0;
		clear_magic (p);
		free (p);
		p = q;
		++count;
	    } while (p);
	}
    }
    return count;
}

/*-----------------------------------------------------------------------------------*/
u8_t pbuf_clen (struct pbuf *p)
{
    u8_t len;
    for (len = 0; p; p = p->next)
	++len;
    return len;
}

/*-----------------------------------------------------------------------------------*/
void pbuf_ref (struct pbuf *p)
{
    if (p)
	p->ref++;
}

/*-----------------------------------------------------------------------------------*/
void pbuf_chain (struct pbuf *h, struct pbuf *t)
{
    struct pbuf *p;
    if (t) {
	for (p = h; p->next; p = p->next);
	p->next = t;
	h->tot_len += t->tot_len;
    }
}

/*-----------------------------------------------------------------------------------*/
struct pbuf *pbuf_dechain (struct pbuf *p)
{
    struct pbuf *q;

    q = p->next;
    if (q != NULL) {
	q->tot_len = p->tot_len - p->len;
    }
    p->tot_len = p->len;
    p->next = NULL;
    return q;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

/* FIXME: this is not tested properly with heavily chained pbufs: */
u16_t pbuf_read (struct pbuf *p, u8_t * buf, u16_t start, u16_t _len)
{
    u16_t len = _len, offset = 0, i_offset = 0;
    struct pbuf *q = p;
    while (q && len > 0) {
	u16_t copy;
	check_magic (q);
	if (offset >= start) {
	    copy = min (q->len - i_offset, len);
	    memcpy (buf, (u8_t *) q->payload + i_offset, copy);
	    len -= copy;
	    buf += copy;
	} else {
	    copy = min (q->len - i_offset, start - offset);
	}
	i_offset += copy;
	offset += copy;
	if (i_offset == q->len) {
	    q = q->next;
	    i_offset = 0;
	}
    }
    return _len - len;
}





