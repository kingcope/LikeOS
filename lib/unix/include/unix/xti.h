#ifndef _XTI_H
#define _XTI_H


/*
* The following are the error codes needed by both the kernel
* level transport providers and the user level library.
*/

#define TBADADDR 	1 	/* incorrect addr format */
#define TBADOPT 	2 	/* incorrect option format */
#define TACCES	 	3 	/* incorrect permissions */
#define TBADF 		4 	/* illegal transport fd */
#define TNOADDR 	5 	/* couldn't allocate addr */
#define TOUTSTATE 	6 	/* out of state */
#define TBADSEQ 	7 	/* bad call sequence number */
#define TSYSERR 	8 	/* system error */
#define TLOOK 		9 	/* event requires attention */
#define TBADDATA 	10 	/* illegal amount of data */
#define TBUFOVFLW 	11 	/* buffer not large enough */
#define TFLOW 		12 	/* flow control */
#define TNODATA 	13 	/* no data */
#define TNODIS 		14 	/* discon_ind not found on queue */
#define TNOUDERR 	15 	/* unitdata error not found */
#define TBADFLAG 	16 	/* bad flags */
#define TNOREL 		17 	/* no ord rel found on queue */
#define TNOTSUPPORT 	18 	/* primitive/action not supported */
#define TSTATECHNG 	19 	/* state is in process of changing */
#define TNOSTRUCTYPE 	20 	/* unsupported struct-type requested */
#define TBADNAME 	21 	/* invalid transport provider name */
#define TBADQLEN 	22 	/* qlen is zero */
#define TADDRBUSY 	23 	/* address in use */
#define TINDOUT 	24 	/* outstanding connection indications */
#define TPROVMISMATCH 	25 	/* transport provider mismatch */
#define TRESQLEN 	26 	/* resfd specified to accept w/qlen >0 */
#define TRESADDR 	27 	/* resfd not bound to same addr as fd */
#define TQFULL 		28 	/* incoming connection queue full */
#define TPROTO	 	29 	/* XTI protocol error */



/*
* The following are the events returned.
*/

#define T_LISTEN 	0x0001 	/* connection indication received */
#define T_CONNECT 	0x0002 	/* connection confirmation received */
#define T_DATA 		0x0004 	/* normal data received */
#define T_EXDATA 	0x0008 	/* expedited data received */
#define T_DISCONNECT 	0x0010 	/* disconnection received */
#define T_UDERR 	0x0040 	/* datagram error indication */
#define T_ORDREL 	0x0080 	/* orderly release indication */
#define T_GODATA 	0x0100 	/* sending normal data is again possible */
#define T_GOEXDATA 	0x0200 	/* sending expedited data is again */
/* possible */


/*
* The following are the flag definitions needed by the
* user level library routines.
*/

#define T_MORE 		0x001 	/* more data */
#define T_EXPEDITED 	0x002 	/* expedited data */
#define T_PUSH 		0x004 	/* send data immediately */
#define T_NEGOTIATE 	0x004 	/* set opts */
#define T_CHECK 	0x008 	/* check opts */
#define T_DEFAULT 	0x010 	/* get default opts */
#define T_SUCCESS 	0x020 	/* successful */
#define T_FAILURE 	0x040 	/* failure */
#define T_CURRENT 	0x080 	/* get current options */
#define T_PARTSUCCESS 	0x100 	/* partial success */
#define T_READONLY 	0x200 	/* read-only */
#define T_NOTSUPPORT 	0x400 	/* not supported */


