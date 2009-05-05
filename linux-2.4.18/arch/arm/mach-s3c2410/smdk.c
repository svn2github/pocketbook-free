/*
 * linux/arch/arm/mach-s3c2410/smdk.c
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
 *     - Initial code
 *
 * This file contains all SMDK2410-specific tewaks.
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

#include "generic.h"

#ifdef CONFIG_PM
struct mz_pm_ops_t mz_pm_ops = {
	machine_pm:	NULL,
	get_power_status:	NULL,
	fb_ioctl:	NULL,
	blank_helper:	NULL,
};
EXPORT_SYMBOL(mz_pm_ops);
#endif

static int __init smdk_init(void)
{
	set_gpio_ctrl(GPIO_LED1);
	set_gpio_ctrl(GPIO_LED2);
	set_gpio_ctrl(GPIO_LED3);
	set_gpio_ctrl(GPIO_LED4);
	
//	init_eint();
	
	return 0;
}

__initcall(smdk_init);

static void __init
fixup_smdk(struct machine_desc *desc, struct param_struct *params,
		char **cmdline, struct meminfo *mi)
{
	/* TODO */
#if 0 /* hacked by nandy. Is these codes need ? */
	struct tag *t = (struct tag *)params;

	if (t->hdr.tag != ATAG_CORE)
		convert_to_tag_list(params, 1);

	if (t->hdr.tag != ATAG_CORE) {
		t->hdr.tag = ATAG_CORE;
		t = tag_next(t);

		t->hdr.tag = ATAG_MEM;
		t->u.mem.start = 0x0c000000;
		t->u.mem.size = 32 * 1024 * 1024;
	}
#endif
}

/*
 * I/O mapping:
 *
 * pNOR_CS8900_BASE		nGCS3, CS8900a
 * pCF_MEM_BASE			nGCS2, PCMCIA Memory
 * pCF_IO_BASE			nGCS2, PCMCIA I/O
 */
static struct map_desc smdk_io_desc[] __initdata = {
    /* virtual    physical    length      domain     r  w  c  b */
    { vCS8900_BASE, pCS8900_BASE, 0x00100000, DOMAIN_IO, 0, 1, 0, 0 },
    { vCF_MEM_BASE, pCF_MEM_BASE, 0x01000000, DOMAIN_IO, 0, 1, 0, 0 },
    { vCF_IO_BASE, pCF_IO_BASE, 0x01000000, DOMAIN_IO, 0, 1, 0, 0 },
    LAST_DESC
};

#ifdef CONFIG_PM
extern void register_wakeup_src(u_int, int, int);
#endif
static void __init smdk_map_io(void)
{
	s3c2410_map_io();
	iotable_init(smdk_io_desc);

	s3c2410_register_uart(0, 0);
	s3c2410_register_uart(1, 1);
	set_gpio_ctrl(GPIO_IR_TXD);
	set_gpio_ctrl(GPIO_IR_RXD);
	s3c2410_register_uart(2, 2);
	

#ifdef CONFIG_PM
	register_wakeup_src(0, EXT_FALLING_EDGE, 1);
	register_wakeup_src(1, EXT_FALLING_EDGE, 1); 
	register_wakeup_src(2, EXT_FALLING_EDGE, 1);
	register_wakeup_src(3, EXT_FALLING_EDGE, 1);
	register_wakeup_src(4, EXT_FALLING_EDGE, 1);
	register_wakeup_src(5, EXT_FALLING_EDGE, 1);
	register_wakeup_src(6, EXT_FALLING_EDGE, 1);
	register_wakeup_src(7, EXT_FALLING_EDGE, 1); 
	register_wakeup_src(8, EXT_FALLING_EDGE, 1); 
	register_wakeup_src(10, EXT_FALLING_EDGE, 1);
	register_wakeup_src(11, EXT_FALLING_EDGE, 1);
	register_wakeup_src(12, EXT_FALLING_EDGE, 1); 
	register_wakeup_src(13, EXT_FALLING_EDGE, 1);
	register_wakeup_src(14, EXT_FALLING_EDGE, 1);
	register_wakeup_src(15, EXT_FALLING_EDGE, 1);
//	register_wakeup_src(18, EXT_FALLING_EDGE, 0);
//	register_wakeup_src(19, EXT_FALLING_EDGE, 0);
		
#endif
}

MACHINE_START(SMDK2410, "Samsung-SMDK2410")
	BOOT_MEM(0x30000000, 0x48000000, 0xe8000000)
	BOOT_PARAMS(0x30000100)
	FIXUP(fixup_smdk)
	MAPIO(smdk_map_io)
	INITIRQ(s3c2410_init_irq)
MACHINE_END
