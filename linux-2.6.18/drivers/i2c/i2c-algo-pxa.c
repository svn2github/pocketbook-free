/*
 *  i2c-algo-pxa.c
 *
 *  I2C algorithm for the PXA I2C bus access.
 *  Byte driven algorithm similar to pcf.
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
 * 
 */
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>          /* struct i2c_msg and others */
#include <linux/i2c-id.h>

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

static int pxa_scan = 1;

static int i2c_pxa_valid_messages( struct i2c_msg msgs[], int num)
{
	int i;
	if (num < 1 || num > MAX_MESSAGES){
		if( i2c_debug) 
			printk(KERN_INFO "Invalid number of messages (max=%d, num=%d)\n", 
				MAX_MESSAGES, num);
		return -EINVAL;
	}

	/* check consistency of our messages */
	for (i=0;i<num;i++){
		if (&msgs[i]==NULL){
			if( i2c_debug) printk(KERN_INFO "Msgs is NULL\n");
			return -EINVAL;
		} else {
			if (msgs[i].len < 0 || msgs[i].buf == NULL){
				if( i2c_debug)printk(KERN_INFO "Length is less than zero");
				return -EINVAL;
			}
			if (msgs[0].len == 0){  /* this is SMBUS_QUICK */
				if( i2c_debug)printk(KERN_INFO "Message length is zero\n");
				return 0;
			}
		}
	}

	return 1;
}

static int i2c_pxa_readbytes(struct i2c_adapter *i2c_adap, char *buf, 
			int count, int last)
{

        int i, timeout=0;
        struct i2c_algo_pxa_data *adap = i2c_adap->algo_data;

        /* increment number of bytes to read by one -- read dummy byte */
        for (i = 0; i <= count; i++) {
                if (i!=0){
                        /* set ACK to NAK for last received byte ICR[ACKNAK] = 1
                                only if not a repeated start */

                        if ((i == count) && last) {
				adap->transfer( last, I2C_RECEIVE, 0);
			}else{
				adap->transfer( 0, I2C_RECEIVE, 1);
                        }

			timeout = adap->wait_for_interrupt(I2C_RECEIVE);

#ifdef DEBUG
                        if (timeout==BUS_ERROR){
				printk(KERN_INFO "i2c_pxa_readbytes: bus error -> forcing reset\n");
                                adap->reset();
                                return I2C_RETRY;
                        }
#endif

                        if (timeout){
                                adap->reset();
                                return I2C_RETRY;
                        }

                }

                if (i) {
                        buf[i - 1] = adap->read_byte();
                } else {
                        adap->read_byte(); /* dummy read */
                }
        }
        return (i - 1);
}

static int i2c_pxa_sendbytes(struct i2c_adapter *i2c_adap, const char *buf,
                         int count, int last)
{

        struct i2c_algo_pxa_data *adap = i2c_adap->algo_data;
        int wrcount, timeout;

        for (wrcount=0; wrcount<count; ++wrcount) {

                adap->write_byte(buf[wrcount]);
                if ((wrcount==(count-1)) && last) {
			adap->transfer( last, I2C_TRANSMIT, 0);
		}else{
			adap->transfer( 0, I2C_TRANSMIT, 1);
                }

		timeout = adap->wait_for_interrupt(I2C_TRANSMIT);

#ifdef DEBUG
                if (timeout==BUS_ERROR) {
			printk(KERN_INFO "i2c_pxa_sendbytes: bus error -> forcing reset.\n");
                        adap->reset();
                        return I2C_RETRY;
                }
#endif

                if (timeout) {
                        adap->reset();
                        return I2C_RETRY;
                }
        }
        return (wrcount);
}


static inline int i2c_pxa_set_ctrl_byte(struct i2c_algo_pxa_data * adap, struct i2c_msg *msg)
{
	u16 flags = msg->flags; 
	u8 addr;                            
	addr = (u8) ( (0x7f & msg->addr) << 1 );
	if (flags & I2C_M_RD ) 
		addr |= 1;      
	if (flags & I2C_M_REV_DIR_ADDR )
		addr ^= 1;
	adap->write_byte(addr);
	return 0;
}       

