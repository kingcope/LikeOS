/* Derived from Etherboot 5.1 */

#include "etherboot.h"
#include "pci.h"
#include "timer.h"
//#include <lib.h>

#define DEBUG_THIS DEBUG_IDE
#include "debug.h"

#define IDE_MAX_CONTROLLERS 4
#define IDE_MAX_DRIVES (IDE_MAX_CONTROLLERS * 2)

#define BSY_SET_DURING_SPINUP 1

typedef unsigned long long sector_t;

/*
 *   UBL, The Universal Talkware Boot Loader 
 *    Copyright (C) 2000 Universal Talkware Inc.
 *    Copyright (C) 2002 Eric Biederman
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version. 
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details. 
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *
 */
struct controller {
	uint16_t cmd_base;
	uint16_t ctrl_base;
};

struct harddisk_info {
	struct controller *ctrl;
	uint16_t heads;
	uint16_t cylinders;
	uint16_t sectors_per_track;
	uint8_t  model_number[41];
	uint8_t  slave;
	sector_t sectors;
	int  address_mode;
#define ADDRESS_MODE_CHS    0
#define ADDRESS_MODE_LBA    1
#define ADDRESS_MODE_LBA48  2
#define ADDRESS_MODE_PACKET 3
	uint32_t hw_sector_size;
	unsigned drive_exists : 1;
	unsigned slave_absent : 1;
	unsigned removable : 1;
};

#define IDE_SECTOR_SIZE 0x200
#define CDROM_SECTOR_SIZE 0x800

#define IDE_BASE0             (0x1F0u) /* primary controller */
#define IDE_BASE1             (0x170u) /* secondary */
#define IDE_BASE2             (0x1E8u) /* third */
#define IDE_BASE3             (0x168u) /* fourth */
#define IDE_BASE4             (0x1E0u) /* fifth */
#define IDE_BASE5             (0x160u) /* sixth */

#define IDE_REG_EXTENDED_OFFSET   (0x204u)

#define IDE_REG_DATA(ctrl)           ((ctrl)->cmd_base + 0u) /* word register */
#define IDE_REG_ERROR(ctrl)          ((ctrl)->cmd_base + 1u)
#define IDE_REG_PRECOMP(ctrl)        ((ctrl)->cmd_base + 1u)
#define IDE_REG_FEATURE(ctrl)        ((ctrl)->cmd_base + 1u)
#define IDE_REG_SECTOR_COUNT(ctrl)   ((ctrl)->cmd_base + 2u)
#define IDE_REG_SECTOR_NUMBER(ctrl)  ((ctrl)->cmd_base + 3u)
#define IDE_REG_LBA_LOW(ctrl)        ((ctrl)->cmd_base + 3u)
#define IDE_REG_CYLINDER_LSB(ctrl)   ((ctrl)->cmd_base + 4u)
#define IDE_REG_LBA_MID(ctrl)	     ((ctrl)->cmd_base + 4u)
#define IDE_REG_CYLINDER_MSB(ctrl)   ((ctrl)->cmd_base + 5u)
#define IDE_REG_LBA_HIGH(ctrl)	     ((ctrl)->cmd_base + 5u)
#define IDE_REG_DRIVEHEAD(ctrl)      ((ctrl)->cmd_base + 6u)
#define IDE_REG_DEVICE(ctrl)	     ((ctrl)->cmd_base + 6u)
#define IDE_REG_STATUS(ctrl)         ((ctrl)->cmd_base + 7u)
#define IDE_REG_COMMAND(ctrl)        ((ctrl)->cmd_base + 7u)
#define IDE_REG_ALTSTATUS(ctrl)      ((ctrl)->ctrl_base + 2u)
#define IDE_REG_DEVICE_CONTROL(ctrl) ((ctrl)->ctrl_base + 2u)

struct ide_pio_command
{
	uint8_t feature;
	uint8_t sector_count;
	uint8_t lba_low;
	uint8_t lba_mid;
	uint8_t lba_high;
	uint8_t device;
#       define IDE_DH_DEFAULT (0xA0)
#       define IDE_DH_HEAD(x) ((x) & 0x0F)
#       define IDE_DH_MASTER  (0x00)
#       define IDE_DH_SLAVE   (0x10)
#       define IDE_DH_LBA     (0x40)
#       define IDE_DH_CHS     (0x00)
	uint8_t command;
	uint8_t sector_count2;
	uint8_t lba_low2;
	uint8_t lba_mid2;
	uint8_t lba_high2;
};

#define IDE_DEFAULT_COMMAND { 0xFFu, 0x01, 0x00, 0x0000, IDE_DH_DEFAULT }

#define IDE_ERR_ICRC	0x80	/* ATA Ultra DMA bad CRC */
#define IDE_ERR_BBK	0x80	/* ATA bad block */
#define IDE_ERR_UNC	0x40	/* ATA uncorrected error */
#define IDE_ERR_MC	0x20	/* ATA media change */
#define IDE_ERR_IDNF	0x10	/* ATA id not found */
#define IDE_ERR_MCR	0x08	/* ATA media change request */
#define IDE_ERR_ABRT	0x04	/* ATA command aborted */
#define IDE_ERR_NTK0	0x02	/* ATA track 0 not found */
#define IDE_ERR_NDAM	0x01	/* ATA address mark not found */

