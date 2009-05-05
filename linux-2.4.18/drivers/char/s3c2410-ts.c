/*
 * s3c2410-ts.c
 *
 * touchScreen driver for SAMSUNG S3C2410
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:04 $ 
 *
 * $Revision: 1.1.1.1 $
 *
 * Based on pt036001b-ts.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * History:
 *
 * 2002-05-27: Janghoon Lyu <nandy@mizi.com>
 *    - Add HOOK_FOR_DRAG
 * 
 * 2003-09-18 : kwanghyun LA<nala.la@samsung.com)
 *    - Add ADC porting optimize
 *    - Dynamic adc convert freq. set
 *    - 20MHz ad convertion freq set
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/irq.h>

//laputa append for get bus clk freq
#include <asm/arch/S3C2410.h>
#include <asm/arch/cpu_s3c2410.h>
#define  ADC_FREQ	2000000           // 2MHz AD convert freq 030918
static int PreScale_n;			      // PCLK / (PreScale_n+1) = ADConversion freq.
//laputa end 030918


#include <asm/hardware.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

/* debug macros */
#undef DEBUG
#ifdef DEBUG
#define DPRINTK( x... )	printk("s3c2410-ts: " ##x)
#else
#define DPRINTK( x... )
#endif

#define PEN_UP	        0		
#define PEN_DOWN	1
#define PEN_FLEETING	2
#define MAX_TS_BUF	16	/* how many do we want to buffer */

#undef USE_ASYNC	1
#define DEVICE_NAME	"s3c2410-ts"
#define TSRAW_MINOR	1

typedef struct {
	unsigned int penStatus;		/* PEN_UP, PEN_DOWN, PEN_SAMPLE */
	TS_RET buf[MAX_TS_BUF];		/* protect against overrun */
	unsigned int head, tail;	/* head and tail for queued events */
	wait_queue_head_t wq;
	spinlock_t lock;
#ifdef USE_ASYNC
	struct fasync_struct *aq;
#endif
#ifdef CONFIG_PM
	struct pm_dev *pm_dev;
#endif
} TS_DEV;

static TS_DEV tsdev;

#define BUF_HEAD	(tsdev.buf[tsdev.head])
#define BUF_TAIL	(tsdev.buf[tsdev.tail])
#define INCBUF(x,mod) 	((++(x)) & ((mod) - 1))

/** add by wanghp (Joe)**/
//#define SCREEN_MIN_X 
//#define SCREEN_MIN_Y

#define FILTER_LIMIT 5

#define ABS(x,y) (x)>(y)?((x)-(y)):((y)-(x))

static int tsMajor = 0;

static void (*tsEvent)(void);

#define HOOK_FOR_DRAG  
#ifdef HOOK_FOR_DRAG
#define TS_TIMER_DELAY  (HZ/100) /* 10 ms */
static struct timer_list ts_timer;
#endif

#define wait_down_int()	{ ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
				XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
				XP_PST(WAIT_INT_MODE); }
#define wait_up_int()	{ ADCTSC = UP_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | \
				YP_AIN | YM_GND | XP_PST(WAIT_INT_MODE); }
#define mode_x_axis()	{ ADCTSC = XP_EXTVLT | XM_GND | YP_AIN | YM_HIZ | \
				XP_PULL_UP_DIS | XP_PST(X_AXIS_MODE); }
#define mode_x_axis_n()	{ ADCTSC = XP_EXTVLT | XM_GND | YP_AIN | YM_HIZ | \
				XP_PULL_UP_DIS | XP_PST(NOP_MODE); }
#define mode_y_axis()	{ ADCTSC = XP_AIN | XM_HIZ | YP_EXTVLT | YM_GND | \
				XP_PULL_UP_DIS | XP_PST(Y_AXIS_MODE); }
				
// laputa modi for 66MHz/(32+1) = 2MHz
// 1/(2MHz/5cycle) = 2.5uS
#define start_adc_x(n)	{ ADCCON = PRESCALE_EN | PRSCVL(n) | \
				ADC_INPUT(ADC_IN5) | ADC_START_BY_RD_EN | \
				ADC_NORMAL_MODE; \
			  ADCDAT0; }
#define start_adc_y(n)	{ ADCCON = PRESCALE_EN | PRSCVL(n) | \
				ADC_INPUT(ADC_IN7) | ADC_START_BY_RD_EN | \
				ADC_NORMAL_MODE; \
			  ADCDAT1; }
//laputa end 030917 
#define disable_ts_adc()	{ ADCCON &= ~(ADCCON_READ_START); }

			  

static int adc_state = 0;
static int x, y;	/* touch screen coorinates */

static unsigned short average_x = 0;
static unsigned short average_y = 0;;
static unsigned short average = 0;
static unsigned short average_last_x = 0;
static unsigned short average_last_y = 0;;

unsigned short temptotx = 0;
unsigned short temptoty = 0;
unsigned short tempcount = 0;
unsigned short tempoffset = 0;

//static unsigned int Have_UP = 1;

static void tsEvent_raw(void)
{
	if (tsdev.penStatus == PEN_DOWN) {
		BUF_HEAD.x = x;
		BUF_HEAD.y = y;
		BUF_HEAD.pressure = PEN_DOWN;

#ifdef HOOK_FOR_DRAG 
		ts_timer.expires = jiffies + TS_TIMER_DELAY;
		add_timer(&ts_timer);
#endif
	} else {
#ifdef HOOK_FOR_DRAG 
		del_timer(&ts_timer);
#endif
		
		BUF_HEAD.x = 0;		/*=0*/
		BUF_HEAD.y = 0;		/*=0*/
		BUF_HEAD.pressure = PEN_UP;
	}

	tsdev.head = INCBUF(tsdev.head, MAX_TS_BUF);
	wake_up_interruptible(&(tsdev.wq));
	
#ifdef USE_ASYNC
	if (tsdev.aq)
		kill_fasync(&(tsdev.aq), SIGIO, POLL_IN);
#endif

#ifdef CONFIG_PM
	pm_access(tsdev.pm_dev);
#endif
}

static int tsRead(TS_RET * ts_ret)
{
	DPRINTK("tsRead\n");
	spin_lock_irq(&(tsdev.lock));
	/******************
	**
	**add by wanghp(Joe)
	**
	*************************/
	ts_ret->pressure = BUF_TAIL.pressure;
	if(ts_ret->pressure == PEN_DOWN){

		tempoffset = tsdev.tail;
		temptotx = 0;
		temptoty = 0;
		tempcount = 0;
		while((tempoffset != tsdev.head)&& tsdev.buf[tempoffset].pressure == PEN_DOWN){
			temptotx += tsdev.buf[tempoffset].x;
			temptoty += tsdev.buf[tempoffset].y;
			tempcount++;
			tempoffset = ((tempoffset+1)&15);
		}
		if(tempcount < 5){
			if(tsdev.buf[tempoffset].pressure == PEN_UP){
//				while(tempcount--) 
					tsdev.tail = tempoffset;//INCBUF(tsdev.tail,MAX_TS_BUF);
			}
			spin_unlock_irq(&(tsdev.lock));
			return -1;
		}

		temptotx/=tempcount;
		temptoty/=tempcount;
		
		average_x = 0;
		average_y = 0;
		average = 0;
		
		while(tempcount--){
			if(ABS(BUF_TAIL.x,temptotx) < FILTER_LIMIT){
				if(ABS(BUF_TAIL.y,temptoty) < FILTER_LIMIT){
					average_x += BUF_TAIL.x;
					average_y += BUF_TAIL.y;
					average++;	
				}
				else;
			}
			else;
			tsdev.tail = INCBUF(tsdev.tail,MAX_TS_BUF);
		}
		if(average >= 3/* && Have_UP*/){		//if up and 
			ts_ret->x = average_y/average;
			ts_ret->y = average_x/average;
			ts_ret->pressure = PEN_DOWN;
			average_last_x	= ts_ret->x;
			average_last_y = ts_ret->y;
//			Have_UP = 0;
		}
		else {
			spin_unlock_irq(&(tsdev.lock));
			return -1;
		}
	}
	else if(ts_ret->pressure == PEN_UP){
/*		if(Have_UP) {	//if down
			average_x = average_y = average = 0;
			tsdev.tail = INCBUF(tsdev.tail,MAX_TS_BUF);
			spin_unlock_irq(&(tsdev.lock));
			return -1;
		}
		Have_UP = 1;		//up
*/
		if(average != 0){
			ts_ret->x = average_y/average;
			ts_ret->y = average_x/average;
		}
		else{
			if(BUF_TAIL.x != 0 && BUF_TAIL.y != 0){
				ts_ret->y = BUF_TAIL.x;
				ts_ret->x = BUF_TAIL.y;
			}
			else{
//				average_x = average_y = average = 0;
				ts_ret->x = average_last_x;
				ts_ret->y = average_last_y;
			}
		 }
		average_x = average_y = average = 0;
		tsdev.tail = INCBUF(tsdev.tail,MAX_TS_BUF);
	}

/*
	ts_ret->x = BUF_TAIL.x;
	ts_ret->y = BUF_TAIL.y;
	ts_ret->pressure = BUF_TAIL.pressure;
	tsdev.tail = INCBUF(tsdev.tail, MAX_TS_BUF);
*/
	spin_unlock_irq(&(tsdev.lock));
	return sizeof(TS_RET);
}


