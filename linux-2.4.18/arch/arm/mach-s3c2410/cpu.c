/*
 * linux/arch/arm/mach-s3c2410/cpu.c
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:55:27 $
 *
 * $Revision: 1.1.1.1 $
 *
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
 *
 *
 * $Id: cpu.c,v 1.1.1.1 2004/02/04 12:55:27 laputa Exp $
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <asm/errno.h>
#include <asm/arch/cpu_s3c2410.h>

static inline unsigned long
cal_bus_clk(unsigned long cpu_clk, unsigned long ratio, int who)
{
	if (!who) {	/* PCLK */


		switch (ratio) {
			case 0:
				return (cpu_clk);
			case 1:
			case 2:
				return (cpu_clk/2);
			case 3:
				return (cpu_clk/4);
			default:
				return 0;
				
		}
	} else {	/* HCLK */
		switch (ratio) {
			case 0:
			case 1:
				return (cpu_clk);
			case 2:
			case 3:
				return (cpu_clk/2);
			default:
				return 0;
		}
	}
}


/*
 * cpu clock = (((mdiv + 8) * FIN) / ((pdiv + 2) * (1 << sdiv)))
 *  FIN = Input Frequency (to CPU)
 */
unsigned long
s3c2410_get_cpu_clk(void)
{
	unsigned long val = MPLLCON;
	return (((GET_MDIV(val) + 8) * FIN) / \
		((GET_PDIV(val) + 2) * (1 << GET_SDIV(val))));
}
EXPORT_SYMBOL(s3c2410_get_cpu_clk);

unsigned long
s3c2410_get_bus_clk(int who)
{
	unsigned long cpu_clk = s3c2410_get_cpu_clk();
	unsigned long ratio = CLKDIVN;
	return (cal_bus_clk(cpu_clk, ratio, who));
}
EXPORT_SYMBOL(s3c2410_get_bus_clk);

#define MEGA	(1000 * 1000)
static int __init s3c2410_cpu_init(void)
{
	unsigned long freq, hclk, pclk;

	freq = s3c2410_get_cpu_clk();
	hclk = s3c2410_get_bus_clk(GET_HCLK);
	pclk = s3c2410_get_bus_clk(GET_PCLK);

	printk(KERN_INFO "CPU clock = %ld.%03ld Mhz,", freq / MEGA, freq % MEGA);
	
	printk(" HCLK = %ld.%03ld Mhz, PCLK = %ld.%03ld Mhz\n",
		 hclk / MEGA, hclk % MEGA, pclk / MEGA, pclk % MEGA);

	return 0;
}

__initcall(s3c2410_cpu_init);
