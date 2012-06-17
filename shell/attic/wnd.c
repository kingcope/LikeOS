/*
	LikeOS Windowing System
	Copyright(c) 2005 Nikolaos Rangos
*/

#include "atapi.h"
#include "fat32.h"
#include "wnd.h"
#include "defs.h"
#include "txtCtrl.h"

unsigned char *wndtop;
unsigned char *wndbtm;
unsigned char *wndleft;
unsigned char *wndright;
unsigned char *wndlt;
unsigned char *wndrt;
unsigned char *wndrb;
unsigned char *wndlb;
unsigned char *wndctl;
unsigned char *iterm; // terminal icon
unsigned char *wallpaper;
unsigned short window_handle_counter=0;

unsigned long *offscreen;	// double buffer
unsigned long *backrect;	// rect back buffer
int num_windows=0;

unsigned long *getOffscreenBuffer() {
	return (unsigned long*)offscreen;
}

void gui_init(unsigned char *wallpaper_ptr) {
	wallpaper = wallpaper_ptr;
	
	wndtop=(unsigned char*) kalloc(cdiso_getfilesize("WNDTOP.RAW"));
	cdiso_readfile("WNDTOP.RAW", wndtop);

	wndbtm=(unsigned char*) kalloc(cdiso_getfilesize("WNDBTM.RAW"));
	cdiso_readfile("WNDBTM.RAW", wndbtm);
	
	wndleft=(unsigned char*) kalloc(cdiso_getfilesize("WNDLEFT.RAW"));
	cdiso_readfile("WNDLEFT.RAW", wndleft);
	
	wndright=(unsigned char*) kalloc(cdiso_getfilesize("WNDRIGHT.RAW"));
	cdiso_readfile("WNDRIGHT.RAW", wndright);
	
	wndlt=(unsigned char*) kalloc(cdiso_getfilesize("WNDLT.RAW"));
	cdiso_readfile("WNDLT.RAW", wndlt);
		
	wndrt=(unsigned char*) kalloc(cdiso_getfilesize("WNDRT.RAW"));
	cdiso_readfile("WNDRT.RAW", wndrt);
		
	wndlb=(unsigned char*) kalloc(cdiso_getfilesize("WNDLB.RAW"));
	cdiso_readfile("WNDLB.RAW", wndlb);
		
	wndrb=(unsigned char*) kalloc(cdiso_getfilesize("WNDRB.RAW"));
	cdiso_readfile("WNDRB.RAW", wndrb);
	
	wndctl=(unsigned char*) kalloc(cdiso_getfilesize("WNDCTL.RAW"));
	cdiso_readfile("WNDCTL.RAW", wndctl);
	
	iterm=(unsigned char*) kalloc(cdiso_getfilesize("ITERM.RAW"));
	cdiso_readfile("ITERM.RAW", iterm);
	
	/* root window, invisible */
	wnd_head=(struct WND*)kalloc(sizeof(struct WND));
	wnd_head->prev=NULL;
	wnd_head->next=NULL;
	wnd_head->rect.x = 0;
	wnd_head->rect.y = 0;
	wnd_head->rect.width = 0;
	wnd_head->rect.height = 0;
	wnd_head->caption = 0;
	wnd_head->needs_repaint = FALSE;
	wnd_head->prev_ctrl=NULL;
	wnd_head->next_ctrl=NULL;
	wnd_head->first_ctrl=NULL;
	wnd_head->control_struct=NULL;
	
	offscreen=(unsigned long*) kalloc(1024*768*3);
	//backrect=(unsigned long*) kalloc(10);
	//backrect=(unsigned long*)kalloc(((search->rect.width*2)+(search->rect.height*2))*3);
	backrect=(unsigned long*)kalloc(1024*768*3);	
	init_bar();	
}

void DrawToken(unsigned char *buffer, unsigned long *target, int x2, int y2, int width, int height) {
	unsigned char r,g,b;
	int i,x,y;
	i=0;

	for (y=0; y<height; y++) {
		if ((y+y2>768) || (y+y2<0)) continue;
		i=width*y*3;
		for (x=0; x<width; x++) {
			if ((x+x2>1024) || (x+x2<0)) continue;
			r=buffer[i];
			g=buffer[i+1];
			b=buffer[i+2];
			i+=3;
						
			target[1024*(y+y2)+(x+x2)] = (r<<16) | (g<<8) | b;
		}
	}	
}

