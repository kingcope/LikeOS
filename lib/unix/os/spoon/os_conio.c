#include <kernel.h>
#include "../os.h"


int	os_puts( const char *str )
{
	smk_conio_puts( str );
	return 0;
}	


