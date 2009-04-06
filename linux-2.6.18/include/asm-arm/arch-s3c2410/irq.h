/*
 * linux/include/asm-arm/arch-s3c2400/irq.h
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
 * Author : Nandy Lyu <nandy@mizi.com>
 *   Date : 13 December 2001
 */

extern void set_EINT_IRQ_edge(int no_eint, int edge);
extern int set_external_irq(int irq, int edge, int pullup);
/*
 * This prototype is required for cascading of multiplexed interrupts.
 * Since it doesn't exist elsewhere, we'll put it here for now.
 */
extern unsigned int fixup_irq(int i);
extern void do_IRQ(int irq, struct pt_regs *regs);
