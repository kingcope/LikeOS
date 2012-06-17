
// FIXME New implementation --> kfree

#include <defs.h>
#include <kalloc.h>
#include <physalloc.h>

ULONG heap_start=0x2005000;
ULONG heap_end=0xa0000000;

struct listhead {
	struct listhead *prev, *next;
	char used;
	long unsigned int size;
	void *ptr;
} *list_head;

struct listhead *list_append(struct listhead *list_append) {
	struct listhead *search;
	search=list_head;

	for (;;) {
		if (search->next == NULL) break;
		search=search->next;
	}

	list_append->next=NULL;
	search->next=list_append;
	list_append->prev=search;

	return list_append;
}

void *kalloc(long unsigned int size) {
	struct listhead *search;
	char found=0;
	struct listhead *app=NULL;
	
	search=list_head;
	
	while (search->next != NULL) {
		search=search->next;
	}
	
	app=(struct listhead*)phys_alloc();
	app->used=1;
	app->size=size;
	app->ptr=(ULONG *) search->ptr+search->size+1;
	list_append(app);	

	found = 1;
	
	if (found==1) {
		app->used=1;		
		return app->ptr;
	} else {
		return NULL;	
	}
}

void kfree(void *addr) {
// addr in der liste suchen und auf unused setzten
	struct listhead *search;
	char found=0;
	
	search=list_head;
	
	while (search->next != NULL) {
		
		if ((search->ptr == addr) && (search->used==1)) {
			found=1;
			break;	
		}
		search=search->next;
	}

	if (found==1) search->used=0;
}


void *kcalloc(unsigned int nelem, unsigned int elsize) {
	return kalloc(nelem*elsize);
}

void *krealloc(void *ptr, unsigned int size) {
	kfree(ptr);
	return kalloc(size);
}

/*void kinit() {
	long unsigned int i,size,j;
	struct listhead *app=NULL;
	
	list_head=(struct listhead*)phys_alloc();
	list_head->prev=NULL;
	list_head->next=NULL;
	list_head->size=0;
	list_head->used=1;
	list_head->ptr=(ULONG *)heap_start;
	size=1;

	for (i=heap_start; i<heap_end; i+=size) {
		size*=3;
		if (size+i > heap_end) {
			size=1;
		}

		for (j=1; j<80; j++) {
			app=(struct listhead*)phys_alloc();
			app->used=0;
			app->size=size;
			app->ptr=(ULONG *) i+(j*size);
			list_append(app);
		}
	}
}
*/

void kinit() {
	list_head=(struct listhead*)phys_alloc();
	list_head->prev=NULL;
	list_head->next=NULL;
	list_head->size=0;
	list_head->used=1;
	list_head->ptr=(ULONG *)heap_start;
}
