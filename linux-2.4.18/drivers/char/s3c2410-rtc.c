/*
 * Real Time Clock interface for Samsung S3C2410 MPU
 *
 * Based on linux/drivers/char/s3c2400-rtc.c
 *
 * Copyright (C) 2003 MIZI Research, Inc.
 *
 * Author: Chan Gyun Jeong <cgjeong@mizi.com>
 * $Id: s3c2410-rtc.c,v 1.1.1.1 2004/02/04 12:56:04 laputa Exp $
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/rtc.h>

#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#define DRIVER_VERSION		"0.1"

#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val)		((val)=((val)&15) + ((val)>>4)*10)
#endif

#ifndef BIN_TO_BCD
#define BIN_TO_BCD(val)		((val)=(((val)/10)<<4) + (val)%10)
#endif

#define RTC_LEAP_YEAR		2000

/* Those are the bits from a classic RTC we want to mimic */
#ifndef RTC_IRQF
#define RTC_IRQF		0x80	/* any of the following 3 is active */
#endif
#ifndef RTC_PF
#define RTC_PF			0x40
#endif
#ifndef RTC_AF
#define RTC_AF			0x20
#endif
#ifndef RTC_UF
#define RTC_UF			0x10
#endif

static unsigned long rtc_status;
static unsigned long rtc_irq_data;

static struct fasync_struct *rtc_async_queue;
static DECLARE_WAIT_QUEUE_HEAD(rtc_wait);

extern spinlock_t rtc_lock;

static void get_rtc_time(int alm, struct rtc_time *rtc_tm)
{
	spin_lock_irq(&rtc_lock);
	if (alm == 1) {
		rtc_tm->tm_year = (unsigned char)ALMYEAR & Msk_RTCYEAR;
		rtc_tm->tm_mon = (unsigned char)ALMMON & Msk_RTCMON;
		rtc_tm->tm_mday = (unsigned char)ALMDAY & Msk_RTCDAY;
		rtc_tm->tm_hour = (unsigned char)ALMHOUR & Msk_RTCHOUR;
		rtc_tm->tm_min = (unsigned char)ALMMIN & Msk_RTCMIN;
		rtc_tm->tm_sec = (unsigned char)ALMSEC & Msk_RTCSEC;
	}
	else {
 read_rtc_bcd_time:
		rtc_tm->tm_year = (unsigned char)BCDYEAR & Msk_RTCYEAR;
		rtc_tm->tm_mon = (unsigned char)BCDMON & Msk_RTCMON;
//		rtc_tm->tm_mday = (unsigned char)BCDDAY & Msk_RTCDAY;
		rtc_tm->tm_mday = (unsigned char)BCDDATE & Msk_RTCDAY;
		rtc_tm->tm_hour = (unsigned char)BCDHOUR & Msk_RTCHOUR;
		rtc_tm->tm_min = (unsigned char)BCDMIN & Msk_RTCMIN;
		rtc_tm->tm_sec = (unsigned char)BCDSEC & Msk_RTCSEC;

		if (rtc_tm->tm_sec == 0) {
			/* Re-read all BCD registers in case of BCDSEC is 0.
			   See RTC section at the manual for more info. */
			goto read_rtc_bcd_time;
		}
	}
	
	spin_unlock_irq(&rtc_lock);
	
//	printk("TIME S:%d M:%d H:%d D:%d M:%d Y:%d \n",rtc_tm->tm_sec,rtc_tm->tm_min,rtc_tm->tm_hour,rtc_tm->tm_mday,rtc_tm->tm_mon,rtc_tm->tm_year);
	

	BCD_TO_BIN(rtc_tm->tm_year);
	BCD_TO_BIN(rtc_tm->tm_mon);
	BCD_TO_BIN(rtc_tm->tm_mday);
	BCD_TO_BIN(rtc_tm->tm_hour);
	BCD_TO_BIN(rtc_tm->tm_min);
	BCD_TO_BIN(rtc_tm->tm_sec);
	
	printk("TIME S:%d M:%d H:%d D:%d M:%d Y:%d \n",rtc_tm->tm_sec,rtc_tm->tm_min,rtc_tm->tm_hour,rtc_tm->tm_mday,rtc_tm->tm_mon,rtc_tm->tm_year);

	/* The epoch of tm_year is 1900 */
	rtc_tm->tm_year += RTC_LEAP_YEAR - 1900;

	/* tm_mon starts at 0, but rtc month starts at 1 */
	rtc_tm->tm_mon--;
}

