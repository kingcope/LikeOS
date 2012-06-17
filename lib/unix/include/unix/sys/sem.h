#ifndef _SYS_SEM_H
#define _SYS_SEM_H

#include <sys/types.h>
#include <sys/ipc.h>

#define	SEM_UNDO		1


#define	GETNCNT			1
#define	GETPID			2
#define	GETVAL			4
#define	GETALL			8
#define	GETZCNT			16
#define	SETVAL			32
#define	SETALL			64


struct semid_ds
{
	struct ipc_perm    sem_perm;
	unsigned short int sem_nsems;
	time_t             sem_otime;
	time_t             sem_ctime;
};




struct semaphore
{
	unsigned short int semval;
	pid_t              sempid;    
	unsigned short int semncnt;
	unsigned short int semzcnt;
};


struct sembuf
{
	unsigned short int sem_num;
	short int          sem_op;
	short int          sem_flg;
};


#ifdef __cplusplus
extern "C" {
#endif
		


int   semctl(int, int, int, ...);
int   semget(key_t, int, int);
int   semop(int, struct sembuf *, size_t);


#ifdef __cplusplus
}
#endif
		


#endif

