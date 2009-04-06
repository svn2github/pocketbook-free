/*
 *  drivers/mtd/nand.c
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@cotw.com)
 *
 *  10-29-2001  Thomas Gleixner (gleixner@autronix.de)
 * 		- Changed nand_chip structure for controlline function to
 *		support different hardware structures (Access to
 *		controllines ALE,CLE,NCE via hardware specific function. 
 *		- exit out of "failed erase block" changed, to avoid
 *		driver hangup
 *		- init_waitqueue_head added in function nand_scan !!
 *
 *  01-30-2002  Thomas Gleixner (gleixner@autronix.de)
 *		change in nand_writev to block invalid vecs entries
 *
 *  02-11-2002  Thomas Gleixner (gleixner@autronix.de)
 *		- major rewrite to avoid duplicated code
 *		  common nand_write_page function  
 *		  common get_chip function 
 *		- added oob_config structure for out of band layouts
 *		- write_oob changed for partial programming
 *		- read cache for faster access for subsequent reads
 *		from the same page.
 *		- support for different read/write address
 *		- support for device ready/busy line
 *		- read oob for more than one page enabled
 *
 *  02-27-2002	Thomas Gleixner (gleixner@autronix.de)
 *		- command-delay can be programmed
 *		- fixed exit from erase with callback-function enabled
 *
 * $Id: nand.c,v 1.1.1.1 2004/02/04 12:56:25 laputa Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This is the generic MTD driver for NAND flash devices. It should be
 *   capable of working with almost all NAND chips currently available.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ids.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#ifdef CONFIG_MTD_NAND_ECC
#include <linux/mtd/nand_ecc.h>
#endif

/*
 * Macros for low-level register control
 */
#define nand_select()	this->hwcontrol(NAND_CTL_SETNCE); \
			nand_command(mtd, NAND_CMD_RESET, -1, -1); \
			udelay (this->chip_delay);

#define nand_deselect() this->hwcontrol(NAND_CTL_CLRNCE);

/*
 * Definition of the out of band configuration structure
 */
struct nand_oob_config {
	int	ecc_pos[6];	/* position of ECC bytes inside oob */
	int	badblock_pos;	/* position of bad block flag inside oob -1 = inactive */
	int	eccvalid_pos;	/* position of ECC valid flag inside oob -1 = inactive */
};

/*
* Constants for oob configuration
*/
#define NAND_NOOB_ECCPOS0	0
#define NAND_NOOB_ECCPOS1	1
#define NAND_NOOB_ECCPOS2	2
#define NAND_NOOB_ECCPOS3	3
#define NAND_NOOB_ECCPOS4	4
#define NAND_NOOB_ECCPOS5	5
#define NAND_NOOB_BADBPOS	-1
#define NAND_NOOB_ECCVPOS	-1

#define NAND_JFFS2_OOB_ECCPOS0		0
#define NAND_JFFS2_OOB_ECCPOS1		1
#define NAND_JFFS2_OOB_ECCPOS2		2
#define NAND_JFFS2_OOB_ECCPOS3		3
#define NAND_JFFS2_OOB_ECCPOS4		6
#define NAND_JFFS2_OOB_ECCPOS5		7
#define NAND_JFFS2_OOB_BADBPOS		5
#define NAND_JFFS2_OOB_ECCVPOS		4

static	struct nand_oob_config oob_config;
/*
 * NAND low-level MTD interface functions
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *ecc_code);
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf);
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf);
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *ecc_code);
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf);
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen);
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr);
static void nand_sync (struct mtd_info *mtd);
static int nand_write_page(struct mtd_info *mtd, struct nand_chip *this,
			int page, int col, int last, u_char *ecc_code);




/*
 * Send command to NAND device
 */
