#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#include <inttypes.h>
#include <sys/types.h>
#include <sys/ipc.h>



typedef	uint32_t msgqnum_t;
typedef uint32_t msglen_t;


#define	MSG_NOERROR		1


struct msqid_ds
{
	    struct ipc_perm msg_perm;
	    msgqnum_t       msg_qnum;
	    msglen_t        msg_qbytes;
	    pid_t           msg_lspid;
	    pid_t           msg_lrpid;
	    time_t          msg_stime;
	    time_t          msg_rtime;
	    time_t          msg_ctime;
};

#ifdef __cplusplus
extern "C" {
#endif
		


int       msgctl(int, int, struct msqid_ds *);
int       msgget(key_t, int);
ssize_t   msgrcv(int, void *, size_t, long int, int);
int       msgsnd(int, const void *, size_t, int);

#ifdef __cplusplus
}
#endif
		


#endif


