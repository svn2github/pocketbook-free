/*
   -------------------------------------------------------------------------
   i2c-algo-s3c2410.c i2c driver algorithms for the Samsung S3C2410X
   processor and SMDK2410 reference board.

   Steve Hein <ssh@sgi.com>
   Copyright 2002 SGI, Inc.

   -------------------------------------------------------------------------
   This file was highly leveraged from i2c-algo-ppc405.c:
   
   Ian DaSilva, MontaVista Software, Inc.
   idasilva@mvista.com or source@mvista.com

   Copyright 2000 MontaVista Software Inc.

   Changes made to support the IIC peripheral on the IBM PPC 405

   ---------------------------------------------------------------------------
   This file was highly leveraged from i2c-algo-pcf.c, which was created
   by Simon G. Vogl and Hans Berglund:


     Copyright (C) 1995-1997 Simon G. Vogl
                   1998-2000 Hans Berglund

   With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and 
   Frodo Looijaard <frodol@dds.nl> ,and also from Martin Bailey
   <mbailey@littlefeet-inc.com>


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
   ---------------------------------------------------------------------------
*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-s3c2410.h>
#include <linux/i2c-s3c2410.h>

//laputa for to get the PCLK freq.  from s3c2410_get_clk(GET_PCLK)
#include <asm/arch/cpu_s3c2410.h>
//laputa debug msg 030901
#define LAPUTA_DEBUG_MSG	1
#include  "dbg_msg.h"

#undef KERN_DEBUG
#define KERN_DEBUG

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

/* ----- global defines ----------------------------------------------- */
#define DEB(x) if (s3c2410_i2c_debug>=1) x
#define DEB2(x) if (s3c2410_i2c_debug>=2) x
#define DEB3(x) if (s3c2410_i2c_debug>=3) x /* print several statistical values*/
#define DEBPROTO(x) if (s3c2410_i2c_debug>=9) x;
 	/* debug the protocol by showing transferred bits */
#define DEF_TIMEOUT 2

/* debugging - slow down transfer to have a look at the data .. 	*/
/* I use this with two leds&resistors, each one connected to sda,scl 	*/
/* respectively. This makes sure that the algorithm works. Some chips   */
/* might not like this, as they have an internal timeout of some mils	*/
/*
#define SLO_IO      jif=jiffies;while(jiffies<=jif+i2c_table[minor].veryslow)\
                        if (need_resched) schedule();
*/


/* ----- global variables ---------------------------------------------	*/

#ifdef SLO_IO
	int jif;
#endif

/* module parameters:
 */
int s3c2410_i2c_debug=0;
//laputa for iic clk freq prescale = 1x
//static int i2c_clkdiv=1;  // -- remove
static int i2c_clkdiv=0;	// += modify 030902

/* --- setting states on the bus with the right timing: ---------------	*/

#define s3c2410_outb(adap, reg, val) adap->setiic(adap->data, reg, val)
#define s3c2410_inb(adap, reg) adap->getiic(adap->data, reg)

#define IIC_SINGLE_XFER		0
#define IIC_COMBINED_XFER	1

/* --- other auxiliary functions --------------------------------------	*/

//
// Description: returns the current speed of the I2C clock in kHz
//
static int s3c2410_clkspeed(void)
{
	unsigned long pdiv = ((i2c_clkdiv / 100) != 0) ? 16 : 512;
	unsigned long div = i2c_clkdiv % 100;

//laputa - what (where) is PCLK ?  
#if 0
	return (PCLK/pdiv)/(div+1)/1024;
#else
//laputa for to get Peripheral clock frequency  030830
	unsigned long PCLK = s3c2410_get_bus_clk(GET_PCLK);  //++ append
	return (PCLK/pdiv)/(div+1)/1024;
#endif
//laputa end 030830
}

//
// Description: Puts this process to sleep for a period equal to timeout 
//
static inline void s3c2410_sleep(unsigned long timeout)
{
	schedule_timeout( timeout * HZ);
}


