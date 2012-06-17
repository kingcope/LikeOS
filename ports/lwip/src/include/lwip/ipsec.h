/* ipsec.h - PaulOS embedded operating system
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


struct crypt_alg {
    void *(*init) (u8_t *, int, u8_t *);
    void (*done) (void *);
    void (*decrypt) (void *, u8_t *, u8_t *, int);
    void (*encrypt) (void *, u8_t *, u8_t *, int);
};

struct auth_alg {
    void *(*init) (u8_t *, int);
    void (*done) (void *);
    void (*auth) (void *, u8_t *, u8_t *, int);
};

struct sa_data;

struct netif *sa_lookup_by_dest (struct ip_addr *dest);
struct pbuf *ipsec_tunnel_decrypt (struct pbuf *p, struct netif **inp);
void sa_update (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
		const struct crypt_alg *alg, const struct auth_alg *alg2, struct ip_addr *mask,
		struct ip_addr *network, struct ip_addr *peer);
void sa_add (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
	     const struct crypt_alg *alg, const struct auth_alg *alg2, struct ip_addr *mask,
	     struct ip_addr *network, struct ip_addr *peer);
void sa_set (u8_t peer_index, u8_t * key, u8_t key_len, u32_t spi_rcv, u32_t spi_snd,
	     const struct crypt_alg *alg_crypt, const struct auth_alg *alg_auth, struct ip_addr *mask,
	     struct ip_addr *network, struct ip_addr *peer);
void sa_disable (u8_t peer_index);

extern const struct crypt_alg blf_alg;
extern const struct crypt_alg des_alg;
extern const struct crypt_alg aes_alg;
extern const struct crypt_alg des3_alg;
extern const struct auth_alg md5_alg;
extern const struct auth_alg null_alg;

