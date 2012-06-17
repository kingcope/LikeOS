#ifndef _STROPTS_H
#define _STROPTS_H

#include <sys/types.h>
#include <unistd.h>

struct bandinfo
{
	unsigned char bi_pri;
	int           bi_flag;
};


struct strpeek
{
	struct strbuf ctlbuf;
	struct strbuf databuf;
	t_uscalar_t   flags;
};


struct strbuf
{
	int           maxlen;
	int           len;
	char         *buf;
};


struct strfdinsert
{
	struct strbuf ctlbuf;
	struct strbuf databuf;
	t_uscalar_t   flags;
	int           fildes;
	int           offset;
};


struct strioctl
{
	int           ic_cmd;
	int           ic_timout;
	int           ic_len;
	char         *ic_dp;
};


struct strrecvfd
{
	int           fd;
	uid_t         uid;
	gid_t         gid;
}


struct str_list
{
	int               sl_nmods;
	struct str_mlist *sl_modlist;
};


struct str_mlist
{
	char          l_name[FMNAMESZ+1];
};





#define	I_PUSH				1
#define	I_POP				2
#define	I_LOOK				4

//TODO: what's the size?
#define	FMNAMESZ			1024

#define	I_FLUSH				8

#define	FLUSHR				1
#define	FLUSHW				2
#define	FLUSHRW				4

#define	I_FLUSHBAND			16

#define	I_SETSIG			32


#define	S_RDNORM			(1<<0)
#define	S_RDBAND			(1<<1)
#define	S_INPUT				(1<<2)
#define	S_HIPRI				(1<<3)
#define	S_OUTPUT			(1<<4)
#define	S_WRNORM			S_OUTPUT
#define	S_WRBAND			(1<<5)
#define	S_MSG				(1<<6)
#define	S_ERROR				(1<<7)
#define	S_HANGUP			(1<<8)
#define	S_BANDURG			(1<<9)

#define	I_GETSIG			1
#define	I_FIND				2
#define	I_PEEK				4

#define	RS_HIPRI			8

#define	I_SRDOPT			16

#define	RNORM				1
#define	RMSGD				2
#define	RMSGN				4
#define	RPROTNORM			8
#define	RPROTDAT			16
#define	RPROTDIS			32

#define	I_GRDOPT			1

#define	I_NREAD				2

#define	I_FDINSERT			3

#define	I_STR				4

#define	I_SWROPT			5

#define	SNDZERO				6

#define	I_GWROPT			7

#define	I_SENDFD			8

#define	I_RECVFD			9

#define	I_LIST				10

#define	I_ATMARK			11

#define	ANYMARK				1
#define	LASTMARK			2

#define	I_CKBAND			1
	
#define	I_GETBAND			2

#define	I_CANPUT			3

#define	I_SETCLTIME			4

#define	I_GETCLTIME			5

#define	I_LINK				6

#define	I_UNLINK			7

#define	MUXID_ALL			8

#define	I_PLINK				9

#define	I_PUNLINK			10


#define	MSG_ANY				(1<<0)
#define	MSG_BAND			(1<<1)
#define	MSG_HIPRI			(1<<2)
#define	MORECTL				(1<<3)
#define	MOREDATA			(1<<4)


#ifdef __cplusplus
extern "C" {
#endif
		


int    isastream(int);
int    getmsg(int, struct strbuf *, struct strbuf *, int *);
int    getpmsg(int, struct strbuf *, struct strbuf *, int *, int *);
int    ioctl(int, int, ... );
int    putmsg(int, const struct strbuf *, const struct strbuf *, int);
int    putpmsg(int, const struct strbuf *, const struct strbuf *, int, int);
int    fattach(int, const char *);
int    fdetach(const char *);


#ifdef __cplusplus
}
#endif
		


#endif