static void nand_command (struct mtd_info *mtd, unsigned command,
				int column, int page_addr)
{
	read_write_clock(1);
	
	register struct nand_chip *this = mtd->priv;
	register unsigned long NAND_IO_ADDR = this->IO_ADDR_W;

	/* Begin command latch cycle */
	this->hwcontrol(NAND_CTL_SETCLE);

	/*
	 * Write out the command to the device.
	 */
	if (command != NAND_CMD_SEQIN)	
		writeb (command, NAND_IO_ADDR);
	else {
		if (mtd->oobblock == 256 && column >= 256) {
			column -= 256;
			writeb(NAND_CMD_RESET, NAND_IO_ADDR);
			writeb(NAND_CMD_READOOB, NAND_IO_ADDR);
			writeb(NAND_CMD_SEQIN, NAND_IO_ADDR);
		}
		else if (mtd->oobblock == 512 && column >= 256) {
			if (column < 512) {
				column -= 256;
				writeb(NAND_CMD_READ1, NAND_IO_ADDR);
				writeb(NAND_CMD_SEQIN, NAND_IO_ADDR);
			}
			else {
				column -= 512;
				writeb(NAND_CMD_READOOB, NAND_IO_ADDR);
				writeb(NAND_CMD_SEQIN, NAND_IO_ADDR);
			}
		}
		else {
			writeb(NAND_CMD_READ0, NAND_IO_ADDR);
			writeb(NAND_CMD_SEQIN, NAND_IO_ADDR);
		}
	}

	/* Set ALE and clear CLE to start address cycle */
	this->hwcontrol(NAND_CTL_CLRCLE);
	
	if (column != -1 || page_addr != -1)
		this->hwcontrol(NAND_CTL_SETALE);

	/* Serially input address */
	if (column != -1)
		writeb (column, NAND_IO_ADDR);
	if (page_addr != -1) {
		writeb ((unsigned char) (page_addr & 0xff), NAND_IO_ADDR);
		writeb ((unsigned char) ((page_addr >> 8) & 0xff), NAND_IO_ADDR);
		/* One more address cycle for higher density devices */
		if (mtd->size & 0x0c000000) {
			writeb ((unsigned char) ((page_addr >> 16) & 0x0f),
					NAND_IO_ADDR);
		}
	}
	/* Latch in address */
	if (column != -1 || page_addr != -1)
		this->hwcontrol(NAND_CTL_CLRALE);

	/* Pause for 15us */
	udelay (this->chip_delay);

}

/*
 *	Get chip for selected access
 */
static inline void nand_get_chip(struct nand_chip *this,int new_state, int *erase_state) {

	DECLARE_WAITQUEUE(wait, current);

	/* Grab the lock and see if the device is available */
retry:
	spin_lock_bh (&this->chip_lock);

	switch (this->state) {
	case FL_READY:
		this->state = new_state;
		if (new_state != FL_ERASING)
			spin_unlock_bh (&this->chip_lock);
		break;

	case FL_ERASING:
		if (new_state == FL_READING) {
			this->state = FL_READING;
			*erase_state = 1;
			spin_unlock_bh (&this->chip_lock);
			break;
		}

	default:
		set_current_state (TASK_UNINTERRUPTIBLE);
		add_wait_queue (&this->wq, &wait);
		spin_unlock_bh (&this->chip_lock);
		schedule();

		remove_wait_queue (&this->wq, &wait);
		goto retry;
	};
}

/*
 *	Nand_page_program function is used for write and writev !
 */