void DrawWindow(int x, int y, int width, int height) {
	int i,x2,y2;
	unsigned char r,g,b;
	char first=0;
	unsigned long col=(31<<16) | (42<<8) | 49;
	
	for (y2=0; y2<height; y2++) {
		if ((y+y2>768) || (y+y2<0)) continue;
		for (x2=0; x2<width; x2++) {
			if ((x+x2>1024) || (x+x2<0)) continue;
			offscreen[1024*(y2+y)+(x2+x)] = col;
		}
	}	
	
	for (i=0; i<(width+WNDRIGHTX)/WNDTOPX; i++) {
		DrawToken(wndtop, offscreen, x+i*WNDTOPX, y, WNDTOPX, WNDTOPY);
	}
	
	for (i=0; i<(width+WNDRIGHTX)/WNDBTMX; i++) {
		DrawToken(wndbtm, offscreen, x+i*WNDBTMX, y+height-WNDBTMY, WNDBTMX, WNDBTMY);	
	}
	
	for (i=0; i<height/WNDLEFTY; i++) {
		DrawToken(wndleft, offscreen, x, y+i*WNDLEFTY, WNDLEFTX, WNDLEFTY);	
	}
	
	for (i=0; i<height/WNDRIGHTY; i++) {
		DrawToken(wndright, offscreen, x+width, y+i*WNDRIGHTY, WNDRIGHTX, WNDRIGHTY);	
	}
	
	DrawToken(wndlt, offscreen, x, y, WNDLTX, WNDLTY);
	DrawToken(wndrt, offscreen, x+width-WNDRTX+WNDRIGHTX, y, WNDRTX, WNDRTY);

	DrawToken(wndlb, offscreen, x, y+height-WNDLBY, WNDLBX, WNDLBY);
	DrawToken(wndrb, offscreen, x+width-WNDRBX+WNDRIGHTX, y+height-WNDRBY, WNDRBX, WNDRBY);
	DrawToken(wndctl, offscreen, x+width-WNDCTLX, y+3, WNDCTLX, WNDCTLY);
	DrawToken(iterm, offscreen, x+3, y+3, ITERMX, ITERMY);
}

struct WND *wnd_append(struct WND *wnd_append) {
	struct WND *search;
	search=wnd_head;
	wnd_append->next=NULL;
	
	while (search->next != NULL) {
		search=search->next;
	}

	search->next=wnd_append;
	wnd_append->prev=search;

	return wnd_append;
}

void update() {
	unsigned char r,g,b;
	unsigned long i,x;
	i=0;
	
	for (x=0; x<1024*768; x++) {
			r=wallpaper[i];
			g=wallpaper[i+1];
			b=wallpaper[i+2];
			i+=3;
			
			offscreen[x] = (r<<16) | (g<<8) | b;
	}	
	
	struct WND *search;
	struct WND *ctrl;
	
	search=wnd_head;
	do {
		search=search->next;
		DrawWindow(search->rect.x, search->rect.y, search->rect.width, search->rect.height);
		putstring(search->caption, search->rect.x+25, search->rect.y+3, offscreen);
		
		if (search->first_ctrl != NULL) {
			ctrl=search->first_ctrl;
			while(1) {
				void (*control_update_function)(struct WND *ctrl,
											    struct WND *window,
											    unsigned char uselfb,
											    unsigned fromKbdDriver);

				control_update_function = ctrl->control_update_function;
				control_update_function(ctrl, search, 0, 0);
				
				if (ctrl->next_ctrl == NULL) break;
				ctrl=ctrl->next_ctrl;	
			}
		}
		
	} while (search->next != NULL);
	
	lfb_update(offscreen);
}

void WNDSystem_SetTopWindow(int handle) {
	struct WND *search, *search2;
	struct WND *this_window;
	
	search=wnd_head;
	
	// find window, delete in chain and append at the end of chain
	do {
		search=search->next;		
		if (search->handle == handle) {
			this_window = search;
			search2 = search->prev;
			search2->next = search->next;
			search2 = search->next;
			search2->prev = search->prev;
			break;
		}
	} while (search->next != NULL);	

	wnd_append(this_window);

	search=wnd_head;
	do {
		search=search->next;
		search->is_topmost_window=0;
		search->active_window=0;		
	} while (search->next != NULL);	
		
	this_window->is_topmost_window=1;
	this_window->active_window=1;
}

