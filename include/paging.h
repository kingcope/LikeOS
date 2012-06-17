/*
	LikeOS
	Paging Funktionen
*/

#ifndef __PAGING_H
#define __PAGING_H

extern void page_kernel();
extern void map(ULONG virt, ULONG phys, ULONG pagetable_offset, ULONG len);
extern void enable_paging();
extern void map_all();

#endif
