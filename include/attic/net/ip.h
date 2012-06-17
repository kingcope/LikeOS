#ifndef __IP_H
#define __IP_H

#define IPPROTO_TCP 6

struct ip_header {
	unsigned char ip_header_length:4;	
	unsigned char ip_version:4;		
	unsigned char ip_type_of_service;
	unsigned short ip_total_length;
	unsigned short ip_identification;
	unsigned short ip_fragment_offset;
	unsigned char ip_time_to_live;
	unsigned char ip_protocol;
	unsigned short ip_header_checksum;
	unsigned int ip_source_address;
	unsigned int ip_destination_address;
} __attribute__ ((packed));

#endif
