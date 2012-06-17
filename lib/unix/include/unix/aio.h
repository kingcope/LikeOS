#ifndef _AIO_H
#define _AIO_H

#include <signal.h>
#include <time.h>

struct aiocb
{
	int             aio_fildes;     /* file descriptor */
	off_t           aio_offset;     /* file offset */
	volatile void*  aio_buf;        /* location of buffer */
	size_t          aio_nbytes;     /* length of transfer */
	int             aio_reqprio;    /* request priority offset */
	struct sigevent aio_sigevent;   /* signal number and value */
	int             aio_lio_opcode; /* operation to be performed */
};



#define AIO_CANCELED				1
#define AIO_NOTCANCELED				2
#define AIO_ALLDONE					3

#define LIO_WAIT					1
#define LIO_NOWAIT					2
#define LIO_READ					3
#define LIO_WRITE					4
#define LIO_NOP						5


#ifdef __cplusplus
extern "C" {
#endif
		

int      aio_cancel(int, struct aiocb *);
int      aio_error(const struct aiocb *);
int      aio_fsync(int, struct aiocb *);
int      aio_read(struct aiocb *);
ssize_t  aio_return(struct aiocb *);
int      aio_suspend(const struct aiocb *const[], int, const struct timespec *);
int      aio_write(struct aiocb *);
int      lio_listio(int, struct aiocb *const[], int, struct sigevent *);


#ifdef __cplusplus
}
#endif
		


#endif