struct WND *NewWindow(char *caption, int x, int y, int width, int height) {
	struct WND *newwindow=(struct WND*) kalloc(sizeof(struct WND));	
	
	newwindow->rect.x = x;
	newwindow->rect.y = y;
	newwindow->rect.width = width;
	newwindow->rect.height = height;
	newwindow->caption = caption;
	newwindow->needs_repaint = FALSE;
	newwindow->handle = ++window_handle_counter;
	newwindow->prev_ctrl=NULL;
	newwindow->next_ctrl=NULL;
	newwindow->first_ctrl=NULL;
	
	num_windows++;
		
	wnd_append(newwindow);
	WNDSystem_SetTopWindow(newwindow->handle);

	return newwindow;
}

int WNDSystem_Windowtop_Clicked(struct WND *wnd, int x, int y) {
	if ((x >= wnd->rect.x) && (x <= wnd->rect.x + wnd->rect.width) &&
	    (y <= (wnd->rect.y+WNDTOPY)) && (y >= wnd->rect.y)) {
		return TRUE;
	}
	
	return FALSE;
}

int WNDSystem_Window_Clicked(struct WND *wnd, int x, int y) {
	if ((x >= wnd->rect.x) && (x <= wnd->rect.x + wnd->rect.width) &&
	    (y <= (wnd->rect.y+wnd->rect.height)) && (y >= wnd->rect.y)) {
		return TRUE;
	}	

	return FALSE;	
}

int WNDSystem_Windowresize_Clicked(struct WND *wnd, int x, int y) {
	
	if ( (x >= wnd->rect.x + (wnd->rect.width-15))
	   &&(x <= wnd->rect.x + wnd->rect.width)
	   &&(y <= wnd->rect.y + (wnd->rect.height))
	   &&(y >= wnd->rect.y + (wnd->rect.height-15)) ) {
			return TRUE;  
	   }
	
	return FALSE;	
}

void SaveRect(int x, int y, int width, int height) {
	int x2,y2;

	for (x2=0; x2<width; x2++)
		backrect[x2] = getp(1024*y+(x+x2));
		
	for (x2=0; x2<width; x2++)
		backrect[width+x2] = getp(1024*(y+height)+(x+x2));
		
	for (y2=0; y2<height; y2++)
		backrect[width*2+y2] = getp(1024*(y+y2)+x);
		
	for (y2=0; y2<height; y2++)
		backrect[width*3+y2] = getp(1024*(y+y2)+(x+width));	
}

void DrawBackRect(int x, int y, int width, int height) {
	int x2,y2;

	for (x2=0; x2<width; x2++)
		putp(1024*y+(x+x2), backrect[x2]);
		
	for (x2=0; x2<width; x2++)
		putp(1024*(y+height)+(x+x2), backrect[width+x2]);
		
	for (y2=0; y2<height; y2++)
		putp(1024*(y+y2)+x, backrect[width*2+y2]);
		
	for (y2=0; y2<height; y2++)
		putp(1024*(y+y2)+(x+width), backrect[width*3+y2]);
}

void DrawRect(int x, int y, int width, int height, int oldx, int oldy, int oldwidth, int oldheight) {
	int x2, y2;

	DrawBackRect(oldx, oldy, oldwidth, oldheight);
	SaveRect(x, y, width, height);
	//************************************************
	// draw rect
	//************************************************
	if (!(y>1024 || y < 0)) {
		for (x2=0; x2<width; x2+=2) {
			if (x+x2>1024 || x+x2 < 0) continue;
			putp(1024*y+(x+x2), 0);
		}
	}

	if (!(y+height>768 || y+height < 0)) {
		for (x2=0; x2<width; x2+=2) {
			if (x+x2>1024 || x+x2 < 0) continue;
			putp(1024*(y+height)+(x+x2), 0);
		}
	}

	if (!(x>1024 || x < 0)) {
		for (y2=0; y2<height; y2+=2) {
			if (y+y2>768 || y+y2 < 0) continue;
			putp(1024*(y+y2)+x, 0);
		}
	}
	
	if (!(x+width>1024 || x+width < 0)) {
		for (y2=0; y2<height; y2+=2) {
			if (y+y2>768 || y+y2 < 0) continue;			
			putp(1024*(y+y2)+(x+width), 0);
		}
	}
}