#ifdef __cplusplus
extern "C" {
#endif
		


/*
*  XTI error return.
*/

/* t_errno is a modifiable lvalue of type int                           */
/* In a single threaded environment a typical definition of t_errno is: */

extern int t_errno;

/* In a multi-threading environment a typical definition of t_errno is: */

extern int *_t_errno(void);
#define t_errno (*(_t_errno()))


/*
* iov maximum
*/
#define T_IOV_MAX  16    /* maximum number of scatter/gather buffers */
/* value is not mandatory but if present    */
/* must be at least 16.                     */

struct  t_iovec {
void          *iov_base;
size_t         iov_len;
};


/* 
*  XTI LIBRARY FUNCTIONS
*/

/* XTI Library Function: t_accept - accept a connection request*/
extern int t_accept(int, int, const struct t_call *);
/* XTI Library Function: t_alloc - allocate a library structure*/
extern void *t_alloc(int, int, int);
/* XTI Library Function: t_bind - bind an address to a transport endpoint*/
extern int t_bind(int, const struct t_bind *, struct t_bind *);
/* XTI Library Function: t_close - close a transport endpoint*/
extern int t_close(int);
/* XTI Library Function: t_connect - establish a connection */
extern int t_connect(int, const struct t_call *, struct t_call *);
/* XTI Library Function: t_error - produce error message*/
extern int t_error(const char *);
/* XTI Library Function: t_free - free a library structure*/
extern int t_free(void *, int);
/* XTI Library Function: t_getinfo - get protocol-specific service */
/* information*/
extern int t_getinfo(int, struct t_info *);
/* XTI Library Function: t_getprotaddr - get protocol addresses*/
extern int t_getprotaddr(int, struct t_bind *, struct t_bind *);
/* XTI Library Function: t_getstate - get the current state*/
extern int t_getstate(int);
/* XTI Library Function: t_listen - listen for a connection indication*/
extern int t_listen(int, struct t_call *);
/* XTI Library Function: t_look - look at current event on a transport */
/* endpoint*/
extern int t_look(int);
/* XTI Library Function: t_open - establish a transport endpoint*/
extern int t_open(const char *, int, struct t_info *);
/* XTI Library Function: t_optmgmt - manage options for a transport */
/* endpoint*/
extern int t_optmgmt(int, const struct t_optmgmt *, struct t_optmgmt *);
/* XTI Library Function: t_rcv - receive data or expedited data on a */
/* connection*/
extern int t_rcv(int, void *, unsigned int, int *);
/* XTI Library Function: t_rcvconnect - receive the confirmation from */
/* a connection request */
extern int t_rcvconnect(int, struct t_call *);
/* XTI Library Function: t_rcvdis - retrieve information from disconnect*/
extern int t_rcvdis(int, struct t_discon *);
/* XTI Library Function: t_rcvrel - acknowledge receipt of */
/* an orderly release indication */
extern int t_rcvrel(int);
/* XTI Library Function: t_rcvreldata - receive an orderly release */
/* indication or confirmation containing user data */
extern int t_rcvreldata(int, struct t_discon *discon)
/* XTI Library Function: t_rcvudata - receive a data unit*/
extern int t_rcvudata(int, struct t_unitdata *, int *);
/* XTI Library Function: t_rcvuderr - receive a unit data error indication*/
extern int t_rcvuderr(int, struct t_uderr *);
/* XTI Library Function: t_rcvv - receive data or expedited data sent*/
/*                              over a connection and put the data    */
/*                              into one or more noncontiguous buffers*/
extern int t_rcvv(int, struct t_iovec *, unsigned int, int *);
/* XTI Library Function: t_rcvvudata - receive a data unit into one  */
/*                                       or more noncontiguous buffers*/
extern int t_rcvvudata(int, struct t_unitdata *, struct t_iovec *, \
unsigned int, int *);
/* XTI Library Function: t_snd - send data or expedited data over a */
/* connection */
extern int t_snd(int, void *, unsigned int, int);
/* XTI Library Function: t_snddis - send user-initiated disconnect request*/
extern int t_snddis(int, const struct t_call *);
/* XTI Library Function: t_sndrel - initiate an orderly release*/
extern int t_sndrel(int);
/* XTI Library Function: t_sndreldata - initiate or respond to an */
/* orderly release with user data */
extern int t_sndreldata(int, struct t_discon *);
/* XTI Library Function: t_sndudata - send a data unit*/
extern int t_sndudata(int, const struct t_unitdata *);
/* XTI Library Function: t_sndv - send data or expedited data,  */
/*            from one or more noncontiguous buffers, on a connection*/
extern int t_sndv(int, const struct t_iovec *, unsigned int iovcount, int);
/* XTI Library Function: t_sndvudata - send a data unit from one or  */
/*                                         more non-contiguous buffers*/
extern int t_sndvudata(int, struct t_unitdata *, struct t_iovec *, unsigned int);
/* XTI Library Function: t_strerror - generate error message string */
extern const char *t_strerror(int);
/* XTI Library Function: t_sync - synchronise transport library*/
extern int t_sync(int);
/* XTI Library Function: t_sysconf - get configurable XTI variables */
extern int t_sysconf(int);
/* XTI Library Function: t_unbind - disable a transport endpoint*/
extern int t_unbind(int);


/*
* Protocol-specific service limits.
*/
struct t_info {
	t_scalar_t addr;     /*max size of the transport protocol address         */
	t_scalar_t options;  /*max number of bytes of protocol-specific options   */
	t_scalar_t tsdu;     /*max size of a transport service data unit          */
	t_scalar_t etsdu;    /*max size of expedited transport service data unit  */
	t_scalar_t connect;  /*max amount of data allowed on connection           */
	/*establishment functions                            */
	t_scalar_t discon;   /*max data allowed on t_snddis and t_rcvdis functions*/
	t_scalar_t servtype; /*service type supported by transport provider       */
	t_scalar_t flags;    /*other info about the transport provider            */
};


/*
* Service type defines.
*/

#define T_COTS 		01 	/* connection-mode transport service */
#define T_COTS_ORD 	02 	/* connection-mode with orderly release */
#define T_CLTS 		03 	/* connectionless-mode transport service */


/*
* Flags defines (other info about the transport provider).
*/

#define T_SENDZERO 	0x001 	/* supports 0-length TSDUs */
#define T_ORDRELDATA 	0x002 	/* supports orderly release data */


/*
* netbuf structure.
*/

struct netbuf {
	unsigned int 	maxlen;
	unsigned int 	len;
	void 	*buf;
};



/*
* t_opthdr structure
*/
struct t_opthdr {
	t_uscalar_t len;      /* total length of option; that is,  */
				/* sizeof (struct t_opthdr) + length */
				/* of option value in bytes          */
	t_uscalar_t level;    /* protocol affected                 */
	t_uscalar_t name;     /* option name                       */
	t_uscalar_t status;   /* status value                      */
	/* followed by the option value */
};


/*
* t_bind - format of the address arguments of bind.
*/

struct t_bind {
	struct netbuf 	addr;
	unsigned int 	qlen;
};


/*
* Options management structure.
*/

struct t_optmgmt {
	struct netbuf 	opt;
	t_scalar_t 	flags;
};


/*
* Disconnection structure.
*/

struct t_discon {
	struct netbuf 	udata; 	/* user data */
	int 	reason; 	/* reason code */
	int 	sequence; 	/* sequence number */
};


/*
* Call structure.
*/

struct t_call {
	struct netbuf 	addr; 	/* address */
	struct netbuf 	opt; 	/* options */
	struct netbuf 	udata; 	/* user data */
	int 	sequence; 	/* sequence number */
};



/*
* Datagram structure.
*/

struct t_unitdata {
	struct netbuf 	addr; 	/* address */
	struct netbuf 	opt; 	/* options */
	struct netbuf 	udata; 	/* user data */
};



/*
* Unitdata error structure.
*/

struct t_uderr {
	struct netbuf 	addr; 	/* address */
	struct netbuf 	opt; 	/* options */
	t_scalar_t 	error; 	/* error code */
};


/*
* The following are structure types used when dynamically
* allocating the above structures via t_alloc().
*/

#define T_BIND 		1 	/* struct t_bind */
#define T_OPTMGMT 	2 	/* struct t_optmgmt */
#define T_CALL 		3 	/* struct t_call */
#define T_DIS 		4 	/* struct t_discon */
#define T_UNITDATA 	5 	/* struct t_unitdata */
#define T_UDERROR 	6 	/* struct t_uderr */
#define T_INFO 		7 	/* struct t_info */


/*
* The following bits specify which fields of the above
* structures should be allocated by t_alloc().
*/

#define T_ADDR 		0x01 	/* address */
#define T_OPT 		0x02 	/* options */
#define T_UDATA 	0x04 	/* user data */
#define T_ALL 		0xffff 	/* all the above fields supported */



/*
* The following are the states for the user.
*/

#define T_UNBND 	1 	/* unbound */
#define T_IDLE 		2 	/* idle */
#define T_OUTCON 	3 	/* outgoing connection pending */
#define T_INCON 	4 	/* incoming connection pending */
#define T_DATAXFER 	5 	/* data transfer */
#define T_OUTREL 	6 	/* outgoing release pending */
#define T_INREL 	7 	/* incoming release pending */



/*
* General purpose defines.
*/

#define 	T_YES 		1
#define 	T_NO 		0
#define 	T_NULL 		0
#define 	T_ABSREQ 	0x8000
#define 	T_INFINITE 	(-1)
#define 	T_INVALID 	(-2)


/*
*  Definitions for t_sysconf
*/
#define _SC_T_IOV_MAX     1  /* value is recommended only, not mandatory */


/*
* General definitions for option management
*/
#define T_UNSPEC  		(~0 - 2)  /* applicable to u_long, t_scalar_t, char .. */
#define T_ALLOPT  		0    /* value is recommended-only, not mandatory */
#define T_ALIGN(p) 		(((t_uscalar_t)(p) + (sizeof (t_scalar_t) - 1)) \
					& ~(sizeof (t_scalar_t) - 1))
