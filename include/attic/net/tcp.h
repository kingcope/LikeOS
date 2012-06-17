#ifndef __TCP_H
#define __TCP_H

#define haddr_compare(a,b) (a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3]&&a[4]==b[4]&&a[5]==b[5])

struct ethernet_header {
	unsigned char ethernet_destination[6];
	unsigned char ethernet_source[6];
	unsigned short ethernet_type;
} __attribute__ ((packed));

struct tcp_header {
	unsigned short tcp_source_port;
	unsigned short tcp_destination_port;
	unsigned int tcp_sequence_number;
	unsigned int tcp_ack_number;
	unsigned int unused:4;	
	unsigned int tcp_header_length:4;
	unsigned char tcp_flags;
	unsigned short tcp_window_size;
	unsigned short tcp_checksum;
	unsigned short tcp_urgent_ptr;
} __attribute__ ((packed));

struct tcp_pseudo_header {
	unsigned int ip_source_address;
	unsigned int ip_destination_address;
	unsigned char unused;
	unsigned char protocol;
	unsigned short tcp_segment_length;	
} __attribute__ ((packed));

#define	TCP_FIN	0x01
#define	TCP_SYN	0x02
#define	TCP_RST	0x04
#define	TCP_PSH	0x08
#define	TCP_ACK	0x10
#define TCP_URG	0x20
#define	TCP_ECE	0x40
#define	TCP_CWR	0x80

extern void tcpip_mainloop();

#endif