static int nand_write_page(struct mtd_info *mtd, struct nand_chip *this,
		int page,int col, int last, u_char *ecc_code) {

	int	i, status;
#ifdef CONFIG_MTD_NAND_ECC
	int ecc_bytes = (mtd->oobblock == 512) ? 6 : 3;
#endif
	/* pad oob area */
	for(i=mtd->oobblock;i < mtd->oobblock+mtd->oobsize; i++)
		this->data_buf[i] = 0xff;

#ifdef CONFIG_MTD_NAND_ECC
	/* Zero out the ECC array */
	for (i=0 ; i < 6 ; i++)
		ecc_code[i] = 0x00;
	
	/* Read back previous written data, if col > 0 */
	if(col) {
		nand_command (mtd, NAND_CMD_READ0, col, page);
		for (i=0 ; i < col ; i++)
			this->data_buf[i] = readb (this->IO_ADDR_R); 
	}
		
	/* Calculate and write the ECC if we have enough data */
	if ((col < mtd->eccsize) && (last >= mtd->eccsize)) {
		nand_calculate_ecc (&this->data_buf[0],	&(ecc_code[0]));
		for (i=0 ; i<3 ; i++)
			this->data_buf[(mtd->oobblock + oob_config.ecc_pos[i])] =
				ecc_code[i];
		if (oob_config.eccvalid_pos != -1)		
			this->data_buf[mtd->oobblock + oob_config.eccvalid_pos] = 0xf0;		
	}

	/* Calculate and write the second ECC if we have enough data */
	if ((mtd->oobblock == 512) && (last == mtd->oobblock)) {
		nand_calculate_ecc (&this->data_buf[256],&(ecc_code[3]));
		for (i=3 ; i<6 ; i++)
			this->data_buf[(mtd->oobblock + oob_config.ecc_pos[i])] =
				ecc_code[i];
		if (oob_config.eccvalid_pos != -1)		
			this->data_buf[mtd->oobblock + oob_config.eccvalid_pos] &= 0x0f;		
	}
#endif
	/* Prepad for partial page programming !!! */	
	for (i=0 ; i < col ; i++)
		this->data_buf[i] = 0xff; 	

	/* Postpad for partial page programming !!! oob is already padded */	
	for (i=last ; i < mtd->oobblock ; i++)
		this->data_buf[i] = 0xff; 	

	/* Send command to begin auto page programming */
	nand_command (mtd, NAND_CMD_SEQIN, 0x00, page);

	/* Write out complete page of data */
	for (i=0 ; i < (mtd->oobblock + mtd->oobsize) ; i++)
		writeb (this->data_buf[i], this->IO_ADDR_W);

	/* Send command to actually program the data */
	nand_command (mtd, NAND_CMD_PAGEPROG, -1, -1);

	/*
	 * Wait for program operation to complete. This could
	 * take up to 3000us (3ms) on some devices, so we try
	 * and exit as quickly as possible.
	 */
	status = 0;
	for (i=0 ; i<24 ; i++) {
		/* Delay for 125us */
		udelay (125);
		/* Check the status */
		if (this->dev_ready) 
			if (!this->dev_ready())
				continue;

		nand_command (mtd, NAND_CMD_STATUS, -1, -1);
		status = (int) readb (this->IO_ADDR_R);
		if (status & 0x40)
			break;
	}
	/* See if device thinks it succeeded */
	if (status & 0x01) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_ecc: " \
			"Failed write, page 0x%08x, " \
			"%6i bytes were succesful\n", page, *retlen);
		return -EIO;
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/*
	 * The NAND device assumes that it is always writing to
	 * a cleanly erased page. Hence, it performs its internal
	 * write verification only on bits that transitioned from
	 * 1 to 0. The device does NOT verify the whole page on a
	 * byte by byte basis. It is possible that the page was
	 * not completely erased or the page is becoming unusable
	 * due to wear. The read with ECC would catch the error
	 * later when the ECC page check fails, but we would rather
	 * catch it early in the page write stage. Better to write
	 * no data than invalid data.
	 */
	
	/* Send command to read back the page */
	if (col < mtd->eccsize)
		nand_command (mtd, NAND_CMD_READ0, col, page);
	else
		nand_command (mtd, NAND_CMD_READ1, col - 256, page);

	/* Loop through and verify the data */
	for (i=col ; i < last ; i++) {
		if (this->data_buf[i] != readb (this->IO_ADDR_R)) {
			DEBUG (MTD_DEBUG_LEVEL0,
				"nand_write_ecc: " \
				"Failed write verify, page 0x%08x, " \
				"%6i bytes were succesful\n",
				page, *retlen);
			return -EIO;
		}
	}

#ifdef CONFIG_MTD_NAND_ECC
	/*
	 * We also want to check that the ECC bytes wrote
	 * correctly for the same reasons stated above.
	 */
	nand_command (mtd, NAND_CMD_READOOB, 0x00, page);
	for (i=0 ; i < mtd->oobsize ; i++)
		this->data_buf[i] = readb(this->IO_ADDR_R);
	for (i=0 ; i < ecc_bytes ; i++) {
		if ( (this->data_buf[(oob_config.ecc_pos[i])] != ecc_code[i]) && ecc_code[i]) {
			DEBUG (MTD_DEBUG_LEVEL0,
				"nand_write_ecc: Failed ECC write " \
				"verify, page 0x%08x, " \
				"%6i bytes were succesful\n",
				page, i);
			return -EIO;
		}
	}
#endif
#endif
	return 0;
}

