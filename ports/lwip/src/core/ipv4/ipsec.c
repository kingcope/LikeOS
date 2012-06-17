/* ipsec.c - PaulOS embedded operating system
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


#ifdef HAVE_IPSEC

#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/icmp.h"
#include "lwip/ipsec.h"
#include "netif/validate.h"

#include <types.h>
#include <assert.h>

void syslog (int, char *, ...);

//#define TR(fmt, args...)      do { syslog (0, "%s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, ## args); } while (0)
//#define TR(fmt, args...)      do { printf ("%s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); printf (fmt, ## args); printf ("\n"); } while (0)
#define TR(fmt, args...)	do { } while (0)

/* This file implements RFC-2406 (ESP - Encapsulated Security
Payload). It supports one SA per remote peer in either
direction. It supports IP-in-IP (tunnel) mode only, and then
only with blowfish encryption. This is what you want for 99%
of things you want to do. */


/* Summary of tunneling from RFC-2406: */

/*
 *
 *
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ----
 *   |               Security Parameters Index (SPI)                 | ^Auth.
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |Cov-
 *   |                      Sequence Number                          | |erage
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ | ----
 *   |                    Payload Data* (variable)                   | |   ^
 *   ~                                                               ~ |   |
 *   |                                                               | |Conf.
 *   +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |Cov-
 *   |               |     Padding (0-255 bytes)                     | |erage*
 *   +-+-+-+-+-+-+-+-+               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |   |
 *   |                               |  Pad Length   | Next Header   | v   v
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ------
 *   |                 Authentication Data (variable)                |
 *   ~                                                               ~
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *
 *            -----------------------------------------------------------
 *      IPv4  | new IP hdr* |     | orig IP hdr*  |   |    | ESP   | ESP|
 *            |(any options)| ESP | (any options) |TCP|Data|Trailer|Auth|
 *            -----------------------------------------------------------
 *                                |<--------- encrypted ---------->|
 *                          |<----------- authenticated ---------->|
 */

#define SEQUENCE_START	0

static struct pbuf *ipsec_tunnel_encrypt (struct pbuf *p, struct sa_data *sa_data, u8_t proto, u8_t zero_seq);

typedef unsigned long long mask_t;

#define MASK_BITS (sizeof (mask_t) * 8)

#define SEND_PING_REPS_RETRIES		100
#define SEND_GET_REPS_RETRIES		200

struct sa_data {
    struct sa_data *next;
    u8_t peer_index;
    u8_t send_get_reps;
    u8_t send_ping_reps;
    u8_t block_size;
    struct netif netif;
    const struct crypt_alg *alg_crypt;
    const struct auth_alg *alg_auth;
    void *alg_crypt_arg;
    void *alg_auth_arg;
    u32_t spi_rcv;
    u32_t spi_snd;
    u32_t seq_rcv;
    mask_t seq_mask;
    u32_t seq_snd;
    struct ip_addr mask;
    struct ip_addr network;
    struct ip_addr peer;
    u8_t rand_vec[8];
};

union ipsec_renegotiation {
    char pad[40];
    struct {
	u32_t pad;
#define RENEG_SET_SEQUENCE	1
#define RENEG_GET_SEQUENCE	2
#define RENEG_DUMMY		0xFF
	u32_t type;
	u32_t seq_rcv;
	u32_t seq_snd;
	u8_t rand_vec[8];
    } data;
} __attribute__ ((packed));

struct sa_list {
    struct sa_data *next;
};

struct esp_hdr {
    u32_t spi;
    u32_t seq;
} __attribute__ ((packed));

struct esp_trailer {
    u8_t padlen;
    u8_t proto;
} __attribute__ ((packed));

struct sa_list sa_list = { NULL };

static err_t ipsec_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr /* not used */ );

static void sa_free (struct sa_data *sa)
{
    assert (sa);
    (*sa->alg_crypt->done) (sa->alg_crypt_arg);
    (*sa->alg_auth->done) (sa->alg_auth_arg);
    mem_free (sa);
}

