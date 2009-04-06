/*
 * linux/include/asm-arm/arch-s3c2400/uncompress.h
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
 */

/*
 * The following code assumes the serial port has already been
 * initialized by the bootloader. We use only UART1 on S3C24xx
 */
#include <linux/config.h>

#ifdef CONFIG_MIZI
#define ULCON			0x0
#define UTRSTAT			0x10
#define UTXH			0x20
#define UTRSTAT_TX_EMPTY	(1 << 2)

#define UART0			0x50000000
#define UART1			0x50004000
#define UART2			0x50008000

#define UART(x)			(*(volatile unsigned long *)(serial_port + (x)))

static void puts(const char *s)
{
	unsigned long serial_port;

	do {
		serial_port = UART0;
		if (UART(ULCON) == 0x3) break;
		serial_port = UART1;
		if (UART(ULCON) == 0x3) break;
		serial_port = UART2;
		if (UART(ULCON) == 0x3) break;
	} while (0);

	for (; *s; s++) {
		/* wait */
		while (!(UART(UTRSTAT) & UTRSTAT_TX_EMPTY));

		/* send the character out. */
		UART(UTXH) = *s;

		/* if a LF, also do CR... */
		if (*s == 10) {
			while (!(UART(UTRSTAT) & UTRSTAT_TX_EMPTY));

			UART(UTXH) = 13;
		}
	}
}

#else

#define UART_UTRSTAT		(*(volatile unsigned long *)0x50000010)
#define UART_UTXH		(*(volatile unsigned long *)0x50000020)	/* littel endian */
#define UTRSTAT_TX_EMPTY	(1 << 2)

static void puts(const char *s)
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
#endif

/*
 * Nothing to do for these
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