/*
 * NAND read
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
#ifdef CONFIG_MTD_NAND_ECC
	struct nand_chip *this = mtd->priv;
	
	return nand_read_ecc (mtd, from, len, retlen, buf, this->ecc_code_buf);
#else
	return nand_read_ecc (mtd, from, len, retlen, buf, NULL);
#endif
}

/*
 * NAND read with ECC
 */
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *ecc_code)
{
	int 	j, col, page;
	int 	erase_state = 0;
	int	ecc_status = 0, ecc_failed = 0;
	struct 	nand_chip *this = mtd->priv;
	u_char	*data_poi;
#ifdef CONFIG_MTD_NAND_ECC
	u_char ecc_calc[6];
#endif

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_read_ecc: from = 0x%08x, len = %i\n", (unsigned int) from,
		(int) len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_read_ecc: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_READING,&erase_state);

	/* First we calculate the starting page */
	page = from >> this->page_shift;

	/* Get raw starting column */
	col = from & (mtd->oobblock - 1);

	/* Initialize return value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select ();

	/* Loop until all data read */
	while (*retlen < len) {

#ifdef CONFIG_MTD_NAND_ECC
		
		/* Do we have this page in cache ? */
		if (this->cache_page == page)
			goto readdata;

		/* Send the read command */
		nand_command (mtd, NAND_CMD_READ0, 0x00, page);
		/* Read in a page + oob data*/
		for (j=0 ; j < mtd->oobblock + mtd->oobsize ; j++) 
			this->data_buf[j] = readb (this->IO_ADDR_R);
		nand_command (mtd, NAND_CMD_READ0, 0x00, page);
		
		/* copy data into cache, for read out of cache and if ecc fails */
		if (this->data_cache)
			memcpy(this->data_cache,this->data_buf,mtd->oobblock+mtd->oobsize);
		
		/* Pick the ECC bytes out of the oob data*/
		for (j=0 ; j < 6 ; j++)
			ecc_code[j] = this->data_buf[(mtd->oobblock + oob_config.ecc_pos[j])];
	
		/* Calculate the ECC and verify it */
		/* If block was not written with ECC, skip ECC */
		if (oob_config.eccvalid_pos != -1 && 
			(this->data_buf[mtd->oobblock+oob_config.eccvalid_pos] & 0x0f) != 0x0f ) {	
			
			nand_calculate_ecc (&this->data_buf[0],	&ecc_calc[0]);
			switch (nand_correct_data (&this->data_buf[0],&ecc_code[0], &ecc_calc[0])) {
			case -1:
				DEBUG (MTD_DEBUG_LEVEL0,"nand_read_ecc: " \
					"Failed ECC read, page 0x%08x\n", page);
				ecc_failed++;
				break;
			case  1:
			case  2:/* transfer ECC corrected data to cache */	
				memcpy(this->data_cache,this->data_buf,256);
				break;
			}	
		}

		if (oob_config.eccvalid_pos != -1 && 
			mtd->oobblock == 512 &&			
			(this->data_buf[mtd->oobblock+oob_config.eccvalid_pos] & 0xf0) != 0xf0 ) {	

			nand_calculate_ecc (&this->data_buf[256],&ecc_calc[3]);
			switch (nand_correct_data (&this->data_buf[256],&ecc_code[3], &ecc_calc[3])) {
			case -1:
				DEBUG (MTD_DEBUG_LEVEL0,"nand_read_ecc: " \
					"Failed ECC read, page 0x%08x\n", page);
				ecc_failed++;
				break;
			case  1:
			case  2:/* transfer ECC corrected data to cache */	
				if (this->data_cache)
					memcpy(&this->data_cache[256],&this->data_buf[256],256);
				break;
			}	
		}
readdata:
		/* Read the data from ECC data buffer into return buffer */
		data_poi = (this->data_cache) ? this->data_cache : this->data_buf;
		data_poi += col; 
		if ((*retlen + (mtd->oobblock - col)) >= len) {
			while (*retlen < len)
				buf[(*retlen)++] = *data_poi++;
		}
		else {
			for (j=col ; j < mtd->oobblock ; j++)
				buf[(*retlen)++] = *data_poi++;
		}
		/* Set cache page address, invalidate, if ecc_failed */
		this->cache_page = (this->data_cache && !ecc_failed) ? page : -1;
		
		ecc_status += ecc_failed;
		ecc_failed = 0;
		
#else
		/* Send the read command */
		nand_command (mtd, NAND_CMD_READ0, col, page);

		/* Read the data directly into the return buffer */ 
		if ((*retlen + (mtd->oobblock - col)) >= len) {
			while (*retlen < len)
				buf[(*retlen)++] = readb (this->IO_ADDR_R);
			/* We're done */
			continue;
		}
		else
			for (j=col ; j < mtd->oobblock; j++)
				buf[(*retlen)++] = readb (this->IO_ADDR_R);
#endif
		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		page++;
	}

	/* De-select the NAND device */
	nand_deselect ();

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	if (erase_state)
		this->state = FL_ERASING;
	else
		this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);
	
	/*
	 * Return success, if no ECC failures, else -EIO
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EIO
	 */
	return ecc_status ? -EIO : 0;
}

