
void *malloc(unsigned long size) {
	void *ptr=kalloc(size);
	return ptr;
}

void free(void *addr) {
	kfree(addr);	
}

void *realloc(void *ptr, unsigned long size) {
	kfree(ptr);
	return kalloc(size);
}
