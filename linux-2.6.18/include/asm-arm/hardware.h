/*
 *  linux/include/asm-arm/hardware.h
 *
 *  Copyright (C) 1996 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Common hardware definitions
 */

#ifndef __ASM_HARDWARE_H
#define __ASM_HARDWARE_H
#include <linux/config.h>

#ifdef __KERNEL__
#include <asm/arch/hardware.h>
#endif	/* __KERNEL__ */

#ifdef CONFIG_MIZI
#ifdef __KERNEL__
#include "linuette_kernel.h"
#else	/* __KERNEL__ */
#include "linuette_ioctl.h"
#endif	/* __KERNEL__ */
#endif	/* CONFIG_MIZI */

#endif
