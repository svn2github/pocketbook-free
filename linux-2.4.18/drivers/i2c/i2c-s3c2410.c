/*
   -------------------------------------------------------------------------
   i2c-s3c2410.c i2c-hw access for the IIC peripheral on the Samsung S3C2410X
   processor and SMDK2400 reference board.

   Steve Hein <ssh@sgi.com>
   Copyright 2002 SGI, Inc.

   -------------------------------------------------------------------------
   This file was highly leveraged from i2c-algo-ppc405.c:
  
   Ian DaSilva, MontaVista Software, Inc.
   idasilva@mvista.com or source@mvista.com

   Copyright 2000 MontaVista Software Inc.

   Changes made to support the IIC peripheral on the IBM PPC 405 


   ----------------------------------------------------------------------------
   This file was highly leveraged from i2c-elektor.c, which was created
   by Simon G. Vogl and Hans Berglund:

 
     Copyright (C) 1995-97 Simon G. Vogl
                   1998-99 Hans Berglund

   With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and even
   Frodo Looijaard <frodol@dds.nl>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   ----------------------------------------------------------------------------
*/


#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-s3c2410.h>
#include <linux/i2c-s3c2410.h>

#undef KERN_DEBUG
#define KERN_DEBUG

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

//extern int s3c2410_i2c_debug;
int s3c2410_i2c_debug=0;
#if (LINUX_VERSION_CODE < 0x020301)
static struct wait_queue *iic_wait = NULL;
#else
static wait_queue_head_t iic_wait;
#endif
static int iic_pending;
static int iic_irq;

/* ----- global defines -----------------------------------------------	*/
#define DEB(x)	if (s3c2410_i2c_debug>=1) x
#define DEB2(x) if (s3c2410_i2c_debug>=2) x
#define DEB3(x) if (s3c2410_i2c_debug>=3) x
#define DEBE(x)	x	/* error messages 				*/


/* ----- local functions ----------------------------------------------	*/

//
// Description: Write a byte to IIC hardware
//
static void iic_s3c2410_setbyte(void *data, int ctl, int val)
{
	DEB3(printk("iic_s3c2410_setbyte: Write IIC register 0x%08x = 0x%02x\n", (u32) ctl, (u32) val));
	// writeb resolves to a write to the specified memory location
        writeb(val, ctl);
}


//
// Description: Read a byte from IIC hardware
//
static int iic_s3c2410_getbyte(void *data, int ctl)
{
	int val;

	//DEB3(printk("iic_s3c2410_getbyte: Read IIC register 0x%08x\n", (u32) ctl));
	val = readb(ctl);
	DEB3(printk("iic_s3c2410_getbyte: Read IIC register 0x%08x = 0x%02x\n", (u32) ctl, val));
	//DEB3(printk("iic_s3c2410_getbyte: Read Data 0x%02X\n", val));
	return (val);
}


//
// Description: Return our slave address.  This is the address
// put on the I2C bus when another master on the bus wants to address us
// as a slave
//
static int iic_s3c2410_getown(void *data)
{
	return (-EINVAL);
}


//
// Description: Return the clock rate
//
static int iic_s3c2410_getclock(void *data)
{
	return (-EINVAL);
}


#if 0
static void iic_s3c2410_sleep(unsigned long timeout)
{
	schedule_timeout( timeout * HZ);
}
#endif


//
// Description:  Put this process to sleep.  We will wake up when the
// IIC controller interrupts.
//
static void iic_s3c2410_waitforpin(void) {

   //int timeout = 2;
   int timeout = HZ/4;   /* 0.25 second timeout */

   //printk("iic_s3c2410_waitforpin: At top of function\n");
   //
   // If interrupts are enabled (which they are), then put the process to
   // sleep.  This process will be awakened by two events -- either the
   // the IIC peripheral interrupts or the timeout expires. 
   //
   if (iic_irq > 0) {
      cli();
      if (iic_pending == 0) {
         //printk("iic_s3c2410_waitforpin: calling interruptible_sleep_on_timeout\n");
  	 interruptible_sleep_on_timeout(&iic_wait, timeout);
      } else
 	 iic_pending = 0;
      sti();
   } else {
      //
      // If interrupts are not enabled then delay for a reasonable amount
      // of time and return.  We expect that by time we return to the calling
      // function that the IIC has finished our requested transaction and
      // the status bit reflects this.
      //
      // udelay is probably not the best choice for this since it is
      // the equivalent of a busy wait
      //
      udelay(100);
   }
   //printk("iic_s3c2410_waitforpin: exitting\n");
}


