#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H


#include <inttypes.h>



typedef uint32_t	socklen_t;
typedef unsigned int	sa_family_t;

struct sockaddr
{
	sa_family_t	sa_family;
	char		sa_data[];
};


struct msghdr
{
	void		*msg_name;
	socklen_t	msg_namelen;
	struct iovec	*msg_iov;
	int		msg_iovlen;
	void		*msg_control;
	socklen_t	msg_controllen;
	int		msg_flags;
};


struct cmsghdr
{
	socklen_t	cmsg_len;
	int		cmsg_level;
	int		cmsg_type;
};

/*
Ancillary data consists of a sequence of pairs, each consisting of a cmsghdr structure followed by a data array. The data array contains the ancillary data message, and the cmsghdr structure contains descriptive information that allows an application to correctly parse the data.

The values for cmsg_level will be legal values for the level argument to the getsockopt() and setsockopt() functions. The system documentation should specify the cmsg_type definitions for the supported protocols.

Ancillary data is also possible at the socket level. The <sys/socket.h> header defines the following macro for use as the cmsg_type value when cmsg_level is SOL_SOCKET:

SCM_RIGHTS
Indicates that the data array contains the access rights to be sent or received. 
    The <sys/socket.h> header defines the following macros to gain access to the data arrays in the ancillary data associated with a message header:
    CMSG_DATA(cmsg)
If the argument is a pointer to a cmsghdr structure, this macro returns an unsigned character pointer to the data array associated with the cmsghdr structure. 
CMSG_NXTHDR(mhdr,cmsg)
	    If the first argument is a pointer to a msghdr structure and the second argument is a pointer to a cmsghdr structure in the ancillary data, pointed to by the msg_control field of that msghdr structure, this macro returns a pointer to the next cmsghdr structure, or a null pointer if this structure is the last cmsghdr in the ancillary data. 
CMSG_FIRSTHDR(mhdr)
	        If the argument is a pointer to a msghdr structure, this macro returns a pointer to the first cmsghdr structure in the ancillary data associated with this msghdr structure, or a null pointer if there is no ancillary data associated with the msghdr structure. 
		The <sys/socket.h> header defines the linger structure that includes at least the following members:
*/

struct linger
{
	int         l_onoff;
	int         l_linger;
};

/*
The <sys/socket.h> header defines the following macros, with distinct integral values:
SOCK_DGRAM
Datagram socket 
SOCK_STREAM
Byte-stream socket 
SOCK_SEQPACKET
Sequenced-packet socket 
			    The <sys/socket.h> header defines the following macro for use as the level argument of setsockopt() and getsockopt().
			    SOL_SOCKET
Options to be accessed at socket level, not protocol level. 
				The <sys/socket.h> header defines the following macros, with distinct integral values, for use as the option_name argument in getsockopt() or setsockopt() calls:
				SO_ACCEPTCONN
Socket is accepting connections. 
SO_BROADCAST
Transmission of broadcast messages is supported. 
SO_DEBUG
Debugging information is being recorded. 
SO_DONTROUTE
bypass normal routing 
SO_ERROR
Socket error status. 
SO_KEEPALIVE
Connections are kept alive with periodic messages. 
SO_LINGER
Socket lingers on close. 
SO_OOBINLINE
Out-of-band data is transmitted in line. 
SO_RCVBUF
Receive buffer size. 
SO_RCVLOWAT
receive "low water mark" 
SO_RCVTIMEO
receive timeout 
SO_REUSEADDR
Reuse of local addresses is supported. 
SO_SNDBUF
Send buffer size. 
SO_SNDLOWAT
send "low water mark" 
SO_SNDTIMEO
send timeout 
SO_TYPE
Socket type. 
												The <sys/socket.h> header defines the following macros, with distinct integral values, for use as the valid values for the msg_flags field in the msghdr structure, or the flags parameter in recvfrom(), recvmsg(), sendto() or sendmsg() calls:
MSG_CTRUNC
Control data truncated. 
MSG_DONTROUTE
Send without using routing tables. 
MSG_EOR
Terminates a record (if supported by the protocol). 
MSG_OOB
Out-of-band data. 
MSG_PEEK
Leave received data in queue. 
MSG_TRUNC
Normal data truncated. 
MSG_WAITALL
Wait for complete message. 
															    The <sys/socket.h> header defines the following macros, with distinct integral values:
															    AF_UNIX
UNIX domain sockets 
AF_UNSPEC
Unspecified 
AF_INET
Internet domain sockets 
																	The <sys/socket.h> header defines the following macros, with distinct integral values:
																	SHUT_RD
Disables further receive operations. 
SHUT_WR
Disables further send operations. 
SHUT_RDWR
Disables further send and receive operations. 

*/

#ifdef __cplusplus
extern "C" {
#endif
		


int     accept(int socket, struct sockaddr *address, socklen_t *address_len);
int     bind(int socket, const struct sockaddr *address, socklen_t address_len);
int     connect(int socket, const struct sockaddr *address, socklen_t address_len);
int     getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
int     getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
int     getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);
int     listen(int socket, int backlog);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len);
ssize_t recvmsg(int socket, struct msghdr *message, int flags);
ssize_t send(int socket, const void *message, size_t length, int flags);
ssize_t sendmsg(int socket, const struct msghdr *message, int flags);
ssize_t sendto(int socket, const void *message, size_t length, int flags,
const struct sockaddr *dest_addr, socklen_t dest_len);
int     setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
int     shutdown(int socket, int how);
int     socket(int domain, int type, int protocol);
int     socketpair(int domain, int type, int protocol, int socket_vector[2]);


#ifdef __cplusplus
}
#endif
		


#endif


