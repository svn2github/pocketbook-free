/*
 * linux/arch/arm/mach-s3c2410/generic.c
 * 
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * History
 *
 * 2002-05-20: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *    - Based on linux/arch/arm/mach-s3c2400/generic.c
 *
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/serial_core.h>

#include <asm/hardware.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

/*
 * Common I/O mapping:
 *
 * 0x4800.0000 ~ 0x5eff.fffff
  (0xe800.0000 ~ 0xfeff.fffff)	GPIO
 */
static struct map_desc standard_io_desc[] __initdata = {
	/* virtual    physical    length      domain     r  w  c  b */
	{ 0xe8000000, 0x48000000, 0x17000000, DOMAIN_IO, 0, 1, 0, 0 },
	LAST_DESC
};

void __init s3c2410_map_io(void)
{
	iotable_init(standard_io_desc);
}
