/*
 * linux/include/asm-arm/arch-pxa/io.h
 *
 * Author:	Nicolas Pitre
 * Created:	Jun 15, 2001
 * Copyright:	MontaVista Software Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#define IO_SPACE_LIMIT 0xffffffff

/*
 * Because of PXA250 erratum #86, we must define our own IO macros to
 * substitute a non-standard inb/outb version.
 */

#if 1
/* 16 bit fixup version */
#define inb(p)	({ \
	unsigned int __p = (unsigned int)(p); \
	unsigned int __v = *(volatile unsigned short *)(__p & ~1); \
	if (__p & 1) __v >>= 8; \
	else __v &= 0xff; \
	__v; })
#else
/* 32 bit fixup version */
#define inb(p)	({ \
	unsigned int __p = (unsigned int)(p); \
	unsigned int __v = *(volatile unsigned int *)(__p & ~3); \
	if (__p & 1) __v >>= 8; \
	if (__p & 2) __v >>= 16; \
	__v & 0xff; })
#endif

#define outb(v,p)                       __raw_writeb(v,p)
#define outw(v,p)                       __raw_writew(v,p)
#define outl(v,p)                       __raw_writel(v,p)

#define inw(p)          ({ unsigned int __v = __raw_readw(p); __v; })
#define inl(p)          ({ unsigned int __v = __raw_readl(p); __v; })

#define outsb(p,d,l)                    __raw_writesb(p,d,l)
#define outsw(p,d,l)                    __raw_writesw(p,d,l)
#define outsl(p,d,l)                    __raw_writesl(p,d,l)

// commented out for now to be sure it's not used
//#define insb(p,d,l)                     __raw_readsb(p,d,l)
#define insw(p,d,l)                     __raw_readsw(p,d,l)
#define insl(p,d,l)                     __raw_readsl(p,d,l)

/*
 * We don't actually have real ISA nor PCI buses, but there is so many 
 * drivers out there that might just work if we fake them...
 */
#define __mem_pci(a)		((unsigned long)(a))
#define __mem_isa(a)		((unsigned long)(a))

/*
 * Generic virtual read/write
 */
#define __arch_getw(a)		(*(volatile unsigned short *)(a))
#define __arch_putw(v,a)	(*(volatile unsigned short *)(a) = (v))

#define iomem_valid_addr(iomem,sz)	(1)
#define iomem_to_phys(iomem)		(iomem)

#endif