static int i2c_pxa_do_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	struct i2c_algo_pxa_data * adap;
	struct i2c_msg *pmsg=NULL;
	int i;
	int ret=0, timeout;

	adap = i2c_adap->algo_data;

	timeout = adap->wait_bus_not_busy();

	if (timeout) {
		return I2C_RETRY;
	}

	for (i = 0;ret >= 0 && i < num; i++) {
		int last = i + 1 == num;
		pmsg = &msgs[i];

		ret = i2c_pxa_set_ctrl_byte(adap,pmsg);

		/* Send START */
		if (i == 0) {
			adap->start();
		}else{
			adap->repeat_start();
		}

		adap->transfer(0, I2C_TRANSMIT, 0);

		/* Wait for ITE (transmit empty) */
		timeout = adap->wait_for_interrupt(I2C_TRANSMIT);

#ifdef DEBUG
		/* Check for ACK (bus error) */
		if (timeout==BUS_ERROR){
			printk(KERN_INFO "i2c_pxa_do_xfer: bus error -> forcing reset\n");
			adap->reset();
			return I2C_RETRY;
		}
#endif

		if (timeout) {
			adap->abort();
			adap->reset();
			return I2C_RETRY;
		}
/* FIXME: handle arbitration... */
#if 0
		/* Check for bus arbitration loss */
		if (adap->arbitration_loss()){
			printk("Arbitration loss detected \n");
			adap->reset();
			return I2C_RETRY;
		}
#endif

		/* Read */
		if (pmsg->flags & I2C_M_RD) {
			/* read bytes into buffer*/
			ret = i2c_pxa_readbytes(i2c_adap, pmsg->buf, pmsg->len, last);
#if DEBUG > 2
			if (ret != pmsg->len) {
				printk(KERN_INFO"i2c_pxa_do_xfer: read %d/%d bytes.\n",
					ret, pmsg->len);
			} else {
				printk(KERN_INFO"i2c_pxa_do_xfer: read %d bytes.\n",ret);
			}
#endif
		} else { /* Write */
			ret = i2c_pxa_sendbytes(i2c_adap, pmsg->buf, pmsg->len, last);
#if DEBUG > 2
			if (ret != pmsg->len) {
				printk(KERN_INFO"i2c_pxa_do_xfer: wrote %d/%d bytes.\n",
					ret, pmsg->len);
			} else {
				printk(KERN_INFO"i2c_pxa_do_xfer: wrote %d bytes.\n",ret);
			}
#endif
		}
	}

	if (ret<0){
		return ret;
	}else{
		return i;
	}
}

static int i2c_pxa_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	int retval = i2c_pxa_valid_messages( msgs, num);
	if( retval > 0)
	{
		int i;
		for (i=i2c_adap->retries; i>=0; i--){
			int retval = i2c_pxa_do_xfer(i2c_adap,msgs,num);
			if (retval!=I2C_RETRY){
				return retval;
			}
			if( i2c_debug)printk(KERN_INFO"Retrying transmission \n");
			udelay(100);
		}   
		if( i2c_debug)printk(KERN_INFO"Retried %i times\n",i2c_adap->retries);
		return -EREMOTEIO;

	}
	return retval;
}

struct i2c_algorithm i2c_pxa_algorithm  = {
        name:                   "PXA-I2C-Algorithm",
        id:                     I2C_ALGO_PXA,
        master_xfer:            i2c_pxa_xfer,
        smbus_xfer:             NULL,
        slave_send:             NULL,
        slave_recv:             NULL,
        algo_control:           NULL,
};

/* 
 * registering functions to load algorithms at runtime 
 */
int i2c_pxa_add_bus(struct i2c_adapter *i2c_adap)
{
        struct i2c_algo_pxa_data *adap = i2c_adap->algo_data;

        printk(KERN_INFO"I2C: Adding %s.\n", i2c_adap->name);

        i2c_adap->algo = &i2c_pxa_algorithm;

        MOD_INC_USE_COUNT;

        /* register new adapter to i2c module... */
        i2c_add_adapter(i2c_adap);

	adap->reset();

	/* scan bus */
	if (pxa_scan) {
		int i;
		printk(KERN_INFO "I2C: Scanning bus ");
		for (i = 0x02; i < 0xff; i+=2) {
			if( i==(I2C_PXA_SLAVE_ADDR<<1)) continue;

			if (adap->wait_bus_not_busy()) {
				printk(KERN_INFO "I2C: scanning bus %s - TIMEOUTed.\n",
						i2c_adap->name);
                                return -EIO;
			}
			adap->write_byte(i);
			adap->start();
			adap->transfer(0, I2C_TRANSMIT, 0);

			if ((adap->wait_for_interrupt(I2C_TRANSMIT) != BUS_ERROR)) {
				printk("(%02x)",i>>1);
				adap->abort();
			} else {
//				printk(".");
			}
			adap->stop();
			udelay(adap->udelay);
		}
		printk("\n");
	}
        return 0;
}

int i2c_pxa_del_bus(struct i2c_adapter *i2c_adap)
{
        int res;
        if ((res = i2c_del_adapter(i2c_adap)) < 0)
                return res;

        MOD_DEC_USE_COUNT;

        printk(KERN_INFO "I2C: Removing %s.\n", i2c_adap->name);

        return 0;
}

static int __init i2c_algo_pxa_init (void)
{
        printk(KERN_INFO "I2C: PXA algorithm module loaded.\n");
        return 0;
}

EXPORT_SYMBOL(i2c_pxa_add_bus);
EXPORT_SYMBOL(i2c_pxa_del_bus);

MODULE_PARM(pxa_scan, "i");
MODULE_PARM_DESC(pxa_scan, "Scan for active chips on the bus");

MODULE_AUTHOR("Intrinsyc Software Inc.");
MODULE_LICENSE("GPL");

module_init(i2c_algo_pxa_init);
