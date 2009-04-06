/*
 * linux/arch/arm/mach-sa1100/leds-system3.c
 *
 * Copyright (C) 2001 Stefan Eletzhofer <stefan.eletzhofer@gmx.de>
 *
 * Original (leds-footbridge.c) by Russell King
 *
 * $Id: leds-system3.c,v 1.1.1.1 2004/02/04 12:55:27 laputa Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * $Log: leds-system3.c,v $
 * Revision 1.1.1.1  2004/02/04 12:55:27  laputa
 * rel-1-0-0 laputa: s3c2410 smdk
 *                   - the file for default configuration is "arch/arm/def-configs/smdk2410"
 *                   - the default lcd controller is set the aiji board base
 *                   - if you need to support the meritech board you should be
 *                     changed aiji item off of menuconfig
 *
 * Revision 1.1.1.1  2004/01/14 04:41:41  laputa
 * dev-0-0-1 laputa: initial import of 2410 kernel 
 *                   - the file for default configuration is "arch/arm/def-configs/smdk2410"
 *                   - the default lcd controller is set the aiji board base
 *
 * Revision 1.1.6.1  2001/12/04 15:19:26  seletz
 * - merged from linux_2_4_13_ac5_rmk2
 *
 * Revision 1.1.4.2  2001/11/19 17:58:53  seletz
 * - cleanup
 *
 * Revision 1.1.4.1  2001/11/16 13:49:54  seletz
 * - dummy LED support for PT Digital Board
 *
 * Revision 1.1.2.1  2001/10/15 16:03:39  seletz
 * - dummy function
 *
 *
 */
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>

#include "leds.h"


#define LED_STATE_ENABLED	1
#define LED_STATE_CLAIMED	2

static unsigned int led_state;
static unsigned int hw_led_state;

void system3_leds_event(led_event_t evt)
{
	/* TODO: support LEDs */
}
