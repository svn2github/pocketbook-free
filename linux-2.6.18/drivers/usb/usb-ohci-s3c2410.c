/*
 *  linux/drivers/usb/usb-ohci-s3c2410.c
 *
 *  OHCI r1.0 compatible, CPU embedded USB host controller
 *
 *  2003-09-03 kwanghyun la<nala.la@samsung.com>
 *  	- to get the USB CLOCK setting value & set register for s3c2410
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/pci.h>
#include <linux/pm.h>
#include <asm/arch/S3C2410.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>

#define OHCI_HW_DRIVER 1
#include "usb-ohci.h"

int __devinit
hc_add_ohci(struct pci_dev *dev, int irq, void *membase, unsigned long flags,
	    ohci_t **ohci, const char *name, const char *slot_name);
extern void hc_remove_ohci(ohci_t *ohci);

extern int pviio_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data);

ohci_t *s3c2410_ohci;



void usb_clk_on(void)
{

//Joe
	CLKCON  &= ~CLKCON_USBH;
	
#if CONFIG_MAX_ROOT_PORTS < 2
	/* 1 host port, 1 slave port*/
	MISCCR &= ~MISCCR_USBPAD;
//Joe
	MISCCR &= 0x3ff01f;
	MISCCR |= 0x10;
	GPHCON |= 0x280000;
        GPHCON &= 0x2bffff;

#else
	/* 2 host port */
	MISCCR |= MISCCR_USBPAD;
//Joe
        MISCCR &= 0x3ff01f;
        MISCCR |= 0x10;
	GPHCON |= 0x280000;
	GPHCON &= 0x2bffff;	
#endif
	

	/* UPLLCON */
// laputa for USB PLL setting value chaged input 12MHz --> output 48MHz 	
//	UPLLCON = FInsrt(0x38, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x02, fPLL_SDIV);
	UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);
	
//Joe
//        printk(KERN_ALERT" usb-ohci-s3c2410.c UPLLCON = %x MISCCR = %x GPHCON = %x\n",UPLLCON,MISCCR,GPHCON);

//Joe
	while(UPLLCON != 0x78023){
//		printk(KERN_ALERT"Reset The UPLLCON \n");
		UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);
		udelay(5);
	}
	/* CLKCON */
	CLKCON  |= CLKCON_USBH;
	
	udelay(11);

//	wait_ms(20);	// ok
//	wait_ms(10);	// ok
	wait_ms(2);
}


void usb_clk_on2(void)
{
//Joe
        CLKCON  &= ~CLKCON_USBH;	

#if CONFIG_MAX_ROOT_PORTS < 2
	/* 1 host port, 1 slave port*/
	MISCCR &= ~MISCCR_USBPAD;
//Joe
        MISCCR &= 0x3ff01f;
        MISCCR |= 0x10;
	GPHCON |= 0x280000;
        GPHCON &= 0x2bffff;

#else
	/* 2 host port */
	MISCCR |= MISCCR_USBPAD;
//Joe
        MISCCR &= 0x3ff01f;
        MISCCR |= 0x10;
	GPHCON |= 0x280000;
        GPHCON &= 0x2bffff;

#endif
	

	/* UPLLCON */
// laputa for USB PLL setting value chaged input 12MHz --> output 48MHz 	
//	UPLLCON = FInsrt(0x38, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x02, fPLL_SDIV);
	UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);
	
//Joe
//        printk(KERN_ALERT" usb-ohci-s3c2410.c UPLLCON = %x MISCCR = %x GPHCON = %x\n",UPLLCON,MISCCR,GPHCON);


//Joe
        while(UPLLCON != 0x78023){
//		printk(KERN_ALERT"Reset The UPLLCON");
                UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);
		udelay(5);
	}
	/* CLKCON */
	CLKCON  |= CLKCON_USBH;
	
	udelay(11);
}


void usb_clk_off(void)
{
	
//	wait_ms(10);

	/* CLKCON */
	CLKCON  &= ~(CLKCON_USBH);

}

void usb_clk_off2(void)
{
	

	/* CLKCON */
	CLKCON  &= ~(CLKCON_USBH);

}

static void __init s3c2410_ohci_configure(void)
{
//Joe
        CLKCON  &= ~CLKCON_USBH;


#if CONFIG_MAX_ROOT_PORTS < 2
	/* 1 host port, 1 slave port*/
	MISCCR &= ~MISCCR_USBPAD;
//Joe
        MISCCR &= 0x3ff01f;
        MISCCR |= 0x10;
	GPHCON |= 0x280000;
        GPHCON &= 0x2bffff;

#else
	/* 2 host port */
	MISCCR |= MISCCR_USBPAD;
//Joe
        MISCCR &= 0x3ff01f;
        MISCCR |= 0x10;
	GPHCON |= 0x280000;
        GPHCON &= 0x2bffff;

#endif

	/* UPLLCON */
// laputa for USB PLL setting value chaged input 12MHz --> output 48MHz 	
//	UPLLCON = FInsrt(0x38, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x02, fPLL_SDIV);
	UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);

//Joe
//        printk(KERN_ALERT" usb-ohci-s3c2410.c UPLLCON = %x MISCCR = %x GPHCON = %x\n",UPLLCON,MISCCR,GPHCON);


//Joe
        while(UPLLCON != 0x78023){
//		printk(KERN_ALERT"Reset The UPLLCON \n");
                UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) | FInsrt(0x03, fPLL_SDIV);
		udelay(5);
	}

	/* CLKCON */
	CLKCON  |= CLKCON_USBH;

	//laputa append begine -->
