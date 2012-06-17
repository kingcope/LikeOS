#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H


#include <signal.h>
#include <sys/time.h>
#include <time.h>


int  pselect(int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
			  const struct timespec *restrict, const sigset_t *restrict);

int  select(int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
				         struct timeval *restrict);


#endif

