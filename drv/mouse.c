/*
**	PS/2 Mouse Driver
**	Copyright (C) 2005-2006 Nikolaos Rangos
*/

#include "defs.h"
#include "atapi.h"
#include "fat32.h"

#define MOUSE_COMMAND 0x60
#define MOUSE_COMMAND2 0x64
#define MOUSE_DATA 0x64
#define MOUSE_COMMAND_STREAMMODE 0xea
#define MOUSE_COMMAND_ACTIVATE 0xf4
#define MOUSE_COMMAND_SETSTANDART 0xf6
#define MOUSE_COMMAND_ZUSATZEINHEIT 0xd4
#define MOUSE_COMMAND_ENABLE_INTERRUPT 0x01
#define MOUSE_SIZEX 12
#define MOUSE_SIZEY 21

#undef VIRTUALMACHINE
#define VIRTUALMACHINE

int mouse_x=0;
int mouse_y=0;
int width=SCREENWIDTH;
int height=SCREENHEIGHT;

unsigned long *mouseback;
unsigned char *mousebuffer;

char left_button_state=0;
int oldx,oldy,curx,cury;
int first=0;

void mouse_draw_mousepointer(int x2, int y2) {
	unsigned char r,g,b;
	int i,x,y;
	i=0;
	
	for (y=0; y<MOUSE_SIZEY; y++) {
		i=MOUSE_SIZEX*y*3;
		for (x=0; x<MOUSE_SIZEX; x++) {
			r=mousebuffer[i];
			g=mousebuffer[i+1];
			b=mousebuffer[i+2];
			i+=3;
			
			mouseback[MOUSE_SIZEX*y+x] = getp(SCREENWIDTH*(y+y2)+(x+x2));
			if ((r != 121) && (g != 27) && (b != 27))	{		
				pp(SCREENWIDTH*(y+y2)+(x+x2), r, g, b);
			}
		}
	}
}

void mouse_draw_mousepointerwoxy() {
	mouse_draw_mousepointer(curx, cury);
}

void mouse_draw_mouseback(int x2, int y2) {
	unsigned char r,g,b;
	int i,x,y;
	i=0;
	
	for (y=0; y<MOUSE_SIZEY; y++) {
		i=MOUSE_SIZEX*y*3;
		for (x=0; x<MOUSE_SIZEX; x++) {
			putp(SCREENWIDTH*(y+y2)+(x+x2), mouseback[MOUSE_SIZEX*y+x]);
		}
	}
}

int mouse_x2, mouse_y2;

void mouse_update_cursor(short x_movement,
						 short y_movement,
						 char x_negative,
						 char y_negative) {
				 
	/*if (x_negative == 1) {
		x_movement -= 256;
	}

	if (y_negative == 1) {
		y_movement -= 256;
	}*/
	
	if (((mouse_x + x_movement) < (width-MOUSE_SIZEX))	&& ((mouse_y - y_movement) < height)
	   && ((mouse_x + x_movement) > 0) && ((mouse_y - y_movement) > 0)) {		    
			mouse_x += x_movement;
			mouse_y -= y_movement;
						
			mouse_draw_mouseback(mouse_x2, mouse_y2);
			mouse_draw_mousepointer(mouse_x, mouse_y);	
	}
}

unsigned char mouse_cycle=0;    //unsigned char
char mouse_byte[4];    			//signed char
void mouse_handler(char mouse_data, char x_movement, char y_movement);
char mouse_haswheel=0;		// zero indicates mouse has no wheel

char mousemoves=0;

void interrupt_mouse()
{
  asm("cli");
  static unsigned char mousecount=0;  
  mousemoves=0;
	if ((inportb(0x64) & 0x01) != 0) {
    	mouse_byte[mousecount++]=inportb(0x60);
    	if (mousecount >= 3+mouse_haswheel) {
	    	mousecount=0;
	    	mousemoves=1;
#ifndef VIRTUALMACHINE
	    		mouse_handler(mouse_byte[0], mouse_byte[1], mouse_byte[2]);
#else
	    		mouse_handler(mouse_byte[2], mouse_byte[0], mouse_byte[1]);
#endif
    	}
	}
	
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
  asm("sti");
}

int mouse_getx() {
	return mouse_x2;	
}

int mouse_gety() {
	return mouse_y2;
}