void sa_delete (u8_t peer_index)
{
    struct sa_data *sa;
    for (sa = (struct sa_data *) &sa_list; sa->next; sa = sa->next)
	if (sa->next->peer_index == peer_index) {
	    struct sa_data *next;
	    TR ("deleting SA peer_index %d", (int) peer_index);
	    next = sa->next->next;
	    sa_free (sa->next);
	    sa->next = next;
	    return;
	}
}

static struct sa_data *sa_find (u8_t peer_index)
{
    struct sa_data *sa;
    for (sa = sa_list.next; sa; sa = sa->next)
	if (sa->peer_index == peer_index)
	    return sa;
    return NULL;
}

void sa_flush (void)
{
    struct sa_data *sa, *next;
    for (sa = sa_list.next; sa; sa = next) {
	next = sa->next;
	sa_free (sa);
    }
    sa_list.next = NULL;
}

int netif_is_ipsec (struct netif *netif)
{
    if (netif->name[0] == 'c' && netif->name[1] == 'r')
	return 1;
    return 0;
}

void sa_update (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
		const struct crypt_alg *alg_crypt, const struct auth_alg *alg_auth, struct ip_addr *mask,
		struct ip_addr *network, struct ip_addr *peer)
{
    struct sa_data *sa;
    struct netif *netif = NULL;
    struct ip_addr gw;
    sa = sa_find (peer_index);
    if (!sa) {
	TR ("SA not found for peer index %d", (int) peer_index);
	return;
    }

    TR ("updating SA peer index %d", (int) peer_index);

/* configure dummy netif device: */
    if (peer->addr)
	netif = ip_route_no_ipsec (peer, &gw);
    sa->netif.ip_addr.addr = netif ? netif->ip_addr.addr : 0;	/* (4) tcp stack needs this to be correct */
    sa->netif.netmask.addr = 0;
    sa->netif.mtu = 2048;
    sa->netif.name[0] = 'c';
    sa->netif.name[1] = 'r';
    sa->netif.output = ipsec_output;
    sa->netif.state = (void *) sa;

    sa->alg_crypt = alg_crypt;
    sa->alg_auth = alg_auth;
    if (sa->alg_crypt_arg)
	(*sa->alg_crypt->done) (sa->alg_crypt_arg);
    if (sa->alg_auth_arg)
	(*sa->alg_auth->done) (sa->alg_auth_arg);
    TR ("initializing crypto with key %.*s", key_len, key);
    sa->alg_crypt_arg = (*sa->alg_crypt->init) (key, key_len, &sa->block_size);
    sa->alg_auth_arg = (*sa->alg_auth->init) (key, key_len);
    TR ("done init");

/* do not reset the sequence number unless the spi has changed */
    if (sa->spi_rcv != spi_rcv) {
	sa->spi_rcv = spi_rcv;
	sa->seq_rcv = SEQUENCE_START;
	sa->seq_mask = ~0;
    }
    if (sa->spi_snd != spi_snd) {
	sa->spi_snd = spi_snd;
	sa->seq_snd = SEQUENCE_START;
    }
    sa->mask = *mask;
    sa->network = *network;
    if (peer->addr && netif)	/* defensive programming ? */
	sa->peer = *peer;
    else
	sa->peer.addr = 0;
    TR ("mask %s, network %s, peer %s", (char *) inet_ntoa (sa->mask), (char *) inet_ntoa (sa->network),
	(char *) inet_ntoa (sa->peer));
}

//void ipsec_test (struct sa_data *sa);

void sa_add (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
	     const struct crypt_alg *alg_crypt, const struct auth_alg *alg_auth, struct ip_addr *mask,
	     struct ip_addr *network, struct ip_addr *peer)
{
    struct sa_data *sa;

/* remove any existing association to this peer */
    sa_delete (peer_index);

/* allocate new structure */
    sa = (struct sa_data *) mem_malloc ((size_t) sizeof (struct sa_data));
    memset (sa, '\0', (size_t) sizeof (struct sa_data));
    sa->seq_mask = ~0;
    sa->peer_index = peer_index;

/* add to list */
    sa->next = sa_list.next;
    sa_list.next = sa;

/* update contents */
    sa_update (peer_index, key, key_len, spi_rcv, spi_snd, alg_crypt, alg_auth, mask, network, peer);
}