static ssize_t s3c2410_ts_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	TS_RET ts_ret;
retry: 
	if (tsdev.head != tsdev.tail){
		int count;
		count = tsRead(&ts_ret);
		if (count) copy_to_user(buffer, (char *)&ts_ret, count);
		return count;
	} else {
		if (filp->f_flags & O_NONBLOCK)		//-11
			return -EAGAIN;
		interruptible_sleep_on(&(tsdev.wq));
		if (signal_pending(current))
			return -ERESTARTSYS;
		goto retry;
	}
	return sizeof(TS_RET);
}

#ifdef USE_ASYNC
static int s3c2410_ts_fasync(int fd, struct file *filp, int mode) 
{
	return fasync_helper(fd, filp, mode, &(tsdev.aq));
}
#endif

static unsigned int s3c2410_ts_poll(struct file *filp, struct poll_table_struct *wait)
{
	poll_wait(filp, &(tsdev.wq), wait);
	return (tsdev.head == tsdev.tail) ? 0 : (POLLIN | POLLRDNORM); 
}

static inline void start_ts_adc(void)
{
	adc_state = 0;
	mode_x_axis();
	start_adc_x(PreScale_n);	 //+- modify for ADC freq 030918
}

static inline void s3c2410_get_XY(void)
{
	
	if (adc_state == 0) { 
		adc_state = 1;
		disable_ts_adc();
		y = (ADCDAT0 & 0x3ff); 
		mode_y_axis();
		start_adc_y(PreScale_n); //+- modify for ADC freq 030918
	} else if (adc_state == 1) { 
		adc_state = 0;  
		disable_ts_adc();
		x = (ADCDAT1 & 0x3ff); 
		tsdev.penStatus = PEN_DOWN;
		DPRINTK("PEN DOWN: x: %08d, y: %08d\n", x, y);
		wait_up_int();
		tsEvent();
	} 
}

static void s3c2410_isr_adc(int irq, void *dev_id, struct pt_regs *reg)
{//
#if 1
	DPRINTK("Occured Touch Screen Interrupt\n");
	DPRINTK("SUBSRCPND = 0x%08lx\n", SUBSRCPND);
#endif

	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_UP)
	  s3c2410_get_XY();
#ifdef HOOK_FOR_DRAG
	else
	  s3c2410_get_XY();
#endif
	spin_unlock_irq(&(tsdev.lock));
}

static void s3c2410_isr_tc(int irq, void *dev_id, struct pt_regs *reg)
{
	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_UP) {
	  start_ts_adc();
	} else {
	  tsdev.penStatus = PEN_UP;
	  DPRINTK("PEN UP: x: %08d, y: %08d\n", x, y);
	  wait_down_int();
	  tsEvent();
	}
	spin_unlock_irq(&(tsdev.lock));
}

#ifdef HOOK_FOR_DRAG
static void ts_timer_handler(unsigned long data)
{
	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_DOWN) {
		start_ts_adc();
	}
	spin_unlock_irq(&(tsdev.lock));
}
#endif

static int s3c2410_ts_open(struct inode *inode, struct file *filp)
{
	tsdev.head = tsdev.tail = 0;
	tsdev.penStatus = PEN_UP;
#ifdef HOOK_FOR_DRAG 
	init_timer(&ts_timer);
	ts_timer.function = ts_timer_handler;
#endif
	tsEvent = tsEvent_raw;
	init_waitqueue_head(&(tsdev.wq));
	MOD_INC_USE_COUNT;
	return 0;
}

