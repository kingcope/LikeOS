#define PIC1 0x20
#define PIC2 0xA0
#define ICW1 0x11
#define ICW4 0x01

#include <defs.h>
#include <multiboot.h>
#include <vesa.h>

unsigned short
htons(unsigned short x)
{
	unsigned char *s = (unsigned char*) &x;
	return (unsigned short)(s[0] << 8 | s[1]);
}

__inline__ unsigned long inl(unsigned short port) { unsigned long ret;
     __asm__ __volatile__("in%L0 (%1)" : "=a" (ret) : "d" (port)); return ret; }

__inline__ void outl(unsigned short port, unsigned long value) {
     __asm__ __volatile__("out%L0 (%1)" : :"a" (value), "d" (port)); }
     
__inline__ unsigned short inw (unsigned short int port)
{
  unsigned short _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

__inline__ void outw(unsigned short port, unsigned short value)
{
    __asm__ __volatile__("outw %%ax, %%dx" : : "d" (port), "a" (value));
} 

__inline__ unsigned inb(unsigned port)
{
	unsigned ret_val;

	__asm__ __volatile__("inb %w1,%b0"
		: "=a"(ret_val)
		: "d"(port));
	return ret_val;
}
/*****************************************************************************
*****************************************************************************/
__inline__ void outb(unsigned port, unsigned val)
{
	__asm__ __volatile__("outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}

__inline__ void outportl(unsigned short port, unsigned long value) {
     __asm__ __volatile__("out%L0 (%1)" : :"a" (value), "d" (port)); }

__inline__ unsigned short inportw (unsigned short int port)
{
  unsigned short _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

__inline__ void outportw(unsigned short port, unsigned short value)
{
    __asm__ __volatile__("outw %%ax, %%dx" : : "d" (port), "a" (value));
} 

__inline__ unsigned inportb(unsigned port)
{
	unsigned ret_val;

	__asm__ __volatile__("inb %w1,%b0"
		: "=a"(ret_val)
		: "d"(port));
	return ret_val;
}
/*****************************************************************************
*****************************************************************************/
__inline__ void outportb(unsigned port, unsigned val)
{
	__asm__ __volatile__("outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}

#define iowait outportb(0x80,0x12);

/* init_pics()
 * init the PICs and remap them
 */
void initialize_pics(void)
{
	static const unsigned irq0_int = 0x20, irq8_int = 0x28;
	unsigned char a1,a2;
/**/

   a1=inportb(0x21);   // save masks
   a2=inportb(0xa1);

/* Initialization Control Word #1 (ICW1) */
	outportb(0x20, 0x11);
	iowait
	outportb(0xA0, 0x11);
	iowait
/* ICW2:
route IRQs 0-7 to INTs 20h-27h */
	outportb(0x21, irq0_int);
	iowait
/* route IRQs 8-15 to INTs 28h-2Fh */
	outportb(0xA1, irq8_int);
	iowait
/* ICW3 */
	outportb(0x21, 0x04);
	iowait
	outportb(0xA1, 0x02);
	iowait
/* ICW4 */
	outportb(0x21, 0x01);
	iowait
	outportb(0xA1, 0x01);
	iowait
	
	outportb(0x21, 0x00);
	outportb(0xA1, 0x00);
}


void fault_clear() {
	int x;
	
	for (x=0; x < SCREENWIDTH * SCREENHEIGHT; x++) {
			putp(x, 0x555);
	}	
}
#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define get_seg_long(seg,addr) ({ \
register unsigned long __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

char*  putx( unsigned int p )
{
   int offset;
   unsigned char c;
   int found = 0;
   char buf[255];
   char *bufp=buf;

   for ( offset = 28; offset >= 0; offset -= 4 )
   {
     c = (p>>offset) & 0xF;
     if ( c != 0 ) found = 1;
     if ( found == 1 )
     {
       if ( c < 10 ) *bufp++ = c + '0';
        	else *bufp++=c - 10 + 'a';
     }
   }
  
   if (found == 0 ) *bufp++='0';
  
   return buf;
}

void fault_showinfo(long esp_ptr,long nr) {
	int i;
	
	putstringc("HALTING",10,40,255,255,255,getlfb());
}

void panic1(char *s) {
	fault_clear();
	putstringc("*****STOP*****",10,10,255,255,255,getlfb());
	putstringc(s,10,25,255,255,255,getlfb());
	fault_showinfo(0, 0);
	asm("cli");
	asm("hlt");	
}

void intr_fault_divide(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Divide Error",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_bound(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Bounds Check",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_opcode(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Invalid Opcode",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_tss(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Invalid Task State Segment Descriptor",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_segment(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Segment not present",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_stack(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Stack Exception",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_gpf(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** General Protection Fault",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}

void intr_fault_pf(long esp, long error_code) {
	fault_clear();
	putstringc("*****STOP***** Page Fault",10,10,255,255,255,getlfb());
	fault_showinfo(esp, error_code);
	asm("cli");
	asm("hlt");	
}