void sa_set (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
	     const struct crypt_alg *alg_crypt, const struct auth_alg *alg_auth, struct ip_addr *mask,
	     struct ip_addr *network, struct ip_addr *peer)
{
    if (!sa_find (peer_index)) {
	sa_add (peer_index, key, key_len, spi_rcv, spi_snd, alg_crypt, alg_auth, mask, network, peer);
    } else {
	sa_update (peer_index, key, key_len, spi_rcv, spi_snd, alg_crypt, alg_auth, mask, network, peer);
    }
}

void sa_disable (u8_t peer_index)
{
    struct sa_data *sa;
    if ((sa = sa_find (peer_index)))
	sa->peer.addr = sa->network.addr = sa->mask.addr = 0;
}

static struct sa_data *ipsec_route (struct ip_addr *dest)
{
    struct sa_data *sa;
    for (sa = sa_list.next; sa; sa = sa->next) {
	if (sa->network.addr && sa->mask.addr) {
	    if (ip_addr_maskcmp (dest, &(sa->network), &(sa->mask))) {
		TR ("routing packet %s through SA %d, SPI %d", (char *) inet_ntoa (*dest),
		    (int) sa->peer_index, (int) sa->spi_snd);
		return sa;
	    }
	    if (sa->peer.addr)
		if (ip_addr_cmp (dest, &(sa->peer))) {
		    TR ("routing packet %s through SA %d, SPI %d", (char *) inet_ntoa (*dest),
			(int) sa->peer_index, (int) sa->spi_snd);
		    return sa;
		}
	}
    }
    return NULL;
}

struct netif *sa_lookup_by_dest (struct ip_addr *dest)
{
    struct sa_data *sa;
    sa = ipsec_route (dest);
    if (!sa)
	return NULL;
    return &sa->netif;
}

static struct sa_data *sa_lookup_by_spi (u32_t spi_rcv)
{
    struct sa_data *sa;
    for (sa = sa_list.next; sa; sa = sa->next)
	if (sa->spi_rcv == spi_rcv)
	    return sa;
    return NULL;
}

#define MAX_INIT_VEC_LEN	16
#define IPSEC_MAC_LEN		12
#define IPSEC_TTL		64
#define IPSEC_MAX_OVERHEAD(sa)		\
    (IP_HLEN + sizeof(struct esp_hdr) + (sa)->block_size + sizeof(struct esp_trailer) + ((sa)->alg_auth->auth ? IPSEC_MAC_LEN : 0))

#define IPSEC_HLEN(iphdr)		 (IPH_HL (iphdr) * 4)
#define IPSEC_FULLLEN(iphdr)		 (ntohs (IPH_LEN (iphdr)))

/* RFC-1700: "any private encryption scheme": */
#define IP_PROTO_any_private	99

#define BAD_PACKET(x)		do { if ((x)) goto error_return; } while (0)
#define IPSEC_LOG		0
#define BAD_PACKET_LOG(x,m)	do { if ((x)) { syslog m ; goto error_return; } } while (0)

/* send a renegotiation "GET" packet. This effectively asks
the peer (on the other end of the SA) what sequence number it
is currently at. It sends a random challenge to prevent
against a reply. */
static void ipsec_init_renegotiation (struct sa_data *sa)
{
    struct pbuf *p, *q;
    union ipsec_renegotiation *n;
    assert (sizeof (union ipsec_renegotiation) == 40);
    p = pbuf_alloc (PBUF_IP, 40, PBUF_RAM);
    n = (union ipsec_renegotiation *) p->payload;
    memset (n, '\0', (size_t) sizeof (*n));
    n->data.pad = htonl (0xFFFFFFFFUL);	/* <-- so as to be sure this never gets confused with an IP packet. (2) */
    n->data.type = htonl (RENEG_GET_SEQUENCE);
    memcpy (n->data.rand_vec, sa->rand_vec, (size_t) sizeof (sa->rand_vec));
    q = ipsec_tunnel_encrypt (p, sa, IP_PROTO_any_private, 1);
    ip_output (q, NULL, &sa->peer, IPSEC_TTL, IP_PROTO_ESP);
    pbuf_free (q);
    pbuf_free (p);
}

