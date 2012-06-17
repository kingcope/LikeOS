#include "defs.h"
#include "multiboot.h"
#include "vesa.h"

#define RGB16_565(r,g,b) ((b&31) | ((g&63) << 5 | ((r&31) << 11)))

static int lfb_px = 0;
static int lfb_py = 0;

static ULONG lfb_width  = 0;
static ULONG lfb_height = 0;
static ULONG lfb_depth  = 0;
static ULONG lfb_type 	= 0;

void lfb_clear()
{
  int i;
  int j;
  
   for ( i = 0; i < (lfb_width-10)*2; i++ )
 	for ( j = 0; j < lfb_height; j++ )
		lfb[ j * lfb_width + i ] = 255;
}

ULONG getDepth() {
	return lfb_depth;	
}

void init_lfb(  multiboot_info_t *multiboot_info )
{
	struct vbe_mode *mode_info;
	void *lfb_ptr;

	lfb_px = 0;
	lfb_py = 0;

	if ( (multiboot_info->flags & (1<<11)) == 0 )
	{
	    lfb_width  = 0;
	    lfb_height = 0;
	    lfb_depth = 0;
	    lfb_type = 0;
	    lfb_ptr = 0;
	}
	else
	{
	    mode_info = (struct vbe_mode*) multiboot_info->vbe_mode_info;
	    lfb_width  = mode_info->x_resolution;
	    lfb_height = mode_info->y_resolution;
	    lfb_depth = mode_info->bits_per_pixel;
	    lfb_type = 0;
	    lfb_ptr = (void*)mode_info->phys_base;

		lfb = (ULONG*)lfb_ptr;
	}
	
//	set_lfb( lfb_width, lfb_height, lfb_depth, lfb_type, lfb_ptr );
}

void putpixel(ULONG x, ULONG y, ULONG col) {
	lfb[(y*SCREENWIDTH)+x]=col;
}

void ppo(ULONG *buffer, ULONG i, unsigned char r, unsigned char g, unsigned char b) {
	buffer[i]=(r<<16) | (g<<8) | b;
}

void pp(ULONG i, unsigned char r, unsigned char g, unsigned char b) {
	lfb[i]=(r<<16) | (g<<8) | b;
}

ULONG getp(ULONG i) {
	return lfb[i];
}

void putp(ULONG i, ULONG col) {
	lfb[i]=col;
}

void lfb_update(unsigned long *buf) {
	unsigned long c;
	unsigned long *p=lfb,*p2=buf;
	
	for (c=0;c<1024*768;c++) {
			*p=*p2;
			p++;
			p2++;
	}
	
}

unsigned long *getlfb() {
	return lfb;	
}
