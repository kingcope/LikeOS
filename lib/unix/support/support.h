#ifndef _STDIO_SUPPORT_H
#define _STDIO_SUPPORT_H

#include <stdarg.h>
#include <stdio.h>

int  supcon_leni(int num);
int  supcon_lenui(unsigned int p);
int  supcon_lenp(unsigned int p);
int  supcon_lenx( unsigned int  p );
void  supcon_puts( unsigned char *c );
void  supcon_putc( unsigned char c );
void  supcon_puti( int num );
void  supcon_putui( unsigned int num );
void  supcon_putp( unsigned int num, char offset );
void  supcon_putx( unsigned int num );
void  supcon_putX( unsigned int num );

int  bufcon_leni(int num);
int  bufcon_lenui(unsigned int p);
int  bufcon_lenp(unsigned int p);
int  bufcon_lenx(unsigned int p);
int  bufcon_lens( char *s );

char*  bufcon_putc( char *buffer, unsigned char c );
char*  bufcon_puts( char *buffer, char *str );
char*  bufcon_puti( char *buffer, int num );
char*  bufcon_putui( char *buffer, unsigned int num );
char*  bufcon_putp( char *buffer, unsigned int p );
char*  bufcon_putx( char *buffer, unsigned int p );
char*  bufcon_putX( char *buffer, unsigned int p );


int printf_buffer(int i);

int 	support_vfprintf(FILE* stream, const char* format, va_list ap);
int 	support_vsprintf(char* buffer, const char* format, va_list ap);


// ---------- File support -----------------
int   support_init_files( int count );
void  support_shutdown_files(void);
FILE* support_retrieve_file( int fd );
FILE* support_remove_file( int fd );
int   support_insert_file( FILE* stream );

unsigned int support_fmodes2flags( const char *mode );

#endif
   