//
// Description: The registered interrupt handler
//
static void iic_s3c2410_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
   u8 ret;
	
   iic_pending = 1;
   DEB2(printk("iic_s3c2410_handler: in interrupt handler\n"));

   // Read status register
   //ret = readb(S3C2410_IICSTAT);
   //DEB2(printk("iic_s3c2410_handler: status = %x\n", ret));

   // Clear interrupt pending bit
   //ret = readb(S3C2410_IICCON);
   //DEB2(printk("iic_s3c2410_handler: ICCON=%x\n", ret));
   //ret &= ~S3C2410_IICCON_INT_PEND;
   //DEB2(printk("iic_s3c2410_handler: set ICCON=%x\n", ret));
   //writeb(ret, S3C2410_IICCON);

   wake_up_interruptible(&iic_wait);
}


//
// Description: This function is very hardware dependent.  First, we lock
// the region of memory where out registers exist.  Next, we request our
// interrupt line and register its associated handler.
//
static int iic_hw_resrc_init(void)
{
	if (request_irq(iic_irq, iic_s3c2410_handler, 0, "s3c2410 IIC", 0) < 0) {
	   printk(KERN_ERR "iic_hw_resrc_init: Request irq%d failed\n", iic_irq);
	   iic_irq = 0;
	} else {
	   DEB3(printk("iic_hw_resrc_init: Enabled interrupt (irq%d)\n", iic_irq));
	   enable_irq(iic_irq);
	}
	return 0;
}


//
// Description: Release irq and memory
//
static void iic_s3c2410_release(void)
{
	if (iic_irq > 0) {
		disable_irq(iic_irq);
		free_irq(iic_irq, 0);
	}
}


//
// Description: Does nothing
//
static int iic_s3c2410_reg(struct i2c_client *client)
{
	return 0;
}


//
// Description: Does nothing
//
static int iic_s3c2410_unreg(struct i2c_client *client)
{
	return 0;
}


//
// Description: If this compiled as a module, then increment the count
//
static void iic_s3c2410_inc_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}


//
// Description: If this is a module, then decrement the count
//
static void iic_s3c2410_dec_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}


/* ------------------------------------------------------------------------
 * Encapsulate the above functions in the correct operations structure.
 * This is only done when more than one hardware adapter is supported.
 */
static struct i2c_algo_s3c2410_data iic_s3c2410_data = {
	NULL,
	iic_s3c2410_setbyte,
	iic_s3c2410_getbyte,
	iic_s3c2410_getown,
	iic_s3c2410_getclock,
	iic_s3c2410_waitforpin,
	80, 80, 100,		/*	waits, timeout */
};

static struct i2c_adapter iic_s3c2410_ops = {
	"Samsung S3C2410X IIC adapter",
	0,   // adapter ID, none for us in i2c-id.h, and not really needed
	NULL,
	&iic_s3c2410_data,
	iic_s3c2410_inc_use,
	iic_s3c2410_dec_use,
	iic_s3c2410_reg,
	iic_s3c2410_unreg,
};


//
// Description: Called when the module is loaded.  This function starts the
// cascade of calls up through the heirarchy of i2c modules (i.e. up to the
//  algorithm layer and into to the core layer)
//
static int __init iic_s3c2410_init(void) 
{

	printk(KERN_INFO "iic_s3c2410_init: Samsung S3C2410X iic adapter module version %s (%s)\n", I2C_VERSION, I2C_DATE);

	iic_irq = IRQ_IIC;

#if (LINUX_VERSION_CODE >= 0x020301)
	init_waitqueue_head(&iic_wait);
#endif
	if (iic_hw_resrc_init() == 0) {
		if (i2c_s3c2410_add_bus(&iic_s3c2410_ops) < 0)
			return -ENODEV;
	} else {
		return -ENODEV;
	}
	//printk(KERN_INFO "iic_s3c2410_init: initialized iic-bus at %#x.\n", PA_IIC_BASE);
	printk(KERN_INFO "iic_s3c2410_init: initialized iic-bus at %#x.\n", VA_IIC_BASE);
	return 0;
}


static void iic_s3c2410_exit(void)
{
	i2c_s3c2410_del_bus(&iic_s3c2410_ops);
        iic_s3c2410_release();
}

EXPORT_NO_SYMBOLS;

//
// If modules is NOT defined when this file is compiled, then the MODULE_*
// macros will resolve to nothing
//
MODULE_AUTHOR("Steve Hein, SGI Inc. <ssh@sgi.com>");
MODULE_DESCRIPTION("I2C-Bus adapter routines for Samsung S3C2410X IIC bus adapter");

MODULE_PARM(s3c2410_i2c_debug,"i");


//
// Description: Called when module is loaded or when kernel is intialized.
// If MODULES is defined when this file is compiled, then this function will
// resolve to init_module (the function called when insmod is invoked for a
// module).  Otherwise, this function is called early in the boot, when the
// kernel is intialized.  Check out /include/init.h to see how this works.
//
module_init(iic_s3c2410_init);



//
// Description: Resolves to module_cleanup when MODULES is defined.
//
module_exit(iic_s3c2410_exit); 
