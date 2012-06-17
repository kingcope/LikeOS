#include <kernel.h>
#include <time.h>

#include "../os.h"



int		os_clock( clock_t *ans )
{
	return NOTIMPLEMENTED;
}


int     os_system_time( time_t *seconds, time_t *milliseconds, int *rc )
{
	*rc = smk_system_time( seconds, milliseconds );
	return 0;
}



int 	os_sleep( unsigned int seconds, unsigned int *rc )
{
	*rc = smk_sleep( seconds );
	return 0;
}

