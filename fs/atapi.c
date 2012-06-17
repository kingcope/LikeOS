/*
**	ATA/ATAPI Driver, read only, no error checking
*/

/* 
** IDE Kommando und Steuerregister 
*/
#define REG_DATEN 0x1f0
#define REG2_DATEN 0x170
#define REG_FEHLER 0x1f1
#define REG2_FEHLER 0x171
#define REG_FEATURE 0x1f1
#define REG2_FEATURE 0x171
#define REG_SEKTORENZAHL 0x1f2
#define REG2_SEKTORENZAHL 0x172
#define REG_SEKTORNUMMER 0x1f3
#define REG2_SEKTORNUMMER 0x173
#define REG_ZYLINDER_LSB 0x1f4
#define REG2_ZYLINDER_LSB 0x174
#define REG_ZYLINDER_MSB 0x1f5
#define REG2_ZYLINDER_MSB 0x175
#define REG_LAUFWERK_KOPF 0x1f6
#define REG2_LAUFWERK_KOPF 0x176
#define REG_STATUS 0x1f7
#define REG2_STATUS 0x177
#define REG_BEFEHL 0x1f7
#define REG2_BEFEHL 0x177
#define REG_STATUS2	0x3f6
#define REG_DOR	0x3f6
#define REG_LAUFWERKSADDR 0x3f7
#define CMD_SEKTORENLESEN 0x20
#define CMD_IDENTIFY 0xEC
#define CMD_ATAPI_IDENTIFY 0xa1
#define CMD_ATAPI_READ 0x28
#define CMD_ATAPI_PACKET 0xa0

#include "atapi.h"
#include "interrupts.h"
#include "defs.h"
#include "multiboot.h"
#include <kalloc.h>
#include "vesa.h"
#include "font.h"

volatile int IRQ14_called;
volatile int IRQ15_called;
unsigned short sectors_per_track[]={0, 0};
unsigned short numheads[]={0, 0};

/*
$$ Primary Controller, controller=1
$$ Secondary Controller, controller=2
$$ MASTER, master=1
$$ SLV, master=0
*/
volatile char controller=1;
volatile char master=0;
/*$$*/

void interrupt_ata() {
	asm("cli");
	IRQ14_called=1;
	outportb(0x20, 0x20);
	outportb(0xa0, 0x20);
	asm("sti");
}

void interrupt_ata2() {
	asm("cli");
	IRQ15_called=1;
	outportb(0x20, 0x20);
	outportb(0xa0, 0x20);
	asm("sti");	
}

void ata_waitbsy() {
	if (controller==1)
		while ((inportb(REG_STATUS) & 0x80) == 0x80);
	else 
		while ((inportb(REG2_STATUS) & 0x80) == 0x80);
}

void ata_waitbsy_primary() {
	while ((inportb(REG_STATUS) & 0x80) == 0x80);
}

void ata_waitbsy_secondary() {
	while ((inportb(REG2_STATUS) & 0x80) == 0x80);
}

void ata_waitbsydrq() {
	if (controller==1)
		while ((inportb(REG_STATUS) & 0x80 == 0x80 
	      	|| inportb(REG_STATUS) & 0x08) != 0x08);
	else
		while ((inportb(REG2_STATUS) & 0x80 == 0x80 
	      	|| inportb(REG2_STATUS) & 0x08) != 0x08);	
}

void ata_read_pio_sector(unsigned short *in_buffer) {
	int i;
	unsigned short port;
	
	if (controller==1) port = REG_DATEN;
		else port = REG2_DATEN;
	
	for (i=0; i<256; i++, in_buffer++) {
		*in_buffer=inportw(port);
	}
}

void ata_lba2chs(unsigned int lba,
				 unsigned int *cylinder,
				 unsigned int *head,
				 unsigned int *sector,
				 unsigned char drive) {
	*cylinder = (lba/sectors_per_track[drive]) / numheads[drive];
	*head = (lba/sectors_per_track[drive]) % numheads[drive];
	*sector = (lba % sectors_per_track[drive]) + 1;	
}

void ata_read_pio_identify(struct __ata_info *ata_info) {
	unsigned short i;
	unsigned short *ata_info_pointer;
	unsigned short port;
	
	if (controller==1) port = REG_DATEN;
		else port = REG2_DATEN;
	
	ata_info_pointer=(unsigned short*)ata_info;
	
	for (i=0; i < 256; i++, *ata_info_pointer++) {
		*ata_info_pointer=inportw(port);
	}
}

void ata_identify() {
	unsigned char Laufwerk;
	
	if (master == 0) Laufwerk=0xb0;
	if (master == 1) Laufwerk=0xa0;
	
	ata_info=(struct __ata_info*) kalloc(sizeof(struct __ata_info));
	ata_waitbsy();

	IRQ14_called=0;
	IRQ15_called=0;

	switch (controller) {
		case 1:	
			outportb(REG_LAUFWERK_KOPF, Laufwerk);
			outportb(REG_BEFEHL, CMD_ATAPI_IDENTIFY);
			while (IRQ14_called != 1);
			break;
		case 2:
			outportb(REG2_LAUFWERK_KOPF, Laufwerk);
			outportb(REG2_BEFEHL, CMD_ATAPI_IDENTIFY);
			while (IRQ15_called != 1);
			break;
	}
		
	ata_read_pio_identify(ata_info);

	sectors_per_track[0] = ata_info->number_of_logical_sectors_per_track;
	numheads[0] = ata_info->number_of_logical_heads;
}