//
// Description: This performs the Samsung S3C2410X IIC initialization sequence
// as described in the S3C2410X data book.
//
static int s3c2410_init(struct i2c_algo_s3c2410_data *adap)
{
	u8 conval = 0;

	// initialize control/status regs to 0
	s3c2410_outb(adap, S3C2410_IICCON, 0);
	s3c2410_outb(adap, S3C2410_IICSTAT, 0);

	// set up a dummy IIC slave addr (even though we never use it!)
	s3c2410_outb(adap, S3C2410_IICADD, 0x10);
	//s3c2410_outb(adap, S3C2410_IICADD, 0xa0);
	

	// set up clock frequency for IIC-bus
	if (i2c_clkdiv/100) {
	   /* IICCLK=PCLK/16 */
	   conval |= (i2c_clkdiv%100);            /* Tx clk = IICCLK/(n+1) */
	} else {
	   conval |= S3C2410_IICCON_TCLK_PCLK512;  /* IICCLK=PCLK/512 */
	   conval |= i2c_clkdiv;                   /* Tx clk = IICCLK/(n+1) */
	}

	// enable interrupts
	conval |= S3C2410_IICCON_INT_EN;

	// enable ACK generation
	conval |= S3C2410_IICCON_ACK_EN;

	// write out the control reg. value
	s3c2410_outb(adap, S3C2410_IICCON, conval);

	// enable I2C bus data output and set master transmit mode
	// to get to a sane state (also generates a STOP condition)
	s3c2410_outb(adap, S3C2410_IICSTAT, S3C2410_IICSTAT_MTX_MODE | S3C2410_IICSTAT_OUT_EN);
	
        DEB2(printk(KERN_DEBUG "s3c2410_init: Initialized IIC on S3C2410X, %dkHz clock\n", s3c2410_clkspeed()));

	mdelay(20);

        return 0;
}


//
// Description: Attempts to reset the I2C controller/bus back to a sane state.
//
static int s3c2410_reset (struct i2c_algo_s3c2410_data *adap)
{
	int ret;
	int count = 0;

	//
	// re-initialize
	//
	s3c2410_init(adap);

	//
	// Assume all is OK if the bus is not busy....
	//
	while((ret = s3c2410_inb(adap, S3C2410_IICSTAT)) & S3C2410_IICSTAT_BUSY) {

	  //
	  // Generate stop condition
	  //
	  DEB2(printk(KERN_DEBUG "s3c2410_reset: Generating STOP condition\n"));
	  s3c2410_outb(adap, S3C2410_IICSTAT, (S3C2410_IICSTAT_MTX_ENABLE & ~S3C2410_IICSTAT_BUSY));

	  //
	  // Clear status register and enable ACK generation
	  //
	  DEB2(printk(KERN_DEBUG "s3c2410_reset: Clearing status register\n"));
	  ret = s3c2410_inb(adap, S3C2410_IICCON);
          ret = (ret & ~S3C2410_IICCON_INT_PEND) | S3C2410_IICCON_ACK_EN;
	  s3c2410_outb(adap, S3C2410_IICCON, ret);

          //
	  // reset I2C again
          //
          s3c2410_init(adap);

#if 0
	  //
	  // If still busy do a dummy read to reset the active slave device
	  //
	  ret = s3c2410_inb(adap, S3C2410_IICSTAT);
	  if (ret & S3C2410_IICSTAT_BUSY) {

	    DEB2(printk(KERN_DEBUG "s3c2410_reset: Clearing status register before dummy read\n"));
	    ret = s3c2410_inb(adap, S3C2410_IICCON);
            ret = ret & ~(S3C2410_IICCON_INT_PEND | S3C2410_IICCON_ACK_EN);
	    s3c2410_outb(adap, S3C2410_IICCON, ret);

	    DEB2(printk(KERN_DEBUG "s3c2410_reset: Dummy read\n"));
	    s3c2410_outb(adap, S3C2410_IICSTAT, S3C2410_IICSTAT_MRX_ENABLE);
	    s3c2410_inb(adap, S3C2410_IICDS);   // dummy read

	    DEB2(printk(KERN_DEBUG "s3c2410_reset: Clearing status register after dummy read\n"));
	    ret = s3c2410_inb(adap, S3C2410_IICCON);
            ret = (ret & ~(S3C2410_IICCON_INT_PEND)) | S3C2410_IICCON_ACK_EN;
	    s3c2410_outb(adap, S3C2410_IICCON, ret);
	  }
#endif

	  //
	  // Bail out after a more than reasonable number of attempts
	  //
	  if (count++ > 50) {
	    printk(KERN_DEBUG "s3c2410_reset: I2C bus stuck BUSY!\n");
	    return -EIO;
	  }
	}

	return 0;
}

