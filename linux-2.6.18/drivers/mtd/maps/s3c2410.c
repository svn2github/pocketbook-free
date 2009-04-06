/*
 * Flash memory access on S3C2410 based devices
 * 
 * (C) 2006 Jeremy Chang <jeremyc@pvi.com>
 * 
 * $Id: S3C2410-flash.c,v 1.1.1.1 2006/03/06 12:56:23 laputa Exp $
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/config.h>

#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/partitions.h>
#endif

#if 0
#define WINDOW_ADDR 0x0100000
#define WINDOW_SIZE 0x0100000
#define BUSWIDTH 2
#endif

#define WINDOW_ADDR 0x0000000
#define WINDOW_SIZE 0x0800000
#define BUSWIDTH 2

static struct mtd_info *mymtd;

__u8 s3c2410_read8(struct map_info *map, unsigned long ofs)
{
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 s3c2410_read16(struct map_info *map, unsigned long ofs)
{
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 s3c2410_read32(struct map_info *map, unsigned long ofs)
{
	return __raw_readl(map->map_priv_1 + ofs);
}

#ifdef CFI_WORD_64
__u64 s3c2410_read64(struct map_info *map, unsigned long ofs)
{
	return __raw_readll(map->map_priv_1 + ofs);
}
#endif

void s3c2410_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy_fromio(to, map->map_priv_1 + from, len);
}

void s3c2410_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void s3c2410_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void s3c2410_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

#ifdef CFI_WORD_64
void s3c2410_write64(struct map_info *map, __u64 d, unsigned long adr)
{
	__raw_writell(d, map->map_priv_1 + adr);
	mb();
}
#endif

void s3c2410_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	memcpy_toio(map->map_priv_1 + to, from, len);
}

struct map_info s3c2410_map = {
	name: "Physically mapped flash",
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	read8: s3c2410_read8,
	read16: s3c2410_read16,
	read32: s3c2410_read32,
#ifdef CFI_WORD_64
	read64: s3c2410_read64,
#endif
	copy_from: s3c2410_copy_from,
	write8: s3c2410_write8,
	write16: s3c2410_write16,
	write32: s3c2410_write32,
#ifdef CFI_WORD_64
	write64: s3c2410_write64,
#endif
	copy_to: s3c2410_copy_to
};



static struct mtd_partition s3c2410_partitions[] = {
        {
                name: "Kernel (896k)",
                size: 0x100000,
               offset: 0x30000,
        },
        {
                name: "root",
                size: 0x0170000,
                offset: 0x130000,
        },
        {
                name: "BMP",
                size: 0x020000,
                offset: 0x7E0000,
        },
        {
		name: "vivi",
		size: 0x20000, 
		offset: 0x00000,
	},
	{
		name: "ebr",
		size: 0x540000,
		offset: 0x2A0000,
	},        
};




#ifdef CONFIG_MTD_PARTITIONS
#ifdef CONFIG_MTD_CMDLINE_PARTS
static struct mtd_partition *mtd_parts = 0;
static int                   mtd_parts_nb = 0;
#endif
#endif

int __init init_s3c2410(void)
{
	static const char *rom_probe_types[] = { "cfi_probe", "jedec_probe", "map_rom", 0 };
	const char **type;

       	printk(KERN_NOTICE "physmap flash device: %x at %x\n", WINDOW_SIZE, WINDOW_ADDR);
	s3c2410_map.map_priv_1 = (unsigned long)ioremap_nocache(WINDOW_ADDR, WINDOW_SIZE);
//	s3c2410_map.map_priv_1 = (unsigned long)ioremap(WINDOW_ADDR, WINDOW_SIZE);

	if (!s3c2410_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
		
	mymtd = 0;
	
	mymtd = do_map_probe("jedec_probe", &s3c2410_map);
	if (!mymtd)
 	mymtd = do_map_probe("cfi_probe", &s3c2410_map);

//	type = rom_probe_types;
//	for(; !mymtd && *type; type++) {
//		mymtd = do_map_probe(*type, &s3c2410_map);
//	}
	if (mymtd) {
		mymtd->module = THIS_MODULE;
		mymtd->erasesize = 0x20000;	//128kb
		add_mtd_device(mymtd);
		
		return add_mtd_partitions(mymtd, s3c2410_partitions, sizeof(s3c2410_partitions)/sizeof(struct mtd_partition));
				
#ifdef CONFIG_MTD_PARTITIONS
#ifdef CONFIG_MTD_CMDLINE_PARTS
		mtd_parts_nb = parse_cmdline_partitions(mymtd, &mtd_parts, 
							"phys");
		if (mtd_parts_nb > 0)
		{
			printk(KERN_NOTICE 
			       "Using command line partition definition\n");
			add_mtd_partitions (mymtd, mtd_parts, mtd_parts_nb);
		}
#endif
#endif
//		return 0;
	}

	iounmap((void *)s3c2410_map.map_priv_1);
	return -ENXIO;
}

static void __exit cleanup_s3c2410(void)
{
	if (mymtd) {
		del_mtd_device(mymtd);
		map_destroy(mymtd);
	}
	if (s3c2410_map.map_priv_1) {
		iounmap((void *)s3c2410_map.map_priv_1);
		s3c2410_map.map_priv_1 = 0;
	}
}

module_init(init_s3c2410);
module_exit(cleanup_s3c2410);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic configurable MTD map driver");
