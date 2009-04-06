/*
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * machine dependent irq handling routine
 *
 * Author: Nandy Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:55:27 $ 
 *
 * $Revision: 1.1.1.1 $

   Tue May 21 2002 Nandy Lyu <nandy@mizi.com>
   - initial

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>

#define	ClearPending(x)	{	\
			  SRCPND = (1 << (x));	\
			  INTPND = (1 << (x));	\
			}

#define EINT_OFFSET(x)		((x) - NORMAL_IRQ_OFFSET + 4)
#define SUBIRQ_OFFSET(x)	((x) - EXT_IRQ_OFFSET)

#define EXTINT_MASK	0x7

#if 0

/*
 * set_GPIO_IRQ_edge - set interrupt signal for External Interrupts
 *
 * parameters:
 *	irq	number of external interrupt (IRQ_EINT0 ~ IRQ_EINT23)
 *	edge	signal method
 */
#define EXTINT_OFFSET	0x4
#define EXTINT_MASK	0x7
int set_EXT_IRQ_mode(int irq, int edge) {
	unsigned long flags;
	int shift_value;

	if (!(((IRQ_EINT4 <= irq) && (irq <= IRQ_EINT23)) ||
	      ((IRQ_EINT0 <= irq) && (irq <= IRQ_EINT3))))
	  return -EINVAL;

	local_irq_save(flags);
	if (irq < IRQ_EINT4) {			/* IRQ_EINT0 ~ IRQ_EINT3 */
	  shift_value = (irq % 8) * EXTINT_OFFSET;
	  EXTINT0 &= ~(EXTINT_MASK << shift_value);
	  EXTINT0 |= (edge << shift_value);
	  ClearPending(irq);
	} else {
	  shift_value = ((irq + 4) % 8) * EXTINT_OFFSET;
	  if (irq < IRQ_EINT8) {		/* IRQ_EINT4 ~ IRQ_EINT7 */
	    EXTINT0 &= ~(EXTINT_MASK << shift_value);
	    EXTINT0 |= (edge << shift_value);
	    EINTPEND = (1 << shift_value);
	    ClearPending(IRQ_EINT4_7);
	  } else if (irq < IRQ_EINT16) {	/* IRQ_EINT8 ~ IRQ_EINT15 */
	    EXTINT1 &= ~(EXTINT_MASK << shift_value);
	    EXTINT1 |= (edge << shift_value);
	    EINTPEND = (1 << shift_value);
	    ClearPending(IRQ_EINT8_23);
	  } else {				/* IRQ_EINT16 ~ IRQ_EINT23 */
	    EXTINT2 &= ~(EXTINT_MASK << shift_value);
	    EXTINT2 |= (edge << shift_value);
	    EINTPEND = (1 << shift_value);
	    ClearPending(IRQ_EINT8_23);
	  }
	}
	irq_desc[irq].valid = 1;

	restore_flags(flags);
	return 0;
}
EXPORT_SYMBOL(set_EXT_IRQ_mode);
#endif

static int inline
fixup_irq_num(int irq)
{
	if (irq < IRQ_EINT4) return irq;
	else return ((irq + 4) - NORMAL_IRQ_OFFSET);
}

static void inline
set_gpios(int irq, int pullup)
{
	int shift;
	if (irq < 8) {
		shift = 2*irq;
		GPFCON &= ~(0x3 << shift);
		GPFCON |= (0x2 << shift);
		GPFUP &= ~(GRAB_PULLUP(pullup) << irq);
		GPFUP |= (GRAB_PULLUP(pullup) << irq);
	} else {
		shift = 2*(irq - 8);
		GPGCON &= ~(0x3 << shift);
		GPGCON |= (0x2 << shift);
		GPGUP &= ~(GRAB_PULLUP(pullup) << (irq - 8));
		GPGUP |= (GRAB_PULLUP(pullup) << (irq - 8));
	} 
}

