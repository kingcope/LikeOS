#ifndef __KALLOC_H
#define __KALLOC_H

#define PAGE_SIZE 0x1000		// 4kb fuer eine Seite
#define TOTAL_MEMORY 32			// RAM in MB

ULONG phys_alloc();
void phys_free(ULONG addr);

#endif
