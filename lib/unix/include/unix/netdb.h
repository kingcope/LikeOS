#ifndef _NETDB_H
#define _NETDB_H

#include <inttypes.h>
#include <netinet/in.h>


struct hostent
{
	char  *h_name;    
	char **h_aliases; 
	int    h_addrtype;
	int    h_length; 
	char **h_addr_list;
};

struct netent
{
	char  *n_name;  
	char **n_aliases;
	int    n_addrtype;
	uint32_t n_net;
};


struct protoent
{
	char  *p_name;
	char **p_aliases;
	int    p_proto;
};


struct servent
{
	char  *s_name;
	char **s_aliases;
	int    s_port;
	char  *s_proto;
};

#ifdef __cplusplus
extern "C" {
#endif
		



#define	IPPORT_RESERVED		65535

extern int h_errno;		// TODO: ensure existence


#define	HOST_NOT_FOUND	-3
#define	NO_DATA		-4
#define	NO_RECOVERY	-5
#define	TRY_AGAIN	-6


void             endhostent(void);
void             endnetent(void);
void             endprotoent(void);
void             endservent(void);
struct hostent  *gethostbyaddr(const void *addr, size_t len, int type);
struct hostent  *gethostbyname(const char *name);
struct hostent  *gethostent(void);
struct netent   *getnetbyaddr(uint32_t net, int type);
struct netent   *getnetbyname(const char *name);
struct netent   *getnetent(void);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);
struct protoent *getprotoent(void);
struct servent  *getservbyname(const char *name, const char *proto);
struct servent  *getservbyport(int port, const char *proto);
struct servent  *getservent(void);
void             sethostent(int stayopen);
void             setnetent(int stayopen);
void             setprotoent(int stayopen);
void             setservent(int stayopen);

#ifdef __cplusplus
}
#endif
		


#endif


