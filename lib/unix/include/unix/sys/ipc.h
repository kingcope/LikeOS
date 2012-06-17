#ifndef _SYS_IPC_H
#define _SYS_IPC_H


#include <sys/types.h>

#ifndef _HAVE_IPC_PERM
#define _HAVE_IPC_PERM
struct ipc_perm
{
	uid_t    uid;
	gid_t    gid;
	uid_t    cuid;
	gid_t    cgid;
	mode_t   mode;
};
#endif




#define	IPC_CREAT			1
#define	IPC_EXCL			2
#define	IPC_NOWAIT			4


#define	IPC_PRIVATE			8


#define	IPC_RMID			1
#define	IPC_SET				2
#define	IPC_STAT			4

#ifdef __cplusplus
extern "C" {
#endif
		

key_t  ftok(const char *, int);

#ifdef __cplusplus
}
#endif
		

#endif