#define T_OPT_DATA(tohp)         /* definition to be added           */

#define T_OPT_NEXTHDR(pbuf,buflen,popt) \
		(((char *)(popt) + T_ALIGN((popt)->len) < \
		(char *)(pbuf) + (buflen)) ? \
		(struct t_opthdr *)((char *)(popt) + T_ALIGN((popt)->len)) : \
		(struct t_opthdr *)0 )

#define T_OPT_FIRSTHDR(nbp)      /* definition to be added           */


/* OPTIONS ON XTI LEVEL */

/* 
*  XTI Level
* 
*  The values defined for the XTI Level are recommended-only,
*  not mandatory.
*/

#define 	XTI_GENERIC 	0xffff



/*
*  XTI-level Options
* 
*  The values defined for these XTI-level Options are recommended-only,
*  not mandatory.
*/

#define 	XTI_DEBUG 	0x0001 	/* enable debugging */
#define 	XTI_LINGER 	0x0080 	/* linger on close if data present */
#define 	XTI_RCVBUF 	0x1002 	/* receive buffer size */
#define 	XTI_RCVLOWAT 	0x1004 	/* receive low-water mark */
#define 	XTI_SNDBUF 	0x1001 	/* send buffer size */
#define 	XTI_SNDLOWAT 	0x1003 	/* send low-water mark */


/*
* Structure used with linger option.
*/
struct t_linger {
	t_scalar_t   l_onoff;     /* option on/off */
	t_scalar_t   l_linger;    /* linger time   */
};


/* SPECIFIC ISO OPTION AND MANAGEMENT PARAMETERS */

/*
* Definition of the ISO transport classes
*/

#define 	T_CLASS0 	0
#define 	T_CLASS1 	1
#define 	T_CLASS2 	2
#define 	T_CLASS3 	3
#define 	T_CLASS4 	4


/*
* Definition of the priorities.
*/

#define 	T_PRITOP 	0
#define 	T_PRIHIGH 	1
#define 	T_PRIMID 	2
#define 	T_PRILOW 	3
#define 	T_PRIDFLT 	4


/*
* Definitions of the protection levels
*/

#define 	T_NOPROTECT 		1
#define 	T_PASSIVEPROTECT 	2
#define 	T_ACTIVEPROTECT 	4


/*
* Default value for the length of TPDUs.
*/

#define 	T_LTPDUDFLT 	128 	/* define obsolete in XPG4 */


/*
* rate structure.
*/
struct rate {
	t_scalar_t targetvalue;       /* target value */
	t_scalar_t minacceptvalue;    /* value of minimum acceptable quality */
};



/*
* reqvalue structure.
*/
struct reqvalue {
	struct rate    called;    /* called rate */
	struct rate    calling;   /* calling rate */
};



/*
* thrpt structure.
*/
struct thrpt {
	struct reqvalue    maxthrpt;    /* maximum throughput */
	struct reqvalue    avgthrpt;    /* average throughput */
};



/*
* transdel structure
*/
struct transdel {
	struct reqvalue    maxdel;    /* maximum transit delay */
	struct reqvalue    avgdel;    /* average transit delay */
};


/*
* Protocol Levels
* 
*  The values defined for these Protocol Levels are recommended-only,
*  not mandatory.
*/

