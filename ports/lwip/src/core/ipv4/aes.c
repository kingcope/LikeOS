/* aes.c - PaulOS embedded operating system
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

#ifndef HAVE_BLOWFISH
#error HAVE_BLOWFISH must be set for IPSEC with Blowfish+MD5 encryption
#endif

#include "lwip/debug.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/ipsec.h"
#include "netif/validate.h"

#include <types.h>
#include <assert.h>
#include <rijndael.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

struct aes_arg {
    RIJNDAEL_KEY ks_encrypt;
    RIJNDAEL_KEY ks_decrypt;
};

struct aes_block {
    u32_t l1, l2, l3, l4;
};

void *aes_init (u8_t * _key, int key_len, u8_t * block_size)
{
    u8_t key[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct aes_arg *aes_arg;
    aes_arg = mem_malloc ((size_t) sizeof (struct aes_arg));
    memset (aes_arg, '\0', sizeof (struct aes_arg));

    *block_size = 16;

    if (key_len < 8)
	key_len = 8;
    if (key_len > 24)
	key_len = 24;
    memcpy (key, _key, key_len);
    key_len = (key_len + 7) & ~7;

/* initialize blowfish key schedule: */
    aes_arg->ks_encrypt.rounds = rijndaelKeySetupEnc ((void *) &aes_arg->ks_encrypt, key, key_len * 8);
    aes_arg->ks_decrypt.rounds = rijndaelKeySetupDec ((void *) &aes_arg->ks_decrypt, key, key_len * 8);

/* return value is opaque */
    return (void *) aes_arg;
}

void aes_done (void *arg)
{
    assert (arg);
    mem_free (arg);
}

void aes_decrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct aes_arg *aes_arg = (struct aes_arg *) alg_arg;
    struct aes_block *x, *d, c, b;
    x = (struct aes_block *) iv;
    d = (struct aes_block *) data;
    while ((data_len -= 16) >= 0) {
	c = *d;
	rijndaelDecrypt ((void *) &aes_arg->ks_decrypt, aes_arg->ks_decrypt.rounds, (u8_t *) d, (u8_t *) d);
	d->l1 ^= x->l1;
	d->l2 ^= x->l2;
	d->l3 ^= x->l3;
	d->l4 ^= x->l4;
	b = c;
	x = &b;
	d++;
    }
}

void aes_encrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct aes_arg *aes_arg = (struct aes_arg *) alg_arg;
    struct aes_block *x, *d;
    x = (struct aes_block *) iv;
    d = (struct aes_block *) data;
    while ((data_len -= 16) >= 0) {
	d->l1 ^= x->l1;
	d->l2 ^= x->l2;
	d->l3 ^= x->l3;
	d->l4 ^= x->l4;
	rijndaelEncrypt ((void *) &aes_arg->ks_encrypt, aes_arg->ks_encrypt.rounds, (u8_t *) d, (u8_t *) d);
	x = d++;
    }
}

const struct crypt_alg aes_alg = { aes_init, aes_done, aes_decrypt, aes_encrypt };

#endif				/* HAVE_IPSEC */
