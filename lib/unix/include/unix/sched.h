#ifndef _SCHED_H
#define _SCHED_H

#include <time.h>

#ifndef _HAVE_PID_T
#define _HAVE_PID_T
typedef	int32_t		pid_t;
#endif


#define		SCHED_FIFO			1
#define		SCHED_RR			2
#define		SCHED_OTHER			3

struct sched_param
{
	int    sched_priority;	// process execution scheduling priority
};

#ifdef __cplusplus
extern "C" {
#endif

int    sched_get_priority_max(int);
int    sched_get_priority_min(int);
int    sched_getparam(pid_t, struct sched_param *);
int    sched_getscheduler(pid_t);
int    sched_rr_get_interval(pid_t, struct timespec *);
int    sched_setparam(pid_t, const struct sched_param *);
int    sched_setscheduler(pid_t, int, const struct sched_param *);
int    sched_yield(void);

#ifdef __cplusplus
}
#endif


#endif