#define 	T_ISO_TP 	0x0100
#define 	ISO_TP 		0x0100 	(LEGACY)



/*
*  Options for Quality of Service and Expedited Data (ISO 8072:1994)
* 
*  The values defined for these QoS and Expedited Data are 
*  recommended-only, not mandatory.
*/

#define 	T_TCO_THROUGHPUT 	0x0001
#define 	TCO_THROUGHPUT 		0x0001 	(LEGACY)
#define 	T_TCO_TRANSDEL 		0x0002
#define 	TCO_TRANSDEL 		0x0002 	(LEGACY)
#define 	T_TCO_RESERRORRATE 	0x0003
#define 	TCO_RESERRORRATE 	0x0003 	(LEGACY)
#define 	T_TCO_TRANSFFAILPROB 	0x0004
#define 	TCO_TRANSFFAILPROB 	0x0004 	(LEGACY)
#define 	T_TCO_ESTFAILPROB 	0x0005
#define 	TCO_ESTFAILPROB 	0x0005 	(LEGACY)
#define 	T_TCO_RELFAILPROB 	0x0006
#define 	TCO_RELFAILPROB 	0x0006 	(LEGACY)
#define 	T_TCO_ESTDELAY 		0x0007
#define 	TCO_ESTDELAY 		0x0007 	(LEGACY)
#define 	T_TCO_RELDELAY 		0x0008
#define 	TCO_RELDELAY 		0x0008 	(LEGACY)
#define 	T_TCO_CONNRESIL 	0x0009
#define 	TCO_CONNRESIL 		0x0009 	(LEGACY)
#define 	T_TCO_PROTECTION 	0x000a
#define 	TCO_PROTECTION 		0x000a 	(LEGACY)
#define 	T_TCO_PRIORITY 		0x000b
#define 	TCO_PRIORITY 		0x000b 	(LEGACY)
#define 	T_TCO_EXPD 		0x000c
#define 	TCO_EXPD 		0x000c 	(LEGACY)
#define 	T_TCL_TRANSDEL 		0x000d
#define 	TCL_TRANSDEL 		0x000d 	(LEGACY)
#define 	T_TCL_RESERRORRATE 	T_TCO_RESERRORRATE
#define 	TCL_RESERRORRATE 	T_TCO_RESERRORRATE 	(LEGACY)
#define 	T_TCL_PROTECTION 	T_TCO_PROTECTION
#define 	TCL_PROTECTION 		T_TCO_PROTECTION 	(LEGACY)
#define 	T_TCL_PRIORITY 		T_TCO_PRIORITY
#define 	TCL_PRIORITY 		T_TCO_PRIORITY 	(LEGACY)



/*
*  Management Options
* 
*  The values defined for these Management Options are 
*  recommended-only, not mandatory.
*/

#define 	T_TCO_LTPDU 		0x0100
#define 	TCO_LTPDU 		0x0100
#define 	T_TCO_ACKTIME 		0x0200
#define 	TCO_ACKTIME 		0x0200
#define 	T_TCO_REASTIME 		0x0300
#define 	TCO_REASTIME 		0x0300
#define 	T_TCO_EXTFORM 		0x0400
#define 	TCO_EXTFORM 		0x0400
#define 	T_TCO_FLOWCTRL 		0x0500
#define 	TCO_FLOWCTRL 		0x0500
#define 	T_TCO_CHECKSUM 		0x0600
#define 	TCO_CHECKSUM 		0x0600
#define 	T_TCO_NETEXP 		0x0700
#define 	TCO_NETEXP 		0x0700
#define 	T_TCO_NETRECPTCF 	0x0800
#define 	TCO_NETRECPTCF 		0x0800
#define 	T_TCO_PREFCLASS 	0x0900
#define 	TCO_PREFCLASS 		0x0900
#define 	T_TCO_ALTCLASS1 	0x0a00
#define 	TCO_ALTCLASS1 		0x0a00
#define 	T_TCO_ALTCLASS2 	0x0b00
#define 	TCO_ALTCLASS2 		0x0b00
#define 	T_TCO_ALTCLASS3 	0x0c00
#define 	TCO_ALTCLASS3 		0x0c00
#define 	T_TCO_ALTCLASS4 	0x0d00
#define 	TCO_ALTCLASS4 		0x0d00
#define 	T_TCL_CHECKSUM 		T_TCO_CHECKSUM
#define 	TCL_CHECKSUM 		T_TCO_CHECKSUM


