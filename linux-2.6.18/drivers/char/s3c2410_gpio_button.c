/*
 * s3c2410_gpio_button.c
 *
 * genric routine for S3C2410-base machine's button
 *
 * Based on sa1100_gpio_button.c
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:04 $
 *
 * $Revision: 1.1.1.1 $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 *
 * 2002-09-03  Initial code by nandy
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/hardware.h>

#include <asm/delay.h>
#include <linux/kbd_ll.h>
#include <linux/irq.h>

#undef DEBUG
#include "s3c2410_gpio_button.h"

static BUTTON_TYPE *btn_list = (BUTTON_TYPE *)&gpio_buttons[0];

#ifdef CONFIG_PM
struct pm_dev *gpio_button_pm_dev;

static int
gpio_button_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{
	BUTTON_TYPE *buttons = btn_list;

	if (req == PM_SUSPEND) {
		while (buttons->gpio != END_OF_LIST) {
			if (buttons->pm_callback != NULL)
				buttons->pm_callback(req, data);
			else {
				if (buttons->irq != MZ_NO_IRQ)
					disable_irq(buttons->irq);
			}
			buttons++;
		}
	} else if (req == PM_RESUME) {
		while (buttons->gpio != END_OF_LIST) {
			if (buttons->pm_callback != NULL)
				buttons->pm_callback(req, data);
			else {
				if (buttons->irq != MZ_NO_IRQ)
					enable_irq(buttons->irq);
			}
			buttons++;
		}
	}

	return 0;
}

static int power_button_pushed = 0;
static void
power_button_task_handler(void *data)
{
	extern void pm_user_suspend(void);
	pm_user_suspend();
}

struct tq_struct power_button_task = {
	routine: power_button_task_handler
};

static void
pwButton_timer_callback(unsigned long nr)
{
	//handle_scancode(SCANCODE_POWER, KEY_PRESSED);
	//handle_scancode(SCANCODE_POWER, KEY_RELEASED);
	power_button_pushed = 0;
}

static struct timer_list pwB_timer = { function : pwButton_timer_callback };
#endif

void
pwButtonHandler(int isButtonDown) 
{
#ifdef CONFIG_PM
#if 0
	printk("This is Power Button handler\n");
	printk("isButtonDown is %d\n", isButtonDown);
	if (isButtonDown) {
		if (!power_button_pushed) {
			power_button_pushed = 1;
			mod_timer(&pwB_timer, jiffies + HZ);
		}
	} else if (del_timer_sync(&pwB_timer)) {
		if (power_button_pushed) {
			printk("Go Go Go\n");
			DO_POWER_BUTTON_TASK;
			power_button_pushed = 0;
		}
	}
	pm_access(gpio_button_pm_dev);
#else
		if (!power_button_pushed) {
			printk("Go push\n");
			DO_POWER_BUTTON_TASK;
			power_button_pushed = 1;
		} else {
			power_button_pushed = 0;
		}
#endif
#endif
}

static void
gpio_button_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	BUTTON_TYPE *buttons = (BUTTON_TYPE *)dev_id;
	int status;

	while (buttons->gpio != END_OF_LIST) {
		if (irq == buttons->irq) {
			status = read_gpio_bit(buttons->gpio) ?
				buttons->active ^ KEY_RELEASED : buttons->active ^ KEY_PRESSED;
			LOG("STATUS is %d\n", status);
			if (buttons->handler != NULL) {
				LOG("(%d) handler calling (%s)..\n", irq, buttons->name);
				buttons->handler(status, buttons->priv);
			} else {
				LOG("%s(%02x), st = %d\n", buttons->name,
					buttons->scancode, status);
				handle_scancode(buttons->scancode, status);
			}
			pm_access(gpio_button_pm_dev);
			break;
		}
		buttons++;
	}
}

static int __init
init_gpio_button(void)
{
	int ret;
	BUTTON_TYPE *buttons = btn_list;

	printk("S3C2410 GPIO buttons: ");

	while (buttons->gpio != END_OF_LIST) {
		if (buttons->irq != MZ_NO_IRQ) {
			ret = set_external_irq(buttons->irq, buttons->edge, \
						buttons->pullup);
			if (ret) {
				printk(" gpio return [%x]",ret);
				return ret;
			}
			ret = request_irq(buttons->irq, gpio_button_interrupt,
					  SA_INTERRUPT, buttons->name, btn_list);
			if (ret) {
				printk(" button IRQ : %d \n", buttons->irq);
				goto fail;
			}
		} else {
			if (buttons->pm_callback != NULL)
				buttons->pm_callback(PM_MZ_INIT, NULL);
		}
		if (buttons->name)
			printk("%s%s ", buttons->name, buttons->handler ? "(special)":"");
		buttons++;
	}
	printk("\n");

#ifdef CONFIG_PM
	init_timer(&pwB_timer);
	gpio_button_pm_dev = pm_register(PM_USER_DEV, PM_USER_INPUT,
					 gpio_button_pm_callback);
#endif


#if defined(CONFIG_S3C2410_SMDK)
	{
		void *s3c2410tk_button_init(void);
		s3c2410tk_button_init();
	}
#endif
	return 0;

fail:
	if (buttons == btn_list)
		return ret;

	do {
		buttons--;
		if (buttons->irq != MZ_NO_IRQ) {
			free_irq(buttons->irq, btn_list);
		} else {
			if (buttons->pm_callback != NULL)
				buttons->pm_callback(PM_MZ_EXIT, NULL);
		}
	} while (buttons != btn_list);

	return ret;
}

static void __exit exit_gpio_button(void)
{
	BUTTON_TYPE *buttons = btn_list;



#if defined(CONFIG_S3C2410_SMDK)
	{
		void *s3c2410tk_button_exit(void);
		s3c2410tk_button_exit();
	}
#endif

	while (buttons->gpio != END_OF_LIST) {
		if (buttons->irq != MZ_NO_IRQ) {
			free_irq(buttons->irq, btn_list);
		} else {
			if (buttons->pm_callback != NULL)
				buttons->pm_callback(PM_MZ_EXIT, NULL);
		}
		buttons++;
	}

#ifdef CONFIG_PM
	del_timer_sync(&pwB_timer);
	pm_unregister(gpio_button_pm_dev);
#endif

}

module_init(init_gpio_button);
module_exit(exit_gpio_button);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Janghoon Lyu <nandy@mizi.com>");
MODULE_DESCRIPTION("Generic gpio(button) driver for S3C2410");