/* now just send a junk packet back - this will set the peer's
->peer field to us because it will have the sequence number
correct and hence pass all the checks like any regular packet.
this will just get written off as a bad packet by
packet_validate() at (3) below: */
static void ipsec_send_ping (struct sa_data *sa)
{
    struct pbuf *p, *q;
    union ipsec_renegotiation *n;
    assert (sizeof (union ipsec_renegotiation) == 40);
    p = pbuf_alloc (PBUF_IP, 40, PBUF_RAM);
    n = (union ipsec_renegotiation *) p->payload;
    memset (n, '\0', (size_t) sizeof (*n));
    n->data.pad = htonl (0xFFFFFFFFUL);
    n->data.type = htonl (RENEG_DUMMY);
    random_string (n->data.rand_vec, (int) sizeof (n->data.rand_vec));
    q = ipsec_tunnel_encrypt (p, sa, IP_PROTO_IPIP, 0);
    ip_output (q, NULL, &sa->peer, IPSEC_TTL, IP_PROTO_ESP);
    pbuf_free (q);
    pbuf_free (p);
}

/* we resend ten times or until we get a reply: */
void ipsec_tmr (void)
{
    static u8_t count = 0;
    if (!(count++ & 7)) {	/* repeat every 4 seconds */
	struct sa_data *sa;
	for (sa = sa_list.next; sa; sa = sa->next) {
	    if (sa->send_get_reps) {
		if (--sa->send_get_reps) {	/* send_get_reps must be non-zero. see (1) */
		    TR ("sending reneg peer index %d", sa->peer_index);
		    ipsec_init_renegotiation (sa);
		}
	    }
	    if (sa->send_ping_reps) {
		if (--sa->send_ping_reps) {
		    TR ("sending ping peer index %d", sa->peer_index);
		    ipsec_send_ping (sa);
		}
	    }
	}
    }
}

/* function to call from outside to explicitly renegotiate an
SA: */
void ipsec_send_renegotiation (u8_t peer_index)
{
    struct sa_data *sa;
    sa = sa_find (peer_index);
    if (sa) {
	random_string (sa->rand_vec, (int) sizeof (sa->rand_vec));
	sa->send_get_reps = SEND_GET_REPS_RETRIES;
	ipsec_init_renegotiation (sa);
    }
}

struct pbuf *ipsec_tunnel_decrypt (struct pbuf *p, struct netif **inp)
{
    u8_t mac[12];
    s16_t len, c;
    u32_t seq, spi;
    mask_t mask = 0;
    s32_t delta;
    struct ip_hdr *h;
    struct esp_hdr *esp;
    struct sa_data *sa_data;
    struct esp_trailer *trailer;
    struct ip_addr src;
    unsigned char *t, *s;

    TR ("entering, p->tot_len = %d, p->len = %d", (int) p->tot_len, (int) p->len);

    if (p->next) {
/* PaulOS does not receive chained packets via ip_input: i.e.
the network devices always return monolithic packets: */
	printf ("***WARNING*** (ipsec) how did we get a chained packet?");
	BAD_PACKET (1);
    }

/* get plain text header: */
    t = (char *) p->payload;
    h = (struct ip_hdr *) t;
    len = IPSEC_FULLLEN (h);
    src = h->src;

    TR ("len = %d, src = %s, dst = %s", (int) len, (char *) inet_ntoa (src), (char *) inet_ntoa (h->dest));

/* get ESP header: */
    t += IPSEC_HLEN (h), len -= IPSEC_HLEN (h);
    esp = (struct esp_hdr *) t;
    seq = ntohl (esp->seq);
    spi = ntohl (esp->spi);
    BAD_PACKET (len < 32);
    BAD_PACKET (!spi);

    TR ("checking packet alignment");

/* check length is 8 byte aligned: */
    BAD_PACKET ((len - IPSEC_MAC_LEN - sizeof (struct esp_hdr)) & 7);

