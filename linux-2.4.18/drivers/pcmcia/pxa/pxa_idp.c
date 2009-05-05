/*
 * linux/drivers/pcmcia/pxa/pxa_idp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) 2002 Accelent Systems, Inc. All Rights Reserved
 * 
 * Platform specific routines for the Accelent PXA250 IDP, based on those
 * first done for the Lubbock.
 *
 * Version 1.0 2002-05-02  Jeff Sutherland <jeffs@accelent.com>
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/delay.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/pcmcia.h>

static int 
pxa_idp_pcmcia_init(struct pcmcia_init *init)
{
	int return_val=0;

  	/* Set PCMCIA Socket 0 power to standby mode.
	*  PXA IDP has dedicated CPLD pins for all this stuff :-)
   	*/
	IDP_CPLD_PCCARD_EN = 0xC3; //both slots disabled, reset NOT active
	IDP_CPLD_PCCARD_PWR = 0; //all power to both slots off
	
	GPDR(IRQ_TO_GPIO_2_80(PCMCIA_S0_CD_VALID)) &= ~GPIO_bit(IRQ_TO_GPIO_2_80(PCMCIA_S0_CD_VALID));
	GPDR(IRQ_TO_GPIO_2_80(PCMCIA_S1_CD_VALID)) &= ~GPIO_bit(IRQ_TO_GPIO_2_80(PCMCIA_S1_CD_VALID));

	set_GPIO_IRQ_edge(IRQ_TO_GPIO_2_80(PCMCIA_S0_CD_VALID),GPIO_BOTH_EDGES);
	set_GPIO_IRQ_edge(IRQ_TO_GPIO_2_80(PCMCIA_S1_CD_VALID),GPIO_BOTH_EDGES);

	//irq's for slots:
	GPDR(IRQ_TO_GPIO_2_80(PCMCIA_S0_RDYINT)) &= ~GPIO_bit(IRQ_TO_GPIO_2_80(PCMCIA_S0_RDYINT));
	GPDR(IRQ_TO_GPIO_2_80(PCMCIA_S1_RDYINT)) &= ~GPIO_bit(IRQ_TO_GPIO_2_80(PCMCIA_S1_RDYINT));

	set_GPIO_IRQ_edge(IRQ_TO_GPIO_2_80(PCMCIA_S0_RDYINT),GPIO_FALLING_EDGE);
	set_GPIO_IRQ_edge(IRQ_TO_GPIO_2_80(PCMCIA_S1_RDYINT),GPIO_FALLING_EDGE);
	
	
	return_val = request_irq(PCMCIA_S0_CD_VALID, init->handler, SA_INTERRUPT, "PXA PCMCIA CD0", NULL);
  	return_val += request_irq(PCMCIA_S1_CD_VALID, init->handler, SA_INTERRUPT, "PXA PCMCIA CD1", NULL);

  	return (return_val<0) ? -1 : 2;
}

static int
pxa_idp_pcmcia_shutdown(void)
{

	free_irq(PCMCIA_S0_CD_VALID, NULL);
	free_irq(PCMCIA_S1_CD_VALID, NULL);
  
	IDP_CPLD_PCCARD_EN = 0xC3; //disable slots
	udelay(100);
	IDP_CPLD_PCCARD_PWR = 0; //shut off all power

	return 0;
}

static int
pxa_idp_pcmcia_socket_state(struct pcmcia_state_array *state_array)
{
	unsigned long status;
	int return_val=1;
	int i;
	volatile unsigned long *stat_regs[2] = { &IDP_CPLD_PCCARD0_STATUS,
						 &IDP_CPLD_PCCARD1_STATUS };

	if (state_array->size < 2) return -1;

	memset(state_array->state, 0, (state_array->size)*sizeof(struct pcmcia_state));
	
	for (i = 0; i < 2; i++) {
		status = *stat_regs[i];

		state_array->state[i].detect = (PCC_DETECT(i) == 0) ? 1 : 0;  // this one is a gpio
		state_array->state[i].ready  = ((status & _PCC_IRQ) == 0) ? 0 : 1;
		state_array->state[i].bvd1   = ((status & PCC_BVD1) == 0) ? 1 : 0;
		state_array->state[i].bvd2   = ((status & PCC_BVD2) == 0) ? 1 : 0;
		state_array->state[i].wrprot = ((status & _PCC_WRPROT) == 0) ? 1 : 0;
		state_array->state[i].vs_3v  = ((status & PCC_VS1) == 0) ? 1 : 0;
		state_array->state[i].vs_Xv  = ((status & PCC_VS2) == 0) ? 1 : 0;
	}

	return return_val;
}

static int
pxa_idp_pcmcia_get_irq_info(struct pcmcia_irq_info *info)
{
	switch (info->sock) {
	    case 0:
		info->irq=PCMCIA_S0_RDYINT;
		break;

	    case 1:
		info->irq=PCMCIA_S1_RDYINT;
		break;

	    default:
		return -1;
	}

	return 0;
}

