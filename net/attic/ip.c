/*
 $ LikeOS TCP/IP Stack
 $ Copyright (C)2006 Microtrends Ltd., Nikolaos Rangos
 $
 */

#include "sys/un.h"
#include "netinet/in.h"
#include "net/net.h"
#include "net/ip.h"
#include "net/tcp.h"
#include "etherboot.h"
#include "nic.h"

extern unsigned int isn;

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
inet_aton(const char *cp, struct in_addr *addr)
{
	register unsigned int val;
	register int base, n;
	register char c;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}

short checksum(char *addr, int count) {
 	/* Compute Internet Checksum for "count" bytes
 	 * beginning at location "addr".
 	 */
 	long sum = 0;
 
 	while( count > 1 ) {
 		/* This is the inner loop */
 		sum += ntohs(* (unsigned short *) addr);
 		addr += sizeof(unsigned short);
 		count -= sizeof(unsigned short);
	}

	/* Add left-over byte, if any */
 	if( count > 0 )
 		sum += * (unsigned char *) addr;

 	/* Fold 32-bit sum to 16 bits */
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);
 
 	return((short)~sum);
}

struct ip_header *ip_build(unsigned int source_addr, unsigned int dest_addr) {
	struct ip_header *ip = (struct ip_header*) kalloc(sizeof(struct ip_header));
	static unsigned short identification=0;
	
	identification++;	
	
	ip->ip_version=4;
	ip->ip_header_length=5;
	ip->ip_type_of_service=0;
	//ip->ip_total_length=htons(sizeof(struct ip_header));
	ip->ip_identification=htons(identification);
	ip->ip_fragment_offset=0;
	ip->ip_time_to_live=128;
	ip->ip_protocol=IPPROTO_TCP;
	ip->ip_source_address=source_addr;
	ip->ip_destination_address=dest_addr;
	//ip->ip_header_checksum=htons(checksum((char*)ip, sizeof(struct ip_header)));
	
	return ip;	
}

int ip_input(void *stripped_packet) {
	int k,iplen = sizeof(struct ip_header);
	int tcplen = sizeof(struct tcp_header);
	struct ip_header *ip = (struct ip_header*) kalloc(iplen);
	struct tcp_header *tcp = (struct tcp_header*) kalloc(tcplen);
	struct in_addr lip;
	struct ip_header *ip_response;
	struct tcp_header *tcp_response;
	char *buf;
	
	inet_aton(localip, &lip);
		
	memcpy(ip, stripped_packet, iplen);
	
	/*$ Assume TCP Protocol $*/
	memcpy(tcp, stripped_packet+iplen, tcplen);
	
	if (ip->ip_destination_address != lip.s_addr)
		return -1;	

	switch (tcp->tcp_flags) {
		case (TCP_SYN|TCP_ACK): {
			/*$$$ Complete Three Way Handshake $$$*/
						
			ip_response = ip_build(lip.s_addr, ip->ip_source_address);
			tcp_response = tcp_build(htons(tcp->tcp_destination_port),
									htons(tcp->tcp_source_port),
									++isn,
									htonl(tcp->tcp_sequence_number)+1,									
									TCP_ACK,
									65535);
			ip_response->ip_total_length = htons(iplen + tcplen);
			ip_response->ip_header_checksum=htons(checksum((char*)ip_response, iplen));
			tcp_response->tcp_checksum = tcp_checksum(tcp_response, ip_response, tcplen);			
			buf = (char*)kalloc(tcplen + iplen);

			memcpy(buf, ip_response, iplen);
			memcpy(buf+iplen, tcp_response, tcplen);
			like_eth_transmit(&eth_addr, 0x0800, iplen+tcplen, buf);
			kfree(buf);			
			/*$$$ Inform of connection establishment $$$*/
			for (k=0;k<connectioncount;k++) {
				if ((connections[k].destination_ip == ip->ip_source_address) &&
					 connections[k].destination_port == tcp->tcp_source_port) {
						connections[k].connected=1;
						connections[k].isn=tcp->tcp_sequence_number;
						connections[k].ack=tcp->tcp_ack_number;
						break;
					}
			}
			break;
			}
		
		case (TCP_RST|TCP_ACK): {
			/*$$$ Inform of connection termination $$$*/
			for (k=0;k<connectioncount;k++) {
				if ((connections[k].destination_ip == ip->ip_source_address) &&
					 connections[k].destination_port == tcp->tcp_source_port) {
						connections[k].connected=-1;
						break;
					}
			}			
			break;
			}
	}	
	
	kfree(tcp);
	kfree(ip);
}
