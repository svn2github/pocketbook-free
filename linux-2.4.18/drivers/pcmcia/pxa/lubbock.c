/*
 * linux/drivers/pcmcia/pxa/lubbock.c
 *
 * Author:	George Davis
 * Created:	Jan 10, 2002
 * Copyright:	MontaVista Software Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Originally based upon linux/drivers/pcmcia/sa1100_neponset.c
 *
 * Lubbock PCMCIA specific routines.
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/delay.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/pcmcia.h>
#include <asm/hardware/sa1111.h>

/*
 * I'd really like to move the INTPOL stuff to arch/arm/mach-sa1100/sa1111.c
 * ... and maybe even arch/arm/mach-pxa/sa1111.c now too!  : )
 */
#define SA1111_IRQMASK_LO(x)	(1 << (x - IRQ_SA1111_START))
#define SA1111_IRQMASK_HI(x)	(1 << (x - IRQ_SA1111_START - 32))

static int lubbock_pcmcia_init(struct pcmcia_init *init){
  int return_val=0;

  /* Set PCMCIA Socket 0 power to standby mode.
   */
  PA_DWR &= ~(GPIO_bit(0) | GPIO_bit(1) | GPIO_bit(2) | GPIO_bit(3));

  /* Set GPIO_A<3:0> to be outputs for PCMCIA (socket 0) power controller.
   * Note that this is done only after first initializing GPIO_A<3:0>
   * output state above to be certain that we drive signals to the same
   * state as the pull-downs connected to these lines. The pull-downs are
   * req'd to make sure PCMCIA power is OFF until we can get around to
   * setting up the GPIO_A<3:0> state and direction.
   */
  PA_DDR &= ~(GPIO_bit(0) | GPIO_bit(1) | GPIO_bit(2) | GPIO_bit(3));

  /* Set CF Socket 1 power to standby mode. */
  LUB_MISC_WR &= ~(GPIO_bit(15) | GPIO_bit(14));

  INTPOL1 |= SA1111_IRQMASK_HI(S0_READY_NINT) |
	     SA1111_IRQMASK_HI(S1_READY_NINT) |
	     SA1111_IRQMASK_HI(S0_CD_VALID) |
	     SA1111_IRQMASK_HI(S1_CD_VALID) |
	     SA1111_IRQMASK_HI(S0_BVD1_STSCHG) |
	     SA1111_IRQMASK_HI(S1_BVD1_STSCHG);

#warning what if a request_irq fails?
  return_val+=request_irq(S0_CD_VALID, init->handler, SA_INTERRUPT,
			  "Lubbock PCMCIA (0) CD", NULL);
  return_val+=request_irq(S1_CD_VALID, init->handler, SA_INTERRUPT,
			  "Lubbock CF (1) CD", NULL);
  return_val+=request_irq(S0_BVD1_STSCHG, init->handler, SA_INTERRUPT,
			  "Lubbock PCMCIA (0) BVD1", NULL);
  return_val+=request_irq(S1_BVD1_STSCHG, init->handler, SA_INTERRUPT,
			  "Lubbock CF (1) BVD1", NULL);

  return (return_val<0) ? -1 : 2;
}

static int lubbock_pcmcia_shutdown(void){

  free_irq(S0_CD_VALID, NULL);
  free_irq(S1_CD_VALID, NULL);
  free_irq(S0_BVD1_STSCHG, NULL);
  free_irq(S1_BVD1_STSCHG, NULL);

  INTPOL1 &= ~(SA1111_IRQMASK_HI(S0_CD_VALID) |
	       SA1111_IRQMASK_HI(S1_CD_VALID) |
	       SA1111_IRQMASK_HI(S0_BVD1_STSCHG) |
	       SA1111_IRQMASK_HI(S1_BVD1_STSCHG));

  return 0;
}

static int lubbock_pcmcia_socket_state(struct pcmcia_state_array
					*state_array){
  unsigned long status;
  int return_val=1;

  if(state_array->size<2) return -1;

  memset(state_array->state, 0, 
	 (state_array->size)*sizeof(struct pcmcia_state));

  status=PCSR;

  state_array->state[0].detect=((status & PCSR_S0_DETECT)==0)?1:0;

  state_array->state[0].ready=((status & PCSR_S0_READY)==0)?0:1;

  state_array->state[0].bvd1=((status & PCSR_S0_BVD1)==0)?0:1;

  state_array->state[0].bvd2=((status & PCSR_S0_BVD2)==0)?0:1;

  state_array->state[0].wrprot=((status & PCSR_S0_WP)==0)?0:1;

  state_array->state[0].vs_3v=((status & PCSR_S0_VS1)==0)?1:0;

  state_array->state[0].vs_Xv=((status & PCSR_S0_VS2)==0)?1:0;

  state_array->state[1].detect=((status & PCSR_S1_DETECT)==0)?1:0;

  state_array->state[1].ready=((status & PCSR_S1_READY)==0)?0:1;

  state_array->state[1].bvd1=((status & PCSR_S1_BVD1)==0)?0:1;

  state_array->state[1].bvd2=((status & PCSR_S1_BVD2)==0)?0:1;

  state_array->state[1].wrprot=((status & PCSR_S1_WP)==0)?0:1;

  state_array->state[1].vs_3v=((status & PCSR_S1_VS1)==0)?1:0;

  state_array->state[1].vs_Xv=((status & PCSR_S1_VS2)==0)?1:0;

  return return_val;
}

