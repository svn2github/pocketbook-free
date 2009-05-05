/*
 * linux/include/asm-arm/arch-s3c2400/mz_dubug_ll.h
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Author : Nandy Lyu <nandy@mizi.com>
 *   Date : 13 December 2001
 *
 * For Low Level Debug
 */

/*
 * The following code assumes the serial port has already been
 * initialized by the bootloader. We use only UART1 on S3C24xx
 */

#define UART_UTRSTAT		(*(volatile unsigned long *)0xf0000010)
#define UART_UTXH		(*(volatile unsigned long *)0xf0000020)
#define UTRSTAT_TX_EMPTY	(1 << 2)

static inline void puts(const char *s)
{
	while (*s) {
		while (!(UART_UTRSTAT & UTRSTAT_TX_EMPTY));

		UART_UTXH = *s;

		if (*s == '\n') {
			while (!(UART_UTRSTAT & UTRSTAT_TX_EMPTY));

			UART_UTXH = '\r';
		}
		s++;
	}
	while (!(UART_UTRSTAT & UTRSTAT_TX_EMPTY));
}

#define LED1	(1 << 12)
#define LED2	(1 << 13)
#define LED3	(1 << 14)
#define LED4	(1 << 15)

static inline void s3c2400_leds(unsigned long what, unsigned long how)
{
	unsigned long mmu;
	volatile unsigned long *base;

	__asm__("mrc p15, 0, %0, c1, c0, 0\n"
			: "=r" (mmu));
	mmu &= 0x1;

	if (mmu) 
		base = (unsigned long *)(0xf5600018);
	else
		base = (unsigned long *)(0x15600018);

	if (how == 1) {	/* LED on */
		*base &= ~(what);
	} else if (how == 0) {
		*base |= what;
	} else {
		; /* error */
	}
}

static void inline s3c2410_go_zero(void)
{
	__asm__("mov r0, #0x0c000000\n
			 add r0, r0, #0x8000\n
			 mov pc, r0\n");
}

#ifdef CONFIG_S3C2400_GAMEPARK

#define PHONEBTN_LED_CON (1 << 10)
#define PHONEBTN_LED_ON (1 << 5)

/* delay between led on/off */
#define PHONEBTN_LED_DELAY_ONOFF 10 /* 100 ms */
#define PHONEBTN_LED_DELAY_END   30 /* 300 ms */

/* Note: the unit of parameter 'tenms' is 10 ms */
static inline void gamepark_delay(int tenms) 
{
	unsigned long j = jiffies + tenms * (HZ/100);

	while (jiffies < j);
}

/* Note: the parameter 'count' means how many times the led is on/off. */
static inline void gamepark_led(int count)
{
	int i;

	PDCON |= PHONEBTN_LED_CON;

	for (i = 0; i < count; i++) {
		if (i != 0)	gamepark_delay(PHONEBTN_LED_DELAY_ONOFF);

		PDDAT |= PHONEBTN_LED_ON;

		gamepark_delay(PHONEBTN_LED_DELAY_ONOFF);

		PDDAT &= ~(PHONEBTN_LED_ON);
	}

	gamepark_delay(PHONEBTN_LED_DELAY_END);
}

#endif /* CONFIG_S3C2400_GAMEPARK */