#define IDE_STATUS_BSY	0x80	/* busy */
#define IDE_STATUS_RDY	0x40	/* ready */
#define IDE_STATUS_DF	0x20	/* device fault */
#define IDE_STATUS_WFT	0x20	/* write fault (old name) */
#define IDE_STATUS_SKC	0x10	/* seek complete */
#define IDE_STATUS_DRQ	0x08	/* data request */
#define IDE_STATUS_CORR	0x04	/* corrected */
#define IDE_STATUS_IDX	0x02	/* index */
#define IDE_STATUS_ERR	0x01	/* error (ATA) */
#define IDE_STATUS_CHK	0x01	/* check (ATAPI) */

#define IDE_CTRL_HD15	0x08	/* bit should always be set to one */
#define IDE_CTRL_SRST	0x04	/* soft reset */
#define IDE_CTRL_NIEN	0x02	/* disable interrupts */


/* Most mandtory and optional ATA commands (from ATA-3), */

#define IDE_CMD_CFA_ERASE_SECTORS            0xC0
#define IDE_CMD_CFA_REQUEST_EXT_ERR_CODE     0x03
#define IDE_CMD_CFA_TRANSLATE_SECTOR         0x87
#define IDE_CMD_CFA_WRITE_MULTIPLE_WO_ERASE  0xCD
#define IDE_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#define IDE_CMD_CHECK_POWER_MODE1            0xE5
#define IDE_CMD_CHECK_POWER_MODE2            0x98
#define IDE_CMD_DEVICE_RESET                 0x08
#define IDE_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#define IDE_CMD_FLUSH_CACHE                  0xE7
#define IDE_CMD_FORMAT_TRACK                 0x50
#define IDE_CMD_IDENTIFY_DEVICE              0xEC
#define IDE_CMD_IDENTIFY_DEVICE_PACKET       0xA1
#define IDE_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#define IDE_CMD_IDLE1                        0xE3
#define IDE_CMD_IDLE2                        0x97
#define IDE_CMD_IDLE_IMMEDIATE1              0xE1
#define IDE_CMD_IDLE_IMMEDIATE2              0x95
#define IDE_CMD_INITIALIZE_DRIVE_PARAMETERS  0x91
#define IDE_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#define IDE_CMD_NOP                          0x00
#define IDE_CMD_PACKET                       0xA0
#define IDE_CMD_READ_BUFFER                  0xE4
#define IDE_CMD_READ_DMA                     0xC8
#define IDE_CMD_READ_DMA_QUEUED              0xC7
#define IDE_CMD_READ_MULTIPLE                0xC4
#define IDE_CMD_READ_SECTORS                 0x20
#define IDE_CMD_READ_SECTORS_EXT             0x24
#define IDE_CMD_READ_VERIFY_SECTORS          0x40
#define IDE_CMD_RECALIBRATE                  0x10
#define IDE_CMD_SEEK                         0x70
#define IDE_CMD_SET_FEATURES                 0xEF
#define IDE_CMD_SET_MAX_ADDR_EXT             0x24
#define IDE_CMD_SET_MULTIPLE_MODE            0xC6
#define IDE_CMD_SLEEP1                       0xE6
#define IDE_CMD_SLEEP2                       0x99
#define IDE_CMD_STANDBY1                     0xE2
#define IDE_CMD_STANDBY2                     0x96
#define IDE_CMD_STANDBY_IMMEDIATE1           0xE0
#define IDE_CMD_STANDBY_IMMEDIATE2           0x94
#define IDE_CMD_WRITE_BUFFER                 0xE8
#define IDE_CMD_WRITE_DMA                    0xCA
#define IDE_CMD_WRITE_DMA_QUEUED             0xCC
#define IDE_CMD_WRITE_MULTIPLE               0xC5
#define IDE_CMD_WRITE_SECTORS                0x30
#define IDE_CMD_WRITE_VERIFY                 0x3C

/* IDE_CMD_SET_FEATURE sub commands */
#define IDE_FEATURE_CFA_ENABLE_8BIT_PIO                     0x01
#define IDE_FEATURE_ENABLE_WRITE_CACHE                      0x02
#define IDE_FEATURE_SET_TRANSFER_MODE                       0x03
#define IDE_FEATURE_ENABLE_POWER_MANAGEMENT                 0x05
#define IDE_FEATURE_ENABLE_POWERUP_IN_STANDBY               0x06
#define IDE_FEATURE_STANDBY_SPINUP_DRIVE                    0x07
#define IDE_FEATURE_CFA_ENABLE_POWER_MODE1                  0x0A
#define IDE_FEATURE_DISABLE_MEDIA_STATUS_NOTIFICATION       0x31
#define IDE_FEATURE_ENABLE_AUTOMATIC_ACOUSTIC_MANAGEMENT    0x42
#define IDE_FEATURE_SET_MAXIMUM_HOST_INTERFACE_SECTOR_TIMES 0x43
#define IDE_FEATURE_DISABLE_READ_LOOKAHEAD                  0x55
#define IDE_FEATURE_ENABLE_RELEASE_INTERRUPT                0x5D
#define IDE_FEATURE_ENABLE_SERVICE_INTERRUPT                0x5E
#define IDE_FEATURE_DISABLE_REVERTING_TO_POWERON_DEFAULTS   0x66
#define IDE_FEATURE_CFA_DISABLE_8BIT_PIO                    0x81
#define IDE_FEATURE_DISABLE_WRITE_CACHE                     0x82
#define IDE_FEATURE_DISABLE_POWER_MANAGEMENT                0x85
#define IDE_FEATURE_DISABLE_POWERUP_IN_STANDBY              0x86
#define IDE_FEATURE_CFA_DISABLE_POWER_MODE1                 0x8A
#define IDE_FEATURE_ENABLE_MEDIA_STATUS_NOTIFICATION        0x95
#define IDE_FEATURE_ENABLE_READ_LOOKAHEAD                   0xAA
#define IDE_FEATURE_DISABLE_AUTOMATIC_ACOUSTIC_MANAGEMENT   0xC2
#define IDE_FEATURE_ENABLE_REVERTING_TO_POWERON_DEFAULTS    0xCC
#define IDE_FEATURE_DISABLE_SERVICE_INTERRUPT               0xDE

