#$$ LikeOS Makefile
#$$

LWIPDIR=ports/lwip/src

INCLUDES=-Iports/lwip/proj/unixsim \
	-I$(LWIPDIR)/arch/paulos/include -I$(LWIPDIR)/include \
	-I$(LWIPDIR)/include/ipv4  -Iapps -I. -Iports/svgagui/

CFLAGS=	-O2 -nostdlib -nodefaultlibs -nostdinc \
	-fno-builtin -static -ffreestanding \
	-fno-exceptions -Wall -Iinclude/ -Iinclude/c++/ \
	-Ilib/unix/include/unix/ -Wl,-Llib/ -Iports/mpeg2dec/include \
	$(INCLUDES) -D__PAULOS__ -DHAVE_ETH -DHAVE_MAGIC -Lports/svgagui/
	
CPPFLAGS=-O2 -nostdlib -nodefaultlibs -nostdinc \
	-fno-builtin -static -ffreestanding \
	-fno-exceptions -Wall -Iinclude/ \
	-Ilib/unix/include/unix/ -Wl,-Llib/ -Lports/svgagui/
	
#LDFLAGS=-Werror -T like.ld -static -Llib/ -Lports/lwip/ -Lports/svgagui/ -Lports/mpeg2dec -llwip -lsvgagui -lunix -lmpeg2 -lmpeg2convert
LDFLAGS=-T like.ld -static -Llib/ -Lports/lwip/ -Lports/svgagui/ -Lports/mpeg2dec -lsvgagui -llwip -lunix -lmpeg2  -lmpeg2convert

CC=gcc

#GET=co

SRCS=	krnl/kernel.c krnl/paging.c krnl/physalloc.c \
	krnl/kalloc.c krnl/task.c gfx/vesa.c \
	krnl/interrupts.c fs/dma.c \
	fs/isofs.c drv/mouse.c \
	shell/font.c drv/keyb.c drv/pci/i386_timer.c \
	drv/net/tulip.c drv/net/eepro100.c drv/net/e1000.c drv/net/pcnet32.c \
	drv/pci/pci.c drv/pci/pci_probe.c \
	drv/pci/pci_io.c drv/net/nic.c \
	app/desq/desq.c app/desq/term.c shell/text.c \
	net/net.c net/netif.c \
	drv/ide_x.c fs/eltorito.c drv/pci_x.c drv/blockdev.c fs/vfs.c

OBJS=krnl/kernel.o krnl/paging.o krnl/physalloc.o \
	krnl/kalloc.o krnl/task.o gfx/vesa.o \
	krnl/interrupts.o fs/dma.o \
	fs/isofs.o drv/mouse.o \
	shell/font.o drv/keyb.o drv/pci/i386_timer.o \
	drv/net/tulip.o drv/net/eepro100.o drv/net/e1000.o drv/net/pcnet32.o \
	drv/pci/pci.o drv/pci/pci_probe.o \
	drv/pci/pci_io.o drv/net/nic.o \
	app/desq/desq.o app/desq/term.o shell/text.o \
	net/net.o net/netif.o \
	drv/ide_x.o fs/eltorito.o drv/pci_x.o drv/blockdev.o fs/vfs.o
	
LD=ld

MKISO=c:/LikeOS/src/boot/mkiso.bat

ASSEMBLER=./nasm.exe

all:	kernel
$(SRCS):
		$(GET) $@
# To make an object from source
		$(CC) $(CFLAGS) -c $*.c

kernel: $(OBJS)
		$(ASSEMBLER) -f elf krnl/kernel_start.asm -o build/ks.o		
		$(LD) -o build/$@.bin build/ks.o $(OBJS) $(LDFLAGS)
		$(MKISO)
#		rm $(OBJS)
#		rm build/ks.o
		
# To install things in the right place
install: kernel
		$(MKISO)
clean: $(OBJS)
		rm build/ks.o
		rm $(OBJS)
