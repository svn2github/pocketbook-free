/*
 * linux/arch/arm/mach-s3c2410/leds-smdk.c
 *
 * Based on linux/arch/arm/mach-sa1100/leds-assabet.c (by John Dorsey)
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Written by Janghoon Lyu <nandy@mizi.com>
 */
#include <linux/config.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>
#include <asm/arch/smdk.h>

#include "leds.h"

#define LED_STATE_ENABLED	1
#define LED_STATE_CLAIMED	2

#define LED0		(1 << 0)
#define LED1		(1 << 1)
#define LED2		(1 << 2)
#define LED3		(1 << 3)	

static unsigned int led_state;
static unsigned int hw_led_state;

static inline void
led_update(unsigned int state)
{
#if 0
	GPFDAT &= ~(0xf << 4);
	GPFDAT |= (state << 4);
#else
	write_gpio_bit(GPIO_LED1, (state & LED0));
	write_gpio_bit(GPIO_LED2, ((state & LED1) >> 1));
	write_gpio_bit(GPIO_LED3, ((state & LED2) >> 2));
	write_gpio_bit(GPIO_LED4, ((state & LED3) >> 3));
#endif
}

void
smdk_leds_event(led_event_t evt)
{
	unsigned long flags;

	local_irq_save(flags);

	switch (evt) {
	case led_start:
		hw_led_state = (LED1 | LED2 | LED3);
		led_state = LED_STATE_ENABLED;
		break;

	case led_stop:
		led_state &= ~LED_STATE_ENABLED;
		hw_led_state = (LED0 | LED1 | LED2 | LED3);
		led_update(hw_led_state);
		break;

	case led_claim:
		led_state |= LED_STATE_CLAIMED;
		hw_led_state = (LED0 | LED1 | LED2 | LED3);
		break;

	case led_release:
		led_state &= ~LED_STATE_CLAIMED;
		hw_led_state = (LED1 | LED2 | LED3);
		break;

#ifdef CONFIG_LEDS_TIMER
	case led_timer:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state ^= LED3;
		break;
#endif

#ifdef CONFIG_LEDS_CPU
	case led_idle_start:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state |= LED2;
		break;

	case led_idle_end:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state &= ~LED2;
		break;
#endif

	case led_halted:
		break;

	case led_green_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~LED2;
		break;

	case led_green_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= LED2;
		break;

	case led_amber_on:
		break;

	case led_amber_off:
		break;

	case led_red_on:
		break;

	case led_red_off:
		break;

	default:
		break;
	}

	if (led_state & LED_STATE_ENABLED)
		led_update(hw_led_state);

	local_irq_restore(flags);
}
