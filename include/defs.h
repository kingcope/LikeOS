#ifndef __DEFS_H
#define __DEFS_H

#define ULONG unsigned long
#define ULONG64 unsigned long long
#define UINT unsigned int
#define UCHAR unsigned char
#define USHORT unsigned short
#define ulong unsigned long
#define uint unsigned int
#define uchar unsigned char
#define ushort unsigned short
#define ulong64 unsigned long long

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TRUE
#define TRUE 0
#endif

#ifndef FALSE
#define FALSE 1
#endif

#define SCREENWIDTH 1024
#define SCREENHEIGHT 768
#define PACKED_STRUCT __attribute__ ((packed));
#endif
