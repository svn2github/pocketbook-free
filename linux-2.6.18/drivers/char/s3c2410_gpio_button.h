/*
 * s3c2410_gpio_button.h
 *
 * generic routine for S3C2410-base machine's button
 *
 * Based on sa1100_gpio_button.h
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:04 $
 *
 * $Revision: 1.1.1.1 $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/pm.h>

#ifndef _S3C2410_GPIO_BUTTON_H_
#define _S3C2410_GPIO_BUTTON_H_

typedef struct button_type {
	int irq;
	u32 gpio;
	u8 scancode;
	u8 active;
	u8 edge;
	u32 pullup;
	char *name;
	void (*handler)(int status, void *priv);
	int (*pm_callback)(pm_request_t req, void *data);
	void *priv;
} BUTTON_TYPE;
#define HIGH_ACTIVE	KEY_PRESSED
#define LOW_ACTIVE	KEY_RELEASED

#define END_OF_LIST	0xff
#define MZ_NO_IRQ	END_OF_LIST
#define END_OF_LIST2	{ MZ_NO_IRQ, END_OF_LIST, 0, 0, 0, NULL, NULL, NULL}
#define BTN_INIT	PM_MZ_INIT
#define BTN_EXIT	PM_MZ_EXIT

#define GPIO_LOW_LEVEL		0
#define GPIO_HIGH_LEVEL		1

#if 0
#define GPIO_FALLING_EDGE	2
#define GPIO_RISING_EDGE	4
#define GPIO_BOTH_EDGES		6
#else
#define GPIO_FALLING_EDGE	EXT_FALLING_EDGE
#define GPIO_RISING_EDGE	EXT_RISING_EDGE
#define GPIO_BOTH_EDGES		EXT_BOTH_EDGES
#endif

extern void pwButtonHandler(int status);
extern struct pm_dev *gpio_button_pm_dev;
extern BUTTON_TYPE gpio_buttons[];
extern struct tq_struct power_button_task;
#define DO_POWER_BUTTON_TASK	schedule_task(&power_button_task)

#ifdef DEBUG
#define LOG(fm, ar...)	printk(__FUNCTION__"(%d): " fm, __LINE__, ##ar)
#else
#define LOG(fm, ar...)
#endif

#endif /* _S3C2410_GPIO_BUTTON_H_ */
