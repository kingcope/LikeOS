/*
	LikeOS
	Paging
*/
#define PAGEDIR_START 0xD00000
#include "defs.h"
#include "paging.h"
#include "physalloc.h"

ULONG *pdir = (ULONG *) PAGEDIR_START;

// Page Table für den Kernel beginned von 0 gleich mappen, da sonst die adressen des kernels
// wo anders hinzeigen (4 MB - kernel beginnt bei 0x100000), dh 3 MB platz für den kernel)
void page_kernel() {
	UINT i;
	UINT addr=0;
	ULONG *page_table = (ULONG *) PAGEDIR_START + 0x1000;

	for(i=0; i<1024; i++)
	{
		page_table[i] = addr | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
		addr = addr + 4096; // 4096 = 4kb
	};

	pdir[0] = (ULONG*)page_table; // attribute set to: supervisor level, read/write, present(011 in binary)
	pdir[0] = pdir[0] | 3;
}

void map_all() {
	UINT i,k;
	UINT addr=0;

	for(k=0;k<1024;k++) {
		ULONG *page_table = (ULONG *) (PAGEDIR_START + PAGE_SIZE + (k*PAGE_SIZE));

		for(i=0; i<1024; i++)
		{
			page_table[i] = addr | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
			addr = addr + 4096; // 4096 = 4kb
		};

		pdir[k] = (ULONG*)page_table; // attribute set to: supervisor level, read/write, present(011 in binary)
		pdir[k] = pdir[k] | 3;
	}
}

// pagetable_offset gibt an wo sich die page tabelle befindet, da eine pagetable 0x1000 gross ist gilt:
// page tabelle im speicher=PAGEDIR_START + 0x1000 + (pagetable_offset * 0x1000)
// len wird in pageframes angegeben (ein pageframe ist 4096 bytes gross)
void map(ULONG virt, ULONG phys, ULONG pagetable_offset, ULONG len) {
	// berechnung der virtuellen Adresse in pagedir/pagetable paare
	ULONG vdir=virt/0x400000;
	ULONG vtable=(virt % 0x400000) / 0x1000;
	
	UINT i;
	ULONG *ptable=(ULONG*) (PAGEDIR_START + 0x1000 + (pagetable_offset*0x1000));
	
	for(i=vtable; i<vtable+len; i++)
	{
		ptable[i] = phys | 3;
		phys = phys + 4096;
	}
	
	pdir[vdir] = (ULONG*)ptable;
	pdir[vdir] = pdir[vdir] | 3;
}

void enable_paging() {
	asm volatile("movl %0, %%cr3" :: "r" ((int)pdir));
	// enable paging
	asm volatile
	(
	 "movl %%cr0, %%eax\n"
	 "orl $0x80010000, %%eax\n" // paging and ring 0 write-protect enable
	 "movl %%eax, %%cr0\n" // do it
	 "jmp 1f\n" // flush cache/pipeline
	 "1:\n"
	 :::"eax"
	);	
}
