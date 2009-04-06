#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/usb.h>

#include <asm/hardware.h>
#include <asm/arch/mmsp20.h>	// oyh
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/pci.h>

#include "usb-ohci.h"

int __devinit
hc_add_ohci(struct pci_dev *dev, int irq, void *membase, unsigned long flags,
	    const char *name, const char *slot_name);
extern void hc_remove_ohci(ohci_t *ohci);

static ohci_t *mmsp2_ohci;

#define DESIRED_CLOCK (48000000)

/*
 * FIXME
 */
static void __init mmsp2_ohci_configure(void)
{
	unsigned long fclk = mmsp2_get_fclk();
	unsigned long aclk = mmsp2_get_aclk();
	unsigned long uclk = mmsp2_get_uclk();
	unsigned long f_fact = fclk / DESIRED_CLOCK;
	unsigned long a_fact = aclk / DESIRED_CLOCK;
	unsigned long u_fact = uclk / DESIRED_CLOCK;
	unsigned long f_clk = (fclk / f_fact);
	unsigned long a_clk = (aclk / a_fact);
	unsigned long u_clk = (uclk / u_fact);
	unsigned long f_err = abs(DESIRED_CLOCK - f_clk);
	unsigned long a_err = abs(DESIRED_CLOCK - a_clk);
	unsigned long u_err = abs(DESIRED_CLOCK - u_clk);

	printk("f_fact = %ld, f_clk = %ld, f_err = %c%ld\n",
			f_fact, f_clk, f_clk > DESIRED_CLOCK ? '+':'-', f_err);
	printk("a_fact = %ld, a_clk = %ld, a_err = %c%ld\n",
			a_fact, a_clk, a_clk > DESIRED_CLOCK ? '+':'-', a_err);
	printk("u_fact = %ld, u_clk = %ld, u_err = %c%ld\n",
			u_fact, u_clk, u_clk > DESIRED_CLOCK ? '+':'-', u_err);

#if 0
	UIRMCSET &= 0xff00;
	UIRMCSET |= ((0x3) << 6) | (a_fact - 1); // A_CLK
#else
	/* from EBOOT */
	UIRMCSET &= 0xff00;
	UIRMCSET |= ((0x02) << 6) | 1;
#endif

//	GPIOPADSEL &= 0xfffc; 		
	gpio_pad_select(USB_PAD_3T, 0);		// oyh, PAD3 --> USB device
	gpio_pad_select(USB_PAD_1T, 0);		// PAD1 --> USB host
#ifdef CONFIG_MMSP2_UPAD3_TO_DEVICE
//	GPIOPADSEL |= 0x2;			
	gpio_pad_select(USB_PAD_3T, 1);		// oyh, PAD3 --> USB device
	gpio_pad_select(USB_PAD_1T, 0);		// PAD1 --> USB host

#endif

#ifdef CONFIG_MACH_MMSP2_MDK
	write_gpio_bit(GPIO_UH2_nPWR_EN, 0);
	set_gpio_ctrl(GPIO_UH2_nPWR_EN, GPIOMD_OUT, GPIOPU_DIS);
	write_gpio_bit(GPIO_UH2_nPWR_EN, 0);
#endif /* CONFIG_MACH_MMSP2_MDK */

	udelay(11);
}

static int __init mmsp2_ohci_init(void)
{
	int ret;

	mmsp2_ohci_configure();

	/*
	 * 주소는 0xc0004300 가 맞다
	 */
	ret = hc_add_ohci((void*)(1), IRQ_USBH,
			  (void *)(io_p2v(0xc0004300)), 0, "usb-ohci", "mmsp2_ohci");
	if((ret & 0xF0000000)==0xC0000000) {
		mmsp2_ohci = (ohci_t *)ret;
		ret = 0;
	}
	return ret;
}

static void __exit mmsp2_ohci_exit(void)
{
	hc_remove_ohci(mmsp2_ohci);
}

module_init(mmsp2_ohci_init);
module_exit(mmsp2_ohci_exit);
