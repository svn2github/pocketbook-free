/*
 *  linux/arch/arm/mach-pxa/cerf.c
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/mach-types.h>
#include <asm/hardware.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/io.h>
#include <asm/arch/irq.h>

#include "generic.h"

/*
 * Set this to zero to remove all the debug statements via
 * dead code elimination.
 */
#define DEBUGGING       1

#if DEBUGGING
static unsigned int cerf_debug = DEBUGGING;
#else
#define cerf_debug       0
#endif

static void __init cerf_init_irq(void)
{
	pxa_init_irq();

	if( cerf_debug > 1)
	{
#if 0
		GPDR0 = 0xc05b9130;
		GPDR1 = 0xfcffab82;
		GPDR2 = 0x0001ffff;
#endif

		printk(KERN_INFO "Pin directions:\n");
		printk(KERN_INFO "GPDR0 0x%08x\n", GPDR0);
		printk(KERN_INFO "GPDR1 0x%08x\n", GPDR1);
		printk(KERN_INFO "GPDR2 0x%08x\n", GPDR2);

		printk(KERN_INFO "Pin State:\n");
		printk(KERN_INFO "GPLR0 0x%08x\n", GPLR0);
		printk(KERN_INFO "GPLR1 0x%08x\n", GPLR1);
		printk(KERN_INFO "GPLR2 0x%08x\n", GPLR2);

		printk(KERN_INFO "Rising Edge:\n");
		printk(KERN_INFO "GRER0 0x%08x\n", GRER0);
		printk(KERN_INFO "GRER1 0x%08x\n", GRER1);
		printk(KERN_INFO "GRER2 0x%08x\n", GRER2);

		printk(KERN_INFO "Falling Edge:\n");
		printk(KERN_INFO "GFER0 0x%08x\n", GFER0);
		printk(KERN_INFO "GFER1 0x%08x\n", GFER1);
		printk(KERN_INFO "GFER2 0x%08x\n", GFER2);
	}

	/* set_GPIO_IRQ_edge has to be called before an irq can be requested */
        set_GPIO_IRQ_edge(  0, GPIO_FALLING_EDGE); /* CPLD               */
#ifdef CONFIG_PXA_CERF_PDA
        set_GPIO_IRQ_edge(  2, GPIO_RISING_EDGE);  /* UART B Interrupt   */
        set_GPIO_IRQ_edge(  3, GPIO_RISING_EDGE);  /* UART A Interrupt   */
        set_GPIO_IRQ_edge( 32, GPIO_RISING_EDGE);  /* UCB1400 Interrupt  */
#endif
        set_GPIO_IRQ_edge( 14, GPIO_FALLING_EDGE); /* PCMCIA Card Detect */
        set_GPIO_IRQ_edge( 21, GPIO_RISING_EDGE);  /* Ethernet Interrupt */
}

static int __init cerf_init(void)
{
	/*
	 * All of the code that was here was SA1111 init code
	 * which we do not have.
	 */
	return 0;
}

__initcall(cerf_init);

static void __init
fixup_cerf(struct machine_desc *desc, struct param_struct *params,
		char **cmdline, struct meminfo *mi)
{
	SET_BANK (0, CERF_RAM_BASE, CERF_RAM_SIZE);
	mi->nr_banks      = 1;

#if 0 // Enable this stuff if you plan on not using jffs2
	setup_ramdisk (1, 0, 0, 8192);
	setup_initrd (__phys_to_virt(0xa1000000), 4*1024*1024);
	ROOT_DEV = MKDEV(RAMDISK_MAJOR,0);
#endif
}

/*
 * IO map for the devices.
 */
