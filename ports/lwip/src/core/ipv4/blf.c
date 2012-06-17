/* blf.c - PaulOS embedded operating system
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
#include <blowfish.h>

struct blf_arg {
    BF_KEY ks;
};

void *blf_init (u8_t * _key, int key_len, u8_t * block_size)
{
    u8_t key[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct blf_arg *blf_arg;
    blf_arg = mem_malloc ((size_t) sizeof (struct blf_arg));
    memset (blf_arg, '\0', sizeof (struct blf_arg));

    *block_size = 8;

    if (key_len < 8)
	key_len = 8;
    if (key_len > 24)
	key_len = 24;
    memcpy (key, _key, key_len);

/* initialize blowfish key schedule: */
    BF_set_key (&blf_arg->ks, key_len, key);

/* return value is opaque */
    return (void *) blf_arg;
}

void blf_done (void *arg)
{
    assert (arg);
    mem_free (arg);
}

void blf_decrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct blf_arg *blf_arg = (struct blf_arg *) alg_arg;
    BF_cbc_encrypt (data, data, data_len, &blf_arg->ks, iv, BF_DECRYPT);
}

void blf_encrypt (void *alg_arg, u8_t * iv, u8_t * data, int data_len)
{
    struct blf_arg *blf_arg = (struct blf_arg *) alg_arg;
    BF_cbc_encrypt (data, data, data_len, &blf_arg->ks, iv, BF_ENCRYPT);
}

const struct crypt_alg blf_alg = { blf_init, blf_done, blf_decrypt, blf_encrypt };

#endif				/* HAVE_IPSEC */
