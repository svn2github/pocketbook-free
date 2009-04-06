/*
 * drivers/char/gpio_button_smdk2410.c
 *
 * button handler for SMDK-2410
 *
 * Based on gpio_button_kings.c
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:02 $
 *
 * $Revision: 1.1.1.1 $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 *
 * 2002-10-08: Janghoon Lyu <nandy@mizi.com>
 *   - not tested completely
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/hardware.h>

#include <linux/kbd_ll.h>
#include <linux/irq.h>
#include <linux/timer.h>

#include "s3c2410_gpio_button.h"

#undef DEBUG
#ifdef DEBUG
#define DPRINTK( x... ) printk( ##x )
#else
#define DPRINTK( x... )
#endif

static struct timer_list button_check_timer;
#define BUTTON_DELAY 3	/* 30ms */
#define TIMER_BT_NUM	4


typedef struct button_type_tk {
	u32 gpio;
	u8 scancode;
	u8 active;
	int status;
	char *name;
} BUTTON_TYPE_TK;

BUTTON_TYPE_TK gpio_buttons_tk[] = {
	{ GPIO_SMDK_UP, SCANCODE_UP, LOW_ACTIVE, KEY_RELEASED, "up"},
	{ GPIO_SMDK_DOWN, SCANCODE_DOWN, LOW_ACTIVE, KEY_RELEASED, "down"},
	{ GPIO_SMDK_LEFT, SCANCODE_LEFT, LOW_ACTIVE, KEY_RELEASED, "left"},
	{ GPIO_SMDK_RIGHT, SCANCODE_RIGHT, LOW_ACTIVE, KEY_RELEASED, "right"}
};

enum {
	KEYPAD_BT1 = 0,
	KEYPAD_BT2,
	KEYPAD_BT3,
	KEYPAD_BT4,
	KEYPAD_BT5,
	KEYPAD_BT6,
	KEYPAD_BT7,
};

static void keypad_handler(int status, void *priv)
{
	int row = (int)priv;
	BUTTON_TYPE *buttons = (BUTTON_TYPE *)&gpio_buttons[row];

	if (status) {
		handle_scancode(buttons->scancode, KEY_PRESSED);
	} else {
		handle_scancode(buttons->scancode, KEY_RELEASED);
	}

	DPRINTK("%s, 0x02%x, st:%d\n", buttons->name, buttons->scancode,status);
}

static void tk_button_check_timer_handler(unsigned long data)
{
	int i;
	BUTTON_TYPE_TK *buttons = (BUTTON_TYPE_TK *)&gpio_buttons_tk[0];
	int pre_status;
	
	cli();
	for (i = 0; i < TIMER_BT_NUM; i++) {
		pre_status = buttons->status;
		buttons->status = read_gpio_bit(buttons->gpio) ?
			KEY_RELEASED : KEY_PRESSED;

		if (pre_status == buttons->status) {
			buttons++;
			continue;
		}

		handle_scancode(buttons->scancode, buttons->status);
		DPRINTK("%s, st:%d\n", buttons->name, buttons->status);

		buttons++;
	}
	sti();

	mod_timer(&button_check_timer, jiffies + (BUTTON_DELAY));
}

void __init s3c2410tk_button_init(void)
{
	set_gpio_ctrl(GPIO_SMDK_UP);
	set_gpio_ctrl(GPIO_SMDK_DOWN);
	set_gpio_ctrl(GPIO_SMDK_LEFT);
	set_gpio_ctrl(GPIO_SMDK_RIGHT);

	init_timer(&button_check_timer);
	button_check_timer.function = tk_button_check_timer_handler;

	button_check_timer.expires = jiffies + (BUTTON_DELAY);
	add_timer(&button_check_timer);
}

void __exit s3c2410tk_button_exit(void)
{
	del_timer_sync(&button_check_timer);
}


BUTTON_TYPE gpio_buttons[] = {
	{ IRQ_SMDK_BT1, GPIO_SMDK_BT1, SCANCODE_POWER, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint2", keypad_handler, 
	  NULL, (void *) KEYPAD_BT1 },

  	{ IRQ_SMDK_BT2, GPIO_SMDK_BT2, SCANCODE_RECORD, LOW_ACTIVE, 
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint11", keypad_handler, 
	  NULL, (void *) KEYPAD_BT2 },
	  
	{ IRQ_SMDK_BT3, GPIO_SMDK_BT3, SCANCODE_ACTION, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint19", keypad_handler, 
	  NULL, (void *) KEYPAD_BT3 },
	  
	{ IRQ_SMDK_BT4, GPIO_SMDK_BT4, SCANCODE_SLIDE_UP, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint10", keypad_handler, 
	  NULL, (void *) KEYPAD_BT4 },
	  
	{ IRQ_SMDK_BT5, GPIO_SMDK_BT5, SCANCODE_SLIDE_CENTER, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint13", keypad_handler, 
	  NULL, (void *) KEYPAD_BT5 },
	  
	{ IRQ_SMDK_BT6, GPIO_SMDK_BT6, SCANCODE_SLIDE_DOWN, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint14", keypad_handler, 
	  NULL, (void *) KEYPAD_BT6 },
	  
	{ IRQ_SMDK_BT7, GPIO_SMDK_BT7, SCANCODE_ENTER, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint15", keypad_handler, 
	  NULL, (void *) KEYPAD_BT7 },

#if CONFIG_PM
	{ IRQ_SMDK_POWER_BT, GPIO_SMDK_POWER_BT, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "Power", pwButtonHandler, 
	  NULL, NULL },
#else
	{ IRQ_SMDK_POWER_BT, GPIO_SMDK_POWER_BT, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "Power", NULL, NULL, NULL },
#endif
	END_OF_LIST2
};