/* INTERNET-SPECIFIC ENVIRONMENT */

/*
*  TCP level
* 
*  The values defined for the TCP Level are recommended-only, 
*  not mandatory.
*/
#define 	T_INET_TCP 	0x6
#define 	INET_TCP 	0x6 	(LEGACY)



/*
*  TCP-level Options
* 
*  The values defined for the TCP-level Options are recommended-only, 
*  not mandatory.
*/
#define 	T_TCP_NODELAY 		0x1 	/* don't delay packets to coalesce */
#define 	TCP_NODELAY 		0x1 	/* (LEGACY) */
#define 	T_TCP_MAXSEG 		0x2 	/* get maximum segment size */
#define 	TCP_MAXSEG 		0x2 	/* (LEGACY) */
#define 	T_TCP_KEEPALIVE 	0x8 	/* check, if connections are alive */
#define 	TCP_KEEPALIVE 		0x8 	/* (LEGACY) */


/*
* Structure used with TCP_KEEPALIVE option.
*/
struct t_kpalive {
	t_scalar_t    kp_onoff;      /* option on/off      */
	t_scalar_t    kp_timeout;    /* timeout in minutes */
};


#define 	T_GARBAGE 	0x02 	/* LEGACY */


/*
*  UDP level
* 
*  The values defined for the UDP Level are recommended-only, 
*  not mandatory.
*/
#define 	T_INET_UDP 	0x11
#define 	INET_UDP 	0x11 	(LEGACY)



/*
*  UDP-level Options
* 
*  The values defined for the UDP-level Options are recommended-only, 
*  not mandatory.
*/

#define 	T_UDP_CHECKSUM 	T_TCO_CHECKSUM 	/* checksum computation */
#define 	UDP_CHECKSUM 	T_TCO_CHECKSUM 	/* LEGACY */

/*
*  IP level
* 
*  The values defined for the IP Level are recommended-only, 
*  not mandatory.
*/

#define 	T_INET_IP 	0x0
#define 	INET_IP 	0x0 	(LEGACY)

/*
*  IP-level Options
* 
*  The values defined for the IP-level Options are recommended-only, 
*  not mandatory.
*/

#define 	T_IP_OPTIONS 	0x1 	/* IP per-packet options */
#define 	IP_OPTIONS 	0x1 	/* (LEGACY) */
#define 	T_IP_TOS 	0x2 	/* IP per-packet type of service */
#define 	IP_TOS 		0x2 	/* (LEGACY) */
#define 	T_IP_TTL 	0x3 	/* IP per-packet time to live */
#define 	IP_TTL 		0x3 	/* (LEGACY) */
#define 	T_IP_REUSEADDR 	0x4 	/* allow local address reuse */
#define 	IP_REUSEADDR 	0x4 	/* (LEGACY) */
#define 	T_IP_DONTROUTE 	0x10 	/* just use interface addresses */
#define 	IP_DONTROUTE 	0x10 	/* (LEGACY) */
#define 	T_IP_BROADCAST 	0x20 	/* permit sending of broadcast msgs */
#define 	IP_BROADCAST 	0x20 	/* (LEGACY) */



/*
* IP_TOS precedence levels
*/

#define 	T_ROUTINE 		0
#define 	T_PRIORITY 		1
#define 	T_IMMEDIATE 		2
#define 	T_FLASH 		3
#define 	T_OVERRIDEFLASH 	4
#define 	T_CRITIC_ECP 		5
#define 	T_INETCONTROL 		6
#define 	T_NETCONTROL 		7


/*
* IP_TOS type of service
*/

#define 	T_NOTOS 		0
#define 	T_LDELAY 		(1 << 4)
#define 	T_HITHRPT 		(1 << 3)
#define 	T_HIREL 		(1 << 2)
#define 	T_LOCOST 		(1 << 1)
#define 	SET_TOS(prec, tos) 	((0x7 & (prec)) << 5 | (0x1c & (tos)))


#ifdef __cplusplus
}
#endif
		


#endif