//
// Description: After we issue a transaction on the IIC bus, this function
// is called.  It puts this process to sleep until we get an interrupt from
// from the controller telling us that the transaction we requested in complete.
//
static int wait_for_pin(struct i2c_algo_s3c2410_data *adap, int *status) {

	int timeout = DEF_TIMEOUT+2;
	//int retval;
	
	*status = s3c2410_inb(adap, S3C2410_IICCON);
	//printk("wait_for_pin: status = %x\n", *status);

	while (timeout-- && !(*status & S3C2410_IICCON_INT_PEND)) {
	   //printk("wait_for_pin: timeout=%d, status=%x\n", timeout, *status);
	   //printk("wait_for_pin: calling waitforpin\n");
	   adap->waitforpin();
           //printk("wait_for_pin: returning from waitforpin\n");
	   *status = s3c2410_inb(adap, S3C2410_IICCON);
	   s3c2410_inb(adap, S3C2410_IICSTAT);
	}

	//printk("wait_for_pin: returning from wait_for_pin\n");
	if (timeout <= 0) {
	   // reset I2C
	   s3c2410_reset(adap);
#if 0
	   /* Issue stop signal on the bus */
           retval = s3c2410_inb(adap, S3C2410_IICSTAT);
           s3c2410_outb(adap, S3C2410_IICSTAT, retval & ~S3C2410_IICSTAT_BUSY);
	   /* Clear pending interrupt bit */
           retval = s3c2410_inb(adap, S3C2410_IICCON);
           s3c2410_outb(adap, S3C2410_IICCON, retval & ~S3C2410_IICCON_INT_PEND);

	   // wait for the busy condition to clear
	   udelay(adap->udelay);

	   // Check the status of the controller.  Does it still see a
	   // pending transfer, even though we've tried to stop any
	   // ongoing transaction?
           retval = s3c2410_inb(adap, S3C2410_IICSTAT);
           if(retval & S3C2410_IICSTAT_BUSY) {
	      // The iic controller didn't stop when we told it to....
	      // The iic controller is hosed.
              s3c2410_init(adap);
	      /* Is the pending transfer bit in the sts reg finally cleared? */
              retval = s3c2410_inb(adap, S3C2410_IICSTAT);
              if(retval  & S3C2410_IICSTAT_BUSY) {
                 printk("The IIC Controller is hosed.  A processor reset is required\n");
              }
           }
#endif
	   return(-ETIMEDOUT);
	}

	return(0);
}


//------------------------------------
// Utility functions
//

//
// Description: This function tries to verify that the device we want to
// talk to on the IIC bus really exists. 
//
#if 0
static inline int try_address(struct i2c_algo_s3c2410_data *adap,
		       unsigned char addr, int retries)
{
	int i, status, ret = -1;
	for (i=0;i<retries;i++) {
		i2c_outb(adap, addr);
		i2c_start(adap);
		status = s3c2410_inb(adap, 1);
		if (wait_for_pin(adap, &status) >= 0) {
			if ((status & I2C_PCF_LRB) == 0) { 
				i2c_stop(adap);
				break;	/* success! */
			}
		}
		i2c_stop(adap);
		udelay(adap->udelay);
	}
	DEB2(if (i) printk("i2c-algo-s3c2410.o: needed %d retries for %d\n",i,
	                   addr));
	return ret;
}
#endif


//
// Description: Whenever we initiate a transaction, the first byte clocked
// onto the bus after the start condition is the address of the
// device we want to talk to.  This function manipulates the address specified
// so that it makes sense to the hardware when written to the IIC peripheral.
//
static inline unsigned char iic_addr(struct i2c_algo_s3c2410_data *adap,
                                        struct i2c_msg *msg) 
{
	unsigned short flags = msg->flags;
	unsigned char addr;

	addr = ( msg->addr << 1 );  
	
	if (flags & I2C_M_RD )
		addr |= 1;
	if (flags & I2C_M_REV_DIR_ADDR )
		addr ^= 1;

	return addr;
}


