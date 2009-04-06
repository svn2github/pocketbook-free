/*
 * linux/include/asm-arm/arch-s3c2410/cpu_s3c2410.h
 *
 * Copyright (C) 2001 MIZI Research, Inc.
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
 * 2002-03-22: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *    
 * 2002-05-21: Janghoon Lyu <nandy@mizi.com>
 *    - for S3C2410
 *
 */

#ifndef _CPU_S3C2410_H_
#define _CPU_S3C2410_H_

#include <asm/hardware.h>

#define GET_PCLK	0
#define GET_HCLK	1

#define GET_MDIV(x)	FExtr(x, fPLL_MDIV)
#define GET_PDIV(x)	FExtr(x, fPLL_PDIV)
#define GET_SDIV(x)	FExtr(x, fPLL_SDIV)

unsigned long s3c2410_get_cpu_clk(void);
unsigned long s3c2410_get_bus_clk(int);
#endif /* _CPU_S3C2410_H_ */