int WNDSystem_ChildControl_Clicked(struct WND *ctrl, int x, int y) {
	int i=0;
	while (1) {
		if (ctrl->clickhandlers[i].handler==NULL) break;
		
		if ((x >= ctrl->clickhandlers[i].x) && 
			(x <= ctrl->clickhandlers[i].x + ctrl->clickhandlers[i].width) &&
	    	(y <= (ctrl->clickhandlers[i].y+ctrl->clickhandlers[i].height)) && 
	    	(y >= ctrl->clickhandlers[i].y)) {
			return i;
		}	
		
		i++;
	}
	return -1;	
}

char isfirst=0;

void WNDSystem_Left_MouseUp(int x, int y, int x2, int y2) {
	struct WND *search;
	struct WND *ctrl;
	char found=0;
	search=wnd_head;
	int tmp1,tmp2,oldw,oldh,ret;
	
	do {
		search=search->next;
		
		if (WNDSystem_Windowresize_Clicked(search, x, y) == TRUE) {
				found=2;
				break;
		}		
		
		if (WNDSystem_Windowtop_Clicked(search, x, y) == TRUE) {
			found=1;
			break;
		}
		
		if (search->first_ctrl != NULL) {
			ctrl=search->first_ctrl;
			while(1) {
				if ((ret=WNDSystem_ChildControl_Clicked(ctrl, x2, y2)) != -1) {
				void (*clickhandler)(struct WND *ctrl);
					clickhandler = ctrl->clickhandlers[ret].handler;
					clickhandler(ctrl);					
				}				
				if (ctrl->next_ctrl == NULL) break;				
				ctrl=ctrl->next_ctrl;
			}
		}				
	} while (search->next != NULL);
	
	if (found==1) {		// window top clicked
		search->rect.x = x2-(search->rect.width/2);
		search->rect.y = y2;
	}
	
	if (found==2) {		// window resize clicked
		
		oldw=search->rect.width;
		oldh=search->rect.height;
		search->rect.width += x2-x;
		search->rect.height += y2-y;
		
		if (search->first_ctrl != NULL) {
			ctrl=search->first_ctrl;
			while(1) {
				tmp1=ctrl->rect.width + search->rect.width - oldw;
				tmp2=ctrl->rect.height + search->rect.height - oldh;
					ctrl->rect.width = tmp1;
					ctrl->rect.height = tmp2;
				if (ctrl->next_ctrl == NULL) break;				
				ctrl=ctrl->next_ctrl;
			}
		}		
	}

	update();
	isfirst=0;
}

char clicked_topmost=0;
unsigned int saved_handle=0;

void WNDSystem_Left_MouseDown(int x, int y, int x2, int y2, int x3, int y3, char drag) {
	struct WND *search;
	struct WND *ctrl;
	char found=0;
	search=wnd_head;
	
	if (drag == 1) {
		do {
			search=search->next;		
			if (WNDSystem_Windowresize_Clicked(search, x, y) == TRUE) {
				found=2;
				break;
			}			
			
			if (WNDSystem_Windowtop_Clicked(search, x, y) == TRUE) {
				found=1;
				break;
			}
		} while (search->next != NULL); // wird das letzte im chain evt. nicht behandelt?
	
		if (found==1) {	// top clicked
			if (isfirst == 0) {
				SaveRect(x3-(search->rect.width/2), y3-10, search->rect.width, search->rect.height);
				isfirst=1;	
			}
			
			DrawRect(x2-(search->rect.width/2),y2-10,
					 search->rect.width,search->rect.height,
					 x3-(search->rect.width/2), y3-10,
					 search->rect.width,search->rect.height
					 );
		}
		
		if (found == 2) {	// resize
			if (isfirst == 0) {
				SaveRect(x3-(search->rect.width/2), y3-10, search->rect.width, search->rect.height);
				isfirst=1;	
			}
			
			DrawRect(search->rect.x,search->rect.y,
					 search->rect.width+x2-x,search->rect.height+y2-y,
					 search->rect.x,search->rect.y, search->rect.width+x3-x, search->rect.height+y3-y
   		    		);
		}
		
	} else {
		clicked_topmost=0;
		saved_handle=0;
		do {
			search=search->next;		
			if (WNDSystem_Window_Clicked(search, x2, y2) == TRUE) {
				found=1;
				saved_handle=search->handle;
				if (search->is_topmost_window == 1) {
					clicked_topmost=1;
				}
			}
		} while (search->next != NULL);
		
		if ((found == 1) && (clicked_topmost == 0)) {
			WNDSystem_SetTopWindow(saved_handle);
			update();
		}
	}	
}
