/*
 * smdk.h
 *
 * s3c2410-SMDK specific definiton
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:39 $ 
 *
 * $Revision: 1.1.1.1 $
 */

#ifndef __ASM_ARCH_HARDWARE_H
#error "include <asm/hardware.h> instead"
#endif

/* definition of IRQ */
#define IRQ_CS8900              IRQ_EINT9
#define IRQ_KBD                 IRQ_EINT1
#define IRQ_nCF_INS             IRQ_EINT3
#define IRQ_CF_RDY              IRQ_EINT8
#define IRQ_SMDK_POWER_BT       IRQ_EINT0
#ifdef CONFIG_SMDK_AIJI
#define IRQ_SMDK_BT0            IRQ_EINT2
#define IRQ_SMDK_BT1            IRQ_EINT11
#define IRQ_SMDK_BT2            IRQ_EINT19
#else   
#define IRQ_SMDK_BT1            IRQ_EINT2
#define IRQ_SMDK_BT2            IRQ_EINT11
#define IRQ_SMDK_BT3            IRQ_EINT19
#endif
#define IRQ_SMDK_BT4            IRQ_EINT10
#define IRQ_SMDK_BT5            IRQ_EINT13
#define IRQ_SMDK_BT6            IRQ_EINT14
#define IRQ_SMDK_BT7            IRQ_EINT15
#define IRQ_nCD_SD              IRQ_EINT18
#define IRQ_SMDK_UP             IRQ_GPB5        
#define IRQ_SMDK_DOWN           IRQ_GPB6
#define IRQ_SMDK_LEFT           IRQ_GPE11
#define IRQ_SMDK_RIGHT          IRQ_GPE12

/* CS8900a, nGCS3 */
#define pCS8900_BASE		0x19000000
#define vCS8900_BASE		0xd0000000

/* PCMCIA, nGCS2 */
#define pCF_MEM_BASE		0x10000000
#define vCF_MEM_BASE		0xd1000000
#define pCF_IO_BASE		0x11000000
#define vCF_IO_BASE		0xd2000000

#define GPIO_YPON	(GPIO_MODE_nYPON | GPIO_PULLUP_DIS | GPIO_G15)
#define GPIO_YMON	(GPIO_MODE_YMON | GPIO_PULLUP_EN | GPIO_G14)
#define GPIO_XPON	(GPIO_MODE_nXPON | GPIO_PULLUP_DIS | GPIO_G13)
#define GPIO_XMON	(GPIO_MODE_XMON | GPIO_PULLUP_EN | GPIO_G12)
#define GPIO_LED1	(GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_F4)
#define GPIO_LED2	(GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_F5)
#define GPIO_LED3	(GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_F6)
#define GPIO_LED4	(GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_F7)

#ifdef CONFIG_SMDK_AIJI
#define GPIO_KBD_nSS            (GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_B6)
#endif
#define GPIO_KBD_PWR            (GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_B0)
#define GPIO_KBD_SPIMISO        (GPIO_MODE_ALT1 | GPIO_PULLUP_DIS | GPIO_G5)
#define GPIO_KBD_SPIMOSI        (GPIO_MODE_ALT1 | GPIO_PULLUP_DIS | GPIO_G6)
#define GPIO_KBD_SPICLK         (GPIO_MODE_ALT1 | GPIO_PULLUP_DIS | GPIO_G7)

#define GPIO_SMDK_POWER_BT      GPIO_F0
#ifdef CONFIG_SMDK_AIJI
#define GPIO_SMDK_BT0           GPIO_F2
#define GPIO_SMDK_BT1           GPIO_G3
#define GPIO_SMDK_BT2           GPIO_G11
#else 
#define GPIO_SMDK_BT1           GPIO_F2
#define GPIO_SMDK_BT2           GPIO_G3
#define GPIO_SMDK_BT3           GPIO_G11
#endif
#define GPIO_SMDK_BT4           GPIO_G2
#define GPIO_SMDK_BT5           GPIO_G5
#define GPIO_SMDK_BT6           GPIO_G6
#define GPIO_SMDK_BT7           GPIO_G7
#define GPIO_SMDK_UP            (GPIO_MODE_IN | GPIO_PULLUP_DIS | GPIO_B5)

#ifndef CONFIG_SMDK_AIJI
#define GPIO_SMDK_DOWN          (GPIO_MODE_IN | GPIO_PULLUP_DIS | GPIO_B6)
#endif

#define GPIO_SMDK_LEFT          (GPIO_MODE_IN | GPIO_PULLUP_DIS | GPIO_E11)
#define GPIO_SMDK_RIGHT         (GPIO_MODE_IN | GPIO_PULLUP_DIS | GPIO_E12)

#define GPIO_nCF_INS		(GPIO_MODE_IN | GPIO_PULLUP_EN | GPIO_F3)
#define GPIO_CF_RDY		(GPIO_MODE_IN | GPIO_PULLUP_EN | GPIO_G8)

#define GPIO_nCD_SD  		(GPIO_MODE_IN | GPIO_PULLUP_DIS | GPIO_G10)

#define GPIO_IR_TXD		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_H6)
#define GPIO_IR_RXD		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_H7)



/*
 | $Id: smdk.h,v 1.1.1.1 2004/02/04 12:57:39 laputa Exp $
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
