#ifndef __STDIO_H
#define __STDIO_H
#define WHITE_TXT 0x07 // white on black text
extern void k_clear_screen();
//extern unsigned int k_printf(char *message, unsigned int line);


#define FILE struct _iobuf
#define handle_t int

struct _iobuf
{
  char *ptr;
  int cnt;
  char *base;
  int flag;
  handle_t file;
  int charbuf;
  int bufsiz;
  char *tmpfname;
};


#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif
