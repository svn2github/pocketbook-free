/*
 * bios-less APM driver for ARM Linux 
 *  Jamey Hicks <jamey@crl.dec.com>
 *  adapted from the APM BIOS driver for Linux by Stephen Rothwell (sfr@linuxcare.com)
 *
 * APM 1.2 Reference:
 *   Intel Corporation, Microsoft Corporation. Advanced Power Management
 *   (APM) BIOS Interface Specification, Revision 1.2, February 1996.
 *
 * [This document is available from Microsoft at:
 *    http://www.microsoft.com/hwdev/busbios/amp_12.htm]
 *
   Thr 29 Nov 2001 Nandy Lyu <nandy@mizi.com>
   - Modified for MIZI Power Management

   Mon 14 Jan 2002 Yong-iL Joh <tolkien@mizi.com>
   - modified followed by MIZI's "kernel vs Application API spec (0.3, draft)"

   Fri May 10 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.33

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/apm_bios.h>
#include <linux/pm.h>
#include <linux/errno.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>

/*
 *  Debug macros 
 */
/* 0 : Quiet 1 : Audible 2 : Loud 3 : Noisy */
#undef CONFIG_APM_DEBUG	1
#ifdef CONFIG_APM_DEBUG
#define DEBUG(n, args...)		\
	if (n <= CONFIG_APM_DEBUG) {	\
	    printk(KERN_INFO args);	\
	}
#else
#define DEBUG(n, args...)
#endif

extern wait_queue_head_t mz_event_queue;
#define INCBUF(x,mod) (((x)+1) & ((mod) - 1))
extern struct mz_event_queue_t mz_event_q;
extern int pm_suggest_suspend(void);
extern int pm_do_suspend(void);