static unsigned short ide_base[] = {
	IDE_BASE0,
	IDE_BASE1, 
	IDE_BASE2, 
	IDE_BASE3, 
	0
};

static struct controller controllers[IDE_MAX_CONTROLLERS];
static struct harddisk_info harddisk_info[IDE_MAX_DRIVES];

static unsigned char ide_buffer[IDE_SECTOR_SIZE];

static int await_ide(int (*done)(struct controller *ctrl), 
	struct controller *ctrl, unsigned long timeout)
{
	int result;
	for(;;) {
		result = done(ctrl);
		if (result) {
			return 0;
		}
		//poll_interruptions();
		if ((timeout == 0) || (currticks() > timeout)) {
			break;
		}
	}
	printf("IDE time out\n");
	return -1;
}

/* The maximum time any IDE command can last 31 seconds,
 * So if any IDE commands takes this long we know we have problems.
 */
#define IDE_TIMEOUT (32*TICKS_PER_SEC)

static int not_bsy(struct controller *ctrl)
{
	return !(inb(IDE_REG_STATUS(ctrl)) & IDE_STATUS_BSY);
}

/* IDE drives assert BSY bit within 400 nsec when SRST is set.
 * Use 2 msec since our tick is 1 msec */
#define IDE_RESET_PULSE (2*TICKS_PER_SEC / 18)

static int bsy(struct controller *ctrl)
{
	return inb(IDE_REG_STATUS(ctrl)) & IDE_STATUS_BSY;
}

#if  !BSY_SET_DURING_SPINUP
static int timeout(struct controller *ctrl)
{
	return 0;
}
#endif

static void print_status(struct controller *ctrl)
{
	debug("IDE: status=%#x, err=%#x\n",
			inb(IDE_REG_STATUS(ctrl)), inb(IDE_REG_ERROR(ctrl)));
}

static int ide_software_reset(struct controller *ctrl)
{	
	/* Wait a little bit in case this is immediately after
	 * hardware reset.
	 */
	mdelay(2);	
	
	/* A software reset should not be delivered while the bsy bit
	 * is set.  If the bsy bit does not clear in a reasonable
	 * amount of time give up.
	 */
	debug("Waiting for ide%d to become ready for reset... ",
			ctrl - controllers);
	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		debug("failed\n");		
		return -1;
	}
	debug("ok\n");

	/* Disable Interrupts and reset the ide bus */
	outb(IDE_CTRL_HD15 | IDE_CTRL_SRST | IDE_CTRL_NIEN, 
		IDE_REG_DEVICE_CONTROL(ctrl));		
	/* If BSY bit is not asserted within 400ns, no device there */
	//if (await_ide(bsy, ctrl, currticks() + IDE_RESET_PULSE) < 0) {
	//	return -1;
	//}	
	
	outb(IDE_CTRL_HD15 | IDE_CTRL_NIEN, IDE_REG_DEVICE_CONTROL(ctrl));

	mdelay(2);

	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
		
	return 0;
}

static void pio_set_registers(
	struct controller *ctrl, const struct ide_pio_command *cmd)
{
	uint8_t device;
	/* Disable Interrupts */
	outb(IDE_CTRL_HD15 | IDE_CTRL_NIEN, IDE_REG_DEVICE_CONTROL(ctrl));

	/* Possibly switch selected device */
	device = inb(IDE_REG_DEVICE(ctrl));
	outb(cmd->device,          IDE_REG_DEVICE(ctrl));
	if ((device & (1UL << 4)) != (cmd->device & (1UL << 4))) {
		/* Allow time for the selected drive to switch,
		 * The linux ide code suggests 50ms is the right
		 * amount of time to use here.
		 */
		mdelay(50); 
	}
	outb(cmd->feature,         IDE_REG_FEATURE(ctrl));
	if (cmd->command == IDE_CMD_READ_SECTORS_EXT) {
		outb(cmd->sector_count2,   IDE_REG_SECTOR_COUNT(ctrl));
		outb(cmd->lba_low2,        IDE_REG_LBA_LOW(ctrl));
		outb(cmd->lba_mid2,        IDE_REG_LBA_MID(ctrl));
		outb(cmd->lba_high2,       IDE_REG_LBA_HIGH(ctrl));
	}
	outb(cmd->sector_count,    IDE_REG_SECTOR_COUNT(ctrl));
	outb(cmd->lba_low,         IDE_REG_LBA_LOW(ctrl));
	outb(cmd->lba_mid,         IDE_REG_LBA_MID(ctrl));
	outb(cmd->lba_high,        IDE_REG_LBA_HIGH(ctrl));
	outb(cmd->command,         IDE_REG_COMMAND(ctrl));
}


static int pio_non_data(struct controller *ctrl, const struct ide_pio_command *cmd)
{
	/* Wait until the busy bit is clear */
	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}

	pio_set_registers(ctrl, cmd);
	ndelay(400);
	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
	/* FIXME is there more error checking I could do here? */
	return 0;
}

