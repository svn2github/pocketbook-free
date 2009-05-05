/*
 *  linux/include/asm-arm/arch-pxa/cerf.h
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*
 * Add CerfBoard Specifics here...
 */

/*
 * Memory sizes
 */

#define CERF_RAM_BASE			0xa0000000

#ifdef CONFIG_PXA_CERF_RAM_128MB
#define CERF_RAM_SIZE			128*1024*1024

#elif defined (CONFIG_PXA_CERF_RAM_64MB)
#define CERF_RAM_SIZE			64*1024*1024

#elif defined (CONFIG_PXA_CERF_RAM_32MB)
#define CERF_RAM_SIZE			32*1024*1024

#elif defined (CONFIG_PXA_CERF_RAM_16MB)
#define CERF_RAM_SIZE			16*1024*1024
#endif

/*
 * CS memory timing via Static Memory Control Register (MSC0-2)
 */

#define MSC_CS(cs,val) ((val)<<((cs&1)<<4))

#define MSC_RBUFF_SHIFT 15 
#define MSC_RBUFF_SLOW (0)
#define MSC_RBUFF_FAST (1)
#define MSC_RBUFF(x) ((x)<<MSC_RBUFF_SHIFT)

#define MSC_RRR_SHIFT 12
#define MSC_RRR(x) ((x)<<MSC_RRR_SHIFT)

#define MSC_RDN_SHIFT 8
#define MSC_RDN(x) ((x)<<MSC_RDN_SHIFT)

#define MSC_RDF_SHIFT 4
#define MSC_RDF(x) ((x)<<MSC_RDF_SHIFT)

#define MSC_RBW_SHIFT 3
#define MSC_RBW(x) ((x)<<MSC_RBW_SHIFT)

#define MSC_RT_SHIFT  0
#define MSC_RT(x) ((x)<<MSC_RT_SHIFT)

/*
 * IO Pins for devices
 */

#define CERF_FLASH_BASE			0xe8000000
#define CERF_FLASH_SIZE			0x02000000
#define CERF_FLASH_PHYS			PXA_CS0_PHYS

#define CERF_ETH_BASE			0xf0000000
#define CERF_ETH_SIZE			0x00100000
#define CERF_ETH_PHYS			PXA_CS1_PHYS

#define CERF_BT_BASE			0xf2000000
#define CERF_BT_SIZE			0x00100000
#define CERF_BT_PHYS			PXA_CS2_PHYS

#define CERF_SERIAL_BASE		0xf3000000
#define CERF_SERIAL_SIZE		0x00100000
#define CERF_SERIAL_PHYS		PXA_CS3_PHYS

#define CERF_CPLD_BASE			0xf1000000
#define CERF_CPLD_SIZE			0x00100000
#define CERF_CPLD_PHYS			PXA_CS4_PHYS

#define CERF_PDA_CPLD_WRCLRINT		(0x0)
#define CERF_PDA_CPLD_BRIGHTNESS	(0x2)
#define CERF_PDA_CPLD_KEYPAD_A		(0x6)
#define CERF_PDA_CPLD_BATTFAULT		(0x8)
#define CERF_PDA_CPLD_KEYPAD_B		(0xa)
#define CERF_PDA_CPLD_SOUND_ENA		(0xc)

#define CERF_PDA_SOUND_ENABLE		0x1
#define CERF_PDA_DEFAULT_BRIGHTNESS	0x9

/*
 * Access functions (registers are 4-bit wide)
 */

#define CERF_PDA_CPLD CERF_CPLD_BASE

#define CERF_PDA_CPLD_Get(x, y)      (*((char*)(CERF_PDA_CPLD + (x))) & (y))
#define CERF_PDA_CPLD_Set(x, y, z)   (*((char*)(CERF_PDA_CPLD + (x))) = (*((char*)(CERF_PDA_CPLD + (x))) & ~(z)) | (y))
#define CERF_PDA_CPLD_UnSet(x, y, z) (*((char*)(CERF_PDA_CPLD + (x))) = (*((char*)(CERF_PDA_CPLD + (x))) & ~(z)) & ~(y))

/* 
 * IO and IRQ settings for cs8900 ethernet chip
 */