/*
 * NAND read out-of-band
 */
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf)
{
	int i, col, page;
	int erase_state = 0;
	struct nand_chip *this = mtd->priv;
	
	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_read_oob: from = 0x%08x, len = %i\n", (unsigned int) from,
		(int) len);

	/* Shift to get page */
	page = ((int) from) >> this->page_shift;

	/* Mask to get column */
	col = from & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_read_oob: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}
	
	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_READING,&erase_state);

	/* can we read out of cache ? */
	if (this->cache_page == page && (col+len <= mtd->oobsize)) {	
		/* Read the data */
		memcpy(buf,&this->data_cache[mtd->oobblock+col],len);
	} else {

		/* Select the NAND device */
		nand_select ();
	
		/* Send the read command */
		nand_command (mtd, NAND_CMD_READOOB, col, page);	
		/* 
		 * Read the data, if we read more than one page
		 * oob data, let the device transfer the data !
		 */
		for (i = 0 ; i < len ; i++) {
			buf[i] = readb (this->IO_ADDR_R);
			if ( (col++ & (mtd->oobsize-1)) == (mtd->oobsize-1) )
				udelay(this->chip_delay);	
		}
		/* De-select the NAND device */
		nand_deselect ();
	}

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	if (erase_state)
		this->state = FL_ERASING;
	else
		this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	/* Return happy */
	*retlen = len;
	return 0;
}

/*
 * NAND write
 */
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
#ifdef CONFIG_MTD_NAND_ECC
	struct nand_chip *this = mtd->priv;
	
	return nand_write_ecc (mtd, to, len, retlen, buf, this->ecc_code_buf);
#else
	return nand_write_ecc (mtd, to, len, retlen, buf, NULL);
#endif
}