static int pio_data_in(struct controller *ctrl, const struct ide_pio_command *cmd,
	void *buffer, size_t bytes)
{
	unsigned int status;

	/* FIXME handle commands with multiple blocks */
	/* Wait until the busy bit is clear */
	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}

	/* How do I tell if INTRQ is asserted? */
	pio_set_registers(ctrl, cmd);
	ndelay(400);
	if (await_ide(not_bsy, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
	status = inb(IDE_REG_STATUS(ctrl));
	if (!(status & IDE_STATUS_DRQ)) {
		print_status(ctrl);
		return -1;
	}
	insw(IDE_REG_DATA(ctrl), buffer, bytes/2);
	status = inb(IDE_REG_STATUS(ctrl));
	if (status & IDE_STATUS_DRQ) {
		print_status(ctrl);
		return -1;
	}
	return 0;
}

static int pio_packet(struct harddisk_info *info, int in,
	const void *packet, int packet_len,
	void *buffer, int buffer_len)
{
	unsigned int status;
	struct ide_pio_command cmd;

	memset(&cmd, 0, sizeof(cmd));

	/* Wait until the busy bit is clear */
	if (await_ide(not_bsy, info->ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}

	/* Issue a PACKET command */
	cmd.lba_mid = (uint8_t) buffer_len;
	cmd.lba_high = (uint8_t) (buffer_len >> 8);
	cmd.device = IDE_DH_DEFAULT | info->slave;
	cmd.command = IDE_CMD_PACKET;
	pio_set_registers(info->ctrl, &cmd);
	ndelay(400);
	if (await_ide(not_bsy, info->ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
	status = inb(IDE_REG_STATUS(info->ctrl));
	if (!(status & IDE_STATUS_DRQ)) {
		debug("no drq after PACKET\n");
		print_status(info->ctrl);
		return -1;
	}

	/* Send the packet */
	outsw(IDE_REG_DATA(info->ctrl), packet, packet_len/2);

	if (await_ide(not_bsy, info->ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
	status = inb(IDE_REG_STATUS(info->ctrl));
	if (buffer_len == 0) {
		if (status & IDE_STATUS_DRQ) {
			debug("drq after non-data command\n");
			print_status(info->ctrl);
			return -1;
		}
		return 0;
	}

	if (!(status & IDE_STATUS_DRQ)) {
		debug("no drq after sending packet\n");
		print_status(info->ctrl);
		return -1;
	}

	insw(IDE_REG_DATA(info->ctrl), buffer, buffer_len/2);

	status = inb(IDE_REG_STATUS(info->ctrl));
	if (status & IDE_STATUS_DRQ) {
		debug("drq after insw\n");
		print_status(info->ctrl);
		return -1;
	}
	return 0;
}

static inline int ide_read_sector_chs(
	struct harddisk_info *info, void *buffer, unsigned long sector)
{
	struct ide_pio_command cmd;
	unsigned int track;
	unsigned int offset;
	unsigned int cylinder;
			
	memset(&cmd, 0, sizeof(cmd));
	cmd.sector_count = 1;

	//debug("ide_read_sector_chs: sector= %ld.\n",sector);

	track = sector / info->sectors_per_track;
	/* Sector number */
	offset = 1 + (sector % info->sectors_per_track);
	cylinder = track / info->heads;
	cmd.lba_low = offset;
	cmd.lba_mid = cylinder & 0xff;
	cmd.lba_high = (cylinder >> 8) & 0xff;
	cmd.device = IDE_DH_DEFAULT |
		IDE_DH_HEAD(track % info->heads) |
		info->slave |
		IDE_DH_CHS;
	cmd.command = IDE_CMD_READ_SECTORS;
	return pio_data_in(info->ctrl, &cmd, buffer, IDE_SECTOR_SIZE);
}

static inline int ide_read_sector_lba(
	struct harddisk_info *info, void *buffer, unsigned long sector)
{
	struct ide_pio_command cmd;
	memset(&cmd, 0, sizeof(cmd));

	cmd.sector_count = 1;
	cmd.lba_low = sector & 0xff;
	cmd.lba_mid = (sector >> 8) & 0xff;
	cmd.lba_high = (sector >> 16) & 0xff;
	cmd.device = IDE_DH_DEFAULT |
		((sector >> 24) & 0x0f) |
		info->slave | 
		IDE_DH_LBA;
	cmd.command = IDE_CMD_READ_SECTORS;
	//debug("%s: sector= %ld, device command= 0x%x.\n",__FUNCTION__,(unsigned long) sector, cmd.device);
	return pio_data_in(info->ctrl, &cmd, buffer, IDE_SECTOR_SIZE);
}

static inline int ide_read_sector_lba48(
	struct harddisk_info *info, void *buffer, sector_t sector)
{
	struct ide_pio_command cmd;
	memset(&cmd, 0, sizeof(cmd));
	//debug("ide_read_sector_lba48: sector= %ld.\n",(unsigned long) sector);

	cmd.sector_count = 1;
	cmd.lba_low = sector & 0xff;
	cmd.lba_mid = (sector >> 8) & 0xff;
	cmd.lba_high = (sector >> 16) & 0xff;
	cmd.lba_low2 = (sector >> 24) & 0xff;
	cmd.lba_mid2 = (sector >> 32) & 0xff;
	cmd.lba_high2 = (sector >> 40) & 0xff;
	cmd.device =  info->slave | IDE_DH_LBA;
	cmd.command = IDE_CMD_READ_SECTORS_EXT;
	return pio_data_in(info->ctrl, &cmd, buffer, IDE_SECTOR_SIZE);
}

static inline int ide_read_sector_packet(
	struct harddisk_info *info, void *buffer, sector_t sector)
{
	char packet[12];
	static uint8_t cdbuffer[CDROM_SECTOR_SIZE];
	static struct harddisk_info *last_disk = 0;
	static sector_t last_sector = (sector_t) -1;
	uint8_t *buf;
	uint32_t hw_sector;

	//debug("sector=%Ld\n", sector);

	if (info->hw_sector_size == CDROM_SECTOR_SIZE) {
		buf = cdbuffer;
		hw_sector = sector >> 2;
	} else {
		buf = buffer;
		hw_sector = sector;
	}

	if (buf==buffer || info != last_disk || hw_sector != last_sector) {
		//debug("hw_sector=%u\n", hw_sector);
		memset(packet, 0, sizeof packet);
		packet[0] = 0x28; /* READ */
		packet[2] = hw_sector >> 24;
		packet[3] = hw_sector >> 16;
		packet[4] = hw_sector >> 8;
		packet[5] = hw_sector >> 0;
		packet[7] = 0;
		packet[8] = 1; /* length */

		if (pio_packet(info, 1, packet, sizeof packet,
					buf, info->hw_sector_size) != 0) {
			debug("read error\n");
			return -1;
		}
		last_disk = info;
		last_sector = hw_sector;
	}

	if (buf != buffer)
		memcpy(buffer, &cdbuffer[(sector & 3) << 9], IDE_SECTOR_SIZE);
	return 0;
}

int ide_read(int drive, sector_t sector, void *buffer)
{
	struct harddisk_info *info = &harddisk_info[drive];
	int result;

	//debug("drive=%d, sector=%ld\n",drive,(unsigned long) sector);
	/* Report the buffer is empty */
	if (sector > info->sectors) {
		return -1;
	}
	if (info->address_mode == ADDRESS_MODE_CHS) {
		result = ide_read_sector_chs(info, buffer, sector);
	}
	else if (info->address_mode == ADDRESS_MODE_LBA) {
		result = ide_read_sector_lba(info, buffer, sector);
	}
	else if (info->address_mode == ADDRESS_MODE_LBA48) {
		result = ide_read_sector_lba48(info, buffer, sector);
	}
	else if (info->address_mode == ADDRESS_MODE_PACKET) {
		result = ide_read_sector_packet(info, buffer, sector);
	}
	else {
		result = -1;
	}
	return result;
}

static int init_drive_x(struct harddisk_info *info, struct controller *ctrl,
		int slave, int drive, unsigned char *buffer, int ident_command)
{
	uint16_t* drive_info;
	struct ide_pio_command cmd;
	int i;


	info->ctrl = ctrl;
	info->heads = 0u;
	info->cylinders = 0u;
	info->sectors_per_track = 0u;
	info->address_mode = IDE_DH_CHS;
	info->sectors = 0ul;
	info->drive_exists = 0;
	info->slave_absent = 0;
	info->removable = 0;
	info->hw_sector_size = IDE_SECTOR_SIZE;
	info->slave = slave?IDE_DH_SLAVE: IDE_DH_MASTER;

	debug("Testing for hd%c\n", 'a'+drive);

	/* Select the drive that we are testing */
	outb(IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | info->slave, 
		IDE_REG_DEVICE(ctrl));
	mdelay(50);
	
	/* Test to see if the drive registers exist,
	 * In many cases this quickly rules out a missing drive.
	 */
	for(i = 0; i < 4; i++) {
		outb(0xaa + i, (ctrl->cmd_base) + 2 + i);
	}
	for(i = 0; i < 4; i++) {
		if (inb((ctrl->cmd_base) + 2 + i) != 0xaa + i) {
			return 1;
		}
	}
	for(i = 0; i < 4; i++) {
		outb(0x55 + i, (ctrl->cmd_base) + 2 + i);
	}
	for(i = 0; i < 4; i++) {
		if (inb((ctrl->cmd_base) + 2 + i) != 0x55 + i) {
			return 1;
		}
	}
	debug("Probing for hd%c\n", 'a'+drive);
	
	memset(&cmd, 0, sizeof(cmd));
	cmd.device = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | info->slave;
	cmd.command = ident_command;

	
	if (pio_data_in(ctrl, &cmd, buffer, IDE_SECTOR_SIZE) < 0) {
		/* Well, if that command didn't work, we probably don't have drive. */
		return 1;
	}
	
	/* Now suck the data out */
	drive_info = (uint16_t *)buffer;
	if (drive_info[2] == 0x37C8) {
		/* If the response is incomplete spin up the drive... */
		memset(&cmd, 0, sizeof(cmd));
		cmd.device = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS |
			info->slave;
		cmd.feature = IDE_FEATURE_STANDBY_SPINUP_DRIVE;
		if (pio_non_data(ctrl, &cmd) < 0) {
			/* If the command doesn't work give up on the drive */
			return 1;
		}
		
	}
	if ((drive_info[2] == 0x37C8) || (drive_info[2] == 0x8C73)) {
		/* The response is incomplete retry the drive info command */
		memset(&cmd, 0, sizeof(cmd));
		cmd.device = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS |
			info->slave;
		cmd.command = ident_command;
		if(pio_data_in(ctrl, &cmd, buffer, IDE_SECTOR_SIZE) < 0) {
			/* If the command didn't work give up on the drive. */
			return 1;
		}
	}
	if ((drive_info[2] != 0x37C8) &&
		(drive_info[2] != 0x738C) &&
		(drive_info[2] != 0x8C73) &&
		(drive_info[2] != 0xC837) &&
		(drive_info[2] != 0x0000)) {
		debugx("Invalid IDE Configuration: %hx\n", drive_info[2]);
		return 1;
	}
	for(i = 27; i < 47; i++) {
		info->model_number[((i-27)<< 1)] = (drive_info[i] >> 8) & 0xff;
		info->model_number[((i-27)<< 1)+1] = drive_info[i] & 0xff;
	}
	info->model_number[40] = '\0';
	info->drive_exists = 1;

	/* See if LBA is supported */
	if (ident_command == IDE_CMD_IDENTIFY_PACKET_DEVICE) {
		info->address_mode = ADDRESS_MODE_PACKET;
		info->removable = 1; /* XXX */
	} else if (drive_info[49] & (1 << 9)) {
		info->address_mode = ADDRESS_MODE_LBA;
		info->sectors = (drive_info[61] << 16) | (drive_info[60]);
//		debug("LBA mode, sectors=%Ld\n", info->sectors);
		/* Enable LBA48 mode if it is present */
		if (drive_info[83] & (1 <<10)) {
			/* Should LBA48 depend on LBA? */
			info->address_mode = ADDRESS_MODE_LBA48;
			info->sectors = 
				(((sector_t)drive_info[103]) << 48) |
				(((sector_t)drive_info[102]) << 32) |
				(((sector_t)drive_info[101]) << 16) |
				(((sector_t)drive_info[100]) <<  0);
//			debug("LBA48 mode, sectors=%Ld\n", info->sectors);
		}
	} else {
		info->address_mode = ADDRESS_MODE_CHS;
		info->heads = drive_info[3];
		info->cylinders = drive_info[1];
		info->sectors_per_track = drive_info[6];
		info->sectors = 
			info->sectors_per_track *
			info->heads *
			info->cylinders;
		debug("CHS mode, sectors_per_track=[%d], heads=[%d], cylinders=[%d]\n",
			info->sectors_per_track,
			info->heads,
			info->cylinders);
//		debug("sectors=%Ld\n", info->sectors);
	}
	/* See if we have a slave */
	if (!info->slave && (((drive_info[93] >> 14) & 3) == 1)) {
		info->slave_absent = !(drive_info[93] & (1 << 5));
	}

	/* See if we need to put the device in CFA power mode 1 */
	if ((drive_info[160] & ((1 << 15) | (1 << 13)| (1 << 12))) ==
		((1 << 15) | (1 << 13)| (1 << 12))) {
		memset(&cmd, 0, sizeof(cmd));
		cmd.device = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | info->slave;
		cmd.feature = IDE_FEATURE_CFA_ENABLE_POWER_MODE1;
		if (pio_non_data(ctrl, &cmd) < 0) {
			/* If I need to power up the drive, and I can't
			 * give up.
			 */
			debugx("Cannot power up CFA device\n");
			return 1;
		}
	}

	/* Some extra steps for older drives.. */
	if (info->address_mode != ADDRESS_MODE_PACKET) {
		/* Initialize drive parameters
		 * This is an obsolete command (disappeared as of ATA-6)
		 * but old drives need it before accessing media. */
		memset(&cmd, 0, sizeof(cmd));
		cmd.device = IDE_DH_DEFAULT | IDE_DH_HEAD(drive_info[3] - 1)
		    | info->slave;
		cmd.sector_count = drive_info[6];
		cmd.command = IDE_CMD_INITIALIZE_DRIVE_PARAMETERS;
		debug("Init device params... ");
		if (pio_non_data(ctrl, &cmd) < 0) {
			debug("failed (ok for newer drives)\n");
		}
		else{
			debug("ok\n");
		}
	}

	printf("hd%c: %s",
		'a'+drive,
		(info->address_mode==ADDRESS_MODE_CHS) ? "CHS" :
		(info->address_mode==ADDRESS_MODE_LBA) ? "LBA" :
		(info->address_mode==ADDRESS_MODE_LBA48) ? "LBA48" :
		(info->address_mode==ADDRESS_MODE_PACKET) ? "ATAPI" : "???");
#if 0
// can not pass compiler	
	if (info->sectors > (10LL*1000*1000*1000/512))
		printf(" %uGB", (unsigned) (info->sectors / (1000*1000*1000/512)));
	else if (info->sectors > (10*1000*1000/512))
		printf(" %uMB", (unsigned) (info->sectors / (1000*1000/512)));
	else if (info->sectors > 0)
		printf(" %uKB", (unsigned) (info->sectors / 2));
#endif
	printf(": %s\n", info->model_number);
	return 0;
}

/* Experimental floating bus detection
 * As Eric mentions, we get stuck when the bus has no drive
 * and floating high. To avoid this, try some heuristics.
 * This is based on a paper on Phoenix website. --ts1 */
static int ide_bus_floating(struct controller *ctrl)
{
	unsigned long timeout;
	unsigned char status;

	/* Test 1: if status reads 0xff, probably no device is present
	 * on the bus. Repeat this for 20msec. */
	timeout = currticks() + 20 * TICKS_PER_SEC / 18;
	status = 0;
	do {
		/* Take logical OR to avoid chattering */
		status |= inb(IDE_REG_STATUS(ctrl));
		/* If it makes 0xff, it's possible to be floating, 
		 * do test2 to ensure. */
		if (status == 0xff)
			goto test2;
		/* If BSY bit is not set, it's harmless to continue probing. */
		if ((status & IDE_STATUS_BSY) == 0)
			return 0;
	} while (currticks() < timeout);
	/* Timed out. Logical ORed status didn't make 0xFF.
	 * We have something there. */
	return 0;

test2:
	/* Test 2: write something to registers, then read back and 
	 * compare. Note that ATA spec inhibits this while BSY is set,
	 * but for many drives this works. This is a confirmation step anyway.
	 */
	outb(0xaa, ctrl->cmd_base + 2);
	outb(0x55, ctrl->cmd_base + 3);
	outb(0xff, ctrl->cmd_base + 4);
	if (inb(ctrl->cmd_base+2) == 0xaa
			&& inb(ctrl->cmd_base+3) == 0x55
			&& inb(ctrl->cmd_base+4) == 0xff) {
		/* We have some registers there. 
		 * Though this does not mean it is not a NIC or something... */
		return 0;
	}

	/* Status port is 0xFF, and other registers are not there.
	 * Most certainly this bus is floating. */
	debugx("Detected floating bus\n");
	return 1;
}

static int init_controller(struct controller *ctrl, int drive, unsigned char *buffer) 
{
	struct harddisk_info *info;

	/* Put the drives ide channel in a know state and wait
	 * for the drives to spinup.  
	 *
	 * In practice IDE disks tend not to respond to commands until
	 * they have spun up.  This makes IDE hard to deal with
	 * immediately after power up, as the delays can be quite
	 * long, so we must be very careful here.
	 *
	 * There are two pathological cases that must be dealt with:
	 *
	 * - The BSY bit not being set while the IDE drives spin up.
	 *   In this cases only a hard coded delay will work.  As
	 *   I have not reproduced it, and this is out of spec for
	 *   IDE drives the work around can be enabled by setting
	 *   BSY_SET_DURING_SPINUP to 0.
	 *
	 * - The BSY bit floats high when no drives are plugged in.
	 *   This case will not be detected except by timing out but
	 *   we avoid the problems by only probing devices we are
	 *   supposed to boot from.  If we don't do the probe we
	 *   will not experience the problem.
	 *
	 * So speed wise I am only slow if the BSY bit is not set
	 * or not reported by the IDE controller during spinup, which
	 * is quite rare.
	 * 
	 */
#if !BSY_SET_DURING_SPINUP
	if (await_ide(timeout, ctrl, currticks() + IDE_TIMEOUT) < 0) {
		return -1;
	}
#endif
	/* ts1: Try some heuristics to avoid waiting for floating bus */
	if (ide_bus_floating(ctrl))
		return -1;

	if (ide_software_reset(ctrl) < 0) {
		return -1;
	}

	/* Note: I have just done a software reset.  It may be
	 * reasonable to just read the boot time signatures 
	 * off of the drives to see if they are present.
	 *
	 * For now I will go with just sending commands to the drives
	 * and assuming filtering out missing drives by detecting registers
	 * that won't set and commands that fail to execute properly.
	 */

	/* Now initialize the individual drives */
	info = &harddisk_info[drive];
	init_drive_x(info, ctrl, 0, drive, buffer, IDE_CMD_IDENTIFY_DEVICE);

	if (!info->drive_exists)
		init_drive_x(info, ctrl, 0, drive, buffer,
				IDE_CMD_IDENTIFY_PACKET_DEVICE);
	if (info->drive_exists && !info->slave_absent) {
		drive++;
		info++;
		init_drive_x(info, ctrl, 1, drive, buffer,
				IDE_CMD_IDENTIFY_DEVICE);
		if (!info->drive_exists)
			init_drive_x(info, ctrl, 1, drive, buffer,
					IDE_CMD_IDENTIFY_PACKET_DEVICE);
	}

	return 0;
}

static int
atapi_request_sense(struct harddisk_info *info, uint8_t *asc, uint8_t *ascq)
{
	uint8_t packet[12];
	uint8_t buf[18];
	int i;

	memset(packet, 0, sizeof packet);
	packet[0] = 0x03; /* REQUEST SENSE */
	packet[4] = sizeof buf;
	if (pio_packet(info, 1, packet, sizeof packet, buf, sizeof buf) != 0)
		return -1;

	for (i = 0; i < sizeof buf; i++)
		debug("%02x ", buf[i]);
	debug("\n");

	if (asc)
		*asc = buf[12];
	if (ascq)
		*ascq = buf[13];
	return 0;
}

static int atapi_detect_medium(struct harddisk_info *info)
{
	uint8_t packet[12];
	uint8_t buf[8];
	uint32_t block_len, sectors;
	unsigned long timeout;
	uint8_t asc, ascq;
	int in_progress;

	memset(packet, 0, sizeof packet);
	packet[0] = 0x25; /* READ CAPACITY */

	/* Retry READ CAPACITY for 5 seconds unless MEDIUM NOT PRESENT
	 * is reported by the drive. If the drive reports "IN PROGRESS",
	 * 30 seconds is added. */
	timeout = currticks() + 5*TICKS_PER_SEC;
	in_progress = 0;
	while (currticks() < timeout) {
		if (pio_packet(info, 1, packet, sizeof packet, buf, sizeof buf)
				== 0)
			goto ok;

		if (atapi_request_sense(info, &asc, &ascq) == 0) {
			if (asc == 0x3a) { /* MEDIUM NOT PRESENT */
				debug("Device reports MEDIUM NOT PRESENT\n");
				return -1;
			}

			if (asc == 0x04 && ascq == 0x01 && !in_progress) {
					/* IN PROGRESS OF BECOMING READY */
				printf("Waiting for drive to detect "
						"the medium... ");
				/* Allow 30 seconds more */
				timeout = currticks() + 30*TICKS_PER_SEC;
				in_progress = 1;
			}
		}

		mdelay(100);
	}
	debug("read capacity failed\n");
	return -1;
ok:

	block_len = (uint32_t) buf[4] << 24
		| (uint32_t) buf[5] << 16
		| (uint32_t) buf[6] << 8
		| (uint32_t) buf[7] << 0;
	debug("block_len=%u\n", block_len);
	if (block_len != IDE_SECTOR_SIZE && block_len != CDROM_SECTOR_SIZE) {
		debugx("Unsupported sector size %u\n", block_len);
		return -1;
	}
	info->hw_sector_size = block_len;

	sectors = (uint32_t) buf[0] << 24
		| (uint32_t) buf[1] << 16
		| (uint32_t) buf[2] << 8
		| (uint32_t) buf[3] << 0;

	debug("sectors=%u\n", sectors);
	if (info->hw_sector_size == CDROM_SECTOR_SIZE)
		sectors <<= 2; /* # of sectors in 512-byte "soft" sector */
	if (sectors != info->sectors)
		printf("%dMB medium detected\n", sectors>>(20-9));
	info->sectors = sectors;
	return 0;
}

static int detect_medium(struct harddisk_info *info)
{
	if (info->address_mode == ADDRESS_MODE_PACKET) {
		if (atapi_detect_medium(info) != 0)
			return -1;
	} else {
		debug("not implemented for non-ATAPI device\n");
		return -1;
	}
	return 0;
}

static int find_ide_controller_compat(struct controller *ctrl, int index)
{
	if (index >= IDE_MAX_CONTROLLERS)
		return -1;
	ctrl->cmd_base  = ide_base[index];
	ctrl->ctrl_base = ide_base[index] + IDE_REG_EXTENDED_OFFSET;
	return 0;
}

static int find_ide_controller(struct controller *ctrl, int ctrl_index)
{
	int pci_index;
	struct pci_device *dev;
	unsigned int mask;
	uint32_t x;

	/* A PCI IDE controller has two channels (pri, sec) */
	pci_index = ctrl_index >> 1;

	/* Find a IDE storage class device */
	dev = pci_find_device_2(-1, -1, 0x0101, 0x0180, -1, pci_index);
	if (!dev) {
		debug("PCI IDE #%d not found\n", pci_index);
		return -1;
	}

	debug("found PCI IDE controller %04x:%04x prog_if=%#x\n",
			dev->vendor, dev->dev_id, ((dev->class>>8) & 0xff));

	/* See how this controller is configured */
	mask = (ctrl_index & 1) ? 4 : 1;
	debug("%s channel: ", (ctrl_index & 1) ? "secodary" : "primary");
	if ( (((dev->class>>8) & 0xff) & mask) || ((dev->class>>16)!= 0x0101)) { // 0x0180 and other must use native PCI mode	
		debug("native PCI mode\n");
		if ((ctrl_index & 1) == 0) {
			
			/* Primary channel */
			 pci_read_config_dword(dev, PCI_BASE_ADDRESS_0,&x);
			ctrl->cmd_base = x;
			pci_read_config_dword(dev, PCI_BASE_ADDRESS_1,&x);
			ctrl->ctrl_base = x;
		} else {
			/* Secondary channel */
                         pci_read_config_dword(dev, PCI_BASE_ADDRESS_2,&x);
                        ctrl->cmd_base = x;
                        pci_read_config_dword(dev, PCI_BASE_ADDRESS_3,&x);
                        ctrl->ctrl_base = x;
		}
		ctrl->cmd_base &= ~3;
		ctrl->ctrl_base &= ~3;
	} else {
		debug("compatibility mode\n");
		if (find_ide_controller_compat(ctrl, ctrl_index) != 0)
			return -1;
	}

	debug("cmd_base=%#x ctrl_base=%#x\n", ctrl->cmd_base, ctrl->ctrl_base);
#if 0
	debug("cmd+0=%0#x\n", inb(ctrl->cmd_base+0));
	debug("cmd+1=%0#x\n", inb(ctrl->cmd_base+1));
	debug("cmd+2=%0#x\n", inb(ctrl->cmd_base+2));
	debug("cmd+3=%0#x\n", inb(ctrl->cmd_base+3));
	debug("cmd+4=%0#x\n", inb(ctrl->cmd_base+4));
	debug("cmd+5=%0#x\n", inb(ctrl->cmd_base+5));
	debug("cmd+6=%0#x\n", inb(ctrl->cmd_base+6));
	debug("cmd+7=%0#x\n", inb(ctrl->cmd_base+7));
	debug("ctrl+0=%0#x\n", inb(ctrl->ctrl_base+0));
	debug("ctrl+1=%0#x\n", inb(ctrl->ctrl_base+1));
	debug("ctrl+2=%0#x\n", inb(ctrl->ctrl_base+2));
	debug("ctrl+3=%0#x\n", inb(ctrl->ctrl_base+3));
#endif
	return 0;
}

int ide_probe(int drive)
{
	struct controller *ctrl;
	int ctrl_index;
	struct harddisk_info *info;
	
	if (drive >= IDE_MAX_DRIVES) {
		debugx("Unsupported drive number\n");
		return -1;
	}

	/* A controller has two drives (master, slave) */
	ctrl_index = drive >> 1;

	ctrl = &controllers[ctrl_index];
	if (ctrl->cmd_base == 0) {
		if (find_ide_controller_compat(ctrl, ctrl_index) != 0) {
			debugx("IDE channel %d not found\n", ctrl_index);
			return -1;
		}
		if (init_controller(ctrl, drive & ~1, ide_buffer) != 0) {
			printf("No drive detected on IDE channel %d\n",
					ctrl_index);
			return -1;
		}
	}

	info = &harddisk_info[drive];
	if (!info->drive_exists) {
		printf("Drive %d does not exist\n", drive);
		return -1;
	}

	if (info->removable) {
		if (detect_medium(info) != 0) {
			printf("Media detection failed\n");
			return -1;
		}
	}

	return 0;
}

/* vim:set sts=8 sw=8: */