static int set_rtc_time(int alm, struct rtc_time *rtc_tm)
{
	unsigned char year, mon, day, hour, min, sec;

	if (rtc_tm->tm_year < (RTC_LEAP_YEAR - 1900)) {
		/* The year must be higher or equal than 2000 */
		return -EINVAL;
	}

	year = (rtc_tm->tm_year + 1900) - RTC_LEAP_YEAR;
	mon = rtc_tm->tm_mon + 1; /* tm_mon starts at zero */
	day = rtc_tm->tm_mday;
	hour = rtc_tm->tm_hour;
	min = rtc_tm->tm_min;
	sec = rtc_tm->tm_sec;

	if(alm)	
	printk("ALM S:%d M:%d H:%d D:%d M:%d Y:%d \n", sec, min, hour, day, mon,year);
	

	BIN_TO_BCD(sec);
	BIN_TO_BCD(min);
	BIN_TO_BCD(hour);
	BIN_TO_BCD(day);
	BIN_TO_BCD(mon);
	BIN_TO_BCD(year);

	spin_lock_irq(&rtc_lock);
 	/* Enable RTC control */
	RTCCON |= RTCCON_EN; 

	if (alm) {
		ALMSEC = sec & Msk_RTCSEC;
		ALMMIN = min & Msk_RTCMIN;
		ALMHOUR = hour & Msk_RTCHOUR;
		ALMDAY = day & Msk_RTCDAY;
		ALMMON = mon & Msk_RTCMON;
		ALMYEAR = year & Msk_RTCYEAR;
		
	} 
	else {
		BCDSEC = sec & Msk_RTCSEC;
		BCDMIN = min & Msk_RTCMIN;
		BCDHOUR = hour & Msk_RTCHOUR;
//		BCDDAY = day & Msk_RTCDAY;
		BCDDATE = day & Msk_RTCDAY;
		BCDMON = mon & Msk_RTCMON;
		BCDYEAR = year & Msk_RTCYEAR;
	}

	/* Disable RTC control */
	RTCCON &= ~RTCCON_EN;
	spin_unlock_irq(&rtc_lock);

	return 0;
}

static void rtc_alm_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	spin_lock (&rtc_lock);
	/* Update IRQ data & counter */
	rtc_irq_data += 0x100;
	rtc_irq_data |= (RTC_AF|RTC_IRQF);
	spin_unlock (&rtc_lock);

	/* Now do the rest of the actions */
	wake_up_interruptible(&rtc_wait);	

	kill_fasync (&rtc_async_queue, SIGIO, POLL_IN);
}

static int rtc_open(struct inode *inode, struct file *file)
{
	MOD_INC_USE_COUNT;

	spin_lock_irq(&rtc_lock);

	if (rtc_status) {
		spin_unlock_irq(&rtc_lock);
		MOD_DEC_USE_COUNT;
		return -EBUSY;
	}

	rtc_status = 1;
	rtc_irq_data = 0;

	spin_unlock_irq(&rtc_lock);

	return 0;
}

static int rtc_release(struct inode *inode, struct file *file)
{
	/*
	 * Turn off all interrupts once the device is no longer
	 * in use, and clear the data.
	 */
	spin_lock_irq(&rtc_lock);

#ifndef CONFIG_MIZI
	/* Disable alarm interrupt */
	RTCCON |= RTCCON_EN;
	RTCALM = RTCALM_DIS;
	RTCCON &= ~RTCCON_EN;
#endif

	rtc_status = 0;
	rtc_irq_data = 0;

	spin_unlock_irq(&rtc_lock);

	MOD_DEC_USE_COUNT;

	return 0;
}

static int rtc_fasync (int fd, struct file *filp, int on)
{
	return fasync_helper (fd, filp, on, &rtc_async_queue);
}

static unsigned int rtc_poll(struct file *file, poll_table *wait)
{
	unsigned long l;

	poll_wait (file, &rtc_wait, wait);

	spin_lock_irq (&rtc_lock);
	l = rtc_irq_data;
	spin_unlock_irq (&rtc_lock);

	if (l != 0) { 
		return POLLIN | POLLRDNORM;
	}
	return 0;
}

ssize_t rtc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long data;
	ssize_t retval;

	if (count < sizeof(unsigned long))
		return -EINVAL;

	add_wait_queue(&rtc_wait, &wait);
	set_current_state(TASK_INTERRUPTIBLE);

	while (1) {
		spin_lock_irq (&rtc_lock);
		data = rtc_irq_data;
		rtc_irq_data = 0;
		spin_unlock_irq (&rtc_lock);

		if (data != 0) {
			break;
		}
		
		if (file->f_flags & O_NONBLOCK) {
			retval = -EAGAIN;
			goto out;
		}

		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			goto out;
		}
		schedule();
	}

	retval = put_user(data, (unsigned long *)buf);
	if (!retval)
		retval = sizeof(unsigned long);

out:
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&rtc_wait, &wait);
	return retval;
}

