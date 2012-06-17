#include "defs.h"
#include "multiboot.h"
#include "vesa.h"
#include "kalloc.h"
#include "string.h"
#include "isocd.h"
#include "font.h"
#include "mouse.h"
#include "draw.h"

void *  operator new (long unsigned int size) 
{
  return kalloc(size);
}

void *  operator new[] (long unsigned int size) 
{
  return kalloc(size);
}

void  operator delete (void *p)
{
  kfree( p );
}

void  operator delete[] (void *p)
{
  kfree(p);
}


void test() {
	Color *col = new Color(50,50,50);
	Draw *draw = new Draw();
	
	draw->DrawLine(50,500,500,col);
}

extern "C" void startx() {
	test();
}
