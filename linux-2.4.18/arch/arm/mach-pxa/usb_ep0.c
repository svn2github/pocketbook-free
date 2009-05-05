/*
 *  Copyright (C) Extenex Corporation 2001
 *  Copyright (C) Compaq Computer Corporation, 1998, 1999
 *  Copyright (C) Intrinsyc, Inc., 2002
 *
 *  PXA USB controller driver - Endpoint zero management
 *
 *  Please see:
 *    linux/Documentation/arm/SA1100/SA1100_USB 
 *  for more info.
 *
 *  02-May-2002
 *   Frank Becker (Intrinsyc) - derived from sa1100 usb_ctl.c
 * 
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/tqueue.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/irq.h>

#include "pxa_usb.h"  /* public interface */
#include "usb_ctl.h"  /* private stuff */
#include "usb_ep0.h"


// 1 == lots of trace noise,  0 = only "important' stuff
#define VERBOSITY 0

enum { true = 1, false = 0 };
typedef int bool;
#ifndef MIN
#define MIN( a, b ) ((a)<(b)?(a):(b))
#endif

#if 1 && !defined( ASSERT )
#  define ASSERT(expr) \
if(!(expr)) { \
	printk( "Assertion failed! %s,%s,%s,line=%d\n",\
#expr,__FILE__,__FUNCTION__,__LINE__); \
}
#else
#  define ASSERT(expr)
#endif

#if VERBOSITY
#define PRINTKD(fmt, args...) printk( fmt , ## args)
#else
#define PRINTKD(fmt, args...)
#endif

static EP0_state ep0_state = EP0_IDLE;

/***************************************************************************
  Prototypes
 ***************************************************************************/
/* "setup handlers" -- the main functions dispatched to by the
   .. isr. These represent the major "modes" of endpoint 0 operation */
static void sh_setup_begin(void);				/* setup begin (idle) */
static void sh_write( void );      				/* writing data */
static int  read_fifo( usb_dev_request_t * p );
static void write_fifo( void );
static void get_descriptor( usb_dev_request_t * pReq );
static void queue_and_start_write( void * p, int req, int act );

/***************************************************************************
  Inline Helpers
 ***************************************************************************/

inline int type_code_from_request( __u8 by ) { return (( by >> 4 ) & 3); }

/* print string descriptor */
static inline void psdesc( string_desc_t * p )
{
	int i;
	int nchars = ( p->bLength - 2 ) / sizeof( __u16 );
	printk( "'" );
	for( i = 0 ; i < nchars ; i++ ) {
		printk( "%c", (char) p->bString[i] );
	}
	printk( "'\n" );
}

#if VERBOSITY
/* "pcs" == "print control status" */
static inline void pcs( void )
{
	__u32 foo = UDCCS0;
	printk( "%08x: %s %s %s %s %s %s\n",
			foo,
			foo & UDCCS0_SA   ? "SA" : "",
			foo & UDCCS0_OPR  ? "OPR" : "",
			foo & UDCCS0_RNE  ? "RNE" : "",
			foo & UDCCS0_SST  ? "SST" : "",
			foo & UDCCS0_FST  ? "FST" : "",
			foo & UDCCS0_DRWF ? "DRWF" : ""
	      );
}
static inline void preq( usb_dev_request_t * pReq )
{
	static char * tnames[] = { "dev", "intf", "ep", "oth" };
	static char * rnames[] = { "std", "class", "vendor", "???" };
	char * psz;
	switch( pReq->bRequest ) {
		case GET_STATUS:        psz = "get stat"; break;
		case CLEAR_FEATURE:     psz = "clr feat"; break;
		case SET_FEATURE:       psz = "set feat"; break;
		case SET_ADDRESS:       psz = "set addr"; break;
		case GET_DESCRIPTOR:    psz = "get desc"; break;
		case SET_DESCRIPTOR:    psz = "set desc"; break;
		case GET_CONFIGURATION: psz = "get cfg"; break;
		case SET_CONFIGURATION: psz = "set cfg"; break;
		case GET_INTERFACE:     psz = "get intf"; break;
		case SET_INTERFACE:     psz = "set intf"; break;
		case SYNCH_FRAME:       psz = "synch frame"; break;
		default:                psz = "unknown"; break;
	}
	printk( "- [%s: %s req to %s. dir=%s]\n", psz,
			rnames[ (pReq->bmRequestType >> 5) & 3 ],
			tnames[ pReq->bmRequestType & 3 ],
			( pReq->bmRequestType & 0x80 ) ? "in" : "out" );
}

#else
static inline void pcs( void ){}
static inline void preq( usb_dev_request_t *x){}
#endif

/***************************************************************************
  Globals
 ***************************************************************************/
static const char pszMe[] = "usbep0: ";

/* pointer to current setup handler */
static void (*current_handler)(void) = sh_setup_begin;

/* global write struct to keep write
   ..state around across interrupts */
static struct {
	unsigned char *p;
	int bytes_left;
} wr;

/***************************************************************************
  Public Interface
 ***************************************************************************/

/* reset received from HUB (or controller just went nuts and reset by itself!)
   so udc core has been reset, track this state here  */
void ep0_reset(void)
{
	PRINTKD( "%sep0_reset\n", pszMe);
	/* reset state machine */
	current_handler = sh_setup_begin;
	wr.p = NULL;
	wr.bytes_left = 0;
	usbd_info.address=0;
}

/* handle interrupt for endpoint zero */
void ep0_int_hndlr( void )
{
	PRINTKD( "%sep0_int_hndlr\n", pszMe);
	pcs();
	(*current_handler)();
}

/***************************************************************************
  Setup Handlers
 ***************************************************************************/
/*
 * sh_setup_begin()
 * This setup handler is the "idle" state of endpoint zero. It looks for OPR
 * (OUT packet ready) to see if a setup request has been been received from the
 * host. 
 *
 */
static void sh_setup_begin( void )
{
	usb_dev_request_t req;
	int request_type;
	int n;
	__u32 cs_reg_in = UDCCS0;

	PRINTKD( "%ssh_setup_begin\n", pszMe);

	/* Be sure out packet ready, otherwise something is wrong */
	if ( (cs_reg_in & UDCCS0_OPR) == 0 ) {
		/* we can get here early...if so, we'll int again in a moment  */
		PRINTKD( "%ssetup begin: no OUT packet available. Exiting\n", pszMe );
		goto sh_sb_end;
	}

	if( ((cs_reg_in & UDCCS0_SA) == 0) && (ep0_state == EP0_IN_DATA_PHASE))
	{
		PRINTKD( "%ssetup begin: premature status\n", pszMe );

		/* premature status, reset tx fifo and go back to idle state*/
		UDCCS0 = UDCCS0_OPR | UDCCS0_FTF;

		ep0_state = EP0_IDLE;
		return;
	}

	if( (UDCCS0 & UDCCS0_RNE) == 0)
	{
		/* zero-length OUT? */
		printk( "%ssetup begin: zero-length OUT?\n", pszMe );
		goto sh_sb_end;
	}

	/* read the setup request */
	n = read_fifo( &req );
	if ( n != sizeof( req ) ) {
		printk( "%ssetup begin: fifo READ ERROR wanted %d bytes got %d. "
				" Stalling out...\n",
				pszMe, sizeof( req ), n );
		/* force stall, serviced out */
		UDCCS0 = UDCCS0_FST;
		goto sh_sb_end;
	}

	/* Is it a standard request? (not vendor or class request) */
	request_type = type_code_from_request( req.bmRequestType );
	if ( request_type != 0 ) {
		printk( "%ssetup begin: unsupported bmRequestType: %d ignored\n",
				pszMe, request_type );
		goto sh_sb_end;
	}

#if VERBOSITY
	{
		unsigned char * pdb = (unsigned char *) &req;
		PRINTKD( "%2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X ",
				pdb[0], pdb[1], pdb[2], pdb[3], pdb[4], pdb[5], pdb[6], pdb[7]
		       );
		preq( &req );
	}
#endif

	/* Handle it */
	switch( req.bRequest ) {

		case SET_ADDRESS:
			PRINTKD( "%sSET_ADDRESS handled by UDC\n", pszMe);
			break;
#if 0 /* NOT_NEEDED */

		case SET_FEATURE:
			PRINTKD( "%sSET_FEATURE handled by UDC\n", pszMe);
			break;

		case CLEAR_FEATURE:
			PRINTKD( "%sCLEAR_FEATURE handled by UDC\n", pszMe);
			break;

		case GET_CONFIGURATION:
			PRINTKD( "%sGET_CONFIGURATION handled by UDC\n", pszMe );
			break;

		case GET_STATUS:
			PRINTKD( "%s%sGET_STATUS handled by UDC\n", pszMe );
			break;

		case GET_INTERFACE:
			PRINTKD( "%sGET_INTERFACE handled by UDC\n", pszMe);
			break;

		case SYNCH_FRAME:
			PRINTKD( "%sSYNCH_FRAME handled by UDC\n", pszMe );
			break;
#endif

		case GET_DESCRIPTOR:
			PRINTKD( "%sGET_DESCRIPTOR\n", pszMe );
			get_descriptor( &req );
			break;

		case SET_INTERFACE:
			PRINTKD( "%sSET_INTERFACE TODO...\n", pszMe);
			break;

		case SET_DESCRIPTOR:
			PRINTKD( "%sSET_DESCRIPTOR TODO...\n", pszMe );
			break;

		case SET_CONFIGURATION:
			PRINTKD( "%sSET_CONFIGURATION %d\n", pszMe, req.wValue);

/*
 * FIXME: Something is not quite right here... I only ever get a 
 * de-configure from the host. Ignoring it for now, since usb
 * ethernet won't do anything unless usb is 'configured'.
 *
 */
#if 0
			switch( req.wValue)
			{
				case 0:
					/* configured */
					usbctl_next_state_on_event( kEvConfig );
					break;
				case 1:
					/* de-configured */
					usbctl_next_state_on_event( kEvDeConfig );
					break;
				default:
					PRINTKD( "%sSET_CONFIGURATION: unknown configuration value (%d)\n", pszMe, req.wValue);
					break;
			}
#endif
			break;
		default :
			printk("%sunknown request 0x%x\n", pszMe, req.bRequest);
			break;
	} /* switch( bRequest ) */

sh_sb_end:
	return;
}

/*
 * sh_write()
 * 
 * Due to UDC bugs we push everything into the fifo in one go.
 * Using interrupts just didn't work right...
 * This should be ok, since control request are small.
 */
static void sh_write()
{
	PRINTKD( "sh_write\n" );
	do
	{
	    write_fifo();
	} while( ep0_state != EP0_END_XFER);
}

/***************************************************************************
  Other Private Subroutines
 ***************************************************************************/
/*
 * queue_and_start_write()
 * data == data to send
 * req == bytes host requested
 * act == bytes we actually have
 *
 * Sets up the global "wr"-ite structure and load the outbound FIFO 
 * with data.
 *
 */
static void queue_and_start_write( void * data, int req, int act )
{
	PRINTKD( "write start: bytes requested=%d actual=%d\n", req, act);

	wr.p = (unsigned char*) data;
	wr.bytes_left = MIN( act, req );

	ep0_state = EP0_IN_DATA_PHASE;
	sh_write();

	return;
}
/*
 * write_fifo()
 * Stick bytes in the endpoint zero FIFO.
 *
 */
static void write_fifo( void )
{
	int bytes_this_time = MIN( wr.bytes_left, EP0_FIFO_SIZE );
	int bytes_written = 0;

	while( bytes_this_time-- ) {
//		PRINTKD( "%2.2X ", *wr.p );
		UDDR0 = *wr.p++;
		bytes_written++;
	}
	wr.bytes_left -= bytes_written;

	usbd_info.stats.ep0_bytes_written += bytes_written;

	if( (wr.bytes_left==0))
	{
		wr.p = NULL;  				/* be anal */

 		if(bytes_written < EP0_FIFO_SIZE)
		{
			int count;
			int udccs0;

			/* We always end the transfer with a short or zero length packet */
			ep0_state = EP0_END_XFER;
			current_handler = sh_setup_begin;

			/* Let the packet go... */
			UDCCS0 = UDCCS0_IPR;

			/* Wait until we get to status-stage, then ack.
			 *
			 * When the UDC sets the UDCCS0[OPR] bit, an interrupt
			 * is supposed to be generated (see 12.5.1 step 14ff, PXA Dev Manual).   
			 * That approach didn't work out. Usually a new SETUP command was
			 * already in the fifo. I tried many approaches but was always losing 
			 * at least some OPR interrupts. Thus the polling below...
			 */
			count = 1000;
			udccs0 = UDCCS0;
			do
			{
				if( (UDCCS0 & UDCCS0_OPR)) 
				{
					/* clear OPR, generate ack */
					UDCCS0 = UDCCS0_OPR;
					break;
				}
				count--;	
				udelay(1);
			} while( count);

			PRINTKD( "write fifo: count=%d UDCCS0=%x UDCCS0=%x\n", count, udccs0, UDCCS0);
		}
	}
	/* something goes poopy if I dont wait here ... */
	udelay(500);

	PRINTKD( "write fifo: bytes sent=%d, bytes left=%d\n", bytes_written, wr.bytes_left);
}

/*
 * read_fifo()
 * Read bytes out of FIFO and put in request.
 * Called to do the initial read of setup requests
 * from the host. Return number of bytes read.
 *
 */
static int read_fifo( usb_dev_request_t * request )
{
	int bytes_read = 0;
	unsigned char * pOut = (unsigned char*) request;

	int udccs0 = UDCCS0;

	if( (udccs0 & SETUP_READY) == SETUP_READY)
	{
		/* ok it's a setup command */
		while( UDCCS0 & UDCCS0_RNE)
		{
			if( bytes_read >= sizeof( usb_dev_request_t))
			{
				/* We've already read enought o fill usb_dev_request_t.
				 * Our tummy is full. Go barf... 
				 */
				printk( "%sread_fifo(): read failure\n", pszMe );
				usbd_info.stats.ep0_fifo_read_failures++;
				break;
			}

			*pOut++ = UDDR0;
			bytes_read++;
		}
	}
	PRINTKD( "read_fifo %d bytes\n", bytes_read );

	/* clear SA & OPR */
	UDCCS0 = SETUP_READY;

	usbd_info.stats.ep0_bytes_read += bytes_read;
	return bytes_read;
}

/*
 * get_descriptor()
 * Called from sh_setup_begin to handle data return
 * for a GET_DESCRIPTOR setup request.
 */
static void get_descriptor( usb_dev_request_t * pReq )
{
	string_desc_t * pString;
	ep_desc_t * pEndpoint = 0;

	desc_t * pDesc = pxa_usb_get_descriptor_ptr();
	int type = pReq->wValue >> 8;
	int idx  = pReq->wValue & 0xFF;

//	PRINTKD( "%sget_descriptor for %d\n", pszMe, type );
	switch( type ) {
		case USB_DESC_DEVICE:
			queue_and_start_write( &pDesc->dev,
					pReq->wLength,
					pDesc->dev.bLength );
			break;

			// return config descriptor buffer, cfg, intf, 2 ep
		case USB_DESC_CONFIG:
			queue_and_start_write( &pDesc->b,
					pReq->wLength,
					sizeof( struct cdb ) );
			break;

			// not quite right, since doesn't do language code checking
		case USB_DESC_STRING:
			pString = pxa_usb_get_string_descriptor( idx );
			if ( pString ) {
				if ( idx != 0 ) {  // if not language index
					printk( "%sReturn string %d: ", pszMe, idx );
					psdesc( pString );
				}
				queue_and_start_write( pString,
						pReq->wLength,
						pString->bLength );
			}
			else {
				printk("%sunkown string index %d Stall.\n", pszMe, idx );
			}
			break;

		case USB_DESC_INTERFACE:
			if ( idx == pDesc->b.intf.bInterfaceNumber ) {
				queue_and_start_write( &pDesc->b.intf,
						pReq->wLength,
						pDesc->b.intf.bLength );
			}
			break;

		case USB_DESC_ENDPOINT: /* correct? 21Feb01ww */
			if ( idx == 1 )
				pEndpoint = &pDesc->b.ep1; //[BULK_IN1];
			else if ( idx == 2 )
				pEndpoint = &pDesc->b.ep2; //[BULK_OUT1];
			else
				pEndpoint = NULL;
			if ( pEndpoint ) {
				queue_and_start_write( pEndpoint,
						pReq->wLength,
						pEndpoint->bLength );
			} else {
				printk("%sunkown endpoint index %d Stall.\n", pszMe, idx );
			}
			break;


		default :
			printk("%sunknown descriptor type %d. Stall.\n", pszMe, type );
			break;

	}
}

/* end usb_ep0.c - who needs this comment? */