//	CLKSLOW &= ~(UCLK_ON | MPLL_OFF | SLOW_BIT); 
	//laputa append end <--

//Joe
	*(volatile unsigned long*)(io_p2v(USBHOST_CTL_BASE+0x48)) |= (0xFFL<<24);	
	udelay(11);
}

#ifdef CONFIG_PM
struct pm_dev *ohci_pm_dev;

extern int hc_reset (ohci_t * ohci);
extern void hc_restart (ohci_t * ohci);

void ohci_suspend()
{
	
	hc_remove_ohci(s3c2410_ohci);
	
}

void *usbhost_ctl_base;

void ohci_resume()
{
	int ret;	
	
/*
	 * Initialise the generic OHCI driver.
	 */
	// FIXME : io_p2v() ?

#if 0	
	ret = hc_add_ohci((struct pci_dev *)1, IRQ_USBH,
			  (void *)(io_p2v(USBHOST_CTL_BASE)), 0, &s3c2410_ohci,
			  "usb-ohci", "s3c2410");
#else

	ret = hc_add_ohci((struct pci_dev *)1, IRQ_USBH,
			  (void *)usbhost_ctl_base, 0, &s3c2410_ohci,
			  "usb-ohci", "s3c2410");		  
			  	
#endif			  
		  
			  
}



static int
ohci_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{

//	pviio_pm_callback(pm_dev,req,data);

	if (req == PM_SUSPEND) {		
		printk("dz: ohci.c suspend 1 \n");								
//		dz_ohci_suspend(s3c2410_ohci);
		ohci_suspend();
		usb_clk_off();
		
	} else if (req == PM_RESUME) 
	{			  
		printk("dz: ohci.c resume 1\n");				
		usb_clk_on();			  
//		dz_ohci_resume(s3c2410_ohci);		
		ohci_resume();	
//		usb_dev_off();				
//		wait_ms(500);
//		usb_dev_on();
		
	}else
	{
		printk("dz: ohci.c else \n");						
	}	

	return 0;
}

#endif 


static int __init s3c2410_ohci_init(void)
{
	int ret;
	printk("\n\n 05_20 add init module for pvi_pm1 \n\n"); 
//	init_module ();
//	printk("06_01 add init module for Eink Driver \n"); 			
//	init_Eink_module ();	
		
	
	printk("dz: usb-ohci-s3c2410.c \n");

	s3c2410_ohci_configure();
	
	printk("dz: init_usb_io() \n");
	init_usb_io();
	
	/*
	 * Initialise the generic OHCI driver.
	 */
	// FIXME : io_p2v() ?
	
#if 1	
	ret = hc_add_ohci((struct pci_dev *)1, IRQ_USBH,
			  (void *)(io_p2v(USBHOST_CTL_BASE)), 0, &s3c2410_ohci,
			  "usb-ohci", "s3c2410");
			  
	usbhost_ctl_base=(void *)(io_p2v(USBHOST_CTL_BASE));			  
#else
	ret = hc_add_ohci((struct pci_dev *)1, IRQ_USBH,
			  (void *)(ioremap_nocache(USBHOST_CTL_BASE,4)), 0, &s3c2410_ohci,
			  "usb-ohci", "s3c2410");
#endif

#ifdef CONFIG_PM
	ohci_pm_dev = pm_register(PM_USER_DEV, 0,
					 ohci_pm_callback);
#endif		




	

	return ret;
}

static void __exit s3c2410_ohci_exit(void)
{
//	Eink_cleanup_module();	

//	cleanup_module ();
	
	hc_remove_ohci(s3c2410_ohci);


	//cleanup_cpu ();
	/*
	 * Stop the USB clock.
	 */
	CLKCON &= ~CLKCON_USBH;

#ifdef CONFIG_PM
	 pm_unregister(ohci_pm_dev);
#endif		

}
// ****************************************** USB Power ******************************************************

typedef struct 
{
	int bit0_7:8;
	int mpwr:1;
}PORTD_DATAREG;

typedef struct 
{
	int bit0_7:16;
	int mpwr:2;
}PORTD_CONREG;


#define PD_IN  0x0
#define PD_OUT 0x01

PORTD_CONREG portd_conreg;
PORTD_DATAREG portd_datareg;

volatile PORTD_CONREG *portd_conregp;
volatile PORTD_DATAREG *portd_dataregp;



void init_usb_io()
{
	portd_conregp=(volatile PORTD_CONREG *)ioremap_nocache(GPDCON,4);
	portd_dataregp=(volatile PORTD_DATAREG *)ioremap_nocache(GPDDAT,4);

	portd_conreg=*portd_conregp;	
	portd_conreg.mpwr=PD_OUT;
	*portd_conregp=portd_conreg;	
}


void usb_dev_off()
{
	portd_conreg=*portd_conregp;	
	portd_conreg.mpwr=PD_OUT;
	*portd_conregp=portd_conreg;	
	
	
	portd_datareg=*portd_dataregp;	
	portd_datareg.mpwr=0;
	*portd_dataregp=portd_datareg;	
}

void usb_dev_on()
{
	portd_conreg=*portd_conregp;	
	portd_conreg.mpwr=PD_OUT;
	*portd_conregp=portd_conreg;	
	
	
	portd_datareg=*portd_dataregp;	
	portd_datareg.mpwr=1;
	*portd_dataregp=portd_datareg;	
}





module_init(s3c2410_ohci_init);
module_exit(s3c2410_ohci_exit);
