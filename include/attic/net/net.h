#ifndef __NET_H
#define __NET_H

#define htons(A)  ((((unsigned short)(A) & 0xff00) >> 8) | \
                   (((unsigned short)(A) & 0x00ff) << 8))
#define htonl(A)  ((((unsigned int)(A) & 0xff000000) >> 24) | \
                   (((unsigned int)(A) & 0x00ff0000) >> 8)  | \
                   (((unsigned int)(A) & 0x0000ff00) << 8)  | \
                   (((unsigned int)(A) & 0x000000ff) << 24))
#define ntohs     htons
#define ntohl     htohl

struct connection {
	unsigned int destination_ip;
	unsigned short destination_port;
	unsigned short source_port;
	volatile int connected;
	unsigned int isn;
	unsigned int ack;
};

#define MAXSOCKETS 256

extern struct connection connections[MAXSOCKETS];
extern int connectioncount;

#endif