//
// Description: This function is waits for an interrupt and checks for
// timeouts and transmit/receive errors.
//
static int s3c2410_wait(struct i2c_adapter *i2c_adap, int check_ack)
{
	struct i2c_algo_s3c2410_data *adap = i2c_adap->algo_data;
	int ret, timeout, status;
	u32 errbits = S3C2410_IICSTAT_ARB_FAILED | ((check_ack) ? S3C2410_IICSTAT_NACK : 0);

	// Wait for transmission to complete.
	DEB2(printk(KERN_DEBUG "s3c2410_wait: Waiting for interrupt\n"));
	timeout = wait_for_pin(adap, &status);
	if(timeout < 0) {
	   // Error handling
           //printk(KERN_ERR "Error: timeout\n");
           return -ETIMEDOUT;
	}
	DEB2(printk(KERN_DEBUG "s3c2410_wait: Got interrupt\n"));

	// Check transfer status
	ret = s3c2410_inb(adap, S3C2410_IICSTAT);
	if (ret & errbits) {
	   if (ret & S3C2410_IICSTAT_ARB_FAILED) {
	      //printk(KERN_ERR "Lost arbitration\n");
	      ret = -EPROTO;
	   }
	   else if (ret & S3C2410_IICSTAT_NACK) {
	      //printk(KERN_ERR "Master transfer aborted by a NACK during the transfer of the address byte\n");
	      ret = -ENODEV;
	   }
	   s3c2410_reset(adap);
	   return ret;
	}
	return 0;
}

//
// Description: This function is called by the upper layers to do the
// grunt work for a master send transaction
//
static int s3c2410_sendbytes(struct i2c_adapter *i2c_adap,
                             struct i2c_msg *pmsg, int xfer_flag)
{
	struct i2c_algo_s3c2410_data *adap = i2c_adap->algo_data;
	int i = 0, count, ret;
	
	// laputa IIC interface is only unsigned char receive & transfer
	//	char *buf;			// -- remove 
	unsigned char *buf;  	// +- modify 030903
    
	u8 addr;
	int rval;
   
	buf = pmsg->buf;
	count = pmsg->len;

	DEB(printk(KERN_DEBUG "I2C WRITE s3c2410_sendbytes: len=%d   addr=0x%04x   flags=0x%04x\n", pmsg->len, pmsg->addr, pmsg->flags));

	// setup transfer.  if NOSTART flag set, then transfer is already in
	// progress so skip init
	if (!(pmsg->flags & I2C_M_NOSTART)) {
	   // load slave address
	   DEB2(printk(KERN_DEBUG "s3c2410_sendbytes: Loading slave address\n"));
	   addr = iic_addr(adap, pmsg);
	   s3c2410_outb(adap, S3C2410_IICDS, addr);

	   //laputa i2c send slave address check 030902 
	   DbgMsg(LAPUTA_DEBUG_MSG,printk("**** outb address [%x] \n",addr));
	   
	   udelay(5);   /* IICSDA setup delay */

	   // set transmit mode
	   DEB2(printk(KERN_DEBUG "s3c2410_sendbytes: Set transmit mode\n"));
	   ret = S3C2410_IICSTAT_MTX_ENABLE;
	   s3c2410_outb(adap, S3C2410_IICSTAT, ret);

	   ret = s3c2410_inb(adap, S3C2410_IICSTAT);
	   ret = s3c2410_inb(adap, S3C2410_IICSTAT);

	   // wait for transmit of slave addr to complete
	   if ((rval = s3c2410_wait(i2c_adap, 1)) < 0)
	      return rval;
	}

	// loop through and send data bytes
	for (i = 0; i < count; i++) {
	   // write next byte to the shift register
	   s3c2410_outb(adap, S3C2410_IICDS, buf[i]);
	   //laputa transfer data checked 030903
	   DbgMsg(LAPUTA_DEBUG_MSG,printk("*** buf[%d] = %x \n",i,(unsigned char)buf[i]));
	   udelay(5);   /* IICSDA setup delay */

	   // clear the pending bit
	   ret = s3c2410_inb(adap, S3C2410_IICCON);
	   s3c2410_outb(adap, S3C2410_IICCON, ret & ~S3C2410_IICCON_INT_PEND);

	   // Wait for transmission to complete.
	   if ((rval = s3c2410_wait(i2c_adap, 1)) < 0)
	      return (i);
	}

