/*
 * generic.c
 *
 * Generic routine for NOR Flash Device
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:23 $ 
 *
 * $Revision: 1.1.1.1 $
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>

__u8 mtd_generic_read8(struct map_info *map, unsigned long ofs) {
    return *(__u8 *)(map->map_priv_1 + ofs);
}

__u16 mtd_generic_read16(struct map_info *map, unsigned long ofs) {
    return *(__u16 *)(map->map_priv_1 + ofs);
}

__u32 mtd_generic_read32(struct map_info *map, unsigned long ofs) {
    return *(__u32 *)(map->map_priv_1 + ofs);
}

void mtd_generic_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len) {
    memcpy(to, (void *)(map->map_priv_1 + from), len);
}

void mtd_generic_write8(struct map_info *map, __u8 d, unsigned long adr) {
    *(__u8 *)(map->map_priv_1 + adr) = d;
}

void mtd_generic_write16(struct map_info *map, __u16 d, unsigned long adr) {
    *(__u16 *)(map->map_priv_1 + adr) = d;
}

void mtd_generic_write32(struct map_info *map, __u32 d, unsigned long adr) {
    *(__u32 *)(map->map_priv_1 + adr) = d;
}

void mtd_generic_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len) {
    memcpy((void *)(map->map_priv_1 + to), from, len);
}

EXPORT_SYMBOL(mtd_generic_read8);
EXPORT_SYMBOL(mtd_generic_read16);
EXPORT_SYMBOL(mtd_generic_read32);
EXPORT_SYMBOL(mtd_generic_copy_from);
EXPORT_SYMBOL(mtd_generic_write8);
EXPORT_SYMBOL(mtd_generic_write16);
EXPORT_SYMBOL(mtd_generic_write32);
EXPORT_SYMBOL(mtd_generic_copy_to);

/*
 | $Id: mtd_map_generic.c,v 1.1.1.1 2004/02/04 12:56:23 laputa Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