    TR ("len = %d, seq = %lu, spi = %lu", (int) len, (unsigned long) seq, spi);

/* lookup Security Association: */
    sa_data = sa_lookup_by_spi (spi);
    BAD_PACKET_LOG (!sa_data,
		    (IPSEC_LOG, "IPSEC: unknown SPI %lu from %s", (long) spi, (char *) inet_ntoa (h->src)));

/* disabled SA's are caught here: */
    BAD_PACKET (!(sa_data->network.addr && sa_data->mask.addr && sa_data->peer.addr && sa_data->spi_rcv && sa_data->spi_snd));

    TR ("found SA");

/* give upper layer a dummy device that this packet came from: */
    *inp = &sa_data->netif;

    if (!seq) {

	TR ("!seq");

/* our special extension to the RFC: zero sequence numbers are
re-negotiation packets - we do not check for replay attacks: */
	if (sa_data->alg_auth->auth) {
	    len -= IPSEC_MAC_LEN;
	    BAD_PACKET (len < 32 + sa_data->block_size);
	    (*sa_data->alg_auth->auth) (sa_data->alg_auth_arg, mac, t, len);
	    BAD_PACKET (memcmp (mac, t + len, 12));
	}
    } else {

/* check for replayed packets: */
	delta = (s32_t) ((u32_t) seq - sa_data->seq_rcv);

	TR ("delta = %d, mask = %08llx, seq_rcv = %lu", (int) delta, (unsigned long long) sa_data->seq_mask,
	    (unsigned long) sa_data->seq_rcv);

	BAD_PACKET_LOG ((s32_t) delta < (s32_t) - MASK_BITS || (s32_t) delta > (s32_t) 16384,
			(IPSEC_LOG, "IPSEC: out of range sequence number %lu from %s", (long) seq,
			 (char *) inet_ntoa (h->src)));
	if (delta <= 0) {
	    mask = (mask_t) 1 << (-delta);
/* packet out of range of the window, or packet already present: */
	    BAD_PACKET_LOG ((sa_data->seq_mask & mask),
			    (IPSEC_LOG, "IPSEC: replayed sequence number %lu from %s", (long) seq,
			     (char *) inet_ntoa (h->src)));
	}

/* authenticate packet: */
	if (sa_data->alg_auth->auth) {
	    len -= IPSEC_MAC_LEN;
	    BAD_PACKET (len < 32 + sa_data->block_size);
	    (*sa_data->alg_auth->auth) (sa_data->alg_auth_arg, mac, t, len);
	    BAD_PACKET (memcmp (mac, t + len, 12));
	}

/* possibly advance window: */
	if (delta > 0) {
	    sa_data->seq_mask <<= delta;
	    mask = 1;
	    sa_data->seq_rcv = seq;
	}

/* record packet within window: */
	sa_data->seq_mask |= mask;
	TR ("seq_rcv = %lu, mask = %08llx", (unsigned long) sa_data->seq_rcv,
	    (unsigned long long) sa_data->seq_mask);
    }

/* get start of encrypted data: */
    t += sizeof (struct esp_hdr), len -= sizeof (struct esp_hdr);
    BAD_PACKET (len < 24 /* IP + TRAILER */  + sa_data->block_size);

/* check length is 8 byte aligned: */
    BAD_PACKET (len % sa_data->block_size);

/* skip over IV */
    s = t;
    t += sa_data->block_size, len -= sa_data->block_size;
    BAD_PACKET (len < 24 /* IP + TRAILER */ );
    TR ("decrypting %d bytes", (int) len);
    (*sa_data->alg_crypt->decrypt) (sa_data->alg_crypt_arg, s, t, len);

/* get ESP trailer */
    trailer = (struct esp_trailer *) (t + len - sizeof (struct esp_trailer));

    TR ("proto = %d, padding = %d", (int) trailer->proto, trailer->padlen);

/* check protocol: */
    BAD_PACKET (trailer->proto != IP_PROTO_IPIP && trailer->proto != IP_PROTO_any_private);

/* our private messages have seq zero: */
    BAD_PACKET (trailer->proto == IP_PROTO_any_private && seq != 0);

/* subtract padding from packet length: */
    len -= trailer->padlen + sizeof (struct esp_trailer);
    BAD_PACKET (len < 20 /* IP */ );