static struct map_desc cerf_io_desc[] __initdata = {
 /* virtual           physical          length            domain     r  w  c  b */
  { CERF_FLASH_BASE , CERF_FLASH_PHYS , CERF_FLASH_SIZE , DOMAIN_IO, 0, 1, 0, 0 },
  { CERF_ETH_BASE   , CERF_ETH_PHYS   , CERF_ETH_SIZE   , DOMAIN_IO, 0, 1, 0, 0 },
#ifdef CONFIG_PXA_CERF_PDA
  { CERF_BT_BASE    , CERF_BT_PHYS    , CERF_BT_SIZE    , DOMAIN_IO, 0, 1, 0, 0 },
  { CERF_SERIAL_BASE, CERF_SERIAL_PHYS, CERF_SERIAL_SIZE, DOMAIN_IO, 0, 1, 0, 0 },
  { CERF_CPLD_BASE  , CERF_CPLD_PHYS  , CERF_CPLD_SIZE  , DOMAIN_IO, 0, 1, 0, 0 },
#endif

  LAST_DESC
};

/*
 * Show memory, run, turbo frequency settings.
 */
static void cerf_show_freqs( void)
{
#define BASE_CLOCK	3686400
#define MAX_MEM_MULT	5
	static int mem_mult[] = { 0, 27, 32, 36, 40, 45 };
#define MAX_RMTM_MULT	7
	static int rmtm_mult[] = { 0, 0, 2, 3, 4, 0, 6, 0 }; //divide by 2 later
#define MAX_MFRM_MULT	3
	static int mfrm_mult[] = { 0, 1, 2, 0};

	int cccr = CCCR;
	int n, m, l;
	int mem_freq;
	int rm_freq;
	int tm_freq;
        int p14val; 

	n = (cccr & CCCR_N_MASK) >> 7;
	m = (cccr & CCCR_M_MASK) >> 5;
	l =  cccr & CCCR_L_MASK;

	if( l > MAX_MEM_MULT)
	{
	    printk(KERN_INFO "cerf.c: Unknown memory freq multiplier.\n");
	    return;
	}

	if( n > MAX_RMTM_MULT)
	{
	    printk(KERN_INFO "cerf.c: Unknown run-mode to turbo-mode freq multiplier.\n");
	    return;
	}

	if( m > MAX_MFRM_MULT)
	{
	    printk(KERN_INFO "cerf.c: Unknown memory to run-mode freq multiplier.\n");
	    return;
	}

	mem_freq = BASE_CLOCK * mem_mult[l]; 
	rm_freq  = mem_freq * mfrm_mult[m];
	tm_freq  = rm_freq * rmtm_mult[n]/2;
	
	printk(KERN_INFO "Memory freq     = %d Mhz\n", mem_freq/1000000);
	printk(KERN_INFO "Run mode freq   = %d Mhz\n", rm_freq/1000000);
	printk(KERN_INFO "Turbo mode freq = %d Mhz\n", tm_freq/1000000);

        asm("mrc%?   p14, 0, %0, c6, c0" : "=r" (p14val));
	printk(KERN_INFO "Turbo mode %s.\n", p14val&0x01 ? "on" : "off");
}

