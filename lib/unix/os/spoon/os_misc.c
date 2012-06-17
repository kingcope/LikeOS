#include <kernel.h>
#include "../os.h"

int os_acquire_spinlock( unsigned int *lock  )
{
	smk_acquire_spinlock( lock );
	return 0;
}

int os_release_spinlock( unsigned int *lock  )
{
	smk_release_spinlock( lock );
	return 0;
}


int	os_rand( int *rc )
{
	*rc = smk_random();
	return 0;
}