	// if this is a single write, stop now that we're done
	if (xfer_flag == IIC_SINGLE_XFER) {
	   // generate a STOP condition
	   ret = s3c2410_inb(adap, S3C2410_IICSTAT);
	   s3c2410_outb(adap, S3C2410_IICSTAT, ret & ~S3C2410_IICSTAT_BUSY);

	   // clear the pending bit
	   ret = s3c2410_inb(adap, S3C2410_IICCON);
	   s3c2410_outb(adap, S3C2410_IICCON, ret & ~S3C2410_IICCON_INT_PEND);
	}

	return (i);
}


//
// Description: This function is called by the upper layers to do the
// grunt work for a master receive transaction
//
static int s3c2410_readbytes(struct i2c_adapter *i2c_adap,
                             struct i2c_msg *pmsg, int xfer_flag)
{
	struct i2c_algo_s3c2410_data *adap = i2c_adap->algo_data;
	int i, count, ret, busystat;

	// laputa IIC interface is only unsigned char receive & transfer
	//	char *buf;			// -- remove 
	unsigned char *buf;  	// +- modify 030903
	u8 addr;
	int rval;

	buf = pmsg->buf;
	count = pmsg->len;
   
	if( count == 0 ) return 0;

	// check for start/busy condition in progress
	busystat = s3c2410_inb(adap, S3C2410_IICSTAT);

	DEB(printk(KERN_DEBUG "I2C READ s3c2410_readbytes: len=%d   addr=0x%04x   flags=0x%04x\n", pmsg->len, pmsg->addr, pmsg->flags));
	DbgMsg(LAPUTA_DEBUG_MSG,printk("I2C READ s3c2410_readbytes: len=%d   addr=0x%04x   flags=0x%04x\n", 
				pmsg->len, pmsg->addr, pmsg->flags));

	// setup transfer.  if NOSTART flag set, then transfer is already in
	// progress so skip init
	if (!(pmsg->flags & I2C_M_NOSTART)) {
	   // load slave address (unless NOSTART flag is set)
	   DEB2(printk(KERN_DEBUG "s3c2410_readbytes: Loading slave address\n"));
	   addr = iic_addr(adap, pmsg);
	   s3c2410_outb(adap, S3C2410_IICDS, addr);
	   udelay(5);   /* IICSDA setup delay */

	   // set receive mode
	   DEB2(printk(KERN_DEBUG "s3c2410_readbytes: Set receive mode\n"));
	   ret = S3C2410_IICSTAT_MRX_ENABLE;
	   s3c2410_outb(adap, S3C2410_IICSTAT, ret);

	   // wait for transmit of slave addr to complete
	   if ((rval = s3c2410_wait(i2c_adap, 1)) < 0)
	      return rval;
	}

	// loop through and read data bytes
	// if a start condition was already in progress, do a dummy read
	for (i = (busystat & S3C2410_IICSTAT_BUSY)?0:1; i < (count+1); i++) {

	   // if this is the last byte, disable ACK generation
           if (i == count) {
               ret = s3c2410_inb(adap, S3C2410_IICCON);
	       s3c2410_outb(adap, S3C2410_IICCON, ret & ~S3C2410_IICCON_ACK_EN);
           }

	   // clear the pending bit
	   ret = s3c2410_inb(adap, S3C2410_IICCON);
	   s3c2410_outb(adap, S3C2410_IICCON, ret & ~S3C2410_IICCON_INT_PEND);

	   // Wait for transmission to complete.
	   if ((rval = s3c2410_wait(i2c_adap, 0)) < 0) {
	      return ((i <= 0) ? rval : i-1);
	   }

	   // read next byte from the shift register
	   if (i == 0)
	       s3c2410_inb(adap, S3C2410_IICDS);   // dummy read
	   else
	       buf[i-1] = s3c2410_inb(adap, S3C2410_IICDS);
	}

	// if this is a single read, stop now that we're done
	if (xfer_flag == IIC_SINGLE_XFER) {
	   // generate a STOP condition
	   ret = s3c2410_inb(adap, S3C2410_IICSTAT);
	   s3c2410_outb(adap, S3C2410_IICSTAT, ret & ~S3C2410_IICSTAT_BUSY);

	   // clear the pending bit and re-enable ACK generation
	   ret = s3c2410_inb(adap, S3C2410_IICCON);
	   ret = (ret & ~S3C2410_IICCON_INT_PEND) | S3C2410_IICCON_ACK_EN;
	   s3c2410_outb(adap, S3C2410_IICCON, ret);
	}

