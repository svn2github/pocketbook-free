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
 */

extern void __init s3c2410_map_io(void);
extern void __init s3c2410_init_irq(void);

/* drivers/serial/serial_s3c2410.c */
extern void __init s3c2410_register_uart(int idx, int port);
