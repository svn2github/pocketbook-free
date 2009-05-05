/*
 *  linux/include/linux/mtd/nand.h
 *
 *  Copyright (c) 2000 David Woodhouse <dwmw2@mvhi.com>
 *                     Steven J. Hill <sjhill@cotw.com>
 *		       Thomas Gleixner <gleixner@autronix.de>
 *
 * $Id: nand.h,v 1.1.1.1 2004/02/04 12:57:56 laputa Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Info:
 *   Contains standard defines and IDs for NAND flash devices
 *
 *  Changelog:
 *   01-31-2000 DMW     Created
 *   09-18-2000 SJH     Moved structure out of the Disk-On-Chip drivers
 *			so it can be used by other NAND flash device
 *			drivers. I also changed the copyright since none
 *			of the original contents of this file are specific
 *			to DoC devices. David can whack me with a baseball
 *			bat later if I did something naughty.
 *   10-11-2000 SJH     Added private NAND flash structure for driver
 *   10-24-2000 SJH     Added prototype for 'nand_scan' function
 *   10-29-2001 TG	changed nand_chip structure to support 
 *			hardwarespecific function for accessing control lines
 *   02-21-2002 TG	added support for different read/write adress and
 *			ready/busy line access function
 *   02-26-2002 TG	added chip_delay to nand_chip structure to optimize
 *			command delay times for different chips
 */
#ifndef __LINUX_MTD_NAND_H
#define __LINUX_MTD_NAND_H

#include <linux/config.h>
#include <linux/sched.h>

/*
 * Searches for a NAND device
 */
extern int nand_scan (struct mtd_info *mtd);
extern int smc_scan (struct mtd_info *mtd);

/*
 * Constants for hardware specific CLE/ALE/NCE function
*/
#define NAND_CTL_SETNCE 	1
#define NAND_CTL_CLRNCE		2
#define NAND_CTL_SETCLE		3
#define NAND_CTL_CLRCLE		4
#define NAND_CTL_SETALE		5
#define NAND_CTL_CLRALE		6
#define NAND_CTL_DAT_IN         7
#define NAND_CTL_DAT_OUT        8

#define SMC_OOB_USER           0
#define SMC_OOB_DAT_FLAG       4
#define SMC_OOB_BLK_FLAG       5
#define SMC_OOB_ADDR1          6
#define SMC_OOB_ECC2           8
#define SMC_OOB_ADDR2          11
#define SMC_OOB_ECC1           13
#define SMC_OOB_SIZE           16

#define SMC_STAT_WRITE_ERR     0x01    /* 1: Error in Program/Erase */
#define SMC_STAT_READY         0x40    /* 0: Busy, 1: Ready */
#define SMC_STAT_NOT_WP        0x80    /* 0: Protected, 1: Not */

#define SMC_OOB256_SIZE        8
#define SMC_OOB256_ECC1        (SMC_OOB_ECC1 - SMC_OOB256_SIZE)
#define SMC_OOB256_ECC2        (SMC_OOB_ECC2 - SMC_OOB256_SIZE)

struct nand_smc_dev {
	unsigned long CpV;      /* Cylinder/Volume */
	unsigned long HpC;      /* Head/Cylinder */
	unsigned long SpH;      /* Sector/Head */
	unsigned long allS;     /* Number of Sectors */
	unsigned long szS;      /* Sector Size */
	unsigned long PBpV;     /* Number of Physical Blocks in Volume */
	unsigned long LBpV;     /* Number of Logical Blocks in Volume */
	unsigned long SpB;      /* Sector/Block */
	unsigned long PpB;      /* Page/Block */
	unsigned long szP;      /* Page Size */
};

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/*
 * Enumeration for NAND flash chip state
 */
typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING
} nand_state_t;


/*
 * NAND Private Flash Chip Data
 *
 * Structure overview:
 *
 *  IO_ADDR_R - address to read the 8 I/O lines of the flash device 
 *
 *  IO_ADDR_W - address to write the 8 I/O lines of the flash device 
 *
 *  hwcontrol - hardwarespecific function for accesing control-lines
 *
 *  dev_ready - hardwarespecific function for accesing device ready/busy line
 *
 *  chip_lock - spinlock used to protect access to this structure
 *
 *  wq - wait queue to sleep on if a NAND operation is in progress
 *
 *  state - give the current state of the NAND device
 *
 *  page_shift - number of address bits in a page (column address bits)
 *
 *  data_buf - data buffer passed to/from MTD user modules
 *
 *  data_cache - data cache for redundant page access and shadow for
 *		 ECC failure
 *
 *  ecc_code_buf - used only for holding calculated or read ECCs for
 *                 a page read or written when ECC is in use
 *
 *  reserved - padding to make structure fall on word boundary if
 *             when ECC is in use
 */
struct nand_chip {
#ifdef CONFIG_MTD_NANDY
	void (*hwcontrol)(int cmd);
	void (*write_cmd)(u_char val);
	void (*write_addr)(u_char val);
	u_char (*read_data)(void);
	void (*write_data)(u_char val);
	void (*wait_for_ready)(void);
	spinlock_t chip_lock;
	wait_queue_head_t wq;
	nand_state_t state;
	int page_shift;
	u_char *data_buf;
	u_char *data_cache;
	int	cache_page;
	struct nand_smc_dev *dev;
	u_char spare[SMC_OOB_SIZE];
#else	/* CONFIG_MTD_NANDY */
	unsigned long IO_ADDR_R;
	unsigned long IO_ADDR_W;
	void (*hwcontrol)(int cmd);
	int (*dev_ready)(void);
	int chip_delay;
	spinlock_t chip_lock;
	wait_queue_head_t wq;
	nand_state_t state;
	int page_shift;
	u_char *data_buf;
	u_char *data_cache;
	int	cache_page;
#ifdef CONFIG_MTD_NAND_ECC
	u_char ecc_code_buf[6];
	u_char reserved[2];
#endif
#endif	/* CONFIG_MTD_NANDY */
};

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec

/*
 * NAND Flash Device ID Structure
 *
 * Structure overview:
 *
 *  name - Complete name of device
 *
 *  manufacture_id - manufacturer ID code of device.
 *
 *  model_id - model ID code of device.
 *
 *  chipshift - total number of address bits for the device which
 *              is used to calculate address offsets and the total
 *              number of bytes the device is capable of.
 *
 *  page256 - denotes if flash device has 256 byte pages or not.
 *
 *  pageadrlen - number of bytes minus one needed to hold the
 *               complete address into the flash array. Keep in
 *               mind that when a read or write is done to a
 *               specific address, the address is input serially
 *               8 bits at a time. This structure member is used
 *               by the read/write routines as a loop index for
 *               shifting the address out 8 bits at a time.
 *
 *  erasesize - size of an erase block in the flash device.
 */
struct nand_flash_dev {
	char * name;
	int manufacture_id;
	int model_id;
	int chipshift;
	char page256;
	char pageadrlen;
	unsigned long erasesize;
};

#endif /* __LINUX_MTD_NAND_H */