static int rtc_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	struct rtc_time tm;

	switch (cmd) {
		/* Alarm interrupt on/off */
	case RTC_AIE_ON:
		spin_lock_irq (&rtc_lock);
		RTCCON |= RTCCON_EN; 
		RTCALM = RTCALM_EN; 
		RTCCON &= ~RTCCON_EN;
		spin_unlock_irq (&rtc_lock);
		
		printk("RTC_AIE_ON\n");
		return 0;

	case RTC_AIE_OFF:
		spin_lock_irq (&rtc_lock);
		RTCCON |= RTCCON_EN;
		RTCALM = RTCALM_DIS;
		RTCCON &= ~RTCCON_EN;
		spin_unlock_irq (&rtc_lock);
		return 0;

		/* Update interrupt on/off */
	case RTC_UIE_ON:
	case RTC_UIE_OFF:

		/* Periodic interrupt on/off */
	case RTC_PIE_ON:
	case RTC_PIE_OFF:
			
	case RTC_WIE_ON:
	case RTC_WIE_OFF:

		/* Periodic interrupt freq get/set */
	case RTC_IRQP_READ:
	case RTC_IRQP_SET:

		/* Epoch get/set */
	case RTC_EPOCH_READ:
	case RTC_EPOCH_SET:

	case RTC_WKALM_SET:
	case RTC_WKALM_RD:
		/* Not supported */
		return -EINVAL;

		/* Alarm get/set */
	case RTC_ALM_READ:
		get_rtc_time(1, &tm);
		return copy_to_user((void *)arg, &tm, sizeof(tm)) ? 
			-EFAULT : 0;

	case RTC_ALM_SET:
	
		if (copy_from_user(&tm, (struct rtc_time*) arg, sizeof(tm))) {
			return -EFAULT;
		}
		
		printk("RTC_ALM_SET\n");
		
		return set_rtc_time(1, &tm);

		/* Time get/set */
	case RTC_RD_TIME:
		get_rtc_time(0, &tm);
		return copy_to_user((void *)arg, &tm, sizeof(tm)) ? 
			-EFAULT : 0;

	case RTC_SET_TIME:
		if (copy_from_user(&tm, (struct rtc_time*) arg, sizeof(tm))) {
			return -EFAULT;
		}
		return set_rtc_time(0, &tm);

	default:
		return -EINVAL;
	}
}

#ifdef CONFIG_PROC_FS
static int rtc_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
	char *p = page;
	int len;
	struct rtc_time tm;

	get_rtc_time(0, &tm);

	p += sprintf(p, "rtc_time\t: %02d:%02d:%02d\n"
		     "rtc_date\t: %04d-%02d-%02d\n"
		     "rtc_epoch\t: %04d\n",
		     tm.tm_hour, tm.tm_min, tm.tm_sec,
		     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
		     RTC_LEAP_YEAR);

	get_rtc_time(1, &tm);

	p += sprintf(p, "alarm_time\t: %02d:%02d:%02d\n"
		     "alarm_date\t: %04d-%02d-%02d\n",
		     tm.tm_hour, tm.tm_min, tm.tm_sec,
		     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	p += sprintf(p, "alarm_IRQ\t: %s\n", 
		     (RTCALM & RTCALM_EN) ? "yes" : "no" );

	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}
#endif

static struct file_operations rtc_fops = {
	owner:		THIS_MODULE,
	llseek:		no_llseek,
	read:		rtc_read,
	poll:		rtc_poll,
	ioctl:		rtc_ioctl,
	open:		rtc_open,
	release:	rtc_release,
	fasync:		rtc_fasync,
};

static struct miscdevice rtc_dev=
{
	RTC_MINOR,
	"rtc",
	&rtc_fops
};

static int __init rtc_init(void)
{
	int ret;

	misc_register(&rtc_dev);

#ifdef CONFIG_PROC_FS
	create_proc_read_entry("driver/rtc", 0, 0, rtc_read_proc, NULL);
#endif

	ret = request_irq(IRQ_RTC, rtc_alm_interrupt, SA_INTERRUPT, 
			  "RTC Alarm", NULL);
	if (ret) {
		printk("s3c2410-rtc: failed to register IRQ_RTC(%d)\n", 
		       IRQ_RTC);
		goto IRQ_RTC_failed;
	}

	printk("S3C2410 Real Time Clock Driver v" DRIVER_VERSION "\n");

	return 0;

IRQ_RTC_failed:
#ifdef CONFIG_PROC_FS
	remove_proc_entry("driver/rtc", NULL);
#endif
	misc_deregister(&rtc_dev);
	return ret;
}

static void __exit rtc_exit(void)
{
	free_irq(IRQ_RTC, NULL);

#ifdef CONFIG_PROC_FS
	remove_proc_entry("driver/rtc", NULL);
#endif

	misc_deregister(&rtc_dev);
}

module_init(rtc_init);
module_exit(rtc_exit);

MODULE_AUTHOR("Chan Gyun Jeong <cgjeong@mizi.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Real Time Clock interface for Samsung S3C2410 MPU");
EXPORT_NO_SYMBOLS;
