/*
 *  i2c_adap_pxa.c
 *
 *  I2C adapter for the PXA I2C bus access.
 *
 *  Copyright (C) 2002 Intrinsyc Software Inc.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  History:
 *    Apr 2002: Initial version [CS]
 *    Jun 2002: Properly seperated algo/adap [FB]
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/irqs.h>              /* for IRQ_I2C */

#include "i2c-pxa.h"

/*
 * Set this to zero to remove all the debug statements via dead code elimination.
 */
//#define DEBUG       1

#if DEBUG
static unsigned int i2c_debug = DEBUG;
#else
#define i2c_debug	0
#endif

static int irq = 0;
static int i2c_pending = 0;             /* interrupt pending when 1 */
static volatile int bus_error = 0;
static volatile int tx_finished = 0;
static volatile int rx_finished = 0;

static wait_queue_head_t i2c_wait;

/* place a byte in the transmit register */
static void i2c_pxa_write_byte(u8 value) 
{
        IDBR = value;
}

/* read byte in the receive register */
static u8 i2c_pxa_read_byte(void) 
{
        return (u8) (0xff & IDBR);
}

static void i2c_pxa_start(void)
{
	ICR |= ICR_START;
	ICR &= ~(ICR_STOP | ICR_ALDIE | ICR_ACKNAK);

	bus_error=0;            /* clear any bus_error from previous txfers */
	tx_finished=0;          /* clear rx and tx interrupts from previous txfers */
	rx_finished=0;
}

static void i2c_pxa_repeat_start(void)
{
	ICR |= ICR_START;
	ICR &= ~(ICR_STOP | ICR_ALDIE);

	bus_error=0;            /* clear any bus_error from previous txfers */
	tx_finished=0;          /* clear rx and tx interrupts from previous txfers */
	rx_finished=0;
}

static void i2c_pxa_stop(void)
{
	ICR |= ICR_STOP;
	ICR &= ~(ICR_START);
}

static void i2c_pxa_midbyte(void)
{
	ICR &= ~(ICR_START | ICR_STOP);
}

static void i2c_pxa_abort(void)
{
	ICR |= ICR_MA;
}

static int i2c_pxa_wait_bus_not_busy( void)
{
        int timeout = DEF_TIMEOUT;

        while (timeout-- && (ISR & ISR_IBB)) {
                udelay(100); /* wait for 100 us */
        }

        return (timeout<=0);
}

static void i2c_pxa_wait_for_ite(void){
	unsigned long flags;
	if (irq > 0) {
		save_flags_cli(flags);
		if (i2c_pending == 0) {
			interruptible_sleep_on_timeout(&i2c_wait, I2C_SLEEP_TIMEOUT );
		}
		i2c_pending = 0;
		restore_flags(flags);
	} else {
		udelay(100);
	}
}

static int i2c_pxa_wait_for_int( int wait_type)
{
        int timeout = DEF_TIMEOUT;
#ifdef DEBUG
	if (bus_error)
		printk(KERN_INFO"i2c_pxa_wait_for_int: Bus error on enter\n");
	if (rx_finished)
		printk(KERN_INFO"i2c_pxa_wait_for_int: Receive interrupt on enter\n");
	if (tx_finished)
		printk(KERN_INFO"i2c_pxa_wait_for_int: Transmit interrupt on enter\n");
#endif

        if (wait_type == I2C_RECEIVE){         /* wait on receive */

                while (timeout-- && !(rx_finished)){
                        i2c_pxa_wait_for_ite();
                }
#ifdef DEBUG
                if (timeout<0){
                        if (tx_finished)
                                printk("Error: i2c-algo-pxa.o: received a tx"
                                        " interrupt while waiting on a rx in wait_for_int");
                }
#endif
        } else {                  /* wait on transmit */
        
                while (timeout-- && !(tx_finished)){
                        i2c_pxa_wait_for_ite();
                }
#ifdef DEBUG
                if (timeout<0){
                        if (rx_finished)
                                printk("Error: i2c-algo-pxa.o: received a rx"
                                        " interrupt while waiting on a tx in wait_for_int");
                }
#endif
        }       

        udelay(ACK_DELAY);      /* this is needed for the bus error */

        tx_finished=0;
        rx_finished=0;

        if (bus_error){
                bus_error=0;
                if( i2c_debug > 2)printk("wait_for_int: error - no ack.\n");
                return BUS_ERROR;
        }

        if (timeout <= 0)
                return(-1);
        else
                return(0);
}

static void i2c_pxa_transfer( int lastbyte, int receive, int midbyte)
{
	if( lastbyte)
	{
		if( receive==I2C_RECEIVE) ICR |= ICR_ACKNAK; 
		i2c_pxa_stop();
	}
	else if( midbyte)
	{
		i2c_pxa_midbyte();
	}
	ICR |= ICR_TB;
}

