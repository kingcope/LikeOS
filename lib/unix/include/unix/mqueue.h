#ifndef _MQUEUE_H
#define _MQUEUE_H


#ifndef _HAVE_MQD_T
#define _HAVE_MQD_T
typedef int mqd_t;
#endif


#include <signal.h>


struct mq_attr
{
	long    mq_flags;    //message queue flags
	long    mq_maxmsg;   //maximum number of messages
	long    mq_msgsize;  //maximum message size
	long    mq_curmsgs;  //number of messages currently queued
};


#ifdef __cplusplus
extern "C" {
#endif
		


int      mq_close(mqd_t);
int      mq_getattr(mqd_t, struct mq_attr *);
int      mq_notify(mqd_t, const struct sigevent *);
mqd_t    mq_open(const char *, int, ...);
ssize_t  mq_receive(mqd_t, char *, size_t, unsigned int *);
int      mq_send(mqd_t, const char *, size_t, unsigned int);
int      mq_setattr(mqd_t, const struct mq_attr *, struct mq_attr *);
int      mq_unlink(const char *);

#ifdef __cplusplus
}
#endif
		


#endif

