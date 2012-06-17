#include <kernel.h>
#include <stdio.h>
#include "../os.h"

int 	os_mem_alloc( int pages, void** ptr )
{
	/*
	printf("(%i)%s: %i, %x, %x, %x\n", 
				smk_getpid(), 
				"os_mem_alloc", 
				pages, 
				__builtin_return_address(0),
				__builtin_return_address(1),
				__builtin_return_address(2)
				);
	*/

	*ptr = (void*)smk_mem_alloc( pages );
	return 0;
}	

int		os_mem_free( void* mem )
{
	smk_mem_free( mem );
	return 0;
}