	//laputa receive complete state message 030903
	DbgMsg(LAPUTA_DEBUG_MSG,printk("&&& algo read buf %c msg.buf %c count=%d\n",buf,pmsg->buf,count));
	
	return count;
}



//
// Description:  This function implements combined transactions.  Combined
// transactions consist of combinations of reading and writing blocks of data.
// Each transfer (i.e. a read or a write) is separated by a repeated start
// condition.
//
static int s3c2410_combined_transaction(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num) 
{
   int i;
   struct i2c_msg *pmsg;
   int ret;

   DEB2(printk(KERN_DEBUG "Beginning combined transaction\n"));

   for(i=0; i<(num-1); i++) {
      pmsg = &msgs[i];
      if(pmsg->flags & I2C_M_RD) {
         DEB2(printk(KERN_DEBUG "  This one is a read\n"));
         ret = s3c2410_readbytes(i2c_adap, pmsg, IIC_COMBINED_XFER);
      }
      else if(!(pmsg->flags & I2C_M_RD)) {
         DEB2(printk(KERN_DEBUG "This one is a write\n"));
         ret = s3c2410_sendbytes(i2c_adap, pmsg, IIC_COMBINED_XFER);
      }
   }
   //
   // Last read or write segment needs to be terminated with a stop
   //
   pmsg = &msgs[i];

   if(pmsg->flags & I2C_M_RD) {
      DEB2(printk(KERN_DEBUG "Doing the last read\n"));
      ret = s3c2410_readbytes(i2c_adap, pmsg, IIC_SINGLE_XFER);
   }
   else if(!(pmsg->flags & I2C_M_RD)) {
      DEB2(printk(KERN_DEBUG "Doing the last write\n"));
      ret = s3c2410_sendbytes(i2c_adap, pmsg, IIC_SINGLE_XFER);
   }

   return ret;
}


//
// Description: Prepares the controller for a transaction (clearing status
// registers, data buffers, etc), and then calls either s3c2410_readbytes or
// s3c2410_sendbytes to do the actual transaction.
//
static int s3c2410_xfer(struct i2c_adapter *i2c_adap,
		    struct i2c_msg msgs[], 
		    int num)
{
	struct i2c_algo_s3c2410_data *adap = i2c_adap->algo_data;
	struct i2c_msg *pmsg;
	int i = 0;
	int ret = 0;
    
	pmsg = &msgs[i];

	//laputa master transfer control  check 030903
	DbgMsg(LAPUTA_DEBUG_MSG,printk("==> 2410 algo xfer msg [%s]\n",msgs[0].buf));

	//
	// get the I2C controller/bus back to a sane state
	//

	if (s3c2410_reset(adap) < 0)
	  return -EIO;

	//
	// Combined transaction (read and write)
	//
	if(num > 1) {
           DEB2(printk(KERN_DEBUG "s3c2410_xfer: Call combined transaction\n"));
           ret = s3c2410_combined_transaction(i2c_adap, msgs, num);
        }
	//
	// Read only
	//
	else if((num == 1) && (pmsg->flags & I2C_M_RD)) {
	   //
	   // Tell device to begin reading data from the  master data 
	   //
	   DEB2(printk(KERN_DEBUG "s3c2410_xfer: Call adapter's read\n"));
	   ret = s3c2410_readbytes(i2c_adap, pmsg, IIC_SINGLE_XFER);
	} 
        //
	// Write only
	//
	else if((num == 1 ) && (!(pmsg->flags & I2C_M_RD))) {
	   //
	   // Write data to master data buffers and tell our device
	   // to begin transmitting
	   //
	   DEB2(printk(KERN_DEBUG "s3c2410_xfer: Call adapter's write\n"));
	   ret = s3c2410_sendbytes(i2c_adap, pmsg, IIC_SINGLE_XFER);
	}	

	return ret;   
}


