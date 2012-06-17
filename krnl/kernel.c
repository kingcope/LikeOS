/*
$$
$$ LikeOS
$$ Copyright (C) 2005-2007 Nikolaos Rangos
$$
$$
*/

#include <defs.h>
#include <string.h>
#include <paging.h>
#include <physalloc.h>
#include <stdio.h>
#include <svgagui.h>
#include <kalloc.h>
#include <multiboot.h>
#include <vesa.h>
#include <interrupts.h>
#include <atapi.h>
#include <fat32.h>
#include <mouse.h>
#include <etherboot.h>
#include <dev.h>
#include <nic.h>
#include <lwip/debug.h>
#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/ip.h>
#include <lwip/udp.h>
#include <lwip/icmp.h>
#include <lwip/tcp.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>
#include <netif/etharp.h>
#include <netif/paulosif.h>
#include <netif/validate.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <isofs.h>
#include <desq.h>

#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/icmp.h"
#include "lwip/tcp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"

void k_clear_screen();
#define k_printf
//UINT k_printf(char *message, UINT line);

extern void tcpip_mainloop();
extern void tcp_timer_callback();

void kernelthread() {
	
}

extern void interrupt_14(ULONG cr2);

char * ___strtok;

char * strtok(char * s,const char * ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___strtok;
	if (!sbegin) {
		return NULL;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___strtok = NULL;
		return( NULL );
	}
	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___strtok = send;
	return (sbegin);
}

void error(char *s) {
/*donothing*/	
}

void usleep(int time) {
/*	int i=0;
	
	for (i=0;i<time*10000;i++) {
		for (i=0;i<time*10000;i++) {
			asm("nop");
		}
	}*/
}

#define	N	256

void bzero(void *a, size_t c) {memset(a, 0, c);}

double
fabs(double arg)
{

	if(arg < 0)
		return -arg;
	return arg;
}

void setup_fault_handlers() {
	int i;
	vector_t v;
	
	v.eip = (unsigned)intr_fault_divide;
	v.access_byte = 0x8E;
	setvect(&v, 0);

	v.eip = (unsigned)intr_fault_bound;
	v.access_byte = 0x8E;
	setvect(&v, 5);
	
	v.eip = (unsigned)intr_fault_opcode;
	v.access_byte = 0x8E;
	setvect(&v, 6);

	v.eip = (unsigned)intr_fault_tss;
	v.access_byte = 0x8E;
	setvect(&v, 0x0A);

	v.eip = (unsigned)intr_fault_segment;
	v.access_byte = 0x8E;
	setvect(&v, 0x0B);

	v.eip = (unsigned)intr_fault_stack;
	v.access_byte = 0x8E;
	setvect(&v, 0x0C);

	v.eip = (unsigned)intr_fault_gpf;
	v.access_byte = 0x8E;
	setvect(&v, 0x0D);

	v.eip = (unsigned)intr_fault_pf;
	v.access_byte = 0x8E;
	setvect(&v, 0x0E);
}

in_addr_t
inet_addr(const char *cp)
{
        struct in_addr val;

        if (inet_aton(cp, &val))
                return (val.s_addr);
        return (-1);
}

void portscan() {
        struct tcp_pcb *pcbS;
        struct ip_addr ipa;
        unsigned int k=31337;
        err_t err;
		return;
//        for (k=1;k<2;k++){
        		pcbS = tcp_new();		
				pcbS->local_port = 31337;
	        	
                ipa.addr = inet_addr("192.168.2.1");
    	        err = tcp_connect(pcbS, &ipa, 80, NULL);
	            if (err == ERR_OK) {
		         	// PORT OPEN!
	            }
	            tcp_close(pcbS);
//		}       
	
}

void k_main(multiboot_info_t *boot_info)
{	
	ULONG i,x;
	vector_t v;
	
	initialize_pics();
	v.eip = (unsigned)interrupt_timer;
	v.access_byte = 0x8E;
	setvect(&v, 0x20);
	
	v.eip = (unsigned)interrupt_keyboard;
	v.access_byte = 0x8E;
	setvect(&v, 0x21);	
/*		
	v.eip = (unsigned)interrupt_ata;
	v.access_byte = 0x8E;
	setvect(&v, 0x2E);	// IRQ14 fuer den ATA/ATAPI Driver	
	
	v.eip = (unsigned)interrupt_ata2;
	v.access_byte = 0x8E;
	setvect(&v, 0x2F);	// IRQ15 fuer den ATA/ATAPI Driver, Secondary Controller
*/
	setup_fault_handlers();
				
	for (i=0x400000; i<0xa000000; i+=PAGE_SIZE) {
		phys_free(i);
	}

	init_lfb(boot_info);
	for (x=0; x < SCREENWIDTH * SCREENHEIGHT; x++) {
			putp(x, 0x608189);
	}

	map_all();
	enable_paging();
	kinit();
	
	asm("sti");	
	
	// setup the virtual file system (vfs uses isofs)
	setup_timers();
	
	int found=0,k=0;
	for (k=0;k<9;k++) {
		int reopen=0;
		
		if (devopen(k, &reopen) != 1) continue;
		if (mount_fs() != 1) continue;
		else {
			// Check signature file existance so we know
			// this is the LikeOS Disk
			if (file_open("/likeos.sig") != 1) continue;
			else {
				found=1;
				break;}
		} 
	}

	if (found != 1) panic1("Could not read from device");

	font_init();
	
	init_multitasking();
	asm("cli");
	int hz = 200;
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
	asm("sti");	
	
	ProcessCreate(kernelthread,"KERNEL");
			
	v.eip = (unsigned) interrupt_mouse;	// IRQ12 Mouse PS/2
	v.access_byte = 0x8E;
	setvect(&v, 0x2C);
	mouse_init();	
	
    ProcessCreate(run_svgagui,"User Interface");  
    
/*	if (like_eth_probe() == PROBE_WORKED) {
		like_tcpip_start();
		ProcessCreate(tcpip_mainloop, "INET");
	}*/
};