static int s3c2410_ts_release(struct inode *inode, struct file *filp)
{
#ifdef HOOK_FOR_DRAG
	del_timer(&ts_timer);
#endif
	MOD_DEC_USE_COUNT;
	return 0;
}

static struct file_operations s3c2410_fops = {
	owner:	THIS_MODULE,
	open:	s3c2410_ts_open,
	read:	s3c2410_ts_read,	
	release:	s3c2410_ts_release,
#ifdef USE_ASYNC
	fasync:	s3c2410_ts_fasync,
#endif
	poll:	s3c2410_ts_poll,
};

void tsEvent_dummy(void) {}
#ifdef CONFIG_PM
static int s3c2410_ts_pm_callback(struct pm_dev *pm_dev, pm_request_t req, 
								   void *data) 
{
    switch (req) {
		case PM_SUSPEND:
			tsEvent = tsEvent_dummy;
			break;
		case PM_RESUME:
			tsEvent = tsEvent_raw;
			wait_down_int();
			break;
    }
    return 0;
}
#endif

#ifdef CONFIG_DEVFS_FS
static devfs_handle_t devfs_ts_dir, devfs_tsraw;
#endif
static int __init s3c2410_ts_init(void)
{
	int ret;

	tsEvent = tsEvent_dummy;
	
// laputa for touchscreen adc delay time setting
	ADCDLY  = 30000;     // it is got from test 20000

	//laputa append for AD converting freq calculate
	ret = s3c2410_get_bus_clk(GET_PCLK);
	PreScale_n = (int)( ret / ADC_FREQ ) - 1 ;
	//laputa end 030918 
	
	ret = register_chrdev(0, DEVICE_NAME, &s3c2410_fops);
	if (ret < 0) {
	  printk(DEVICE_NAME " can't get major number\n");
	  return ret;
	}
	tsMajor = ret;
	/* set gpio to XP, YM, YP and  YM */
	set_gpio_ctrl(GPIO_YPON); 
	set_gpio_ctrl(GPIO_YMON);
	set_gpio_ctrl(GPIO_XPON);
	set_gpio_ctrl(GPIO_XMON);

	/* Enable touch interrupt */
	ret = request_irq(IRQ_ADC_DONE, s3c2410_isr_adc, SA_INTERRUPT, 
			  DEVICE_NAME, s3c2410_isr_adc);
	if (ret) goto adc_failed;
	ret = request_irq(IRQ_TC, s3c2410_isr_tc, SA_INTERRUPT, 
			  DEVICE_NAME, s3c2410_isr_tc);
	if (ret) goto tc_failed;

	/* Wait for touch screen interrupts */
	wait_down_int();

#ifdef CONFIG_DEVFS_FS
	devfs_ts_dir = devfs_mk_dir(NULL, "touchscreen", NULL);
	devfs_tsraw = devfs_register(devfs_ts_dir, "0", DEVFS_FL_DEFAULT,     // +- 031127 dev/touchscreen/0 for standard interface		
			tsMajor, TSRAW_MINOR, S_IFCHR | S_IRUSR | S_IWUSR,
			&s3c2410_fops, NULL);

#endif

#ifdef CONFIG_PM
	tsdev.pm_dev = pm_register(PM_DEBUG_DEV, PM_USER_INPUT,
				   s3c2410_ts_pm_callback);
#endif
	printk(DEVICE_NAME " initialized\n");
	
	return 0;
 tc_failed:
	free_irq(IRQ_ADC_DONE, s3c2410_isr_adc);
 adc_failed:
	return ret;
}

static void __exit s3c2410_ts_exit(void)
{
#ifdef CONFIG_DEVFS_FS	
	devfs_unregister(devfs_tsraw);
	devfs_unregister(devfs_ts_dir);
#endif	
	unregister_chrdev(tsMajor, DEVICE_NAME);
#ifdef CONFIG_PM
	pm_unregister(tsdev.pm_dev);
#endif
	free_irq(IRQ_ADC_DONE, s3c2410_isr_adc);
	free_irq(IRQ_TC, s3c2410_isr_tc);
}

module_init(s3c2410_ts_init);
module_exit(s3c2410_ts_exit);
