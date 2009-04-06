/*
 * linux/arch/arm/boot/compressed/head-s3c2410.S
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * s3c2410 specific tweaks. This is merged into head.S by the linker.
 *
 */

#include <linux/config.h>
#include <linux/linkage.h>
#include <asm/mach-types.h>

	.section	".start", #alloc, #execinstr

__S3C2410_start:

	@ Preserve r8/r7 i.e. kernel entry values
	@ What is it?
	@ Nandy

	@ Data cache, Intstruction cache, MMU might be active.
	@ Be sure to flush kernel binary out of the cache,
	@ whatever state it is, before it is turned off.
	@ This is done by fetching through currently executed
	@ memory to be sure we hit the same cache

	bic	r2, pc, #0x1f
	add r3, r2, #0x4000		@ 16 kb is quite enough...
1:	ldr r0, [r2], #32
	teq	r2, r3
	bne	1b
	mcr	p15, 0, r0, c7, c10, 4	@ drain WB
	mcr	p15, 0, r0, c7, c7, 0	@ flush I & D caches

#if 0
	@ disabling MMU and caches
	mrc	p15, 0, r0, c1, c0, 0	@ read control register
	bic	r0, r0, #0x05			@ disable D cache and MMU
	bic r0, r0, #1000			@ disable I cache
	mcr p15, 0, r0, c1, c0, 0
#endif

	/*
	 * Pause for a short time so that we give enough time
	 * for the host to start a terminal up.
	 */
	mov	r0, #0x00200000
1:	subs	r0, r0, #1
	bne	1b