    TR ("checking padding...");
/* check padding contents */
    for (c = 1, s = t + len; c <= trailer->padlen; c++, s++) {
	BAD_PACKET (s < t || *s != c);
    }
    TR ("padding ok.");

/* adjust pbuf: */
    p->payload = t;
    p->tot_len = p->len = len;

/* our special extension to the RFC: zero sequence numbers are
re-negotiation packets: */
    if (!seq) {
	struct pbuf *q;
	union ipsec_renegotiation *n;
	BAD_PACKET (len != sizeof (union ipsec_renegotiation));
	n = (union ipsec_renegotiation *) p->payload;
	TR ("!seq type = %lu", (unsigned long) ntohl (n->data.type));

//printf ("after decryption:\n");
//{int j;
//for (j = 0; j < p->tot_len; j++)
//printf ("%02x%c", (unsigned int) ((unsigned char *) p->payload)[j], ((j + 1) % 8) ? ' ' : '\n');
//printf ("\n\n");}

	switch (ntohl (n->data.type)) {
	case RENEG_SET_SEQUENCE:
	    BAD_PACKET (!sa_data->send_get_reps);	/* (1) */
/* check if the received random bytes match the sent bytes: */
	    BAD_PACKET (memcmp (n->data.rand_vec, sa_data->rand_vec, sizeof (sa_data->rand_vec)));
	    TR ("good packet RENEG_SET_SEQUENCE");
/* it does - so the peer is authentic. we can trust as much as
to set our IP address. this is redundant since we would have
to have the correct peer address to have been able to send a
RENEG_GET_SEQUENCE in the first place: */
	    sa_data->peer = src;
/* allow no more RENEG_SET_SEQUENCE packets: */
	    memset (sa_data->rand_vec, '\0', (size_t) sizeof (sa_data->rand_vec));
	    sa_data->send_get_reps = 0;
	    sa_data->seq_snd = ntohl (n->data.seq_rcv) + 1;
	    sa_data->seq_mask = ~0;
	    sa_data->seq_rcv = ntohl (n->data.seq_snd);
	    syslog (IPSEC_LOG, "IPSEC: syncing sequence numbers to %lu:%lu for SPI pair %lu - %lu",
		    (unsigned long) sa_data->seq_rcv, (unsigned long) sa_data->seq_snd,
		    (unsigned long) sa_data->spi_rcv, (unsigned long) sa_data->spi_snd);
/* now tell our IP to the peer: */
	    sa_data->send_ping_reps = SEND_PING_REPS_RETRIES;
	    TR ("sending ping");
	    ipsec_send_ping (sa_data);
	    break;
	case RENEG_GET_SEQUENCE:
	    TR ("good packet RENEG_GET_SEQUENCE");
	    n->data.pad = htonl (0xFFFFFFFFUL);	/* see (2) */
	    n->data.type = htonl (RENEG_SET_SEQUENCE);
	    n->data.seq_rcv = htonl (sa_data->seq_rcv);
	    n->data.seq_snd = htonl (sa_data->seq_snd);
	    TR ("sending RENEG_SET_SEQUENCE to %s", (char *) inet_ntoa (src));
	    q = ipsec_tunnel_encrypt (p, sa_data, IP_PROTO_any_private, 1);
/* we must send to "src" and not "->peer", because the IP
address of the peer could have changed. Note that we send no
privileged information, nor do we perform any modifications to
ourselves. Hence the security and integrity of the protocol is
immune to RENEG_GET_SEQUENCE packets: */
	    ip_output (q, NULL, &src, IPSEC_TTL, IP_PROTO_ESP);
	    pbuf_free (q);
	    break;
	}
	pbuf_free (p);
	return NULL;
    }

/* the packet was validated, therefore we are pretty sure the
peer is who he claims. So we set the peer IP to that of the
src address. This is in case we are dealing with a dynamic IP
peer whose IP may change. */
    if (sa_data->peer.addr != src.addr) {
	TR ("peer changed from %s to %s", (char *) inet_ntoa (sa_data->peer.addr), (char *) inet_ntoa (src));
	syslog (IPSEC_LOG, "IPSEC: peer has changed to IP address %s for SPI pair %lu - %lu", inet_ntoa (src),
		(unsigned long) sa_data->spi_rcv, (unsigned long) sa_data->spi_snd);
	sa_data->peer = src;
    }

/* check that this is a regular TCP/UDP/ICMP packet: */
    BAD_PACKET (packet_validate (p, NULL, NULL));	/* (3) */

