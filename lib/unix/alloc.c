#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "os/os.h"


// **********************************************
//
//	Durand's Super Duper Memory functions.
//
// 
// --------------------------------------------

#define LIBALLOC_MAGIC	0xc001c0de
#define LIBALLOC_MARK	0xF1F2F3F4


// RECOMMENDED SETTINGS
//	Each app will allocate PAGES * PAGESIZE bytes and then
//	subdivide amongst themselves.
#define LIBALLOC_PAGESIZE	4096
#define LIBALLOC_PAGES		16



struct liballoc_major
{
	struct liballoc_major *prev;
	struct liballoc_major *next;
	uint32_t pages;				// these are 4MB pages...
	uint32_t usage;				// bytes used
	struct liballoc_minor *first;	// The first thingy in this page.
} __attribute__ ((__packed__)) ;


struct	liballoc_minor
{
	struct liballoc_minor *prev;
	struct liballoc_minor *next;
	struct liballoc_major *block;
	uint32_t magic;
	uint32_t size; 
} __attribute__ ((__packed__)) ;


static struct liballoc_major *liballoc_root = NULL;
static uint32_t liballoc_spinlock = 0;


// --------------------------------------------

static struct liballoc_major *allocate_new_page( uint32_t size )
{
	uint32_t st;
	struct liballoc_major *maj;
	void *tmp;

		// This is how much space is required.
		st  = size + sizeof(struct liballoc_major);
		st += sizeof(struct liballoc_minor);

				// Perfect amount of space?
		if ( (st % LIBALLOC_PAGESIZE) == 0 )
			st  = st / (LIBALLOC_PAGESIZE);
		else
			st  = st / (LIBALLOC_PAGESIZE) + 1;
							// No, add the buffer. 

		
		// Make sure it's >= the minimum size.
		if ( st < LIBALLOC_PAGES ) st = LIBALLOC_PAGES;
		
		if ( os_mem_alloc( st, &tmp ) == NOTIMPLEMENTED ) 
								os_freakout();

		maj = (struct liballoc_major*)tmp;

		if ( maj == NULL ) return NULL;	// uh oh, we ran out of memory.
		
		maj->prev = NULL;
		maj->next = NULL;
		maj->pages = st;
		maj->usage = sizeof(struct liballoc_major);
		maj->first = NULL;

      return maj;
}