/*
 * NAND write with ECC
 */
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *ecc_code)
{
	int i, page, col, cnt, ret = 0;
	struct nand_chip *this = mtd->priv;

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_write_ecc: to = 0x%08x, len = %i\n", (unsigned int) to,
		(int) len);

	/* Do not allow write past end of device */
	if ((to + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_oob: Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_WRITING,NULL);

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Get the starting column */
	col = to & (mtd->oobblock - 1);

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select ();

	/* Check the WP bit */
	nand_command (mtd, NAND_CMD_STATUS, -1, -1);
	if (!(readb (this->IO_ADDR_R) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_ecc: Device is write protected!!!\n");
		ret = -EIO;
		goto out;
	}

	/* Loop until all data is written */
	while (*retlen < len) {
		/* Invalidate cache, if we write to this page */
		if (this->cache_page == page)
			this->cache_page = -1;	

		/* Write data into buffer */
		if ((col + len) >= mtd->oobblock)
			for(i=col, cnt=0 ; i < mtd->oobblock ; i++, cnt++)
				this->data_buf[i] = buf[(*retlen + cnt)];
		else
			for(i=col, cnt=0 ; cnt < (len - *retlen) ; i++, cnt++)
				this->data_buf[i] = buf[(*retlen + cnt)];
		
		/* We use the same function for write and writev !) */
		ret = nand_write_page(mtd,this,page,col,i,ecc_code);
		if (ret) 
			goto out;
		
		/* Next data start at page boundary */
		col = 0;

		/* Update written bytes count */
		*retlen += cnt;

		/* Increment page address */
		page++;
	}

	/* Return happy */
	*retlen = len;

out:
	/* De-select the NAND device */
	nand_deselect ();

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	return ret;
}

/*
 * NAND write out-of-band
 */
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf)
{
	int i, column, page, status, ret = 0;
	struct nand_chip *this = mtd->priv;
	
	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_write_oob: to = 0x%08x, len = %i\n", (unsigned int) to,
		(int) len);

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Mask to get column */
	column = to & 0x1f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow write past end of page */
	if ((column + len) > mtd->oobsize) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_oob: Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_WRITING,NULL);

	/* Select the NAND device */
	nand_select ();

	/* Check the WP bit */
	nand_command (mtd, NAND_CMD_STATUS, -1, -1);
	if (!(readb (this->IO_ADDR_R) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_oob: Device is write protected!!!\n");
		ret = -EIO;
		goto out;	
	}

	/* Invalidate cache, if we write to this page */
	if (this->cache_page == page)
		this->cache_page = -1;	

	/* Write ones for partial page programming */
	for (i=mtd->oobblock ; i < (mtd->oobblock + mtd->oobsize) ; i++)
		this->data_buf[i] = 0xff;

	/* Transfer data */
	for (i = 0; i< len; i++) 
		this->data_buf[i+mtd->oobblock+column] = buf[i];
		
	/* Write out desired data */
	nand_command (mtd, NAND_CMD_SEQIN, mtd->oobblock, page);
	for (i=0 ; i<mtd->oobsize ; i++)
		writeb (this->data_buf[i+mtd->oobblock], this->IO_ADDR_W);

	/* Send command to program the OOB data */
	nand_command (mtd, NAND_CMD_PAGEPROG, -1, -1);

	/*
	 * Wait for program operation to complete. This could
	 * take up to 3000us (3ms) on some devices, so we try
	 * and exit as quickly as possible.
	 */
	status = 0;
	for (i=0 ; i<24 ; i++) {
		/* Delay for 125us */
		udelay (125);
		/* Check the status */
		if (this->dev_ready) 
			if (!this->dev_ready())
				continue;

		/* Check the status */
		nand_command (mtd, NAND_CMD_STATUS, -1, -1);
		status = (int) readb (this->IO_ADDR_R);
		if (status & 0x40)
			break;
	}

	/* See if device thinks it succeeded */
	if (status & 0x01) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_write_oob: " \
			"Failed write, page 0x%08x\n", page);
		ret = -EIO;
		goto out;	
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	nand_command (mtd, NAND_CMD_READOOB, column, page);

	/* Loop through and verify the data */
	for (i=0 ; i<len ; i++) {
		if (buf[i] != readb (this->IO_ADDR_R)) {
			DEBUG (MTD_DEBUG_LEVEL0,
				"nand_write_oob: " \
				"Failed write verify, page 0x%08x\n", page);
			ret = -EIO;
			goto out;	
		}
	}
#endif
	/* Return happy */
	*retlen = len;

out:
	/* De-select the NAND device */
	nand_deselect ();

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	return ret;
}

