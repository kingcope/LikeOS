#ifndef __ARP_H
#define __ARP_H

#define ARP_REQUESTREPLY 0x0806
#define ARP_OPTYPE_REQUEST 1
#define ARP_OPTYPE_REPLY 2

struct arp_header {
	unsigned short arp_hardware_addr_type;
	unsigned short arp_frame_type;
	unsigned char arp_haddr_size; /*$ 6 $*/
	unsigned char arp_protocol_size; /*$ 4 $*/
	unsigned short arp_optype; 
	unsigned char arp_source_mac[6];
	unsigned int arp_source_ip;
	unsigned char arp_destination_mac[6];
	unsigned int arp_destination_ip;
} __attribute__ ((packed));

#endif
