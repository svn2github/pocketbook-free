/*
 * s3c2410-adc.c
 *
 * S3C2410 ADC 
 *  exclusive with s3c2410-ts.c
 *
 * Author: SeonKon Choi <bushi@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:04 $ 
 *
 * $Revision: 1.1.1.1 $
 *

   Fri Dec 03 2002 SeonKon Choi <bushi@mizi.com>
   - initial

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/semaphore.h>

#undef DEBUG

#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define START_ADC_AIN(x) \
	{ \
		ADCCON = PRESCALE_EN | PRSCVL(255) | ADC_INPUT((x)) ; \
		ADCCON |= ADC_START; \
	}

static struct semaphore adc_lock;
static wait_queue_head_t *adc_wait;

static void adcdone_int_handler(int irq, void *dev_id, struct pt_regs *reg)
{
	wake_up(adc_wait);
}

int s3c2410_adc_read(int ain, wait_queue_head_t *wait)
{
	int ret = 0;

	if (down_interruptible(&adc_lock))
		return -ERESTARTSYS;

	adc_wait = wait;

	START_ADC_AIN(ain);
	sleep_on_timeout(adc_wait, HZ/100); /* 10ms */

#if 0
	if (signal_pending(current)) {
		up(&adc_lock);
		return -ERESTARTSYS;
	}
#endif

	ret = ADCDAT0 ;

	up(&adc_lock);

	adc_wait = NULL;

	DPRINTK("AIN[%d] = 0x%04x, %d\n", ain, ret, ADCCON & 0x80 ? 1:0);

	return (ret & 0x3ff);
}

int __init s3c2410_adc_init(void)
{
	init_MUTEX(&adc_lock);

	/* normal ADC */
	ADCTSC = 0; //XP_PST(NOP_MODE);

	if (request_irq(IRQ_ADC_DONE, adcdone_int_handler, SA_INTERRUPT,
			"ADC", NULL) < 0)
		goto irq_err;

	return 0;

irq_err:

	return 1;
}

module_init(s3c2410_adc_init);

#ifdef MODULE
void __exit s3c2410_adc_exit(void)
{
	free_irq(IRQ_ADC_DONE, NULL);
}

module_exit(s3c2410_adc_exit);
MODULE_LICENSE("GPL");
#endif
