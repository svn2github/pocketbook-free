/*
 * Generic receive layer for the PXA USB client function
 *
 * This code was loosely inspired by the original version which was
 * Copyright (c) Compaq Computer Corporation, 1998-1999
 * Copyright (c) 2001 by Nicolas Pitre
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 02-May-2002
 *   Frank Becker (Intrinsyc) - derived from sa1100 usb_recv.c
 * 
 * TODO: Add support for DMA.
 *
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <asm/dma.h>
#include <asm/system.h>

#include "pxa_usb.h"
#include "usb_ctl.h"

#if DEBUG
static unsigned int usb_debug = DEBUG;
#else
#define usb_debug 0     /* gcc will remove all the debug code for us */
#endif

static char *ep_bulk_out1_buf;
static int   ep_bulk_out1_len;
static int   ep_bulk_out1_remain;
static usb_callback_t ep_bulk_out1_callback;
static int rx_pktsize;

static void
ep_bulk_out1_start(void)
{
	/* disable DMA */
	UDCCS2 &= ~UDCCS_BO_DME;

	/* enable interrupts for endpoint 2 (bulk out) */
        UICR0 &= ~UICR0_IM2;
}

static void
ep_bulk_out1_done(int flag)
{
	int size = ep_bulk_out1_len - ep_bulk_out1_remain;

	if (!ep_bulk_out1_len)
		return;

	ep_bulk_out1_len = 0;
	if (ep_bulk_out1_callback) {
		ep_bulk_out1_callback(flag, size);
	}
}

void
ep_bulk_out1_state_change_notify( int new_state )
{
}

void
ep_bulk_out1_stall( void )
{
	/* SET_FEATURE force stall at UDC */
	UDCCS2 |= UDCCS_BO_FST;
}

int
ep_bulk_out1_init(int chn)
{
	desc_t * pd = pxa_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu( pd->b.ep1.wMaxPacketSize );
	ep_bulk_out1_done(-EAGAIN);
	return 0;
}

void
ep_bulk_out1_reset(void)
{
	desc_t * pd = pxa_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu( pd->b.ep1.wMaxPacketSize );
	UDCCS2 &= ~UDCCS_BO_FST;
	ep_bulk_out1_done(-EINTR);
}

void
ep_bulk_out1_int_hndlr(int udcsr)
{
	int status = UDCCS2;
	if( usb_debug) printk("ep_bulk_out1_int_hndlr: UDCCS2=%x\n", status);

	if( (status & (UDCCS_BO_RNE | UDCCS_BO_RSP)) == UDCCS_BO_RSP)
	{
		/* zero-length packet */
	}

	if( status & UDCCS_BO_RNE)
	{
		int len;
		int i;
		char *buf = ep_bulk_out1_buf + ep_bulk_out1_len - ep_bulk_out1_remain;

		/* bytes in FIFO */
		len = (UBCR2 & 0xff) +1;
		
		if( usb_debug) printk("usb_recv: "
			"len=%d out1_len=%d out1_remain=%d\n",
			len,ep_bulk_out1_len,ep_bulk_out1_remain);

		if( len > ep_bulk_out1_remain)
		{
			/* FIXME: if this happens, we need a temporary overflow buffer */
			printk("usb_recv: Buffer overwrite warning...\n");
			len = ep_bulk_out1_remain;
		}

		/* read data out of fifo */
		for( i=0; i<len; i++)
		{
			*buf++ = UDDR2 & 0xff;
		}

		ep_bulk_out1_remain -= len;
		ep_bulk_out1_done((len) ? 0 : -EPIPE);
	}

	/* ack RPC - FIXME: '|=' we may ack SST here, too */
	UDCCS2 |= UDCCS_BO_RPC;
	return;
}

int
pxa_usb_recv(char *buf, int len, usb_callback_t callback)
{
	int flags;

	if (ep_bulk_out1_len)
		return -EBUSY;

	local_irq_save(flags);
	ep_bulk_out1_buf = buf;
	ep_bulk_out1_len = len;
	ep_bulk_out1_callback = callback;
	ep_bulk_out1_remain = len;
	ep_bulk_out1_start();
	local_irq_restore(flags);

	return 0;
}

void
pxa_usb_recv_reset(void)
{
	ep_bulk_out1_reset();
}

void
pxa_usb_recv_stall(void)
{
	ep_bulk_out1_stall();
}

EXPORT_SYMBOL(pxa_usb_recv_stall);
EXPORT_SYMBOL(pxa_usb_recv);
EXPORT_SYMBOL(pxa_usb_recv_reset);