static int lubbock_pcmcia_get_irq_info(struct pcmcia_irq_info *info){

  switch(info->sock){
  case 0:
    info->irq=S0_READY_NINT;
    break;

  case 1:
    info->irq=S1_READY_NINT;
    break;

  default:
    return -1;
  }

  return 0;
}

static int lubbock_pcmcia_configure_socket(const struct pcmcia_configure
					    *configure){
  unsigned long pccr=PCCR, gpio=PA_DWR, misc_wr = LUB_MISC_WR;

  /* Lubbock uses the Maxim MAX1602, with the following connections:
   *
   * Socket 0 (PCMCIA):
   *	MAX1602	Lubbock		Register
   *	Pin	Signal
   *	-----	-------		----------------------
   *	A0VPP	S0_PWR0		SA-1111 GPIO A<0>
   *	A1VPP	S0_PWR1		SA-1111 GPIO A<1>
   *	A0VCC	S0_PWR2		SA-1111 GPIO A<2>
   *	A1VCC	S0_PWR3		SA-1111 GPIO A<3>
   *	VX	VCC
   *	VY	+3.3V
   *	12IN	+12V
   *	CODE	+3.3V		Cirrus  Code, CODE = High (VY)
   *
   * Socket 1 (CF):
   *	MAX1602	Lubbock		Register
   *	Pin	Signal
   *	-----	-------		----------------------
   *	A0VPP	GND		VPP is not connected
   *	A1VPP	GND		VPP is not connected
   *	A0VCC	S1_PWR0		MISC_WR<14>
   *	A1VCC	S1_PWR0		MISC_WR<15>
   *	VX	VCC
   *	VY	+3.3V
   *	12IN	GND		VPP is not connected
   *	CODE	+3.3V		Cirrus  Code, CODE = High (VY)
   *
   */

  switch(configure->sock){
  case 0:

    switch(configure->vcc){
    case 0:
      pccr = (pccr & ~PCCR_S0_FLT);
      gpio &= ~(GPIO_bit(2) | GPIO_bit(3));
      break;

    case 33:
      pccr = (pccr & ~PCCR_S0_PSE) | PCCR_S0_FLT | PCCR_S0_PWAITEN;
      gpio = (gpio & ~(GPIO_bit(2) | GPIO_bit(3))) | GPIO_bit(3);
      break;

    case 50:
      pccr = (pccr | PCCR_S0_PSE | PCCR_S0_FLT | PCCR_S0_PWAITEN);
      gpio = (gpio & ~(GPIO_bit(2) | GPIO_bit(3))) | GPIO_bit(2);
      break;

    default:
      printk(KERN_ERR "%s(): unrecognized Vcc %u\n", __FUNCTION__,
	     configure->vcc);
      return -1;
    }

    switch(configure->vpp){
    case 0:
      gpio &= ~(GPIO_bit(0) | GPIO_bit(1));
      break;

    case 120:
      gpio = (gpio & ~(GPIO_bit(0) | GPIO_bit(1))) | GPIO_bit(1);
      break;

    default:
      /* REVISIT: I'm not sure about this? Is this correct?
         Is it always safe or do we have potential problems
         with bogus combinations of Vcc and Vpp settings? */
      if(configure->vpp == configure->vcc)
        gpio = (gpio & ~(GPIO_bit(0) | GPIO_bit(1))) | GPIO_bit(0);
      else {
	printk(KERN_ERR "%s(): unrecognized Vpp %u\n", __FUNCTION__,
	       configure->vpp);
	return -1;
      }
    }

    pccr = (configure->reset) ? (pccr | PCCR_S0_RST) : (pccr & ~PCCR_S0_RST);

    break;

  case 1:
    switch(configure->vcc){
    case 0:
      pccr = (pccr & ~PCCR_S1_FLT);
      misc_wr &= ~(GPIO_bit(15) | GPIO_bit(14));
      break;

    case 33:
      pccr = (pccr & ~PCCR_S1_PSE) | PCCR_S1_FLT | PCCR_S1_PWAITEN;
      misc_wr = (misc_wr & ~(GPIO_bit(15) | GPIO_bit(14))) |  GPIO_bit(15);
      gpio = (gpio & ~(GPIO_bit(2) | GPIO_bit(3))) | GPIO_bit(2);
      break;

    case 50:
      pccr = (pccr | PCCR_S1_PSE | PCCR_S1_FLT | PCCR_S1_PWAITEN);
      misc_wr = (misc_wr & ~(GPIO_bit(15) | GPIO_bit(14))) |  GPIO_bit(14);
      break;

    default:
      printk(KERN_ERR "%s(): unrecognized Vcc %u\n", __FUNCTION__,
	     configure->vcc);
      return -1;
    }

    if(configure->vpp!=configure->vcc && configure->vpp!=0){
      printk(KERN_ERR "%s(): CF slot cannot support Vpp %u\n", __FUNCTION__,
	     configure->vpp);
      return -1;
    }

    pccr = (configure->reset)?(pccr | PCCR_S1_RST):(pccr & ~PCCR_S1_RST);

    break;

  default:
    return -1;
  }

  PCCR = pccr;
  LUB_MISC_WR = misc_wr;
  PA_DWR = gpio;

  return 0;
}

struct pcmcia_low_level lubbock_pcmcia_ops = { 
  lubbock_pcmcia_init,
  lubbock_pcmcia_shutdown,
  lubbock_pcmcia_socket_state,
  lubbock_pcmcia_get_irq_info,
  lubbock_pcmcia_configure_socket
};
