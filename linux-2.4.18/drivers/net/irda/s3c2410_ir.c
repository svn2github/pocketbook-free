/*
 * drivers/net/irda/s3c2410_ir.c
 *
 * IR port driver for the SMDK2410
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:38 $
 *
 * $Revision: 1.1.1.1 $
 *
   Wed Jun 11 2003 Yong-iL Joh <tolkien@mizi.com>
   - initial based on ep7211_ir.c

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/init.h>

#include <net/irda/irda.h>
#include <net/irda/irmod.h>
#include <net/irda/irda_device.h>

#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/cpu_s3c2410.h>

#define MIN_DELAY	25      /* 15 us, but wait a little more to be sure */
#define MAX_DELAY	10000   /* 1 ms */

static void s3c2410_ir_open(dongle_t *self, struct qos_info *qos);
static void s3c2410_ir_close(dongle_t *self);
static int  s3c2410_ir_change_speed(struct irda_task *task);
static int  s3c2410_ir_reset(struct irda_task *task);

static struct dongle_reg dongle = {
	Q_NULL,
	IRDA_S3C2410_IR,
	s3c2410_ir_open,
	s3c2410_ir_close,
	s3c2410_ir_reset,
	s3c2410_ir_change_speed,
};

static void s3c2410_ir_open(dongle_t *self, struct qos_info *qos) {
	unsigned int flags;
	unsigned long clk;

	save_flags(flags); cli();

	/* Turn on the SIR encoder.
	   No parity, 8bit, 1 stop bit */
  	ULCON2 |= ULCON_IR | ULCON_PAR_NONE | ULCON_WL8 | ULCON_ONE_STOP;

	/* select PCLK for UART baud rate */
	UCON2 |= (UCON2 & ~UCON_CLK_SEL) | UCON_CLK_PCLK;
	clk = s3c2410_get_bus_clk(GET_PCLK);
	UBRDIV2 = (clk / (115200 * 16) ) - 1;

	restore_flags(flags);
	MOD_INC_USE_COUNT;
}

static void s3c2410_ir_close(dongle_t *self) {
	unsigned int flags;

	save_flags(flags); cli();

	/* Turn off the SIR encoder. */
	ULCON2 &= ~ULCON_IR;

	restore_flags(flags);
	MOD_DEC_USE_COUNT;
}

static int s3c2410_ir_change_speed(struct irda_task *task) {
	irda_task_next_state(task, IRDA_TASK_DONE);
	return 0;
}

static int s3c2410_ir_reset(struct irda_task *task) {
	irda_task_next_state(task, IRDA_TASK_DONE);
	return 0;
}

static int __init s3c2410_ir_init(void) {
	return irda_device_register_dongle(&dongle);
}

static void __exit s3c2410_ir_cleanup(void) {
	irda_device_unregister_dongle(&dongle);
}

MODULE_AUTHOR("Yong-iL Joh <tolkien@mizi.com>");
MODULE_DESCRIPTION("S3C2410 I/R driver");
MODULE_LICENSE("GPL");
		
module_init(s3c2410_ir_init);
module_exit(s3c2410_ir_cleanup);
/*
 | $Id: s3c2410_ir.c,v 1.1.1.1 2004/02/04 12:56:38 laputa Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