//
// Description: Implements device specific ioctls.  Higher level ioctls can
// be found in i2c-core.c and are typical of any i2c controller (specifying
// slave address, timeouts, etc).  These ioctls take advantage of any hardware
// features built into the controller for which this algorithm-adapter set
// was written.
//
static int algo_control(struct i2c_adapter *adapter, 
	unsigned int cmd, unsigned long arg)
{
	if (cmd == I2C_S3C2410_SET_SPEED) {
		if ((arg % 100) > 15  ||  (arg / 100) > 1)
			return -EINVAL;
		i2c_clkdiv = arg;
		return 0;
	} else if (cmd == I2C_S3C2410_GET_SPEED) {
		/* return speed in kHz */
		unsigned long speed = s3c2410_clkspeed();
		if (copy_to_user( (unsigned long*)arg, &speed, sizeof(speed)) ) 
			return -EFAULT;
		return 0;
	}

	return -EINVAL;
}




static u32 s3c2410_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING; 
}


/* ----- exported algorithm data: -------------------------------------	*/

static struct i2c_algorithm s3c2410_algo = {
	"Samsung S3C2410X algorithm",
	I2C_ALGO_S3C2410,
	s3c2410_xfer,
	NULL,
	NULL,				/* slave_xmit		*/
	NULL,				/* slave_recv		*/
	algo_control,			/* ioctl		*/
	s3c2410_func,			/* functionality	*/
};


/* 
 * registering functions to load algorithms at runtime 
 */


//
// Description: Register bus structure
//
int i2c_s3c2410_add_bus(struct i2c_adapter *adap)
{
//	int i, status;
	struct i2c_algo_s3c2410_data *s3c2410_adap = adap->algo_data;

	DEB2(printk(KERN_DEBUG "i2c-algo-s3c2410.o: hw routines for %s registered.\n",
	            adap->name));

	/* register new adapter to i2c module... */

	adap->id |= s3c2410_algo.id;
	adap->algo = &s3c2410_algo;

	adap->timeout = 100;		/* default values, should	*/
	adap->retries = 3;		/* be replaced by defines	*/

#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif

	s3c2410_init(s3c2410_adap);
	i2c_add_adapter(adap);
        printk("s3c2410_init: Initialized IIC on S3C2410X, %dkHz clock\n",
               s3c2410_clkspeed());

	return 0;
}


//
// Done
//
int i2c_s3c2410_del_bus(struct i2c_adapter *adap)
{
	int res;
	if ((res = i2c_del_adapter(adap)) < 0)
		return res;
	DEB2(printk(KERN_DEBUG "i2c-algo-s3c2410.o: adapter unregistered: %s\n",adap->name));

#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}


//
// Done
//
int __init i2c_algo_s3c2410_init (void)
{
	printk(KERN_INFO "Samsung S3C2410X (i2c) algorithm module version %s (%s)\n", I2C_VERSION, I2C_DATE);
	return 0;
}


void i2c_algo_s3c2410_exit(void)
{
	return;
}


EXPORT_SYMBOL(i2c_s3c2410_add_bus);
EXPORT_SYMBOL(i2c_s3c2410_del_bus);


#ifndef MODULE

static int __init i2cdebug_setup(char *str)
{
	s3c2410_i2c_debug = simple_strtoul(str,NULL,10);
	return 1;
}

static int __init i2cclk_setup(char *str)
{
	i2c_clkdiv = simple_strtoul(str,NULL,10);
	return 1;
}


__setup("i2c_debug=", i2cdebug_setup);
__setup("i2c_clk=", i2cclk_setup);

#endif /* MODULE */


//
// The MODULE_* macros resolve to nothing if MODULES is not defined
// when this file is compiled.
//
MODULE_AUTHOR("Steve Hein   SGI Inc. <ssh@sgi.com>");
MODULE_DESCRIPTION("Samsung S3C2410X/SMDK2410 algorithm");

MODULE_PARM(s3c2410_i2c_debug,"i");

MODULE_PARM_DESC(s3c2410_i2c_debug,
        "debug level - 0 off; 1 normal; 2,3 more verbose; 9 iic-protocol");


//
// This function resolves to init_module (the function invoked when a module
// is loaded via insmod) when this file is compiled with MODULES defined.
// Otherwise (i.e. if you want this driver statically linked to the kernel),
// a pointer to this function is stored in a table and called
// during the intialization of the kernel (in do_basic_setup in /init/main.c) 
//
// All this functionality is complements of the macros defined in linux/init.h
module_init(i2c_algo_s3c2410_init);


//
// If MODULES is defined when this file is compiled, then this function will
// resolved to cleanup_module.
//
module_exit(i2c_algo_s3c2410_exit);
