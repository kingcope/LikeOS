#ifndef FS_H
#define FS_H

//#include <stdint.h>

typedef unsigned long long sector_t;

int ide_probe(int drive);
int ide_read(int drive, sector_t sector, void *buffer);

#define DISK_IDE 1
#define DISK_MEM 2
#define DISK_USB 3

int devopen(int drive, int *reopen);
int devread(unsigned long sector, unsigned long byte_offset,
	unsigned long byte_len, void *buf);

int file_open(const char *filename);
int file_read(void *buf, unsigned long len);
int file_seek(unsigned long offset);
unsigned long file_size(void);

#define PARTITION_UNKNOWN 0xbad6a7

int open_eltorito_image(int part, unsigned long *start, unsigned long *length);

extern int using_devsize;

#endif /* FS_H */
