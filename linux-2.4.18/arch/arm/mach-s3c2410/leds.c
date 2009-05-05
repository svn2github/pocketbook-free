/*
 * linux/arch/mach-s3c2410/leds.c
 *
 * Based on linux/arch/arm/mach-sa1100/leds.c
 *
 * This code is GPL.
 *
 * 2002-10-02: Janghoon Lyu <nandy@mizi.com>
 */
#include <linux/init.h>

#include <asm/leds.h>
#include <asm/mach-types.h>

#include "leds.h"

static int __init
s3c2410_leds_init(void)
{
	if (machine_is_smdk2410())
		leds_event = smdk_leds_event;

	leds_event(led_start);
	return 0;
}

__initcall(s3c2410_leds_init);
