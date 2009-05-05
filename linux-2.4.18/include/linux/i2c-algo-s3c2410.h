/* ------------------------------------------------------------------------- */
/* i2c-algo-s3c2410.h i2c driver algorithms for Samsung S3C2410X */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 1995-97 Simon G. Vogl
                   1998-99 Hans Berglund

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */

/* With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and even
   Frodo Looijaard <frodol@dds.nl> */

/* Modifications by MontaVista Software, August 2000
   Changes made to support the IIC peripheral on the IBM PPC 405 */

/* Modifications by Steve Hein <ssh@sgi.com> of SGI Inc.
   Changes made to support the IIC peripheral on the Samsung S3C2410X */

/* $Id: i2c-algo-s3c2410.h,v 1.1 2004/02/06 13:19:45 laputa Exp $ */

//laputa for define tag name miss-match 
//at [static struct i2c_algorithm s3c2410_algo] in "i2c_algo_s3c2410.c"
/* 
#ifndef I2C_ALGO_S3C2410_H
#define I2C_ALGO_S3C2410_H 1
*/
//laputa for to matching at "i2c_algo_s3c2410.c"[static struct i2c_algorithm s3c2410_algo] 
#ifndef I2C_ALGO_S3C2410
#define I2C_ALGO_S3C2410 1
// laputa end 

/* --- Defines for s3c2410-adapters ---------------------------------------	*/
#include <linux/i2c.h>

struct i2c_algo_s3c2410_data {
	void *data;		/* private data for lolevel routines	*/
	void (*setiic) (void *data, int ctl, int val);
	int  (*getiic) (void *data, int ctl);
	int  (*getown) (void *data);
	int  (*getclock) (void *data);
	void (*waitforpin) (void);     

	/* local settings */
	int udelay;
	int mdelay;
	int timeout;
};

#define I2C_S3C2410_SET_SPEED       0x780   /* set IIC clock speed */
#define I2C_S3C2410_GET_SPEED       0x781   /* get IIC clock speed */

#define I2C_S3C2410_ADAP_MAX	16

int i2c_s3c2410_add_bus(struct i2c_adapter *);
int i2c_s3c2410_del_bus(struct i2c_adapter *);

#endif /* I2C_ALGO_S3C2410_H */