static void __init cerf_map_io(void)
{
	pxa_map_io();
	iotable_init(cerf_io_desc);

	if( cerf_debug) cerf_show_freqs();
	
	if( cerf_debug > 1)
	{
		printk(KERN_INFO "origMCS0 = 0x%08x\n", MSC0);
		printk(KERN_INFO "origMCS1 = 0x%08x\n", MSC1);
		printk(KERN_INFO "origMCS2 = 0x%08x\n", MSC2);
	}

	/* setup memory timing for CS0/1 */
        MSC0 = MSC_CS(0, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(7) |
			 MSC_RDN(15) |
			 MSC_RDF(15) |
			 MSC_RBW(0) |
			 MSC_RT(0)) |
#ifdef CONFIG_PXA_CERF_PDA
               MSC_CS(1, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(7) |
			 MSC_RDN(15) |
			 MSC_RDF(15) |
			 MSC_RBW(1) |
			 MSC_RT(0));
#elif defined(CONFIG_PXA_CERF_BOARD)
               MSC_CS(1, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(1) |
			 MSC_RDN(2) |
			 MSC_RDF(4) |
			 MSC_RBW(0) |
			 MSC_RT(4));
#endif
	printk(KERN_INFO "MCS0 = 0x%08x\n", MSC0);

	/* setup memory timing for CS2/3 */
        MSC1 = MSC_CS(2, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(5) |
			 MSC_RDN(10) |
			 MSC_RDF(10) |
			 MSC_RBW(1) |
			 MSC_RT(0)) |
               MSC_CS(3, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(5) |
			 MSC_RDN(10) |
			 MSC_RDF(10) |
			 MSC_RBW(1) |
			 MSC_RT(0));
	printk(KERN_INFO "MCS1 = 0x%08x\n", MSC1);

	/* setup memory timing for CS4/5 */
        MSC2 = MSC_CS(4, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(2) |
			 MSC_RDN(4) |
			 MSC_RDF(4) |
			 MSC_RBW(1) |
			 MSC_RT(0)) |
               MSC_CS(5, MSC_RBUFF(MSC_RBUFF_SLOW) | 
			 MSC_RRR(2) |
			 MSC_RDN(4) |
			 MSC_RDF(4) |
			 MSC_RBW(1) |
			 MSC_RT(0));
	printk(KERN_INFO "MCS2 = 0x%08x\n", MSC2);

#ifdef CONFIG_SOUND_PXA_AC97
	printk(KERN_INFO "Enabling sound amp for pxa cerf pda.\n");
        outw( CERF_PDA_SOUND_ENABLE, CERF_CPLD_BASE+CERF_PDA_CPLD_SOUND_ENA);
#endif

#ifdef CONFIG_FB_PXA
	printk(KERN_INFO "Setting LCD to brightness to %d/15\n", CERF_PDA_DEFAULT_BRIGHTNESS);
	outw( CERF_PDA_DEFAULT_BRIGHTNESS, CERF_CPLD_BASE+CERF_PDA_CPLD_BRIGHTNESS);
#endif

#ifdef CONFIG_IRDA
	/* Enable IrDA UART (SIR)*/
	CKEN |= CKEN5_STUART;

	/* We want to get our goods from the STUART */
	set_GPIO_mode(GPIO46_STRXD_MD);
	set_GPIO_mode(GPIO47_STTXD_MD);

	/* make sure FIR ICP is off */
	ICCR0 = 0;

	/* configure STUART to for SIR
	 * NOTE: RCVEIR and XMITIR must not be set at the same time!
	 * Start with receive in IR mode, and switch transmit to IR only
	 * when we need to send something in serial driver.
	 */
	STISR = IrSR_IR_RECEIVE_ON;
#endif

#if 0
	/* Connect FIR ICP to GPIO pins */
	CKEN |= CKEN13_FICP;
	set_GPIO_mode(GPIO46_ICPRXD_MD);
	set_GPIO_mode(GPIO47_ICPTXD_MD);
	ICCR0 = 0x1 | 0x18; //ICP unit enable
#endif

#if 0
	/* Enable BT UART */
	CKEN |= CKEN7_BTUART;
	set_GPIO_mode(GPIO42_BTRXD_MD);
	set_GPIO_mode(GPIO43_BTTXD_MD);
	set_GPIO_mode(GPIO44_BTCTS_MD);
	set_GPIO_mode(GPIO45_BTRTS_MD);
#endif

	if( cerf_debug > 1)
	{
		printk(KERN_INFO "GPDR1 0x%08x\n", GPDR1);
		printk(KERN_INFO "GPLR1 0x%08x\n", GPLR1);
		printk(KERN_INFO "GAFR1_L 0x%08x\n", GAFR1_L);
		printk(KERN_INFO "CKEN  = 0x%08x\n", CKEN);
		printk(KERN_INFO "ICCR0 = 0x%08x\n", ICCR0);
		printk(KERN_INFO "STISR = 0x%08x\n", STISR);
	}
}

MACHINE_START(PXA_CERF, "CerfBoard PXA Reference Board")
	MAINTAINER("Intrinsyc Software Inc.")
	BOOT_MEM(0xa0000000, 0x40000000, 0xfc000000)
	BOOT_PARAMS(0xa0000100)
	FIXUP(fixup_cerf)
	MAPIO(cerf_map_io)
	INITIRQ(cerf_init_irq)
MACHINE_END
