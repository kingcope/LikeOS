/* md5.c - PaulOS embedded operating system
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

#ifndef HAVE_MD5
#error HAVE_MD5 must be set for IPSEC with Blowfish+MD5 encryption
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
#include <md5.h>

#define HMAC_BLOCK_LEN		64
#define HMAC_IPAD_VAL		0x36
#define HMAC_OPAD_VAL		0x5C

static const u8_t ipad[64] = {
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
};

static const u8_t opad[64] = {
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
    0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C
};

struct md5_arg {
    MD5_CTX icontext;
    MD5_CTX ocontext;
};

void *md5_init (u8_t * _key, int key_len)
{
    u8_t k, key[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct md5_arg *md5_arg;
    md5_arg = mem_malloc ((size_t) sizeof (struct md5_arg));
    memset (md5_arg, '\0', sizeof (struct md5_arg));

    if (key_len < 8)
	key_len = 8;
    if (key_len > 24)
	key_len = 24;
    memcpy (key, _key, key_len);

    printf ("%.*s\n", key_len, key);

/* initialize MD5 MAC contexts: */
    for (k = 0; k < key_len; k++)
	key[k] ^= HMAC_IPAD_VAL;

    MD5Init (&md5_arg->icontext);
    MD5Update (&md5_arg->icontext, key, key_len);
    MD5Update (&md5_arg->icontext, ipad, HMAC_BLOCK_LEN - key_len);

    for (k = 0; k < key_len; k++)
	key[k] ^= (HMAC_IPAD_VAL ^ HMAC_OPAD_VAL);

    MD5Init (&md5_arg->ocontext);
    MD5Update (&md5_arg->ocontext, key, key_len);
    MD5Update (&md5_arg->ocontext, opad, HMAC_BLOCK_LEN - key_len);

    printf ("alg_arg = %p\n", md5_arg);

/* return value is opaque */
    return (void *) md5_arg;
}

void md5_done (void *arg)
{
    assert (arg);
    mem_free (arg);
}

void md5_auth (void *alg_arg, u8_t * mac, u8_t * data, int data_len)
{
    unsigned char r[16];
    struct md5_arg *md5_arg = (struct md5_arg *) alg_arg;
    MD5_CTX context;

/* get ipad context: */
    memcpy (&context, &md5_arg->icontext, sizeof (context));
/* do 1st hash: */
    MD5Update (&context, data, data_len);
    MD5Final (r, &context);

/* get opad context: */
    memcpy (&context, &md5_arg->ocontext, sizeof (context));
/* do 2nd hash: */
    MD5Update (&context, r, 16);
    MD5Final (r, &context);

/* only 96 bits of the MAC are used for IPSEC: */
    memcpy (mac, r, 12);
}

const struct auth_alg md5_alg = { md5_init, md5_done, md5_auth };

#endif				/* HAVE_IPSEC */
