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
#define DEBUG
#include "s3c2410_gpio_button.h"

BUTTON_TYPE gpio_buttons[] = {
	{ IRQ_SMDK_BT2, GPIO_SMDK_BT2, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint19", NULL, NULL },
	{ IRQ_SMDK_BT1, GPIO_SMDK_BT1, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint11", NULL, NULL },
	{ IRQ_SMDK_BT0, GPIO_SMDK_BT0, 0, LOW_ACTIVE, 
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "eint2", NULL, NULL },
#if CONFIG_PM
	{ IRQ_SMDK_POWER_BT, GPIO_SMDK_POWER_BT, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "Power", pwButtonHandler, NULL },
#else
	{ IRQ_SMDK_POWER_BT, GPIO_SMDK_POWER_BT, 0, LOW_ACTIVE,
	  GPIO_BOTH_EDGES, GPIO_PULLUP_EN, "Power", NULL, NULL },
#endif
	END_OF_LIST2
};