int 
set_external_irq(int irq, int edge, int pullup)
{
	unsigned long flags;
	int real_irq, reg_ofs, shift;
	volatile u32 *extint = (volatile u32 *)io_p2v(0x56000088);

	//printk(__FUNCTION__" called\n");
	if (((irq < IRQ_EINT0) && (irq > IRQ_EINT23)) ||
	    ((irq > IRQ_EINT3) && (irq < IRQ_EINT4)))
		return -EINVAL;

	real_irq = fixup_irq_num(irq);
	//printk(__FUNCTION__"(): real_irq = %d\n", real_irq);

	set_gpios(real_irq, pullup);

	local_irq_save(flags);

	reg_ofs = (real_irq / 8);
	//printk(__FUNCTION__"(): regs_ofs = %d\n", reg_ofs);
	shift = 4 * (real_irq - 8 * reg_ofs);
	extint += reg_ofs;

	*extint &= ~(EXTINT_MASK << shift);
	*extint |= (edge << shift);

	if (irq < 4) {
		SRCPND |= (1 << real_irq);
		INTPND |= (1 << real_irq);
	} else {
		EINTPEND |= (1 << real_irq);
	}

	irq_desc[irq].valid = 1;

	restore_flags(flags);

	return 0;
}
EXPORT_SYMBOL(set_external_irq);


/*
 * Defined irq handlers
 */
static void s3c2410_mask_ack_irq(unsigned int irq)
{
	INTMSK |= (1 << irq);
	SRCPND = (1 << irq);
	INTPND = (1 << irq);
}

static void s3c2410_mask_irq(unsigned int irq)
{
	INTMSK |= (1 << irq);
}

static void s3c2410_unmask_irq(unsigned int irq)
{
	INTMSK &= ~(1 << irq);
}

/* for EINT? */
static void EINT4_23mask_ack_irq(unsigned int irq)
{
	irq = EINT_OFFSET(irq);
	EINTMASK |= (1 << irq);
	EINTPEND = (1 << irq);

	if (irq < EINT_OFFSET(IRQ_EINT8)) {
//	  INTMSK |= (1 << SHIFT_EINT4_7);
	  ClearPending(SHIFT_EINT4_7);
	} else {
//	  INTMSK |= (1 << SHIFT_EINT8_23);
	  ClearPending(SHIFT_EINT8_23);
	}
}

static void EINT4_23mask_irq(unsigned int irq)
{
#if 0
	if (irq < IRQ_EINT8) {
	  INTMSK |= (1 << SHIFT_EINT4_7);
	} else {
	  INTMSK |= (1 << SHIFT_EINT8_23);
	}
#endif
	irq = EINT_OFFSET(irq);
	EINTMASK |= (1 << irq);
}

static void EINT4_23unmask_irq(unsigned int irq)
{
	EINTMASK &= ~(1 << EINT_OFFSET(irq));

	if (irq < IRQ_EINT8) {
	  INTMSK &= ~(1 << SHIFT_EINT4_7);
	} else {
	  INTMSK &= ~(1 << SHIFT_EINT8_23);
	}
}


/* for sub_IRQ */
static void SUB_mask_ack_irq(unsigned int irq)
{
	INTSUBMSK |= (1 << SUBIRQ_OFFSET(irq));
	SUBSRCPND = (1 << SUBIRQ_OFFSET(irq));

	if (irq <= IRQ_ERR0) {
	  ClearPending(SHIFT_UART0);
        } else if (irq <= IRQ_ERR1) {
	  ClearPending(SHIFT_UART1);
	} else if (irq <= IRQ_ERR2){
	  ClearPending(SHIFT_UART2);
        } else {	/* if ( irq <= IRQ_ADC_DONE ) { */
	  ClearPending(SHIFT_ADCTC);
	}
}

static void SUB_mask_irq(unsigned int irq)
{
	INTSUBMSK |= (1 << SUBIRQ_OFFSET(irq));
}

static void SUB_unmask_irq(unsigned int irq)
{
	INTSUBMSK &= ~(1 << SUBIRQ_OFFSET(irq));

	if (irq <= IRQ_ERR0) {
		INTMSK &= ~(1 << SHIFT_UART0); 
        } else if (irq <= IRQ_ERR1) {
		INTMSK &= ~(1 << SHIFT_UART1);
	} else if (irq <= IRQ_ERR2){
	    	INTMSK &= ~(1 << SHIFT_UART2);
        } else {	/* if ( irq <= IRQ_ADC_DONE ) { */
		INTMSK &= ~(1 << SHIFT_ADCTC);
        }
}

/*
 *  fixup_irq() for do_IRQ() in kernel/irq.c
 */
inline unsigned int get_subIRQ(int irq, int begin, int end, int fail_irq) {
	int i;

	for(i=begin; i <= end; i++) {
	  if (irq & (1 << i))
	    return (EXT_IRQ_OFFSET + i);
	}
	return fail_irq;
}

