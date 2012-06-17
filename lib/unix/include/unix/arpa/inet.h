#ifndef _ARPA_INET_H
#define _ARPA_INET_H


#include <inttypes.h>
#include <netinet/in.h>


#ifdef __cplusplus
extern "C" {
#endif
		

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

in_addr_t      inet_addr(const char *cp);
in_addr_t      inet_lnaof(struct in_addr in);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t lna);
in_addr_t      inet_netof(struct in_addr in);
in_addr_t      inet_network(const char *cp);
char          *inet_ntoa(struct in_addr in);

#ifdef __cplusplus
}
#endif
		


#endif

