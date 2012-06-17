#ifndef __FAT32_H
#define __FAT32_H

struct __fat32_bootsector {
	char jmp_code[3];
	char oem_string[8];
	unsigned short bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned short reserved_sectors;
	unsigned char number_of_fats;
	unsigned short max_rootdir_entries;
	unsigned short reserved1;
	unsigned char media_descriptor;
	unsigned short reserved2;
	unsigned short sectors_per_track;
	unsigned short total_heads;
	unsigned int num_hidden_sectors;
	unsigned int num_sectors;
	unsigned int fat32_sectors_per_fat;
	unsigned short active_fat;
	unsigned short revision_number;
	unsigned int rootdir_starting_cluster;
	unsigned short filesystem_information_sector;
	unsigned short bootsector_copy_sector;
	unsigned char reserved3[12];
	unsigned char drive_number;
	unsigned char reserved4;	
	unsigned char signature;
	unsigned int serial_number;
	unsigned char volume_label[11];
	unsigned char fs_id[8];
	unsigned char machine_code[8];
	unsigned char boot_signature[2];
} __attribute__ ((packed));

/* 32 bytes */
struct __fat32_dir_entry {	
	unsigned char filename[8];
	unsigned char extension[3];
	unsigned char attribute;
	
	unsigned char reserved1;
	unsigned char creation_thenth_seconds;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short date_last_accessed;
	unsigned short cluster_high;		// high word of cluster number
	
	unsigned short lastwrite_time;
	unsigned short lastwrite_date;
	unsigned short cluster_low;		// starting cluster
	unsigned int filesize;
} __attribute__ ((packed));

#define DIR_ENTRY_SIZE 32

void fat32_read_bootsector();

#define ATTR_LONG_NAME 0x0f
#define ATTR_DIRECTORY 0x10
#define FAT32_DELETED 0xE5
#define FAT32_LASTFILE 0x00
#define FAT32_MAX_ROOTDIR_ENTRIES_PER_SECTOR 16

#endif
