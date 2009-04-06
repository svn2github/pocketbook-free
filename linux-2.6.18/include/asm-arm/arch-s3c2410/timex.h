/*
 * linux/include/asm-arm/arch-s3c2400/timex.h
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:39 $
 *
 * $Revision: 1.3 
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
 */

/* If a value of TCFG1 is a, a value of divider is 2 << a */
#define CLK_DIVIDER		2
/* a value of TCFG0_PRE1 */
#define CLK_PRESCALE		15
/* PCLK */
#define CLK_INPUT		50000000

/*#define CLOCK_TICK_RATE		1562500	*/
#define CLOCK_TICK_RATE		(CLK_INPUT / (CLK_PRESCALE + ) / DIVIDER)
#define CLOCK_TICK_FACTOR	80