static int apm_bios_ioctl(struct inode * inode, struct file *filp,
		    u_int cmd, u_long arg)
{
    int ret = 0;
    struct pm_dev *pm_dev = NULL;
    static int apm_lcd_status = LCD_ON;

    switch (cmd) {
    case APM_IOC_SUSPEND:
      pm_suggest_suspend();
      break;
    case APM_MZ_SLEEP:
      pm_do_suspend();
      break;
    case APM_LCD_OFF:
      DEBUG(2, __FILE__ ": LCD OFF\n");
      while ((pm_dev = pm_find(PM_USER_DEV, pm_dev)) != NULL) {
	if ((pm_dev->id == PM_USER_LCD) || (pm_dev->id == PM_USER_LIGHT)) {
	  DEBUG(2, __FILE__ ": find LCD device\n");
	  ret = pm_send(pm_dev, PM_SUSPEND, (void *)2);
	  if (ret) {
	    DEBUG(1, __FILE__ ": error in pm_send(0x%lx)\n", pm_dev->id);
	    return ret;
	  }
	}
      }
      apm_lcd_status = LCD_OFF;
      break;
    case APM_LCD_ON:
      DEBUG(2, __FILE__ ": LCD ON\n");
      while ((pm_dev = pm_find(PM_USER_DEV, pm_dev)) != NULL) {
	if ((pm_dev->id == PM_USER_LCD) || (pm_dev->id == PM_USER_LIGHT)) {
	  DEBUG(2, __FILE__ ": find LCD device\n");
	  ret = pm_send(pm_dev, PM_RESUME, (void *)0);
	  if (ret) {
	    DEBUG(1, __FILE__ ": error in pm_send(0x%lx)\n", pm_dev->id);
	    return ret;
	  }
	}
      }
      apm_lcd_status = LCD_ON;
      break;
    case APM_DEV_LIST:
#ifdef CONFIG_APM_DEBUG
      printk("type \t\t id \t\t state \t prev_state \n");
      while ((pm_dev = pm_find(PM_UNKNOWN_DEV, pm_dev)) != NULL) {
	printk("0x%08x \t 0x%08lx \t %d \t %d\n",
	       (int)pm_dev->type, (unsigned long)pm_dev->id,
	       (int)pm_dev->state, (int)pm_dev->prev_state);
      }
#endif
      break;
    case APM_DEV_ONOFF: {
      struct pm_dev *pm_dev2;

      pm_dev2 = (struct pm_dev *)kmalloc(sizeof(struct pm_dev), GFP_KERNEL);
      if (pm_dev2 == NULL)
	return -ENOMEM;

      ret = copy_from_user(pm_dev2, (struct pm_dev *)arg,
			   sizeof(struct pm_dev));
      if (ret)
	return ret;

      while ((pm_dev = pm_find(pm_dev2->type, pm_dev)) != NULL) {
	if (pm_dev->id == pm_dev2->id)
	  ret = pm_send(pm_dev, pm_dev2->state, pm_dev2->data);
	break;
      }

      kfree(pm_dev2);
      return ret;
    } break;
    case GET_BATTERY_STATUS: {
      BATTERY_RET bat_dev = {
	sec:	0,
	level:	-1,
	ac:	AC_UNKNOWN,
	battery: BATTERY_UNKNOWN,
      };

      if (mz_pm_ops.get_power_status == NULL)
	return -EIO;

      if (!(ret = (*(mz_pm_ops.get_power_status))(&bat_dev))) {
	if (copy_to_user((BATTERY_RET *)arg, &bat_dev, sizeof(BATTERY_RET)))
	    return -EINVAL;
      } else return -EIO;
    } break;
    case GET_JIFFIES:
      return put_user(pm_last_jiffies, (unsigned long *)arg);
      break;
    case GET_LCD_STATUS:
      return put_user(apm_lcd_status, (unsigned int *)arg);
      break;
    case GET_MZ_EVENT:
      if (mz_event_q.head == mz_event_q.tail)
	return -EAGAIN;
      else {
	ret = put_user(mz_event_q.buf[mz_event_q.tail], (unsigned long *)arg);
	mz_event_q.tail = INCBUF(mz_event_q.tail, MZ_EVENT_BUF_SIZE);
	return ret;
      }
      break;
    case SET_INPUT_DEV: {
      unsigned int apm_input_tmp;

      ret = get_user(apm_input_tmp, (unsigned int *)arg);
      if (ret)
	return ret;

      while ((pm_dev = pm_find(PM_USER_DEV, pm_dev)) != NULL) {
	if (pm_dev->id == PM_USER_INPUT) {
	  if (apm_input_tmp == INPUT_DEV_ON)
	    ret = pm_send(pm_dev, PM_RESUME, NULL);
	  else if (apm_input_tmp == INPUT_DEV_OFF)
	    ret = pm_send(pm_dev, PM_SUSPEND, NULL);
	  else
	    return -EINVAL;
	}
      }
      return ret;
    } break;
    default:
      return -ENOIOCTLCMD;
    }
    return 0;
}

static unsigned int apm_bios_poll(struct file *filp, poll_table *wait)
{
    poll_wait(filp, &mz_event_queue, wait);

    return (mz_event_q.head == mz_event_q.tail ? 0 : (POLLIN | POLLRDNORM));
}

struct file_operations apm_bios_fops = {
    owner:	THIS_MODULE,
    ioctl:	apm_bios_ioctl,
    poll:	apm_bios_poll,
};

static struct miscdevice apm_device = {
	MISC_DYNAMIC_MINOR,
	"apm_bios",
	&apm_bios_fops
};

/*
 * Just start the APM thread. We do NOT want to do APM BIOS
 * calls from anything but the APM thread, if for no other reason
 * than the fact that we don't trust the APM BIOS. This way,
 * most common APM BIOS problems that lead to protection errors
 * etc will have at least some level of being contained...
 *
 * In short, if something bad happens, at least we have a choice
 * of just killing the apm thread..
 */
static int __init apm_init(void)
{
    if (PM_IS_ACTIVE()) {
      printk(KERN_NOTICE "apm: overridden by ACPI.\n");
      return -1;
    }
    pm_active = 1;
    misc_register(&apm_device);

    return 0;
}

static void __exit apm_exit(void)
{
    misc_deregister(&apm_device);
    pm_active = 0;
}

module_init(apm_init);
module_exit(apm_exit);

MODULE_AUTHOR("Yong-iL Joh");
MODULE_DESCRIPTION("Advanced Power Management by MIZI");

EXPORT_NO_SYMBOLS;
