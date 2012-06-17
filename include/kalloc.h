#ifndef __KALLOC_H
#define __KALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void kinit();
void *kalloc(long unsigned int size);
void kfree(void *addr);
void *kcalloc(unsigned int nelem, unsigned int elsize);
void *krealloc(void *ptr, unsigned int size);
#ifdef __cplusplus
}
#endif

#endif
