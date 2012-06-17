#ifndef __DRAW_H
#define __DRAW_H

#include "defs.h"
#include "multiboot.h"
#include "vesa.h"
#include "kalloc.h"
#include "string.h"
#include "isocd.h"
#include "font.h"
#include "mouse.h"

#define OUTPUT_FRONT 0
#define OUTPUT_BACK 1

extern unsigned long *offscreen;

class Color {
	public:
	
	Color(unsigned char R, unsigned char G, unsigned char B) {
		r=R;
		g=G;
		b=B;	
	}
	
	unsigned char getR() {
		return r;	
	}
	
	unsigned char getG() {
		return g;	
	}
	
	unsigned char getB() {
		return b;	
	}
	
	void setR(unsigned char R) {
		r=R;
	}
	
	void setG(unsigned char G) {
		g=G;	
	}
	
	void setB(unsigned char B) {
		b=B;	
	}
	
	private:
	 unsigned char r,g,b;	
};

class Draw {
	public:
	 Draw() {
		SetOutputBuffer(OUTPUT_BACK);
	 }
	
	 void SetOutputBuffer(unsigned char outputbuffer) {
		 OutputBuffer = outputbuffer;
		 
		 if (OutputBuffer = OUTPUT_BACK)
		 	xbuffer = offscreen;
		 else 
		 	xbuffer = getlfb();
	 }
	
	 inline void DrawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
		 xbuffer[SCREENWIDTH*y+SCREENHEIGHT] = (r<<16) | (g<<8) | b;
	 }
	
	 void DrawLine(int fromx, int tox, int y, Color *col) {
		 int x;
		 
		 for (x=fromx;x<tox;x++) {
			DrawPixel(x,y,col->getR(),col->getG(),col->getB());
		 }
	 }
	 
	 private:
	 unsigned char OutputBuffer;
	 unsigned long *xbuffer;
};

#endif