char left_button=0;
char right_button=0;
char middle_button=0;
char oleftbutton=0;

int mouse_getbutton() {
	
	if (left_button == 0) {
		return 0;
	}	
	
	if (left_button==1) {
		return 1;
	}
		
	if (right_button == 1) {
		right_button = 0;
		return 2;	
	}
	
	return 3;
}

void mouse_setposition(int x, int y) {
	mouse_x2=curx=x;
	mouse_y2=cury=y;
}

int mouse_update() {
	return !mousemoves;
}

char wasdrag=0;
void mouse_handler(char mouse_data, char x_movement, char y_movement) {
//	char mouse_data;
	char x_negative;
	char y_negative;
//	unsigned char status;
//	unsigned char x_movement; // byte two
//	unsigned char y_movement; // byte three
	//if ((inportb(0x64) & 0x21) != 0x21) return;	// check if output buffer full
	
	x_negative=0;
	y_negative=0;
	
	left_button = 0;
	right_button = 0;
	if ((mouse_data & 0x01) == 0x01) left_button=1;
	if ((mouse_data & 0x02) == 0x02) right_button=1;
	if ((mouse_data & 0x10)) x_negative=1;
	if ((mouse_data & 0x20)) y_negative=1;

	if (((mouse_x + x_movement) < (width-MOUSE_SIZEX))	&& ((mouse_y - y_movement) < height)
	   && ((mouse_x + x_movement) > 0) && ((mouse_y - y_movement) > 0)) {		    
		
		mouse_x += x_movement;
		mouse_y -= y_movement;

		mouse_x2=curx=mouse_x;
		mouse_y2=cury=mouse_y;

	}		
	//dispatch_mouseevents(mouse_x2, mouse_y2, left_button);
	//mouse_update_cursor(x_movement, y_movement, x_negative, y_negative);	
}
/*
void mouse_waitdata() {
	while ((inportb(MOUSE_DATA) & 0x02) == 0x02);
}
*/

/*void mouse_mousepointer_init() {	
	int fd = file_open("/MOUSE.RAW");
	if (fd != 1) {
		panic1("Could not read from device");	
	}
	mousebuffer=(unsigned char*) kalloc(file_size());
	file_read(mousebuffer, file_size);
	file_close();
}*/

inline void mouse_wait(unsigned char a_type) //unsigned char
{
  unsigned int _time_out=100000; //unsigned int
  if(a_type==0)
  {
    while(_time_out--) //Data
    {
      if((inportb(0x64) & 1)==1)
      {
        return;
      }
    }
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inportb(0x64) & 2)==0)
      {
        return;
      }
    }
    return;
  }
}

inline void mouse_write(unsigned char a_write) //unsigned char
{
  //Wait to be able to send a command
  mouse_wait(1);
  //Tell the mouse we are sending a command
  outportb(0x64, 0xD4);
  //Wait for the final part
  mouse_wait(1);
  //Finally write
  outportb(0x60, a_write);
}

unsigned char mouse_read()
{
  //Get's response from mouse
  mouse_wait(0);
  return inportb(0x60);
}

void mouse_init() {
	int x,y;
	unsigned char status;
	/*
	mouseback=(unsigned long*) kalloc(MOUSE_SIZEX*MOUSE_SIZEY*sizeof(unsigned long));	
	for (y=0; y<MOUSE_SIZEY; y++) {
		for (x=0; x<MOUSE_SIZEX; x++) {
			mouseback[MOUSE_SIZEX*y+x] = getp(SCREENWIDTH*(y+mouse_y)+(x+mouse_x));
		}
	}
*/
 	//Enable the auxiliary mouse device
  	mouse_wait(1);
  	outportb(0x64, 0xA8);
 
  	//Enable the interrupts
  	mouse_wait(1);
  	outportb(0x64, 0x20);
  	mouse_wait(0);
  	status=(inportb(0x60) | 2);
  	mouse_wait(1);
  	outportb(0x64, 0x60);
  	mouse_wait(1);
  	outportb(0x60, status);
 
  	//Tell the mouse to use default settings
  	mouse_write(0xF6);
	mouse_read();  //Acknowledge
 
  	//Enable the mouse
  	mouse_write(0xF4);
  	mouse_read();  //Acknowledge	

	//mouse_mousepointer_init();	
}
