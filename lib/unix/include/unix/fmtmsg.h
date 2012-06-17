#ifndef _FMTMSG_H
#define _FMTMSG_H

#define		MM_HARD			(1<<0)	/* Source of the condition is hardware.  */
#define		MM_SOFT			(1<<1)	/* Source of the condition is software.  */
#define		MM_FIRM			(1<<2)	/* Source of the condition is firmware.  */
#define		MM_APPL			(1<<3)	/* Condition detected by application.  */
#define		MM_UTIL			(1<<4)	/* Condition detected by utility.  */
#define		MM_OPSYS		(1<<5)	/* Condition detected by operating system.  */
#define		MM_RECOVER		(1<<6)	/* Recoverable error.  */
#define		MM_NRECOV		(1<<7)	/* Non-recoverable error.  */
#define		MM_HALT			(1<<8)	/* Error causing application to halt.  */
#define		MM_ERROR		(1<<9)	/* Application has encountered a non-fatal fault.  */
#define		MM_WARNING		(1<<10)	/* Application has detected unusual non-error condition.  */
#define		MM_INFO			(1<<11)	/* Informative message.  */
#define		MM_NOSEV		(1<<12)	/* No severity level provided for the message.  */
#define		MM_PRINT		(1<<13)	/* Display message on standard error.  */
#define		MM_CONSOLE		(1<<14)	/* Display message on system console.  */


#define		MM_OK				0	/* The function succeeded.  */
#define		MM_NOTOK			-1	/* The function failed completely.  */
#define		MM_NOMSG			-2	/* The function was unable to generate a message on standard error, but otherwise succeeded.  */
#define		MM_NOCON			-3	/* The function was unable to generate a console message, but otherwise succeeded.  */


#ifdef __cplusplus
extern "C" {
#endif
		

int fmtmsg(long, const char*, int, const char*, const char*, const char*);

#ifdef __cplusplus
}
#endif
		



#endif

