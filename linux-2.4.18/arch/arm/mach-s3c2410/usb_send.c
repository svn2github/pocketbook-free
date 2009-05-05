/*
 * usb_send.c 
 *
 * S3C2410 USB send function
 * endpoint 2
 *
 * bushi<bushi@mizi.com>
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <asm/hardware.h>
#include <asm/dma.h>
#include <asm/system.h>
#include <asm/byteorder.h>

#include "s3c2410_usb.h"
#include "usb_ctl.h"

//#define USB_DEBUG 1

#ifdef USB_DEBUG
#define LOG(arg...) printk(__FILE__":"__FUNCTION__"(): " ##arg)
#else
#define LOG(arg...) (void)(0)
#endif

/* Enable DMA on the endpoint 2. */

static char *ep2_buf;
static int ep2_len;
static usb_callback_t ep2_callback;
#ifdef USE_USBD_DMA
static dma_addr_t ep2_dma;
static dma_addr_t ep2_curdmapos;
#else
static char *ep2_curbufpos;
#endif
static int ep2_curdmalen;
static int ep2_remain;
static int dmachn_tx;
static int tx_pktsize;
void 
ep2_dma_callback(void *buf_id, int size)
{
	LOG("\n");
#ifdef USE_USBD_DMA
	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 |= UD_ICSR1_PKTRDY;
#endif
}


/* set feature stall executing, async */
void
ep2_stall( void )
{
	LOG("\n");

	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 |= UD_ICSR1_SENDSTL; /* send stall, force stall */
}

static void
ep2_start(void)
{
#ifndef USE_USBD_DMA
    int i;
#endif

	LOG("\n");

	if (!ep2_len) {
		LOG("!ep2_len\n");
		return;
	}

	ep2_curdmalen = tx_pktsize; /* 64 */

	if (ep2_curdmalen > ep2_remain)
		ep2_curdmalen = ep2_remain;

	LOG("ep2_curdmalen = %d\n", ep2_curdmalen);
 
#ifdef USE_USBD_DMA
	UD_DMAUC1 = 0x01;
	UD_DMAFC1 = (u_char) ep2_curdmalen & UD_DMAFCx_DATA;

	backup = UD_INDEX;
	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 &= ~UD_ICSR1_CLRDT;
	UD_INDEX = backup;

	UD_DMACON1 = UD_DMACONx_DMAMDOE | UD_DMACONx_IRUN;
	s3c2410_dma_queue_buffer(dmachn_tx, NULL, ep2_curdmapos, ep2_curdmalen, 
							 DMA_BUF_WR);
#else
	for (i = 0; i < ep2_curdmalen; i++) {
		LOG("ep2_curbufpos[i] = 0x%02x\n", ep2_curbufpos[i]);
	    UD_FIFO2 = (u_char) ep2_curbufpos[i] & UD_FIFO2_DATA;
	}
	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 |= UD_ICSR1_PKTRDY;
#endif
}

static void
ep2_done(int flag)
{
	int size = ep2_len - ep2_remain;

	LOG("ep2_len = %d, ep2_remain = %d\n", ep2_len, ep2_remain);
	
	if (ep2_len) {
#ifdef USE_USBD_DMA
		pci_unmap_single(NULL, ep2_dma, ep2_len, PCI_DMA_TODEVICE);
#endif
		ep2_len = 0;
		if (ep2_callback)
			ep2_callback(flag, size);
	}
}

int
ep2_init(int chn)
{
	desc_t * pd = s3c2410_usb_get_descriptor_ptr();
	tx_pktsize = __le16_to_cpu( pd->b.ep2.wMaxPacketSize ); /* 64 */

	LOG("tx_pktsize = %d\n", tx_pktsize);

	dmachn_tx = chn;
#ifdef USE_USBD_DMA
	s3c2410_dma_flush_all(dmachn_tx);
#endif
	ep2_done(-EAGAIN);
	return 0;
}

void
ep2_reset(void)
{
	desc_t * pd = s3c2410_usb_get_descriptor_ptr();
	tx_pktsize = __le16_to_cpu( pd->b.ep2.wMaxPacketSize ); /* 64 */

	LOG("tx_pktsize = %d\n", tx_pktsize);

	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 &= ~(UD_ICSR1_SENDSTL); // write 0 to clear

#ifdef USE_USBD_DMA
	s3c2410_dma_flush_all(dmachn_tx);
#endif
	ep2_done(-EINTR);
}

void
ep2_int_hndlr(int udcsr)
{
	int status;

	UD_INDEX = UD_INDEX_EP2;
	status = UD_ICSR1;

	LOG("status = 0x%08x\n", status);

	if( status & UD_ICSR1_SENTSTL ) {
	    UD_INDEX = UD_INDEX_EP2;
	    UD_ICSR1 &= ~UD_ICSR1_SENTSTL; // clear_ep1_sent_stall;
	   return;
	}
	LOG("[%d]\n", __LINE__);
	
	if ( !(status & UD_ICSR1_PKTRDY) ) {
		LOG("[%d]\n", __LINE__);
#ifdef USE_USBD_DMA
		s3c2410_dma_flush_all(dmachn_tx);
		ep2_curdmapos += ep2_curdmalen;
#else
		ep2_curbufpos += ep2_curdmalen;
#endif
		ep2_remain -= ep2_curdmalen;

		if (ep2_remain != 0) {
			LOG("[%d]\n", __LINE__);
			ep2_start();
		} else {
			LOG("[%d]\n", __LINE__);
			ep2_done(0);
		}
	}
}

int
s3c2410_usb_send(char *buf, int len, usb_callback_t callback)
{
	int flags;

	LOG("[%d]\n", __LINE__);

	if (usbd_info.state != USB_STATE_CONFIGURED)
		return -ENODEV;

	if (ep2_len)
		return -EBUSY;

	local_irq_save(flags);
	ep2_buf = buf;
	ep2_len = len;
#ifdef USE_USBD_DMA 
	ep2_dma = pci_map_single(NULL, ep2_buf, ep2_len, PCI_DMA_TODEVICE);
	ep2_curdmapos = ep2_dma;
#else
	ep2_curbufpos = ep2_buf;
#endif
	ep2_callback = callback;
	ep2_remain = len;
	ep2_start();
	local_irq_restore(flags);

	return 0;
}


void
s3c2410_usb_send_reset(void)
{
	LOG("\n");
	ep2_reset();
}

int s3c2410_usb_xmitter_avail( void )
{
	LOG("\n");

	if (usbd_info.state != USB_STATE_CONFIGURED) {
		LOG("[%d]\n", __LINE__);
		return -ENODEV;
	}
	if (ep2_len) {
		LOG("[%d]\n", __LINE__);
		return -EBUSY;
	}
	return 0;
}


EXPORT_SYMBOL(s3c2410_usb_xmitter_avail);
EXPORT_SYMBOL(s3c2410_usb_send);
EXPORT_SYMBOL(s3c2410_usb_send_reset);