    TR ("good packet from %s", (char *) inet_ntoa (src));

    TR ("src=%s dst=%s", (char *) inet_ntoa (((struct ip_hdr *) p->payload)->src),
	(char *) inet_ntoa (((struct ip_hdr *) p->payload)->dest));

    return p;

  error_return:
    pbuf_free (p);
    return NULL;
}

static struct pbuf *ipsec_tunnel_encrypt (struct pbuf *p, struct sa_data *sa_data, u8_t proto, u8_t zero_seq)
{
    u8_t *s, *t, *ivp, *e, iv[MAX_INIT_VEC_LEN];
    struct esp_hdr *esp;
    struct esp_trailer *trailer;
    struct pbuf *q;
    u16_t len, padding, e_len, auth_len, c;

/* calculate the full length of the encrypted part, by first
adding 2 and then rounding up to eight + the IV: */
    {
	u16_t padded_len;
	len = sa_data->block_size + p->tot_len + sizeof (struct esp_trailer);
	padded_len = (len + (sa_data->block_size - 1));
	padded_len -= padded_len % sa_data->block_size;
	padding = padded_len - len;
	len = padded_len;
    }

    TR ("e_len = %d, padding = %d", (int) len, (int) padding);

/* add header */
    e_len = len - sa_data->block_size;
    len += sizeof (struct esp_hdr);

    auth_len = len;

    TR ("len = %d", (int) len);

/* add 12 bytes for mac if required: */
    if (sa_data->alg_auth->auth)
	len += IPSEC_MAC_LEN;

    TR ("len = %d", (int) len);

/* allocate monolithic packet: */
    q = pbuf_alloc (PBUF_IP, len, PBUF_RAM);

    t = (u8_t *) q->payload;

/* fill in the esp header */
    esp = (struct esp_hdr *) t;
    esp->spi = htonl (sa_data->spi_snd);
    if (zero_seq) {		/* our special protocol */
	esp->seq = 0;
    } else {
	sa_data->seq_snd++;
	if (!sa_data->seq_snd)
	    sa_data->seq_snd++;
	if (sa_data->seq_snd > 1000000000UL)
	    syslog (IPSEC_LOG, "Time to change your IPSEC password for peer %s", inet_ntoa (sa_data->peer));
	esp->seq = htonl (sa_data->seq_snd);
    }
    t += sizeof (struct esp_hdr);

    TR ("spi = %lu", (unsigned long) ntohl (esp->spi));
    TR ("seq = %lu", (unsigned long) ntohl (esp->seq));

/* get IV offset */
    ivp = t;
    t += sa_data->block_size;

/* fill in full packet data */
    pbuf_read (p, e = t, 0, p->tot_len);

    t += p->tot_len;

/* fill the padding bytes: */
    for (c = 1; c <= padding; c++)
	*t++ = c;

/* fill in the trailer */
    trailer = (struct esp_trailer *) t;
    trailer->padlen = padding;
    trailer->proto = proto;
    t += sizeof (struct esp_trailer);

/* perform encryption: */
    random_string (iv, (int) sa_data->block_size);
    memcpy (ivp, iv, (size_t) sa_data->block_size);
    (*sa_data->alg_crypt->encrypt) (sa_data->alg_crypt_arg, iv, e, e_len);

    TR ("auth_len = %d", (int) (e_len + sizeof (struct esp_hdr)));

    s = (u8_t *) q->payload;
    TR ("data = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", s[0], s[1], s[2],
	s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11]);

