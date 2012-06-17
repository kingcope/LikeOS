#ifndef __ATAPI_H
#define __ATAPI_H

struct __ata_info {
	unsigned char general_info[2];
	unsigned short number_of_logical_cylinders;
	unsigned short reserved1;
	unsigned short number_of_logical_heads;
	unsigned short number_of_unformatted_bytes_for_track;
	unsigned short number_of_unformatted_bytes_per_sector;
	unsigned short number_of_logical_sectors_per_track;
	unsigned short reserved_and_bytes_in_phase_lock;
	unsigned short inter_sector_gap;
	unsigned short number_of_vendor;
	unsigned char serial_number[20];
	unsigned short controller_type;
	unsigned short buffer_size;
	unsigned short number_of_ecc_bytes;
	unsigned char firmware_revision[8];
	unsigned char model_number[40];
	unsigned short read_write_multiples_implemented;
	unsigned short supports_double_word_io_transfer;
	unsigned short reserved2;
	unsigned short reserved3;
	unsigned short minimum_pio_data_transfer_cycle_time;
	unsigned short minimum_dma_data_transfer_cycle_time;
	unsigned short all_words_past_this_point_are_reserver;	
} __attribute__ ((packed));

struct __ata_info *ata_info;
void interrupt_ata();
void interrupt_ata2();
void ata_readsector(unsigned char drive, int sector);
void ata_identify();
void ata_findcdrom();
#endif
