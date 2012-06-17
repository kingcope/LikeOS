/* The Multiboot information. */
typedef struct aout_symbol_table
{
  ULONG tabsize;
  ULONG strsize;
  ULONG addr;
  ULONG reserved;
} aout_symbol_table_t;

/* The section header table for ELF. */
typedef struct elf_section_header_table
{
  ULONG num;
  ULONG size;
  ULONG addr;
  ULONG shndx;
} elf_section_header_table_t;


typedef struct multiboot_info
{
  ULONG flags			;
  ULONG mem_lower		;
  ULONG mem_upper		;
  unsigned char boot_device1;
  unsigned char boot_device2;
  unsigned char boot_device3;
  unsigned char boot_device_drive;
  ULONG cmdline		;
  ULONG mods_count		;
  ULONG mods_addr		;
  union
  {
     aout_symbol_table_t aout_sym;
     elf_section_header_table_t elf_sec;
  } u;
  ULONG mmap_length		;
  ULONG mmap_addr		;
  ULONG drives_length		;
  ULONG drives_addr		;
  ULONG config_table		;
  ULONG boot_loader_name	;
  ULONG apm_table		;
  ULONG vbe_control_info	;
  ULONG vbe_mode_info		;
  ULONG vbe_mode		;
  ULONG vbe_interface_seg	;
  ULONG vbe_interface_off	;
  ULONG vbe_interface_len	;
} multiboot_info_t;


/* VBE controller information.  */
struct vbe_controller
{
  unsigned char signature[4];
  unsigned short version;
  unsigned long oem_string;
  unsigned long capabilities;
  unsigned long video_mode;
  unsigned short total_memory;
  unsigned short oem_software_rev;
  unsigned long oem_vendor_name;
  unsigned long oem_product_name;
  unsigned long oem_product_rev;
  unsigned char reserved[222];
  unsigned char oem_data[256];
} __attribute__ ((packed));

/* VBE mode information.  */
struct vbe_mode
{
  unsigned short mode_attributes;
  unsigned char win_a_attributes;
  unsigned char win_b_attributes;
  unsigned short win_granularity;
  unsigned short win_size;
  unsigned short win_a_segment;
  unsigned short win_b_segment;
  unsigned long win_func;
  unsigned short bytes_per_scanline;

  /* >=1.2 */
  unsigned short x_resolution;
  unsigned short y_resolution;
  unsigned char x_char_size;
  unsigned char y_char_size;
  unsigned char number_of_planes;
  unsigned char bits_per_pixel;
  unsigned char number_of_banks;
  unsigned char memory_model;
  unsigned char bank_size;
  unsigned char number_of_image_pages;
  unsigned char reserved0;

  /* direct color */
  unsigned char red_mask_size;
  unsigned char red_field_position;
  unsigned char green_mask_size;
  unsigned char green_field_position;
  unsigned char blue_mask_size;
  unsigned char blue_field_position;
  unsigned char reserved_mask_size;
  unsigned char reserved_field_position;
  unsigned char direct_color_mode_info;

  /* >=2.0 */
  unsigned long phys_base;
  unsigned long reserved1;
  unsigned short reversed2;

  /* >=3.0 */
  unsigned short linear_bytes_per_scanline;
  unsigned char banked_number_of_image_pages;
  unsigned char linear_number_of_image_pages;
  unsigned char linear_red_mask_size;
  unsigned char linear_red_field_position;
  unsigned char linear_green_mask_size;
  unsigned char linear_green_field_position;
  unsigned char linear_blue_mask_size;
  unsigned char linear_blue_field_position;
  unsigned char linear_reserved_mask_size;
  unsigned char linear_reserved_field_position;
  unsigned long max_pixel_clock;

  unsigned char reserved3[189];
} __attribute__ ((packed));
