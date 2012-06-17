/*
**	Simple FAT32 Driver
*/

#include "fat32.h"
#include "atapi.h"

#define FAT32_LBA_FIRST_SECTOR 63

struct __fat32_bootsector *fat32_bootsector;
unsigned char bootsector_read=0;

unsigned int FirstDataSector;

void fat32_read_bootsector() {
	fat32_bootsector = (struct __fat32_bootsector*) kalloc(512);
	ata_readsectors(0, FAT32_LBA_FIRST_SECTOR, 1, fat32_bootsector);
	bootsector_read=1;
}

unsigned int fat32_get_first_sector_of_cluster(unsigned int cluster) {
	return ((cluster - 2) * fat32_bootsector->sectors_per_cluster) + FirstDataSector;
}

void fat32_getfilename(char *fname, char *filename) {
	int i;

	for (i=0; i<8; i++) {
		if (fname[i] != ' ')
			filename[i]=fname[i];
		else break;	
	}
	
	filename[i]='.';	
	filename[++i]=fname[8];
	filename[++i]=fname[9];
	filename[++i]=fname[10];
	
	filename[++i]=0;
}



unsigned int fat32_get_clusternumber(unsigned short cluster_low, 
									unsigned short cluster_high) {
	return cluster_high<<8 | cluster_low;
}

void fat32_read(unsigned int cluster, char *inbuffer) {
	unsigned int first_cluster;
	
	first_cluster=fat32_get_first_sector_of_cluster(cluster);
	
	ata_readsectors(0, FAT32_LBA_FIRST_SECTOR+first_cluster, 1, inbuffer);
}

unsigned int fat32_get_nextcluster(unsigned int cluster) {
	char FAT_Table[512];
	unsigned int next_cluster;
	unsigned int FATOffset;
	
	FATOffset = (cluster*4)/512;

	ata_readsectors(0, FAT32_LBA_FIRST_SECTOR+fat32_bootsector->reserved_sectors + FATOffset,
					1, &FAT_Table);
		
	next_cluster=(*((unsigned int *) &FAT_Table[(cluster*4) % 512])) & 0x0FFFFFFF;

	return next_cluster;
}

void fat32_read_file(struct __fat32_dir_entry *entry, char *inbuffer) {
	unsigned int clusternumber = fat32_get_clusternumber(entry->cluster_low, entry->cluster_high);
	char *inbuffer_pointer=inbuffer;
	int i=0;
	
	while (clusternumber < 0x0FFFFFF8) {		
		fat32_read(clusternumber, inbuffer_pointer);
		clusternumber=fat32_get_nextcluster(clusternumber);
		inbuffer_pointer+=512;		
	}
}

void fat32_find_file(char *myfile, struct __fat32_dir_entry *entry) {
	struct __fat32_dir_entry *fat32_dir_entry=0;
	char *buffer=(char*) kalloc(512);
	char *filename=(char*) kalloc(13);
	int i=0;
	unsigned int clusternumber;
	char *buffer_pointer;
	
	if (bootsector_read == 0) fat32_read_bootsector();

	fat32_dir_entry = (struct __fat32_dir_entry*) kalloc(DIR_ENTRY_SIZE);
	
	FirstDataSector = fat32_bootsector->reserved_sectors + 
					  (fat32_bootsector->number_of_fats * fat32_bootsector->fat32_sectors_per_fat);
	/*
	ata_readsectors(0, FAT32_LBA_FIRST_SECTOR+FirstDataSector, 1, buffer);
	*/
	
	clusternumber=fat32_bootsector->rootdir_starting_cluster;
	
	while (clusternumber < 0x0FFFFFF8) {

		buffer_pointer=buffer;
		fat32_read(clusternumber, buffer_pointer);			
		clusternumber=fat32_get_nextcluster(clusternumber);
		
		for (i=0; i<FAT32_MAX_ROOTDIR_ENTRIES_PER_SECTOR*2; i++) {
			memcpy(fat32_dir_entry, buffer_pointer, DIR_ENTRY_SIZE);
			if (fat32_dir_entry->filename[0] == FAT32_LASTFILE) break;
			if ((fat32_dir_entry->attribute != ATTR_LONG_NAME) 
				&& (fat32_dir_entry->filename[0] != FAT32_DELETED)) {

					fat32_getfilename(fat32_dir_entry->filename, filename);
					if (strcmp(filename, myfile)==0) goto file_found;
			}
		
			buffer_pointer+=DIR_ENTRY_SIZE;
		}
		
	}

file_found:
			
	memcpy(entry, fat32_dir_entry, DIR_ENTRY_SIZE);

	kfree(fat32_dir_entry);
	kfree(filename);
	kfree(buffer);
}
