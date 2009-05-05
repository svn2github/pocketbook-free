/*
 *  Map driver for the PXA Cerf.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>


#define WINDOW_ADDR 	0
#if defined (CONFIG_PXA_CERF_FLASH_64MB)
#define WINDOW_SIZE 	64*1024*1024
#elif defined (CONFIG_PXA_CERF_FLASH_32MB)
#define WINDOW_SIZE 	32*1024*1024
#elif defined (CONFIG_PXA_CERF_FLASH_16MB)
#define WINDOW_SIZE 	16*1024*1024
#endif
#define BUSWIDTH 	4

static __u8 pxa_cerf_read8(struct map_info *map, unsigned long ofs)
{
	return *(__u8 *)(map->map_priv_1 + ofs);
}

static __u16 pxa_cerf_read16(struct map_info *map, unsigned long ofs)
{
	return *(__u16 *)(map->map_priv_1 + ofs);
}

static __u32 pxa_cerf_read32(struct map_info *map, unsigned long ofs)
{
	return *(__u32 *)(map->map_priv_1 + ofs);
}

static void pxa_cerf_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy(to, (void *)(map->map_priv_1 + from), len);
}

static void pxa_cerf_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	*(__u8 *)(map->map_priv_1 + adr) = d;
}

static void pxa_cerf_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	*(__u16 *)(map->map_priv_1 + adr) = d;
}

static void pxa_cerf_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	*(__u32 *)(map->map_priv_1 + adr) = d;
}

static void pxa_cerf_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	memcpy((void *)(map->map_priv_1 + to), from, len);
}

static struct map_info pxa_cerf_map = {
	name: "PXA Cerf Flash",
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	read8:		pxa_cerf_read8,
	read16:		pxa_cerf_read16,
	read32:		pxa_cerf_read32,
	copy_from:	pxa_cerf_copy_from,
	write8:		pxa_cerf_write8,
	write16:	pxa_cerf_write16,
	write32:	pxa_cerf_write32,
	copy_to:	pxa_cerf_copy_to
};

static struct mtd_partition pxa_cerf_partitions[] = {
	{
		name:		"Bootloader",
		size:		0x00040000,
		offset:		0,
		mask_flags:	MTD_WRITEABLE  /* force read-only */
	},{
		name:		"Partition Tables",
		size:		0x00080000,
		offset:		0x00040000,
	},{
		name:		"Kernel",
		size:		0x00100000,
		offset:		0x000C0000,
	},{
		name:		"Filesystem",
		size:		WINDOW_SIZE-0x001C0000, //MTDPART_SIZ_FULL,
		offset:		0x001C0000
	}
};

#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))

static struct mtd_info *mymtd;
static struct mtd_partition *parsed_parts;

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);

static int __init init_pxa_cerf(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;
	int parsed_nr_parts = 0;
	char *part_type = "static";

	printk("Probing PXA Cerf flash at physical address 0x%08x\n", WINDOW_ADDR);
	pxa_cerf_map.map_priv_1 = (unsigned long)__ioremap(WINDOW_ADDR, WINDOW_SIZE, 0);
	if (!pxa_cerf_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	mymtd = do_map_probe("cfi_probe", &pxa_cerf_map);
	if (!mymtd) {
		iounmap((void *)pxa_cerf_map.map_priv_1);
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
		parts = pxa_cerf_partitions;
		nb_parts = NB_OF(pxa_cerf_partitions);
	}
	if (nb_parts) {
		printk(KERN_NOTICE "Using %s partition definition\n", part_type);
		add_mtd_partitions(mymtd, parts, nb_parts);
	} else {
		add_mtd_device(mymtd);
	}
	return 0;
}

static void __exit cleanup_pxa_cerf(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
		if (parsed_parts)
			kfree(parsed_parts);
	}
	if (pxa_cerf_map.map_priv_1)
		iounmap((void *)pxa_cerf_map.map_priv_1);
	return;
}

module_init(init_pxa_cerf);
module_exit(cleanup_pxa_cerf);

