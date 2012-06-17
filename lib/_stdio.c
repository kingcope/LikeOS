#include "defs.h"
#include "stdio.h"
#include "fat32.h"

#define NULL 0

void k_clear_screen() // clear the entire text screen
{
	char *vidmem = (char *) 0xb8000;
	UINT i=0;
	while(i < (80*25*2))
	{
		vidmem[i]=' ';
		i++;
		vidmem[i]=WHITE_TXT;
		i++;
	};
};

UINT k_printf(char *message, UINT line) // the message and then the line #
{
	char *vidmem = (char *) 0xb8000;
	UINT i=0;

	i=(line*80*2);

	while(*message!=0)
	{
		if(*message=='\n') // check for a new line
		{
			line++;
			i=(line*80*2);
			*message++;
		} else {
			vidmem[i]=*message;
			*message++;
			i++;
			vidmem[i]=WHITE_TXT;
			i++;
		};
	};

	return(1);
};

char *itoa (int v, char *s, int r) {
	int i,neg = 0;
	char *p = s;
	char *q = s;

	if (r < 0 || r > 35) {
		*s = 0;
		return (s);
		}
	if (r == 0) r = 10;
	if (v == 0) {
		*p++ = '0';
		*p = 0;
		return (s);
		}
	if (v < 0) {
		neg = 1;
		v = -v;
		}
	while (v > 0) {
		i = v % r;
		if (i > 9) i += 7;
		*p++ = '0' + i;
		v /= r;
		}
	if (neg) *p++ = '-';
	*p-- = 0;
	q = s;
	while (p > q) {
		i = *q;
		*q++ = *p;
		*p-- = i;
		}
	return (s);
	}

print_integer(int zahl, int l) {
	char debug[255];
	itoa(zahl,debug,10);
	k_printf(debug, l);
}


/* File functions */

/*
struct _iobuf 
{
  char *ptr;
  int cnt;
  char *base;
  int flag;
  handle_t file
  int charbuf;
  int bufsiz;
  char *tmpfname;
};
*/

int open_filehandles=0;

FILE *fopen(char *path, char *mode) {
FILE *f=(FILE*) kalloc(sizeof(FILE));

	if ((strcmp(mode, "r")==0) || (strcmp(mode, "r+")==0) || (strcmp(mode, "rb")==0)) {
		struct __fat32_dir_entry *entry = (struct __fat32_dir_entry*) kalloc(DIR_ENTRY_SIZE);
		fat32_find_file(path, entry);
		
		open_filehandles++;
		f->file=open_filehandles;
		f->bufsiz=entry->filesize;
		f->ptr=(char*) kalloc(entry->filesize+512);
		f->base=f->ptr;
		f->cnt=0;
		
		fat32_read_file(entry, f->ptr);
		
		return f;
	} else {
		return 0;	
	}

}

int fclose(FILE *f) {
	open_filehandles--;
	kfree(f->ptr);
	kfree(f);
}

unsigned int fread( void *ptr, unsigned int size, unsigned int nmemb, FILE *stream) {
	memcpy(ptr, stream->ptr, size*nmemb);
	
	return (size*nmemb);
} 

int fseek(FILE *stream, long offset, int whence) {

	if (whence == SEEK_SET) {
		stream->ptr = stream->base + offset;
		stream->cnt = offset;	
	}
	
	if (whence == SEEK_CUR) {
		stream->ptr += offset;
		stream->cnt += offset;
	}
		
	if (whence == SEEK_END) {
		stream->ptr = stream->base + stream->bufsiz + offset;
		stream->cnt = stream->bufsiz + offset;
	}
	
	return 0;	
}

long ftell(FILE *stream) {
	return stream->cnt;	
}
