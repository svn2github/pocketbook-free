/*
 *  linux/arch/arm/mach-pxa/leds-cerf.c
 *
 *  Copyright (C) 2000 John Dorsey <john+@cs.cmu.edu>
 *
 *  Copyright (c) 2001 Jeff Sutherland <jeffs@accelent.com>
 *
 *  Original (leds-footbridge.c) by Russell King
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


#include <linux/config.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>

#include "leds.h"


#define LED_STATE_ENABLED	1
#define LED_STATE_CLAIMED	2

static unsigned int led_state;
static unsigned int hw_led_state;

void pxa_cerf_leds_event(led_event_t evt)
{
	unsigned long flags;

	local_irq_save(flags);

	switch (evt) {
	case led_start:
		hw_led_state = CERF_HEARTBEAT_LED;
		led_state = LED_STATE_ENABLED;
		break;

	case led_stop:
		led_state &= ~LED_STATE_ENABLED;
		break;

	case led_claim:
		led_state |= LED_STATE_CLAIMED;
		hw_led_state = CERF_HEARTBEAT_LED;
		break;

	case led_release:
		led_state &= ~LED_STATE_CLAIMED;
		hw_led_state = CERF_HEARTBEAT_LED;
		break;

#ifdef CONFIG_LEDS_TIMER
	case led_timer:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state ^= CERF_HEARTBEAT_LED;
		break;
#endif

#ifdef CONFIG_LEDS_CPU
	case led_idle_start:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state |= CERF_SYS_BUSY_LED;
		break;

	case led_idle_end:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state &= ~CERF_SYS_BUSY_LED;
		break;
#endif

	case led_halted:
		break;

	case led_green_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~CERF_HEARTBEAT_LED;
		break;

	case led_green_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= CERF_HEARTBEAT_LED;
		break;

	case led_amber_on:
		break;

	case led_amber_off:
		break;

#ifndef CONFIG_PXA_CERF_PDA
	case led_red_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~CERF_SYS_BUSY_LED;
		break;

	case led_red_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= CERF_SYS_BUSY_LED;
		break;
#endif
	default:
		break;
	}

	if  (led_state & LED_STATE_ENABLED)
	{
		switch (hw_led_state) {
		case 0: // all on
			CERF_HEARTBEAT_LED_ON;
			CERF_SYS_BUSY_LED_ON;
			break;
		case 1: // turn off heartbeat, status on:
			CERF_HEARTBEAT_LED_OFF;
			CERF_SYS_BUSY_LED_ON;
			break;
		case 2: // status off, heartbeat on:
			CERF_HEARTBEAT_LED_ON;
			CERF_SYS_BUSY_LED_OFF;
			break;
		case 3: // turn them both off...
			CERF_HEARTBEAT_LED_OFF;
			CERF_SYS_BUSY_LED_OFF;
			break;
		default:
			break;
		}
	}
	local_irq_restore(flags);
}