static void i2c_pxa_reset( void)
{
#ifdef DEBUG
	printk("Resetting I2C Controller Unit\n");
	printk("Status Register  %#x\n",ISR);
	printk("Control Register %#x\n",ICR);
	printk("Data Buffer      %#x\n",IDBR);
	printk("Bus Monitor Reg  %#x\n",IBMR);
#endif
        /* disable unit */
	ICR &= ~ICR_IUE;

        /* reset the unit */
	ICR |= ICR_UR;
        udelay(100);

        /* disable unit */
	ICR &= ~ICR_IUE;
        
        /* set the global I2C clock on */
        CKEN |= CKEN14_I2C;

        /* set our slave address */
	ISAR = I2C_PXA_SLAVE_ADDR;

        /* set control register values */
	ICR = I2C_ICR_INIT;

        /* set clear interrupt bits */
	ISR = I2C_ISR_INIT;

        /* enable unit */
	ICR |= ICR_IUE;
        udelay(100);
}

static void i2c_pxa_handler(int this_irq, void *dev_id, struct pt_regs *regs) 
{
        int status;
        status = (ISR);

        if (status & ISR_BED){
                (ISR) |= ISR_BED;
                bus_error=ISR_BED;
        }
        if (status & ISR_ITE){
                (ISR) |= ISR_ITE;
                tx_finished=ISR_ITE;
        }
        if (status & ISR_IRF){
                (ISR) |= ISR_IRF;
                rx_finished=ISR_IRF;
        }
        i2c_pending = 1;
        wake_up_interruptible(&i2c_wait);
}

static int i2c_pxa_resource_init( void)
{
        init_waitqueue_head(&i2c_wait);

        if (request_irq(IRQ_I2C, &i2c_pxa_handler, SA_INTERRUPT, "I2C_PXA", 0) < 0) {
                irq = 0;
                if( i2c_debug)
			printk(KERN_INFO "I2C: Failed to register I2C irq %i\n", IRQ_I2C);
                return -ENODEV;
        }else{
                irq = IRQ_I2C;
                enable_irq(irq);
        }
        return 0;
}

static void i2c_pxa_resource_release( void)
{
        if( irq > 0)
        {
                disable_irq(irq);
                free_irq(irq,0);
                irq=0;
        }
}

static void i2c_pxa_inc_use(struct i2c_adapter *adap)
{
#ifdef MODULE
        MOD_INC_USE_COUNT;
#endif
}

static void i2c_pxa_dec_use(struct i2c_adapter *adap)
{
#ifdef MODULE
        MOD_DEC_USE_COUNT;
#endif
}

static int i2c_pxa_client_register(struct i2c_client *client)
{
        return 0;
}

static int i2c_pxa_client_unregister(struct i2c_client *client)
{
        return 0;
}

static struct i2c_algo_pxa_data i2c_pxa_data = {
        write_byte:		i2c_pxa_write_byte,
        read_byte:		i2c_pxa_read_byte,

        start:			i2c_pxa_start,
        repeat_start:		i2c_pxa_repeat_start,
        stop:			i2c_pxa_stop,
        abort:			i2c_pxa_abort,

        wait_bus_not_busy:	i2c_pxa_wait_bus_not_busy,
        wait_for_interrupt:	i2c_pxa_wait_for_int,
        transfer:		i2c_pxa_transfer,
        reset:			i2c_pxa_reset,

	udelay:			10,
	timeout:		DEF_TIMEOUT,
};

static struct i2c_adapter i2c_pxa_ops = {
        name:                   "PXA-I2C-Adapter",
        id:                     I2C_ALGO_PXA,
        algo_data:              &i2c_pxa_data,
        inc_use:                i2c_pxa_inc_use,
        dec_use:                i2c_pxa_dec_use,
        client_register:        i2c_pxa_client_register,
        client_unregister:      i2c_pxa_client_unregister,
        retries:                2,
};

extern int i2c_pxa_add_bus(struct i2c_adapter *);
extern int i2c_pxa_del_bus(struct i2c_adapter *);

static int __init i2c_adap_pxa_init(void)
{
        if( i2c_pxa_resource_init() == 0) {

                if (i2c_pxa_add_bus(&i2c_pxa_ops) < 0) {
                        i2c_pxa_resource_release();
                        printk(KERN_INFO "I2C: Failed to add bus\n");
                        return -ENODEV;
                }
        } else {
                return -ENODEV;
        }

        printk(KERN_INFO "I2C: Successfully added bus\n");

        return 0;
}

static void i2c_adap_pxa_exit(void)
{
        i2c_pxa_del_bus( &i2c_pxa_ops);
        i2c_pxa_resource_release();

        printk(KERN_INFO "I2C: Successfully removed bus\n");
}

module_init(i2c_adap_pxa_init);
module_exit(i2c_adap_pxa_exit);
