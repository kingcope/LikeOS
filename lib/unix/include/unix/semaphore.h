#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H


#ifndef _HAVE_SEM_T
#define _HAVE_SEM_T
typedef int sem_t;
#endif


#define SEM_FAILED	((sem_t)-1)



#ifdef __cplusplus
extern "C" {
#endif


int    sem_close(sem_t *);
int    sem_destroy(sem_t *);
int    sem_getvalue(sem_t *, int *);
int    sem_init(sem_t *, int, unsigned int);
sem_t *sem_open(const char *, int, ...);
int    sem_post(sem_t *);
int    sem_trywait(sem_t *);
int    sem_unlink(const char *);
int    sem_wait(sem_t *);


#ifdef __cplusplus
}
#endif


#endif