void WaitABit() {
	unsigned long wait;
	
	for (wait=0; wait < 40000000; wait++) {
			asm("nop");	
	}
}

void ata_setcdrom(unsigned long boot_device) {
	static char called=0;
	
	controller = 1;
	master = 0;
	char tst[254];
	
		switch (boot_device) {
		case 0x80:	{controller = 1; master = 1; 	break;}
		case 0x81:	{controller = 1; master = 0;	break;}
		case 0x82:	{controller = 2; master = 1;	break;}
		case 0x83:	{controller = 2; master = 0;	break;}
		default:	{ controller = 2; master = 1; break;}
		}
	
	controller = 1; master = 1;
		
	if (called == 1) {
	
	sprintf(tst, "%d", boot_device);
	
	putstring(tst, 100, 100, getlfb());
	}
	called = 1;
}

/*
$$
$$ XXX ata_readsectors is for Primary Controller
$$
*/
void ata_readsectors(unsigned char drive, 
					 unsigned int lba,
					 unsigned char num_sectors, 
					 unsigned char *in_buffer
					) {
	unsigned short *sector_buffer_pointer;
	unsigned int cylinder, head, sector;
	unsigned char Laufwerk=(drive==0 ? 0xa0:0xb0);
	unsigned int i;
	unsigned char d;

	if ((sectors_per_track[drive]==0) || (numheads[drive]==0)) {
		ata_identify();
	}	

	ata_lba2chs(lba, &cylinder, &head, &sector, drive);

	sector_buffer_pointer=(unsigned short*) in_buffer;

	ata_waitbsy();
	
	IRQ14_called=0;

	outportb(REG_SEKTORENZAHL, num_sectors);		// Anzahl der zu lesenden Sektoren
	outportb(REG_SEKTORNUMMER, sector);				// Sektornummer
	outportb(REG_ZYLINDER_LSB, cylinder & 0xff);	// Zylinder LSB		LSB+MSB=10Bit
	outportb(REG_ZYLINDER_MSB, cylinder & 0x300);	// Zylinder MSB
	outportb(REG_LAUFWERK_KOPF, Laufwerk+head);		// Laufwerk/Kopf, enthält den Kopf und das Laufwerk
	outportb(REG_BEFEHL, CMD_SEKTORENLESEN);

	while (IRQ14_called != 1);

	for (i=0; i<num_sectors; i++) {
		ata_read_pio_sector(sector_buffer_pointer);
		sector_buffer_pointer+=256;
	}

	ata_waitbsy();
}

void atapi_readsectors(unsigned int sector,
					   unsigned char *in_buffer,
					   unsigned int buflen
					   ) {
	unsigned int i;
	unsigned char readpacket[12];
	unsigned short *readpkt;
	unsigned short *inbufferp;
	unsigned int readlen;
	unsigned int n;
	unsigned int l;
	unsigned char drive = (master==0) ? 0xb0 : 0xa0;
	unsigned short int regdata;
	regdata = (controller==1) ? REG_DATEN : REG2_DATEN;
	
	l=1;
	n=sector;
	readpacket[0]=CMD_ATAPI_READ;
	readpacket[1]=0;
	readpacket[2]=n >> 24;
	readpacket[3]=(n >> 16) & 0xFF;
	readpacket[4]=(n >> 8) & 0xFF;
	readpacket[5]=n & 0xFF;
	readpacket[6]=0;
	readpacket[7]=(l >> 8) & 0xFF;
	readpacket[8]=l & 0xFF;
	readpacket[9]=0;
	readpacket[10]=0;
	readpacket[11]=0;

	IRQ14_called=0;
	IRQ15_called=0;
	
	ata_waitbsy();

	if (controller==1) {
		outportb(REG_FEHLER, 0);
		outportb(REG_SEKTORENZAHL, 0);
		outportb(REG_SEKTORNUMMER, 0);
		outportb(REG_ZYLINDER_LSB, (unsigned char) (buflen & 0xFF));
		outportb(REG_ZYLINDER_MSB, (unsigned char) (buflen >> 8));
		outportb(REG_LAUFWERK_KOPF, drive);
		outportb(REG_BEFEHL, CMD_ATAPI_PACKET);
	} else {
		outportb(REG2_FEHLER, 0);
		outportb(REG2_SEKTORENZAHL, 0);
		outportb(REG2_SEKTORNUMMER, 0);
		outportb(REG2_ZYLINDER_LSB, (unsigned char) (buflen & 0xFF));
		outportb(REG2_ZYLINDER_MSB, (unsigned char) (buflen >> 8));
		outportb(REG2_LAUFWERK_KOPF, drive);
		outportb(REG2_BEFEHL, CMD_ATAPI_PACKET);		
	}
	ata_waitbsy();
	readpkt = (unsigned short*)readpacket;
	for (i=0;i<6;i++) {
		outportw(regdata,*readpkt);
		readpkt++;
	}
	if (controller==1)
		while (IRQ14_called != 1);
	else {
		while (IRQ15_called != 1);	
	}
	ata_waitbsy();

	inbufferp=(unsigned short*)in_buffer;
	for (i=0; i<buflen/2; i++, inbufferp++) {
		ata_waitbsy();		
		*inbufferp=inportw(regdata);
	}
		
	for (i=buflen/2; i<=2048/2; i++) {
		ata_waitbsy();
		inportw(regdata);
	}

	ata_waitbsy();
}

/*$$$*/