inline unsigned int get_extIRQ(int irq, int begin, int end, int fail_irq) {
	int i;

	for(i=begin; i <= end; i++) {
	  if (irq & (1 << i))
	    return (NORMAL_IRQ_OFFSET - 4 + i);
	}
	return fail_irq;
}

unsigned int fixup_irq(int irq) {
    unsigned int ret;
    unsigned long sub_mask, ext_mask;

    if (irq == OS_TIMER)
      return irq;

    switch (irq) {
    case IRQ_UART0:
      sub_mask = SUBSRCPND & ~INTSUBMSK;
      ret = get_subIRQ(sub_mask, 0, 2, irq);
      break;
    case IRQ_UART1:
      sub_mask = SUBSRCPND & ~INTSUBMSK;
      ret = get_subIRQ(sub_mask, 3, 5, irq);
      break;
    case IRQ_UART2:
      sub_mask = SUBSRCPND & ~INTSUBMSK;
      ret = get_subIRQ(sub_mask, 6, 8, irq);
      break;
    case IRQ_ADCTC:
      sub_mask = SUBSRCPND & ~INTSUBMSK;
      ret = get_subIRQ(sub_mask, 9, 10, irq);
      break;
    case IRQ_EINT4_7:
      ext_mask = EINTPEND & ~EINTMASK;
      ret = get_extIRQ(ext_mask, 4, 7, irq);
      break;
    case IRQ_EINT8_23:
      ext_mask = EINTPEND & ~EINTMASK;
      ret = get_extIRQ(ext_mask, 8, 23, irq);
      break;
    default:
      ret = irq;
    }
	
    return ret;
}

static struct resource irq_resource = {
	name:	"irqs",
	start:	0x4a000000,
	end:	0x4a00001f,
};

static struct resource eint_resource = {
	name:	"ext irqs",
	start:	0x56000088,
	end:	0x560000ab,
};

void __init s3c2410_init_irq(void) {
    int irq;

    request_resource(&iomem_resource, &irq_resource);
    request_resource(&iomem_resource, &eint_resource);

    /* disable all IRQs */
    INTMSK = 0xffffffff;
    INTSUBMSK = 0x7ff;
    EINTMASK = 0x00fffff0;

    /* all IRQs are IRQ, not FIQ
       0 : IRQ mode
       1 : FIQ mode
    */
    INTMOD = 0x00000000;

    /* clear Source/Interrupt Pending Register */
    SRCPND = 0xffffffff;
    INTPND = 0xffffffff;
    SUBSRCPND = 0x7ff;
    EINTPEND = 0x00fffff0;

    /* Define irq handler */
    for (irq=0; irq < NORMAL_IRQ_OFFSET; irq++) {
      irq_desc[irq].valid	= 1;
      irq_desc[irq].probe_ok	= 1;
      irq_desc[irq].mask_ack	= s3c2410_mask_ack_irq;
      irq_desc[irq].mask	= s3c2410_mask_irq;
      irq_desc[irq].unmask	= s3c2410_unmask_irq;
    }

      irq_desc[IRQ_RESERVED6].valid	= 0;
      irq_desc[IRQ_RESERVED24].valid	= 0;

      irq_desc[IRQ_EINT4_7].valid	= 0;
      irq_desc[IRQ_EINT8_23].valid	= 0;

      irq_desc[IRQ_EINT0].valid		= 0;
      irq_desc[IRQ_EINT1].valid		= 0;
      irq_desc[IRQ_EINT2].valid		= 0;
      irq_desc[IRQ_EINT3].valid		= 0;

    for (irq=NORMAL_IRQ_OFFSET; irq < EXT_IRQ_OFFSET; irq++) {
      irq_desc[irq].valid	= 0;
      irq_desc[irq].probe_ok	= 1;
      irq_desc[irq].mask_ack	= EINT4_23mask_ack_irq;
      irq_desc[irq].mask	= EINT4_23mask_irq;
      irq_desc[irq].unmask	= EINT4_23unmask_irq;
    }

    for (irq=EXT_IRQ_OFFSET; irq < SUB_IRQ_OFFSET; irq++) {
      irq_desc[irq].valid	= 1;
      irq_desc[irq].probe_ok	= 1;
      irq_desc[irq].mask_ack	= SUB_mask_ack_irq;
      irq_desc[irq].mask	= SUB_mask_irq;
      irq_desc[irq].unmask	= SUB_unmask_irq;
    }  
}
/*
 | $Id: irq.c,v 1.1.1.1 2004/02/04 12:55:27 laputa Exp $
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
