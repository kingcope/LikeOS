#include "defs.h"
#include "multiboot.h"
#include "vesa.h"
#include "kalloc.h"
#include "string.h"
#include "isofs.h"
#include "font.h"
#include "mouse.h"
#include <stdio.h>

unsigned char fontr=0;
unsigned char fontg=0;
unsigned char fontb=0;
char *inbuf;

void text_update() {
	unsigned int x,x2,y2;
	unsigned long *screen = getlfb();
	
	for (x=0; x<SCREENWIDTH*SCREENHEIGHT; x++) {
			putp(x, 0x555555);
	}
	
	x2 = 10;
	y2 = 10;
	for (x=0;x<strlen(inbuf);x++) {
		if ((inbuf[x] == '\r') || (inbuf[x] == '\n')) {
			x++;
			if (inbuf[x] == '\n') x++;
			y2+=15;
			x2=10;	
		}
		putcharc(inbuf[x], x2, y2, fontr, fontg, fontb, screen);
		x2+=FONT_WIDTH;
	}
}

void text_init() {
	inbuf = (char*)kalloc(65535);
	
	inbuf[0]=0;
	strcat(inbuf,"Welcome to LikeOS\nCopyright (C) 2005,2006 Nikolaos Rangos\n#");
	text_update();	
}

void t_dispatch_keyboardevents(char *s) {
	strcat(inbuf, s);
	text_update();
}
