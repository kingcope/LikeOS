#include <string.h>
#include <sys/utsname.h>



int uname(struct utsname *nm)
{
	static struct utsname temp =
	{
			"libUNIX",
			"nodename",
			"development",
			"version",
			"machine"
	};

	memcpy( nm, &temp, sizeof(struct utsname) );
	return 0;
}

