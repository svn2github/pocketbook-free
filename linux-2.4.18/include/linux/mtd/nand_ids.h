/*
 *  linux/include/linux/mtd/nand_ids.h
 *
 *  Copyright (c) 2000 David Woodhouse <dwmw2@mvhi.com>
 *                     Steven J. Hill <sjhill@cotw.com>
 *
 * $Id: nand_ids.h,v 1.1.1.1 2004/02/04 12:57:56 laputa Exp $
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
 *   2000-10-13 BE      Moved out of 'nand.h' - avoids duplication.
 */

#ifndef __LINUX_MTD_NAND_IDS_H
#define __LINUX_MTD_NAND_IDS_H

static struct nand_flash_dev nand_flash_ids[] = {
	{"Toshiba TC5816BDC",     NAND_MFR_TOSHIBA, 0x64, 21, 1, 2, 0x1000},	// 2Mb 5V
	{"Toshiba TC58V16BDC",    NAND_MFR_TOSHIBA, 0xea, 21, 1, 2, 0x1000},	// 2Mb 3.3V
	{"Toshiba TC5832DC",      NAND_MFR_TOSHIBA, 0x6b, 22, 0, 2, 0x2000},	// 4Mb 5V
	{"Toshiba TC58V32DC",     NAND_MFR_TOSHIBA, 0xe5, 22, 0, 2, 0x2000},	// 4Mb 3.3V
	{"Toshiba TC58V64AFT/DC", NAND_MFR_TOSHIBA, 0xe6, 23, 0, 2, 0x2000},	// 8Mb 3.3V
	{"Toshiba TH58V128DC",    NAND_MFR_TOSHIBA, 0x73, 24, 0, 2, 0x4000},	// 16Mb
	{"Toshiba TC58256FT/DC",  NAND_MFR_TOSHIBA, 0x75, 25, 0, 2, 0x4000},	// 32Mb
	{"Toshiba TH58512FT",     NAND_MFR_TOSHIBA, 0x76, 26, 0, 3, 0x4000},	// 64Mb
	{"Toshiba TH58NS100/DC",  NAND_MFR_TOSHIBA, 0x79, 27, 0, 3, 0x4000},	// 128Mb
	{"Samsung KM29N16000",    NAND_MFR_SAMSUNG, 0x64, 21, 1, 2, 0x1000},	// 2Mb 5V
	{"Samsung KM29W16000",    NAND_MFR_SAMSUNG, 0xea, 21, 1, 2, 0x1000},	// 2Mb 3.3V
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0x6b, 22, 0, 2, 0x2000},	// 4Mb 5V
	{"Samsung KM29W32000",    NAND_MFR_SAMSUNG, 0xe3, 22, 0, 2, 0x2000},	// 4Mb 3.3V
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0xe5, 22, 0, 2, 0x2000},	// 4Mb 3.3V
	{"Samsung KM29U64000",    NAND_MFR_SAMSUNG, 0xe6, 23, 0, 2, 0x2000},	// 8Mb 3.3V
	{"Samsung KM29U128T",     NAND_MFR_SAMSUNG, 0x73, 24, 0, 2, 0x4000},	// 16Mb
	{"Samsung KM29U256T",     NAND_MFR_SAMSUNG, 0x75, 25, 0, 2, 0x4000},	// 32Mb
	{"Samsung K9D1208V0M",    NAND_MFR_SAMSUNG, 0x76, 26, 0, 3, 0x4000},	// 64Mb
	{"Samsung K9D1G08V0M",    NAND_MFR_SAMSUNG, 0x79, 27, 0, 3, 0x4000},	// 128Mb
	{NULL,}
};

#if defined(CONFIG_MTD_SMC) || defined(CONFIG_MTD_SMC_MODULE)
static struct nand_smc_dev nand_smc_info[8] = {
/*  CpV, HpC, SpH,   allS, szS, PBpV, LBpV, SpB, PpB, szP */
  { 125,   4,   4,   2000, 512,  256,  250,   8,  16, 256},    /* DI_1M */
  { 125,   4,   8,   4000, 512,  512,  500,   8,  16, 256},    /* DI_2M */
  { 250,   4,   8,   8000, 512,  512,  500,  16,  16, 512},    /* DI_4M */
  { 250,   4,  16,  16000, 512, 1024, 1000,  16,  16, 512},    /* DI_8M */
  { 500,   4,  16,  32000, 512, 1024, 1000,  32,  32, 512},    /* DI_16M */
  { 500,   8,  16,  64000, 512, 2048, 2000,  32,  32, 512},    /* DI_32M */
  { 500,   8,  32, 128000, 512, 4096, 4000,  32,  32, 512},    /* DI_64M */
  { 500,  16,  32, 256000, 512, 8192, 8000,  32,  32, 512}     /* DI_128M */
};

#define DI_1M                  0
#define DI_2M                  1
#define DI_4M                  2
#define DI_8M                  3
#define DI_16M                 4
#define DI_32M                 5
#define DI_64M                 6
#define DI_128M                7
#define MAX_DI_NUM             8
#define GET_DI_NUM(x)          ((x) - 20)
#endif
#endif /* __LINUX_MTD_NAND_IDS_H */
