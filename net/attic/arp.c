/*
 $ LikeOS TCP/IP Stack
 $ Copyright (C)2006 Microtrends Ltd., Nikolaos Rangos
 $
 */
 
#include "stdio.h"
#include "net/net.h"
#include "net/arp.h"
#include "net/ip.h"
#include "net/tcp.h"
#include "etherboot.h"
#include "nic.h"
#include "sys/un.h"
#include "netinet/in.h"

int arplen = sizeof(struct arp_header);

struct arp_header *arp_build(unsigned short arp_optype,
							 unsigned char *arp_source_mac,
							 unsigned int arp_source_ip,
							 unsigned char *arp_destination_mac,
							 unsigned int arp_destination_ip) {
	struct arp_header *arph = (struct arp_header*) kalloc(arplen);
	
	arph->arp_hardware_addr_type = htons(1);
	arph->arp_frame_type = htons(0x0800); // IP FRAME TYPE
	arph->arp_haddr_size = 6;
	arph->arp_protocol_size = 4;
	arph->arp_optype = htons(arp_optype);
	memcpy(arph->arp_source_mac, arp_source_mac, 6);
	arph->arp_source_ip = arp_source_ip;
	memcpy(arph->arp_destination_mac, arp_destination_mac, 6);
	arph->arp_destination_ip = arp_destination_ip;
	
	return arph;
}

void arp_reply(unsigned char *arp_destination_mac, unsigned int arp_destination_ip) {
	struct arp_header *arph = (struct arp_header*) kalloc(arplen);
	struct in_addr s;
	
	inet_aton(localip, &s);
	arph = arp_build(ARP_OPTYPE_REPLY, nic.node_addr, s.s_addr, arp_destination_mac, arp_destination_ip);
	
	like_eth_transmit(&eth_addr, ARP_REQUESTREPLY, arplen, arph);
}

void arp_input(struct arp_header *arph) {
	struct in_addr s;
	inet_aton(localip, &s);
	
	if ((htons(arph->arp_optype) == ARP_OPTYPE_REQUEST) &&
	    (arph->arp_destination_ip == s.s_addr)) {
		arp_reply(arph->arp_source_mac, arph->arp_source_ip);	    
	}
}
