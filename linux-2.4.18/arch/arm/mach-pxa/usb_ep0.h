/*
 *  Copyright (C) Intrinsyc, Inc., 2002
 *
 *  usb_ep0.h - PXA USB controller driver.
 *              Endpoint zero management
 *
 *  Please see:
 *    linux/Documentation/arm/SA1100/SA1100_USB
 *  for details.
 *
 *  02-May-2002
 *   Frank Becker (Intrinsyc) - 
 * 
 */

#ifndef __USB_EP0_H
#define __USB_EP0_H

#define EP0_FIFO_SIZE	16
#define SETUP_READY (UDCCS0_SA | UDCCS0_OPR)

/*================================================
 * USB Protocol Stuff
 */

/* Request Codes   */
enum { 
	GET_STATUS		=0,
	CLEAR_FEATURE		=1,
	/* reserved		=2 */
	SET_FEATURE		=3,
	/* reserved		=4 */
	SET_ADDRESS		=5,        
	GET_DESCRIPTOR		=6,
	SET_DESCRIPTOR		=7,
	GET_CONFIGURATION	=8,
	SET_CONFIGURATION	=9,
	GET_INTERFACE		=10,
	SET_INTERFACE		=11,
	SYNCH_FRAME		=12
};

typedef enum {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_END_XFER,
	EP0_OUT_DATA_PHASE
} EP0_state;

/* USB Device Requests */
typedef struct
{
	__u8 bmRequestType;
	__u8 bRequest;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;
} usb_dev_request_t  __attribute__ ((packed));

/* Data extraction from usb_request_t fields */
enum { 
	kTargetDevice	=0,
	kTargetInterface=1,
	kTargetEndpoint	=2 
};
#endif