/* add 96 bit mac: */
    if (sa_data->alg_auth->auth)
	(*sa_data->alg_auth->auth) (sa_data->alg_auth_arg, t, q->payload, auth_len);

    return q;
}

static void ipsec_encap (struct pbuf *q, struct ip_addr *src, struct ip_addr *dst, u8_t ttl)
{
    struct ip_hdr *iphdr;
    u16_t ip_id;

    pbuf_header (q, IP_HLEN);
    iphdr = (struct ip_hdr *) q->payload;
/* fill in IP header: */
    IPH_TTL_SET (iphdr, ttl);
    IPH_PROTO_SET (iphdr, IP_PROTO_ESP);
    IPH_VHLTOS_SET (iphdr, 4, IP_HLEN / 4, 0);
    IPH_LEN_SET (iphdr, htons (q->tot_len));
    IPH_OFFSET_SET (iphdr, (u16_t) 0);
    ip_id = random ();
    IPH_ID_SET (iphdr, ip_id);

    ip_addr_set (&(iphdr->dest), dst);
    ip_addr_set (&(iphdr->src), src);

    IPH_CHKSUM_SET (iphdr, 0);
    IPH_CHKSUM_SET (iphdr, inet_chksum (iphdr, IP_HLEN));
}

/* This function could be a simple encryption and then sending
of the packet through ip_output(). however ip_output() calls
ip_output_if() which may try to send a ICMP_DUR_FRAG to the
src address (which is us in this case). We need to take into
account MTU (and sending of ICMP_DUR_FRAG) from info both in
the plaintext packet and info in the encrypted packet. Hence
this function is somewhat complex. */
static err_t ipsec_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr /* not used */ )
{
    struct sa_data *sa;
    struct ip_addr gw;
    struct pbuf *q;
    u16_t frag;

/* get the Security Association: */
    sa = (struct sa_data *) netif->state;

/* if it is disabled simply drop packets. ipsec routes must be
available even if they are not working so that packets are not
rejected. */
    if (!sa->spi_rcv || !sa->spi_snd || !sa->peer.addr)
	return ERR_OK;

/* route the packet first to get the true netif device: */
    netif = ip_route_no_ipsec (&sa->peer, &gw);
    if (!netif)
	return ERR_OK;

/* in case the routes have changed (4): FIXME: there is a rare
situation when the routes could have changed and TCP stack
outputs before getting to this line. */
    sa->netif.ip_addr = netif->ip_addr;

    TR ("ipsec_output src=%s dst=%s --> %s via %c%c",
	(char *) inet_ntoa (((struct ip_hdr *) p->payload)->src),
	(char *) inet_ntoa (((struct ip_hdr *) p->payload)->dest),
	(char *) inet_ntoa (sa->peer), (int) netif->name[0], (int) netif->name[1]);

/* Before we try send the packet, check if it was BOTH too
long for the MTU AND if the DF bit was explicitly set. This is
done for clients trying to discover the MTU of the path: */
    frag = ntohs (IPH_OFFSET ((struct ip_hdr *) p->payload));
    if (p->len + IPSEC_MAX_OVERHEAD (sa) > netif->mtu && (frag & (IP_DF | IP_MF | IP_OFFMASK)) == IP_DF) {
	TR ("sending ICMP_DUR_FRAG");
	icmp_dest_unreach (p, ICMP_DUR_FRAG, netif->mtu - IPSEC_MAX_OVERHEAD (sa));
	return ERR_OK;
    }

/* perform encryption, keeping the original packet: */
    q = ipsec_tunnel_encrypt (p, sa, IP_PROTO_IPIP, 0);
    if (!q)
	return ERR_OK;

/* prepend an encapsulation header: */
    ipsec_encap (q, &netif->ip_addr, &sa->peer, IPH_TTL ((struct ip_hdr *) p->payload));

/* send the packet: */
    TR ("sending packet through %c%c", (int) netif->name[0], (int) netif->name[1]);
    ip_output_if (q, NULL, IP_HDRINCL, 0, 0, netif, &gw);
    pbuf_free (q);
    return ERR_OK;
}

#endif				/* HAVE_IPSEC */

    
