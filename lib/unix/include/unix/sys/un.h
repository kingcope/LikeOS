#ifndef _SYS_UN_H
#define _SYS_UN_H

typedef unsigned int	sa_family_t;

struct sockaddr_un
{
	sa_family_t sun_family;
	char        sun_path[]; 
};



#endif

