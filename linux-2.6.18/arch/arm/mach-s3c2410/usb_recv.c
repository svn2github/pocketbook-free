/*
 * usb_recv.c
 * 
 * S3C2410 USB receive function 
 * endpoint 1
 *
 * bushi <bushi@mizi.com>
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <asm/dma.h>
#include <asm/system.h>
#include <linux/delay.h>

#include "s3c2410_usb.h"
#include "usb_ctl.h"

//#define USB_DEBUG 1

#ifdef USB_DEBUG
#define LOG(arg...) printk(__FILE__":"__FUNCTION__"(): " ##arg)
#else
#define LOG(arg...) (void)(0)
#endif

static unsigned char *ep1_buf;
static unsigned int ep1_len;
static usb_callback_t ep1_callback;
static unsigned char *ep1_curdmabuf;
static dma_addr_t ep1_curdmapos;
static unsigned int ep1_curdmalen;
static unsigned int ep1_remain;
static unsigned int dmachn_rx;
static unsigned int rx_pktsize;

void 
ep1_dma_callback(void *buf_id, int size)
{
	LOG("\n");

#ifdef USE_USBD_DMA
	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 |= UD_OCSR1_PKTRDY;
#endif
}

static void
ep1_start(void)
{
	LOG("\n");
#if 0
	s3c2410_dma_flush_all(dmachn_rx);
	if (!ep1_curdmalen) {
	  	ep1_curdmalen = rx_pktsize;
		if (ep1_curdmalen > ep1_remain)
			ep1_curdmalen = ep1_remain;
		ep1_curdmapos = pci_map_single(NULL, ep1_curdmabuf, ep1_curdmalen,
					       PCI_DMA_FROMDEVICE);
	}
	s3c2410_dma_queue_buffer(dmachn_rx, NULL, ep1_curdmapos, ep1_curdmalen, DMA_BUF_RD);
#endif
}

static void
ep1_done(int flag)
{
	int size = ep1_len - ep1_remain;

	LOG(" start\n");

	if (!ep1_len)
		return;
#if 0
	if (ep1_curdmalen)
		pci_unmap_single(NULL, ep1_curdmapos, ep1_curdmalen,
				 PCI_DMA_FROMDEVICE);
#endif
	ep1_len = ep1_curdmalen = 0;
	if (ep1_callback) {
		ep1_callback(flag, size);
	}
	LOG(" end\n");
}


void
ep1_stall( void )
{
	LOG(" start\n");

	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 |= UD_OCSR1_SENDSTL; /* send stall, force stall */
	LOG(" end\n");
}

int
ep1_init(int chn)
{
	desc_t * pd = s3c2410_usb_get_descriptor_ptr();

	LOG(" start\n");

	rx_pktsize = __le16_to_cpu( pd->b.ep1.wMaxPacketSize );

	LOG("rx_pktsize = %d\n", rx_pktsize);

	dmachn_rx = chn;
#if 0
	s3c2410_dma_flush_all(dmachn_rx);
#endif
	ep1_done(-EAGAIN);
	LOG(" end\n");
	return 0;
}

void
ep1_reset(void)
{
	desc_t * pd = s3c2410_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu( pd->b.ep1.wMaxPacketSize );
#if 0
	s3c2410_dma_flush_all(dmachn_rx);
#endif

	LOG("rx_pktsize = %d\n", rx_pktsize);

	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 &= ~(UD_OCSR1_SENDSTL);

	ep1_done(-EINTR);
}

void
ep1_int_hndlr(int udcsr)
{
	dma_addr_t dma_addr;
	unsigned int len;
	int status;
	int recv_cnt;

	UD_INDEX = UD_INDEX_EP1;
	status = UD_OCSR1;

	LOG("0x%02x\n", UD_OCSR1);
	LOG("0x%02x\n", UD_OCSR2);
	LOG("0x%02x\n", UD_ICSR1);
	LOG("0x%02x\n", UD_ICSR2);

	if (status & UD_OCSR1_SENTSTL) {
	    UD_INDEX = UD_INDEX_EP1;	
	    UD_OCSR1 &= ~UD_OCSR1_SENTSTL;
		LOG("UD_OCSR1_SENTSTL\n");
	     return;//?? !!
	}

	if (status & UD_OCSR1_PKTRDY) { 
		LOG("ep1_len=%x\n",ep1_len);
	    
		if (!ep1_len) {
			printk("usb_recv: RPC for non-existent buffer\n");
			UD_INDEX = UD_INDEX_EP1;
			UD_OCSR1 |= UD_OCSR1_FFLUSH;
			//udelay(20);
			UD_INDEX = UD_INDEX_EP1;
			UD_OCSR1 &= ~UD_OCSR1_PKTRDY;
			return;
		}
		UD_INDEX = UD_INDEX_EP1;
		recv_cnt = ((UD_OFCNTH << 8) | UD_OFCNTL) &0xffff;
		LOG("recv_count = %d\n", recv_cnt);

		for (len = 0; len < recv_cnt; len++) {
		    ep1_buf[len] = (u_char) UD_FIFO1;
		    LOG("ep1_buf[%d]=%x\n", len, ep1_buf[len]);
		}


		ep1_remain -= recv_cnt;
		ep1_done(0);
		UD_INDEX = UD_INDEX_EP1;
		UD_OCSR1 &= ~UD_OCSR1_PKTRDY;


	}

}

int
s3c2410_usb_recv(char *buf, int len, usb_callback_t callback)
{
	int flags;

	LOG("\n");

	if (ep1_len)
		return -EBUSY;

	local_irq_save(flags);
	ep1_buf = buf;
	ep1_len = len;
	ep1_callback = callback;
	ep1_remain = len;
#if 0
	ep1_curdmabuf = buf;
	ep1_curdmalen = 0;
#endif
	ep1_start();
	local_irq_restore(flags);

	return 0;
}

EXPORT_SYMBOL(s3c2410_usb_recv);

void
s3c2410_usb_recv_reset(void)
{
	LOG("\n");
	ep1_reset();
}

EXPORT_SYMBOL(s3c2410_usb_recv_reset);

void
s3c2410_usb_recv_stall(void)
{
	LOG("\n");
	ep1_stall();
}

EXPORT_SYMBOL(s3c2410_usb_recv_stall);

