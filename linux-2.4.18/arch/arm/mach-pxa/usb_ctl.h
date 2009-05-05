/*
 *  Copyright (C) Compaq Computer Corporation, 1998, 1999
 *  Copyright (C) Extenex Corporation 2001
 *  Copyright (C) Intrinsyc, Inc., 2002
 *
 *  usb_ctl.h
 *
 *  PRIVATE interface used to share info among components of the PXA USB
 *  core: usb_ctl, usb_ep0, usb_recv and usb_send. Clients of the USB core
 *  should use pxa_usb.h.
 *
 *  02-May-2002
 *   Frank Becker (Intrinsyc) - derived from sa1100 usb_ctl.h
 *
 */

#ifndef _USB_CTL_H
#define _USB_CTL_H

/* Interrupt mask bits and UDC enable bit */
#define UDCCR_MASK_BITS         (UDCCR_REM | UDCCR_SRM | UDCCR_UDE)

/*
 * These states correspond to those in the USB specification v1.0
 * in chapter 8, Device Framework.
 */
enum { 
	USB_STATE_NOTATTACHED	=0,
	USB_STATE_ATTACHED	=1,
	USB_STATE_POWERED	=2,
	USB_STATE_DEFAULT	=3,
	USB_STATE_ADDRESS	=4,
	USB_STATE_CONFIGURED	=5,
	USB_STATE_SUSPENDED	=6
};

struct usb_stats_t {
	 unsigned long ep0_fifo_write_failures;
	 unsigned long ep0_bytes_written;
	 unsigned long ep0_fifo_read_failures;
	 unsigned long ep0_bytes_read;
};

struct usb_info_t
{
	 char * client_name;
	 dmach_t dmach_tx, dmach_rx;
	 int state;
	 unsigned char address;
	 struct usb_stats_t stats;
};

/* in usb_ctl.c */
extern struct usb_info_t usbd_info;

/*
 * Function Prototypes
 */
enum { 
	kError		=-1,
	kEvSuspend	=0,
	kEvReset	=1,
	kEvResume	=2,
	kEvAddress	=3,
	kEvConfig	=4,
	kEvDeConfig	=5 
};
int usbctl_next_state_on_event( int event );

/* endpoint zero */
void ep0_reset(void);
void ep0_int_hndlr(void);

/* receiver */
void ep_bulk_out1_state_change_notify( int new_state );
int  ep_bulk_out1_recv(void);
int  ep_bulk_out1_init(int chn);
void ep_bulk_out1_int_hndlr(int status);
void ep_bulk_out1_reset(void);
void ep_bulk_out1_stall(void);

/* xmitter */
void ep_bulk_in1_state_change_notify( int new_state );
void ep_bulk_in1_reset(void);
int  ep_bulk_in1_init(int chn);
void ep_bulk_in1_int_hndlr(int status);
void ep_bulk_in1_stall(void);

#endif /* _USB_CTL_H */