static int
pxa_idp_pcmcia_configure_socket(const struct pcmcia_configure *configure)
{
  /* The PXA Idp uses the Maxim MAX1602, with the following connections:
   *
   * Socket 0 (PCMCIA):
   *	MAX1602	PXA_IDP		Register
   *	Pin	Signal  	IDP_CPLD_PCCARD_PWR:
   *	-----	-------		----------------------
   *	A0VPP	PCC0_PWR0	bit0
   *	A1VPP	PCC0_PWR1	bit1	
   *	A0VCC	PCC0_PWR2	bit2
   *	A1VCC	PCC0_PWR3	bit3
   *	VX	VCC
   *	VY	+3.3V
   *	12IN	+12V
   *	CODE	+3.3V		Cirrus  Code, CODE = High (VY)
   *
   * Socket 1 (PCMCIA):
   *	MAX1602	PXA_IDP		Register
   *	Pin	Signal		IDP_CPLD_PCCARD_PWR:
   *	-----	-------		----------------------
   *	A0VPP	PCC1_PWR0	bit4
   *	A1VPP	PCC1_PWR1	bit5
   *	A0VCC	PCC1_PWR2	bit6
   *	A1VCC	PCC1_PWR3	bit7
   *	VX	VCC
   *	VY	+3.3V
   *	12IN	+12V		
   *	CODE	+3.3V		Cirrus  Code, CODE = High (VY)
   *
   */

	switch(configure->sock){
	    case 0:
		switch(configure->vcc){
		    case 0:
			IDP_CPLD_PCCARD_EN |= PCC0_ENABLE; // disable socket
			udelay(100);
			IDP_CPLD_PCCARD_PWR &= ~(PCC0_PWR2 | PCC0_PWR3);
			break;

		    case 33:
			IDP_CPLD_PCCARD_PWR &= ~(PCC0_PWR2 | PCC0_PWR3);
			IDP_CPLD_PCCARD_PWR |= PCC0_PWR3;
			IDP_CPLD_PCCARD_EN &= ~PCC0_ENABLE; //turn it on
			break;

		    case 50:
			IDP_CPLD_PCCARD_PWR &= ~(PCC0_PWR2 | PCC0_PWR3);
			IDP_CPLD_PCCARD_PWR |= PCC0_PWR2;
			IDP_CPLD_PCCARD_EN &= ~PCC0_ENABLE;
			break;

		    default:
			printk(KERN_ERR "%s(): unrecognized Vcc %u\n", __FUNCTION__,configure->vcc);
			return -1;
		}

		switch(configure->vpp){
		    case 0:
			IDP_CPLD_PCCARD_PWR &= ~(PCC0_PWR0 | PCC0_PWR1);
			break;

		    case 120:
			IDP_CPLD_PCCARD_PWR &= ~(PCC0_PWR0 | PCC0_PWR1);
			IDP_CPLD_PCCARD_PWR |= PCC0_PWR1;
			break;

		    default:
			if(configure->vpp == configure->vcc)
				IDP_CPLD_PCCARD_PWR = (IDP_CPLD_PCCARD_PWR & ~(PCC0_PWR0 | PCC0_PWR1)) | PCC0_PWR0;
			else {
				printk(KERN_ERR "%s(): unrecognized Vpp %u\n", __FUNCTION__, configure->vpp);
			return -1;
		}
	    }

	    IDP_CPLD_PCCARD_EN  = (configure->reset) ? (IDP_CPLD_PCCARD_EN | PCC0_RESET) : (IDP_CPLD_PCCARD_EN & ~PCC0_RESET);
	    break;

	case 1:
	    switch(configure->vcc){
		    case 0:
			IDP_CPLD_PCCARD_EN |= PCC1_ENABLE; // disable socket
			udelay(100);
			IDP_CPLD_PCCARD_PWR &= ~(PCC1_PWR2 | PCC1_PWR3);
			break;

		    case 33:
			IDP_CPLD_PCCARD_PWR &= ~(PCC1_PWR2 | PCC1_PWR3);
			IDP_CPLD_PCCARD_PWR |= PCC1_PWR3;
			IDP_CPLD_PCCARD_EN &= ~PCC1_ENABLE; //turn it on
			break;

		    case 50:
			IDP_CPLD_PCCARD_PWR &= ~(PCC1_PWR2 | PCC1_PWR3);
			IDP_CPLD_PCCARD_PWR |= PCC1_PWR2;
			IDP_CPLD_PCCARD_EN &= ~PCC1_ENABLE;
			break;

		    default:
			printk(KERN_ERR "%s(): unrecognized Vcc %u\n", __FUNCTION__,configure->vcc);
			return -1;
		}

		switch(configure->vpp){
		    case 0:
			IDP_CPLD_PCCARD_PWR &= ~(PCC1_PWR0 | PCC1_PWR1);
			break;

		    case 120:
			IDP_CPLD_PCCARD_PWR &= ~(PCC1_PWR0 | PCC1_PWR1);
			IDP_CPLD_PCCARD_PWR |= PCC1_PWR1;
			break;

		    default:
			if(configure->vpp == configure->vcc)
				IDP_CPLD_PCCARD_PWR = (IDP_CPLD_PCCARD_PWR & ~(PCC1_PWR0 | PCC1_PWR1)) | PCC1_PWR0;
			else {
				printk(KERN_ERR "%s(): unrecognized Vpp %u\n", __FUNCTION__, configure->vpp);
				return -1;
			}
		}
	    IDP_CPLD_PCCARD_EN  = (configure->reset) ? (IDP_CPLD_PCCARD_EN | PCC1_RESET) : (IDP_CPLD_PCCARD_EN & ~PCC1_RESET);
	    break;
	}
	return 0;
}

struct pcmcia_low_level pxa_idp_pcmcia_ops = { 
  pxa_idp_pcmcia_init,
  pxa_idp_pcmcia_shutdown,
  pxa_idp_pcmcia_socket_state,
  pxa_idp_pcmcia_get_irq_info,
  pxa_idp_pcmcia_configure_socket
};
