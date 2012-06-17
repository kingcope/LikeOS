#define PHYSALLOC_STACKORIGIN	 0x402000		// hier befindet sich der kalloc stack
#include "defs.h"

ULONG *stackp=(ULONG *)PHYSALLOC_STACKORIGIN;
ULONG freepages=0;

void *phys_alloc() {
	stackp-=sizeof(ULONG);
	ULONG addr=*stackp;
	freepages--;
	return addr;
}

void phys_free(ULONG addr) {
	*stackp=addr;
	stackp+=sizeof(ULONG);
	freepages++;
}