/*
 * NAND write with iovec
 */
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen)
{
	int i, page, col, cnt, len, total_len, ret = 0;
	struct nand_chip *this = mtd->priv;

	/* Calculate total length of data */
	total_len = 0;
	for (i=0 ; i < count ; i++)
		total_len += (int) vecs[i].iov_len;

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_writev: to = 0x%08x, len = %i, count = %ld\n", (unsigned int) to,
			(unsigned int) total_len, count);

	/* Do not allow write past end of page */
	if ((to + total_len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_writev: Attempted write past end of device\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_WRITING,NULL);

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Get the starting column */
	col = to & (mtd->oobblock - 1);

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select ();

	/* Check the WP bit */
	nand_command (mtd, NAND_CMD_STATUS, -1, -1);
	if (!(readb (this->IO_ADDR_R) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_writev: Device is write protected!!!\n");
		ret = -EIO;
		goto out;
	}

	/* Loop until all iovecs' data has been written */
	cnt = col;
	len = 0;
	while (count) {
		/* Invalidate cache, if we write to this page */
		if (this->cache_page == page)
			this->cache_page = -1;	

		/* Do any need pre-fill for partial page programming */
		for (i=0 ; i < cnt ; i++)
			this->data_buf[i] = 0xff;

		/*
		 * Read data out of each tuple until we have a full page
		 * to write or we've read all the tuples.
		 */
		 
		while ((cnt < mtd->oobblock) && count) {
			if (vecs->iov_base!=NULL && vecs->iov_len) {
				this->data_buf[cnt++] = 
					((u_char *) vecs->iov_base)[len++];
			}
			if (len >= (int) vecs->iov_len) {
				vecs++;
				len = 0;
				count--;
			}
		}
		
		/* We use the same function for write and writev !) */
		ret = nand_write_page(mtd,this,page,col,cnt,this->ecc_code_buf);
		if (ret)
			goto out;

		/* Update written bytes count */
		*retlen += (cnt - col);

		/* Reset written byte counter and column */
		col = cnt = 0;

		/* Increment page address */
		page++;
	}

out:
	/* De-select the NAND device */
	nand_deselect ();

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	/* Return happy */
	return ret;
}

/*
 * NAND erase a block
 */
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	int i, page, len, status, pages_per_block, ret;
	struct nand_chip *this = mtd->priv;
	DECLARE_WAITQUEUE(wait, current);

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_erase: start = 0x%08x, len = %i\n",
		(unsigned int) instr->addr, (unsigned int) instr->len);

	/* Start address must align on block boundary */
	if (instr->addr & (mtd->erasesize - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len & (mtd->erasesize - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	nand_get_chip(this,FL_ERASING,NULL);

	/* Shift to get first page */
	page = (int) (instr->addr >> this->page_shift);

	/* Calculate pages in each block */
	pages_per_block = mtd->erasesize / mtd->oobblock;

	/* Select the NAND device */
	nand_select ();

	/* Check the WP bit */
	nand_command (mtd, NAND_CMD_STATUS, -1, -1);
	if (!(readb (this->IO_ADDR_R) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Device is write protected!!!\n");
		nand_deselect ();
		this->state = FL_READY;
		spin_unlock_bh (&this->chip_lock);
		return -EIO;
	}

	/* Loop through the pages */
	len = instr->len;
	
	instr->state = MTD_ERASING;

	while (len) {
		if (oob_config.badblock_pos != -1) {
			/* Check if we have a bad block, we do not erase bad blocks !*/
			nand_command (mtd, NAND_CMD_READOOB, oob_config.badblock_pos, page);
			if ( readb(this->IO_ADDR_R) != 0xff) {
				printk(KERN_WARNING "nand_erase: attempt to erase a bad block at page 0x%08x\n",page);
				instr->state = MTD_ERASE_FAILED;
				goto erase_exit;
			}
		}		
		
		/* Send commands to erase a page */
		nand_command(mtd, NAND_CMD_ERASE1, -1, page);
		nand_command(mtd, NAND_CMD_ERASE2, -1, -1);

		/*
		 * Wait for program operation to complete. This could
		 * take up to 4000us (4ms) on some devices, so we try
		 * and exit as quickly as possible.
		 */
		status = 0;
		for (i=0 ; i<32 ; i++) {
			/* Delay for 125us */
			udelay (125);
			/* Check the status */
			if (this->dev_ready) 
				if (!this->dev_ready())
					continue;
			/* Check the status */
			nand_command (mtd, NAND_CMD_STATUS, -1, -1);
			status = (int) readb (this->IO_ADDR_R);
			if (status & 0x40)
				break;
		}

		/* See if block erase succeeded */
		if (status & 0x01) {
			DEBUG (MTD_DEBUG_LEVEL0,
				"nand_erase: " \
				"Failed erase, page 0x%08x\n", page);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}
	
		/* Invalidate cache, if last_page is inside erase-block */
		if (this->cache_page >= page && this->cache_page < (page + pages_per_block) ) 
			this->cache_page = -1;

		/* Increment page address and decrement length */
		len -= mtd->erasesize;
		page += pages_per_block;

		/* Release the spin lock */
		spin_unlock_bh (&this->chip_lock);

erase_retry:
		/* Check the state and sleep if it changed */
		spin_lock_bh (&this->chip_lock);
		if (this->state == FL_ERASING) {
			/* Select the NAND device again, if we were interrupted*/
			nand_select ();
			continue;
		}
		else {
			set_current_state (TASK_UNINTERRUPTIBLE);
			add_wait_queue (&this->wq, &wait);
			spin_unlock_bh (&this->chip_lock);
			schedule();

			remove_wait_queue (&this->wq, &wait);
			goto erase_retry;
		}
	} 
	instr->state = MTD_ERASE_DONE;
	
erase_exit:	
	spin_unlock_bh (&this->chip_lock);

	/* De-select the NAND device */
	nand_deselect ();

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;;
	/* Do call back function */
	if (!ret && instr->callback)
		instr->callback (instr);

	/* The device is ready */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	spin_unlock_bh (&this->chip_lock);

	/* Return more or less happy */
	return ret;
}

/*
 * NAND sync
 */
static void nand_sync (struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;
	DECLARE_WAITQUEUE(wait, current);

	DEBUG (MTD_DEBUG_LEVEL3, "nand_sync: called\n");

retry:
	/* Grab the spinlock */
	spin_lock_bh(&this->chip_lock);

	/* See what's going on */
	switch(this->state) {
	case FL_READY:
	case FL_SYNCING:
		this->state = FL_SYNCING;
		spin_unlock_bh (&this->chip_lock);
		break;

	default:
		/* Not an idle state */
		add_wait_queue (&this->wq, &wait);
		spin_unlock_bh (&this->chip_lock);
		schedule ();

		remove_wait_queue (&this->wq, &wait);
		goto retry;
	}

        /* Lock the device */
	spin_lock_bh (&this->chip_lock);

	/* Set the device to be ready again */
	if (this->state == FL_SYNCING) {
		this->state = FL_READY;
		wake_up (&this->wq);
	}

        /* Unlock the device */
	spin_unlock_bh (&this->chip_lock);
}

/*
 * Scan for the NAND device
 */
int nand_scan (struct mtd_info *mtd)
{
	int i, nand_maf_id, nand_dev_id;
	struct nand_chip *this = mtd->priv;

	/* Select the device */
	nand_select ();

	/* Send the command for reading device ID */
	nand_command (mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	nand_maf_id = readb (this->IO_ADDR_R);
	nand_dev_id = readb (this->IO_ADDR_R);

	/* Print and store flash device information */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (nand_maf_id == nand_flash_ids[i].manufacture_id &&
		    nand_dev_id == nand_flash_ids[i].model_id) {
			if (!mtd->size) {
				mtd->name = nand_flash_ids[i].name;
				mtd->erasesize = nand_flash_ids[i].erasesize;
				mtd->size = (1 << nand_flash_ids[i].chipshift);
				mtd->eccsize = 256;
				if (nand_flash_ids[i].page256) {
					mtd->oobblock = 256;
					mtd->oobsize = 8;
					this->page_shift = 8;
				}
				else {
					mtd->oobblock = 512;
					mtd->oobsize = 16;
					this->page_shift = 9;
				}
			}
			printk (KERN_INFO "NAND device: Manufacture ID:" \
				" 0x%02x, Chip ID: 0x%02x (%s)\n",
			       nand_maf_id, nand_dev_id, mtd->name);
			break;
		}
	}

	/* Initialize state, waitqueue and spinlock */
	this->state = FL_READY;
	init_waitqueue_head(&this->wq);
	spin_lock_init(&this->chip_lock);

	/* De-select the device */
	nand_deselect ();
	/* 
	 * Preset out of band configuration
	*/
#ifdef CONFIG_MTD_NAND_ECC_JFFS2
	oob_config.ecc_pos[0] = NAND_JFFS2_OOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_JFFS2_OOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_JFFS2_OOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_JFFS2_OOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_JFFS2_OOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_JFFS2_OOB_ECCPOS5;
	oob_config.badblock_pos = 5;
	oob_config.eccvalid_pos = 4;
#else
	oob_config.ecc_pos[0] = NAND_NOOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_NOOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_NOOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_NOOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_NOOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_NOOB_ECCPOS5;
	oob_config.badblock_pos = NAND_NOOB_BADBPOS; 
	oob_config.eccvalid_pos = NAND_NOOB_ECCVPOS;
#endif

	/* Print warning message for no device */
	if (!mtd->size) {
		printk (KERN_WARNING "No NAND device found!!!\n");
		return 1;
	}

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH | MTD_ECC;
	mtd->module = THIS_MODULE;
	mtd->ecctype = MTD_ECC_SW;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_ecc = nand_read_ecc;
	mtd->write_ecc = nand_write_ecc;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->readv = NULL;
	mtd->writev = nand_writev;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = NULL;
	mtd->resume = NULL;

	/* Return happy */
	return 0;
}

EXPORT_SYMBOL(nand_scan);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven J. Hill <sjhill@cotw.com");
MODULE_DESCRIPTION("Generic NAND flash driver code");
