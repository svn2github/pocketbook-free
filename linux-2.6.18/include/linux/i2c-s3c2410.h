/*
   --------------------------------------------------------------------
   i2c-s3c2410.h: Global defines for the I2C controller on board the    
                  Samsung S3C2410X processor.                                
   --------------------------------------------------------------------

   Steve Hein, SGI Inc.   <ssh@sgi.com>
   Copyright 2002 SGI Inc.


   This file was heavily leveraged from:

   i2c-ppc405.h: Global defines for the I2C controller on board the    
                 IBM 405 PPC processor.                                

   Ian DaSilva, MontaVista Software, Inc.
   idasilva@mvista.com or source@mvista.com

   Copyright 2000 MontaVista Software Inc.

 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef I2C_S3C2410_H
#define I2C_S3C2410_H 1

#include <asm/arch/hardware.h>

/*
 * I2C registers
 */

#define VA_IIC_BASE			  io_p2v(0x54000000)
#define S3C2410_IICCON        (VA_IIC_BASE + 0x0)
#define S3C2410_IICSTAT       (VA_IIC_BASE + 0x4)
#define S3C2410_IICADD        (VA_IIC_BASE + 0x8)
#define S3C2410_IICDS         (VA_IIC_BASE + 0xC)

/* IICCON bits */
#define S3C2410_IICCON_ACK_EN        (1<<7)
#define S3C2410_IICCON_TCLK_PCLK512  (1<<6)
#define S3C2410_IICCON_INT_EN        (1<<5)
#define S3C2410_IICCON_INT_PEND      (1<<4)
#define S3C2410_IICCON_TCLK_MSK      (0xff)

/* IICSTAT bits */
#define S3C2410_IICSTAT_MRX_MODE      (2<<6)
#define S3C2410_IICSTAT_MTX_MODE      (3<<6)
#define S3C2410_IICSTAT_SRX_MODE      (0<<6)
#define S3C2410_IICSTAT_STX_MODE      (1<<6)
#define S3C2410_IICSTAT_MODE_MSK      (3<<6)
#define S3C2410_IICSTAT_BUSY          (1<<5)
#define S3C2410_IICSTAT_OUT_EN        (1<<4)
#define S3C2410_IICSTAT_ARB_FAILED    (1<<3)
#define S3C2410_IICSTAT_SLAVEADDR     (1<<2)
#define S3C2410_IICSTAT_ADDRZERO      (1<<1)
#define S3C2410_IICSTAT_NACK          (1<<0)
#define S3C2410_IICSTAT_MRX_ENABLE (S3C2410_IICSTAT_MRX_MODE | S3C2410_IICSTAT_BUSY | S3C2410_IICSTAT_OUT_EN)
#define S3C2410_IICSTAT_MTX_ENABLE (S3C2410_IICSTAT_MTX_MODE | S3C2410_IICSTAT_BUSY | S3C2410_IICSTAT_OUT_EN)

#endif  /* I2C_S3C2410_H */