void *malloc(size_t size)
{
	void *p = NULL;
	uint32_t diff;
	struct liballoc_major *maj;
	struct liballoc_minor *min;
	struct liballoc_minor *new_min;


	os_acquire_spinlock( & liballoc_spinlock );

	if ( liballoc_root == NULL )
	{
		// This is the first time we are being used.
		liballoc_root = allocate_new_page( size );
		if ( liballoc_root == NULL )
		{
		  os_release_spinlock( & liballoc_spinlock );
		  return NULL;
		}
	}

	// Now we need to bounce through every major and find enough space....

	maj = liballoc_root;

	while ( maj != NULL )
	{
		diff  = maj->pages * (LIBALLOC_PAGESIZE) - maj->usage;	
										// free memory in the block

			
		// CASE 1:  There is not enough space in this major block.
		if ( diff < (size + sizeof( struct liballoc_minor )) )
		{
				// Another major block next to this one?
			if ( maj->next != NULL ) 
			{
				maj = maj->next;		// Hop to that one.
				continue;
			}

			// Create a new major block next to this one and...
			maj->next = allocate_new_page( size );	// next one will be okay.
			if ( maj->next == NULL ) break;			// no more memory.
			maj->next->prev = maj;
			maj = maj->next;
			// .. fall through to CASE 2 ..
		}
		
		// CASE 2: It's a brand new block.
		if ( maj->first == NULL )
		{
			maj->first = (struct liballoc_minor*)(  (uint32_t)maj + sizeof(struct liballoc_major) );
			maj->first->magic 	= LIBALLOC_MAGIC;
			maj->first->prev 	= NULL;
			maj->first->next 	= NULL;
			maj->first->block 	= maj;
			maj->first->size 	= size;
			maj->usage 			+= size + sizeof( struct liballoc_minor );

			p = (void*)((uint32_t)(maj->first) + sizeof( struct liballoc_minor ));

			
			os_release_spinlock( & liballoc_spinlock );		// release the lock
			return p;
		}
				

		// CASE 3: Block in use and enough space at the start of the block.
		diff = (uint32_t)(maj->first) - (uint32_t)maj - sizeof(struct liballoc_major);

		if ( diff >= (size + sizeof(struct liballoc_minor)) )
		{
			// Yes, space in front. Squeeze in.
			maj->first->prev = (struct liballoc_minor*)(  (uint32_t)maj + sizeof(struct liballoc_major) );
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;
				
			maj->first->magic 	= LIBALLOC_MAGIC;
			maj->first->prev 	= NULL;
			maj->first->block 	= maj;
			maj->first->size 	= size;
			maj->usage 			+= size + sizeof( struct liballoc_minor );

			p = (void*)((uint32_t)(maj->first) + sizeof( struct liballoc_minor ));
			os_release_spinlock( & liballoc_spinlock );		// release the lock
			return p;
		}
		

		
		// CASE 4: There is enough space in this block. But is it contiguous?
		min = maj->first;
		
			// Looping within the block now...
		while ( min != NULL )
		{
				// CASE 4.1: End of minors in a block. Space from last and end?
				if ( min->next == NULL )
				{
					// the rest of this block is free...  is it big enough?
					diff = (uint32_t)(maj) + (LIBALLOC_PAGESIZE) * maj->pages;
					diff -= ((uint32_t) min + sizeof( struct liballoc_minor ) + min->size); 
						// minus already existing usage..

					if ( diff >= size + sizeof( struct liballoc_minor ) )
					{
						// yay....
						min->next = (void*)((uint32_t)min + sizeof( struct liballoc_minor ) + min->size);
						min->next->prev = min;
						min = min->next;
						min->next = NULL;
						min->magic = LIBALLOC_MAGIC;
						min->block = maj;
						min->size = size;
						maj->usage += size + sizeof( struct liballoc_minor );
						p = (void*)((uint32_t)min + sizeof( struct liballoc_minor ));
						os_release_spinlock( & liballoc_spinlock );		// release the lock
						return p;
					}
				}



				// CASE 4.2: Is there space between two minors?
				if ( min->next != NULL )
				{
					// is the difference between here and next big enough?
					diff =  (uint32_t)(min->next) - (uint32_t)min  - sizeof( struct liballoc_minor ) - min->size;
										// minus our existing usage.

					if ( diff >= size + sizeof( struct liballoc_minor ) )
					{
						// yay......
						new_min = (void*)((uint32_t)min + sizeof( struct liballoc_minor ) + min->size);

						
						new_min->magic = LIBALLOC_MAGIC;
						new_min->next = min->next;
						new_min->prev = min;
						new_min->size = size;
						new_min->block = maj;
						min->next->prev = new_min;
						min->next = new_min;
						maj->usage += size + sizeof( struct liballoc_minor );
						p = (void*)((uint32_t)new_min + sizeof( struct liballoc_minor ));
						os_release_spinlock( & liballoc_spinlock );		// release the lock
						return p;
					}
				}	// min->next != NULL

				min = min->next;
		} // while min != NULL ...


		// CASE 5: Block full! Ensure next block and loop.
		if ( maj->next == NULL ) 
		{
			// we've run out. we need more...
			maj->next = allocate_new_page( size );		// next one guaranteed to be okay
			if ( maj->next == NULL ) break;			//  uh oh,  no more memory.....
			maj->next->prev = maj;
		}

		maj = maj->next;
	} // while (maj != NULL)

	
	os_release_spinlock( & liballoc_spinlock );		// release the lock
	return NULL;
}





void free(void *ptr)
{
	struct liballoc_minor *min;
	struct liballoc_major *maj;

	if ( ptr == NULL ) return;


	os_acquire_spinlock( & liballoc_spinlock );		// lockit

	min = (void*)((uint32_t)ptr - sizeof( struct liballoc_minor ));

	
	if ( min->magic != LIBALLOC_MAGIC ) 
	{
		// being lied to...
		os_release_spinlock( & liballoc_spinlock );		// release the lock
		return;
	}

		maj = min->block;

		maj->usage -= (min->size + sizeof( struct liballoc_minor ));
		min->magic  = 0;		// No mojo.

		if ( min->next != NULL ) min->next->prev = min->prev;
		if ( min->prev != NULL ) min->prev->next = min->next;

		if ( min->prev == NULL ) maj->first = min->next;	
							// Might empty the block. This was the first
							// minor.


	// We need to clean up after the majors now....

	if ( maj->first == NULL )	// Block completely unused.
	{
		if ( liballoc_root == maj ) liballoc_root = maj->next;
		if ( maj->prev != NULL ) maj->prev->next = maj->next;
		if ( maj->next != NULL ) maj->next->prev = maj->prev;

		if ( os_mem_free( maj ) == NOTIMPLEMENTED ) os_freakout();
	}
	
	os_release_spinlock( & liballoc_spinlock );		// release the lock
}





void* calloc(size_t nobj, size_t size)
{
       int32_t real_size;
       void *p;

       real_size = nobj * size;
       
       p = malloc( real_size );

       memset( p, 0, real_size );

       return p;
}



void*   realloc(void *p, size_t size)
{
	void *ptr;
	struct liballoc_minor *min;
	int32_t real_size;
	
	if ( size == 0 )
	{
		free( p );
		return NULL;
	}
	if ( p == NULL ) return malloc( size );

	os_acquire_spinlock( & liballoc_spinlock );		// lockit
		min = (void*)((uint32_t)p - sizeof( struct liballoc_minor ));
		real_size = min->size;
	os_release_spinlock( & liballoc_spinlock );

	if ( real_size > size ) real_size = size;

	ptr = malloc( size );
	memcpy( ptr, p, real_size );
	free( p );

	return ptr;
}



