/*
 *  linux/include/asm-arm/proc-armv/uncompress.h
 *
 *  Copyright (C) 1997 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

static inline void proc_decomp_setup (void)
{
	__asm__ __volatile__("						\n\
	mrc	p15, 0, r0, c0, c0					\n\
	eor	r0, r0, #0x44 << 24					\n\
	eor	r0, r0, #0x01 << 16					\n\
	eor	r0, r0, #0xA1 << 8					\n\
	movs	r0, r0, lsr #5						\n\
	mcreq	p15, 0, r0, c7, c5, 0		@ flush I cache		\n\
	mrceq	p15, 0, r0, c1, c0					\n\
	orreq	r0, r0, #1 << 12					\n\
	mcreq	p15, 0, r0, c1, c0		@ enable I cache	\n\
	mov	r0, #0							\n\
	mcreq	p15, 0, r0, c15, c1, 2		@ enable clock switching\n\
	" : : : "r0", "cc", "memory");
}
