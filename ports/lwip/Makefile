LWIPDIR=src
INCLUDES=-I../../include -Iproj/unixsim -I$(LWIPDIR)/arch/paulos/include -I$(LWIPDIR)/include -I$(LWIPDIR)/include/ipv4 -I../../lib/unix/include/unix -Iapps -I.
CDEFS=-DIPv4 -DNO_THREADS -DIP_FORWARDING -DBIND_CHECK -DHAVE_MAGIC -DHAVE_ETH
CFLAGS=-O2 -nostdlib -nodefaultlibs -nostdinc -fno-builtin -static -fno-rtti -ffreestanding -fno-exceptions -Wall -Wl,-L../../lib/ -D__PAULOS__

IPV4=$(LWIPDIR)/sequencer.c $(LWIPDIR)/core/ipv4/icmp.c $(LWIPDIR)/core/ipv4/ip.c	\
	$(LWIPDIR)/core/inet.c $(LWIPDIR)/core/ipv4/ip_addr.c	\
	$(LWIPDIR)/core/ipv4/ipreass.c				\
	$(LWIPDIR)/core/ipv4/ipsec.c $(LWIPDIR)/core/ipv4/blf.c	\
	$(LWIPDIR)/core/ipv4/md5.c $(LWIPDIR)/core/ipv4/des3.c	\
	$(LWIPDIR)/core/ipv4/des.c $(LWIPDIR)/core/ipv4/aes.c	\
	$(LWIPDIR)/core/ipv4/null.c

CWFILES=$(LWIPDIR)/api/tcpip.c $(LWIPDIR)/api/err.c		\
	$(LWIPDIR)/netif/loopif.c $(LWIPDIR)/netif/etharp.c	\
	$(LWIPDIR)/core/mem.c $(LWIPDIR)/core/memp.c		\
	$(LWIPDIR)/core/netif.c $(LWIPDIR)/core/pbuf.c		\
	$(LWIPDIR)/core/tcp.c $(LWIPDIR)/core/tcp_input.c	\
	$(LWIPDIR)/core/tcp_output.c $(LWIPDIR)/core/udp.c

UNIXFILES=$(wildcard $(LWIPDIR)/arch/paulos/*.c $(LWIPDIR)/arch/paulos/netif/*.c) apps/identd.c paulos.c

#ACFILES=$(UNIXFILES) $(IPV4) $(CWFILES)
ACFILES=$(IPV4) $(CWFILES)
ACOBJECTS=$(ACFILES:.c=.o)

LIB=liblwip.a

HAVE_NET = 1
CC=gcc

ifeq ($(HAVE_NET),1)
all: $(LIB)
else
all:
endif

.PHONY: all

clean:
	rm -f *.o $(ACOBJECTS) $(LIB)

$(LIB): $(ACOBJECTS)
	rm -f $(LIB)
	ar cr $(LIB) $(ACOBJECTS)
	ranlib $(LIB)

.c.o:
	$(CC) $(INCLUDES) $(CDEFS) $(CFLAGS) -o $(<:.c=.o) -c $< -lunix

