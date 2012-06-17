/*
 $ LikeOS TCP/IP Stack
 $ Copyright (C)2006 Microtrends Ltd., Nikolaos Rangos
 $
 */

#include "stdio.h"
#include "net/net.h"
#include "net/tcp.h"
#include "net/ip.h"
#include "etherboot.h"
#include "nic.h"
#include "net/arp.h"
#include "sys/un.h"
#include "netinet/in.h"

unsigned int isn=0;
struct connection connections[MAXSOCKETS];
int connectioncount=0;
int tcplen = sizeof(struct tcp_header);
int iplen = sizeof(struct ip_header);

unsigned short tcp_checksum(struct tcp_header *tcp, struct ip_header *ip, unsigned short tcp_segment_length) {
/*$ XXX If data length is odd add a padding byte $*/
	struct tcp_pseudo_header *tp = (struct tcp_pseudo_header*) kalloc(sizeof(struct tcp_pseudo_header));
	char *buf;
	unsigned short chksum;
	int tcplen = sizeof(struct tcp_header);
	int tcpplen = sizeof(struct tcp_pseudo_header);
	
	tp->ip_source_address = ip->ip_source_address;
	tp->ip_destination_address = ip->ip_destination_address;
	tp->unused=0;
	tp->protocol=IPPROTO_TCP;
	tp->tcp_segment_length = htons(tcp_segment_length);
	
	buf = (char*) kalloc(tcplen + tcpplen);

	memcpy(buf, tp, tcpplen);
	memcpy(buf+tcpplen, tcp, tcplen);
	
	chksum = htons(checksum((char*)buf, tcplen + tcpplen));
	
	kfree(buf);
	kfree(tp);
	
	return chksum;
}

struct tcp_header *tcp_build(unsigned short source_port,
							 unsigned short dest_port,
							 unsigned int sn,
							 unsigned int ackn,
							 unsigned char flags,
							 unsigned short window_size
							 ) {
	struct tcp_header *tcp = (struct tcp_header*) kalloc(sizeof(struct tcp_header));
	
	tcp->tcp_source_port = htons(source_port);
	tcp->tcp_destination_port = htons(dest_port);
	tcp->tcp_sequence_number = htonl(sn);
	tcp->tcp_ack_number = htonl(ackn);
	tcp->tcp_header_length = 5;
	tcp->unused = 0;
	tcp->tcp_flags = flags; /*$$$*/
	tcp->tcp_window_size = htons(window_size);
	tcp->tcp_urgent_ptr = 0;
	
	return tcp;
}

int tcp_send(int sock, char *data, unsigned short len) {
	struct tcp_header *tcp;
	struct ip_header *ip;	
	struct in_addr s;
	char *buf;
	unsigned short lent;
	
	inet_aton(localip, &s);
	
	if (connections[sock].connected != 1)
		return -1;
	
	if (len > 65535)
		return -1;
	
	isn++;
	ip = ip_build(s.s_addr, connections[sock].destination_ip);
	
	tcp=tcp_build(	htons(connections[sock].source_port),
					htons(connections[sock].destination_port),
					htonl(connections[sock].ack),
					htonl(connections[sock].isn),
					TCP_PSH|TCP_ACK,
					4096);
	
	buf = (char*)kalloc(tcplen + iplen + len);
	lent = iplen + tcplen + len;
	ip->ip_total_length = htons(lent);
	ip->ip_header_checksum=htons(checksum((char*)ip, htons(iplen)));
	memcpy(buf, ip, iplen);	
/*	memcpy(buf+iplen, tcp, tcplen);	
	memcpy(buf+iplen+tcplen, data, len);
	tcp->tcp_checksum = tcp_checksum(buf+iplen, ip, tcplen + len);
	memcpy(buf+iplen, tcp, tcplen);*/
		
	like_eth_transmit(&eth_addr, 0x0800, iplen, buf);
	
	kfree(buf);
	return 0;
}

int tcp_establish (char *source_ip,
					unsigned short source_port,
					char *dest_ip,
					unsigned short dest_port
					) {
	struct tcp_header *tcp;
	struct ip_header *ip;
	char *buf;
	struct in_addr s,d;
	int c;
	inet_aton(source_ip, &s);
	inet_aton(dest_ip, &d);
	
	c=connectioncount;
	connections[connectioncount].destination_ip=d.s_addr;
	connections[connectioncount].destination_port=htons(dest_port);
	connections[connectioncount].source_port=htons(source_port);
	connections[connectioncount].connected=0;
	connectioncount++;
	
	isn++;
				
	ip = ip_build(s.s_addr, d.s_addr);
	tcp=tcp_build(	source_port,
					dest_port,
					isn,
					0,
					TCP_SYN,
					4096);

	ip->ip_total_length = htons(iplen + tcplen);
	ip->ip_header_checksum=htons(checksum((char*)ip, iplen));
	tcp->tcp_checksum = tcp_checksum(tcp, ip, tcplen);
	buf = (char*)kalloc(tcplen + iplen);

	memcpy(buf, ip, iplen);
	memcpy(buf+iplen, tcp, tcplen);
	
	like_eth_transmit(&eth_addr, 0x0800, iplen+tcplen, buf);	
	
	kfree(buf);
	
	while (connections[c].connected == 0); //$$$ XXX add timeout $$$
	return c;
}

void tcpip_mainloop() {
	int ethhl = sizeof(struct ethernet_header);
	struct ethernet_header *ethh = (struct ethernet_header*) kalloc(ethhl);
	int onetime=0;
	unsigned char ethernet_broadcast[]={255,255,255,255,255,255};
	char *p;
	
	for(;;) {	
		if (like_eth_poll(1) == 1) {
			memcpy(ethh, nic.packet, ethhl);

			if ((haddr_compare(ethh->ethernet_destination, nic.node_addr)) || 
				(haddr_compare(ethh->ethernet_destination, ethernet_broadcast))) {				
				switch(htons(ethh->ethernet_type)) {
					case ARP_REQUESTREPLY: {
							p=nic.packet;
							p+=ethhl;
							arp_input((struct arp_header*)p);
							break;
						}
					case 0x0800: {		//$ IP PROTOCOL $
							p=nic.packet;
							p+=ethhl;
							ip_input(p);
							break;
						}
					
				}
				
			}
		}

	}
}
