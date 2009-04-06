/*
 * $Id:
 *
 * Map driver for the Lubbock developer platform.
 *
 * Author:	Nicolas Pitre
 * Copyright:	(C) 2001 MontaVista Software Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>


//#define WINDOW_ADDR 	0
#define WINDOW_ADDR 	0x04000000
#define WINDOW_SIZE 	32*1024*1024
#define BUSWIDTH 	4

static __u8 lubbock_read8(struct map_info *map, unsigned long ofs)
{
	return *(__u8 *)(map->map_priv_1 + ofs);
}

static __u16 lubbock_read16(struct map_info *map, unsigned long ofs)
{
	return *(__u16 *)(map->map_priv_1 + ofs);
}

static __u32 lubbock_read32(struct map_info *map, unsigned long ofs)
{
	return *(__u32 *)(map->map_priv_1 + ofs);
}

static void lubbock_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy(to, (void *)(map->map_priv_1 + from), len);
}

static void lubbock_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	*(__u8 *)(map->map_priv_1 + adr) = d;
}

static void lubbock_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	*(__u16 *)(map->map_priv_1 + adr) = d;
}

static void lubbock_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	*(__u32 *)(map->map_priv_1 + adr) = d;
}

static void lubbock_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	memcpy((void *)(map->map_priv_1 + to), from, len);
}

static struct map_info lubbock_map = {
	name: "Lubbock flash",
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	read8:		lubbock_read8,
	read16:		lubbock_read16,
	read32:		lubbock_read32,
	copy_from:	lubbock_copy_from,
	write8:		lubbock_write8,
	write16:	lubbock_write16,
	write32:	lubbock_write32,
	copy_to:	lubbock_copy_to
};

static struct mtd_partition lubbock_partitions[] = {
	{
		name:		"Bootloader",
		size:		0x00040000,
		offset:		0,
		mask_flags:	MTD_WRITEABLE  /* force read-only */
	},{
		name:		"Kernel",
		size:		0x00100000,
		offset:		0x00040000,
	},{
		name:		"Filesystem",
		size:		0x00e00000, //MTDPART_SIZ_FULL,
		offset:		0x00140000
	}
};

#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))

static struct mtd_info *mymtd;
static struct mtd_partition *parsed_parts;

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);

static int __init init_lubbock(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;
	int parsed_nr_parts = 0;
	char *part_type = "static";

	printk("Probing Lubbock flash at physical address 0x%08x\n", WINDOW_ADDR);
	lubbock_map.map_priv_1 = (unsigned long)__ioremap(WINDOW_ADDR, WINDOW_SIZE, 0);
	if (!lubbock_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	mymtd = do_map_probe("cfi_probe", &lubbock_map);
	if (!mymtd) {
		iounmap((void *)lubbock_map.map_priv_1);
		return -ENXIO;
	}
	mymtd->module = THIS_MODULE;

#ifdef CONFIG_MTD_REDBOOT_PARTS
	if (parsed_nr_parts == 0) {
		int ret = parse_redboot_partitions(mymtd, &parsed_parts);

		if (ret > 0) {
			part_type = "RedBoot";
			parsed_nr_parts = ret;
		}
	}
#endif

	if (parsed_nr_parts > 0) {
		parts = parsed_parts;
		nb_parts = parsed_nr_parts;
	} else {
		parts = lubbock_partitions;
		nb_parts = NB_OF(lubbock_partitions);
	}
	if (nb_parts) {
		printk(KERN_NOTICE "Using %s partition definition\n", part_type);
		add_mtd_partitions(mymtd, parts, nb_parts);
	} else {
		add_mtd_device(mymtd);
	}
	return 0;
}

static void __exit cleanup_lubbock(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
		if (parsed_parts)
			kfree(parsed_parts);
	}
	if (lubbock_map.map_priv_1)
		iounmap((void *)lubbock_map.map_priv_1);
	return 0;
}

module_init(init_lubbock);
module_exit(cleanup_lubbock);