#define CERF_ETH_IO		CERF_ETH_BASE
#define CERF_ETH_IRQ 		GPIO_2_80_TO_IRQ(21)

/*
 * We only have one LED on the XScale CerfPDA so only the
 * time or idle should ever be selected.
 */
#define CERF_HEARTBEAT_LED 0x1
#define CERF_SYS_BUSY_LED  0x2

#define CERF_HEARTBEAT_LED_GPIO	16 // GPIO 4
#define CERF_SYS_BUSY_LED_GPIO	16 // GPIO 4

#define CERF_HEARTBEAT_LED_ON  (GPSR0 = CERF_HEARTBEAT_LED_GPIO)
#define CERF_HEARTBEAT_LED_OFF (GPCR0 = CERF_HEARTBEAT_LED_GPIO)
#define CERF_SYS_BUSY_LED_ON  (GPSR0 = CERF_SYS_BUSY_LED_GPIO)
#define CERF_SYS_BUSY_LED_OFF (GPCR0 = CERF_SYS_BUSY_LED_GPIO)

/*
 * UCB 1400 gpio
 */

#define CERF_GPIO_UCB1400_IRQ 32

#define UCB_IO_0                (1 << 0)
#define UCB_IO_1                (1 << 1)
#define UCB_IO_2                (1 << 2)
#define UCB_IO_3                (1 << 3)
#define UCB_IO_4                (1 << 4)
#define UCB_IO_5                (1 << 5)
#define UCB_IO_6                (1 << 6)
#define UCB_IO_7                (1 << 7)
#define UCB_IO_8                (1 << 8)
#define UCB_IO_9                (1 << 9)

#define UCB1400_GPIO_CONT_CS      UCB_IO_0
#define UCB1400_GPIO_CONT_DOWN    UCB_IO_1
#define UCB1400_GPIO_CONT_INC     UCB_IO_2
#define UCB1400_GPIO_CONT_ENA     UCB_IO_3
#define UCB1400_GPIO_LCD_RESET    UCB_IO_4
#define UCB1400_GPIO_IRDA_ENABLE  UCB_IO_5
#define UCB1400_GPIO_BT_ENABLE    UCB_IO_6
#define UCB1400_GPIO_TEST_P1      UCB_IO_7
#define UCB1400_GPIO_TEST_P2      UCB_IO_8
#define UCB1400_GPIO_TEST_P3      UCB_IO_9

/*
 * IRQ for devices
 */
#define UCB1400_IRQ(x)          (NR_IRQS + 1 + (x))

#define IRQ_UCB1400_IO0         UCB1400_IRQ(0)
#define IRQ_UCB1400_IO1         UCB1400_IRQ(1)
#define IRQ_UCB1400_IO2         UCB1400_IRQ(2)
#define IRQ_UCB1400_IO3         UCB1400_IRQ(3)
#define IRQ_UCB1400_IO4         UCB1400_IRQ(4)
#define IRQ_UCB1400_IO5         UCB1400_IRQ(5)
#define IRQ_UCB1400_IO6         UCB1400_IRQ(6)
#define IRQ_UCB1400_IO7         UCB1400_IRQ(7)
#define IRQ_UCB1400_IO8         UCB1400_IRQ(8)
#define IRQ_UCB1400_IO9         UCB1400_IRQ(9)

#define IRQ_UCB1400_CONT_CS     IRQ_UCB1400_IO0
#define IRQ_UCB1400_CONT_DOWN   IRQ_UCB1400_IO1
#define IRQ_UCB1400_CONT_INC    IRQ_UCB1400_IO2
#define IRQ_UCB1400_CONT_ENA    IRQ_UCB1400_IO3
#define IRQ_UCB1400_LCD_RESET   IRQ_UCB1400_IO4
#define IRQ_UCB1400_IRDA_ENABLE IRQ_UCB1400_IO5
#define IRQ_UCB1400_BT_ENABLE   IRQ_UCB1400_IO6
#define IRQ_UCB1400_TEST_P1     IRQ_UCB1400_IO7
#define IRQ_UCB1400_TEST_P2     IRQ_UCB1400_IO8
#define IRQ_UCB1400_TEST_P3     IRQ_UCB1400_IO9

