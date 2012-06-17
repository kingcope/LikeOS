#ifndef __FAT12_H
#define __FAT12_H

struct fat12_header {
   unsigned char NameVer[11];
   unsigned short bytes_per_sector;
   unsigned char sec_per_cluster;
   unsigned short res_sectors;
   unsigned char fats_amount;
   unsigned short rootdir_entries;
   unsigned short sec_in_vol;
   unsigned char media_desc;
   unsigned short sec_per_fat;
   unsigned short sec_per_track;
   unsigned short heads_no;
   unsigned int hidden_sectors;
} __attribute__ ((packed));

struct fat12_directory {
   unsigned char filename[11];
   unsigned char attribute;
   unsigned char reserved[10];
   unsigned short time;
   unsigned short date;
   unsigned short entry_cluster;
   unsigned int filesize;
} __attribute__ ((packed));

struct fat12_directory_clean {
   unsigned char filename[13];
   unsigned char attribute;
   unsigned char reserved[10];
   unsigned short time;
   unsigned short date;
   unsigned short entry_cluster;
   unsigned int filesize;
} __attribute__ ((packed));

#define ATTR_LONG_NAME 0x0f
#define ATTR_DIRECTORY 0x10
#define FAT12_DELETED 0xE5
#define FAT12_LASTFILE 0x00
#define FAT12_MAXFILES 224

/*
TODO:

struct fat12_lfn {	// Lange Dateinamen
   unsigned char lfnpart; //Bits 0-5 give the LFN part number, bit 6 is set if this is the last entry for the file.
   unsigned char letters1[10];  //1st 5 letters of LFN entry, Unicode
   unsigned char reserved;
   unsigned char checksum;
   unsigned char letters2[12]; // Next 6 letters of LFN entry. Also Unicode.
   unsigned char zero[2];
   unsigned char letters3[4]; // Last 2 letters of LFN entry. Also Unicode.
} __attribute__ ((packed));
*/

struct CHS {
   unsigned short sector;
   unsigned short cylinder;
   unsigned short head;
};

int fat12_getrootdir(struct fat12_directory_clean dir_clean[FAT12_MAXFILES]);

#endif
