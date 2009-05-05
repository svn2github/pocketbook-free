/*
 *  This program is derived from Extenex Corporation's SA-1100 usb
 *  controller core driver by MIZI.
 * 
 *  usb_ctl.c
 *
 *  S3C2410 USB controller core driver.
 *
 *  This file provides interrupt routing and overall coordination
 *  of the five endpoints.
 *  
 *  Seungbum Lim <shawn@mizi.com>
 */

/*
 * ep0 - register
 * ep2~4 - dual port async. RAM (interrupt or DMA)
 *
 * config:
 *  ep0.
 *  ep2 : input - DMA_CH0 ?
 *  ep1 : output - DMA_CH3 ?
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/tqueue.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include "s3c2410_usb.h"
#include "usb_ctl.h"

//////////////////////////////////////////////////////////////////////////////
// Prototypes
//////////////////////////////////////////////////////////////////////////////
int usbctl_next_state_on_event( int event );
static void udc_int_hndlr(int, void *, struct pt_regs *);
static void initialize_descriptors( void );
void ChangeUPllValue(int mdiv, int pdiv, int sdiv);
void reset_usbd(void);
void reconfig_usbd(void);
//#define USB_DEBUG 1

#ifdef USB_DEBUG
#define LOG(arg...) printk(__FILE__":"__FUNCTION__"(): " ##arg)
#else
#define LOG(arg...) (void)(0)
#endif

#if CONFIG_PROC_FS
#define PROC_NODE_NAME "usb"
static int usbctl_read_proc(char *page, char **start, off_t off,
							int count, int *eof, void *data);
#endif

//////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////
static const char pszMe[] = "usbctl: ";
struct usb_info_t usbd_info;  /* global to ep0, usb_recv, usb_send */
//static int cnt=0;


/* device descriptors */
static desc_t desc;

#define MAX_STRING_DESC 8
static string_desc_t * string_desc_array[ MAX_STRING_DESC ];
static string_desc_t sd_zero;  /* special sd_zero holds language codes */

// called when configured
static usb_notify_t configured_callback = NULL;

enum {	kStateZombie  = 0,  kStateZombieSuspend  = 1,
		kStateDefault = 2,  kStateDefaultSuspend = 3,
		kStateAddr    = 4,  kStateAddrSuspend    = 5,
		kStateConfig  = 6,  kStateConfigSuspend  = 7
};

static int device_state_machine[8][6] = {
//                suspend               reset          resume     adddr config deconfig
/* zombie */  {  kStateZombieSuspend,  kStateDefault, kError,    kError, kError, kError },
/* zom sus */ {  kError, kStateDefault, kStateZombie, kError, kError, kError },
/* default */ {  kStateDefaultSuspend, kError, kStateDefault, kStateAddr, kError, kError },
/* def sus */ {  kError, kStateDefault, kStateDefault, kError, kError, kError },
/* addr */    {  kStateAddrSuspend, kStateDefault, kError, kError, kStateConfig, kError },
/* addr sus */{  kError, kStateDefault, kStateAddr, kError, kError, kError },
/* config */  {  kStateConfigSuspend, kStateDefault, kError, kError, kError, kStateAddr },
/* cfg sus */ {  kError, kStateDefault, kStateConfig, kError, kError, kError }
};

/* "device state" is the usb device framework state, as opposed to the
   "state machine state" which is whatever the driver needs and is much
   more fine grained
*/
static int sm_state_to_device_state[8] =
//  zombie           zom suspend          default            default sus
{ USB_STATE_POWERED, USB_STATE_SUSPENDED, USB_STATE_DEFAULT, USB_STATE_SUSPENDED,
// addr              addr sus             config                config sus
  USB_STATE_ADDRESS, USB_STATE_SUSPENDED, USB_STATE_CONFIGURED, USB_STATE_SUSPENDED
};

static char * state_names[8] =
{ "zombie", "zombie suspended", "default", "default suspended",
  "address", "address suspended", "configured", "config suspended"
};

static char * event_names[6] =
{ "suspend", "reset", "resume",
  "address assigned", "configure", "de-configure"
};

static char * device_state_names[] =
{ "not attached", "attached", "powered", "default",
  "address", "configured", "suspended" };

static int sm_state = kStateZombie;

//////////////////////////////////////////////////////////////////////////////
// Reset Fucntions
//////////////////////////////////////////////////////////////////////////////

void reset_usbd(void)
{
    int i;

    UD_PWR = UD_PWR_DEFAULT | UD_PWR_RESET; /* UD_PWR default value, MCU_RESET */
    UD_PWR;
    UD_PWR = UD_PWR_DEFAULT;

	LOG("UD_PWR = 0x%08x\n", UD_PWR);

    for(i = 0; i< 0x100; i++) ;
}


void reconfig_usbd(void)
{

	LOG("\n");

    /* sec like, shawn */
    ep0_state = EP0_STATE_IDLE;
    set_configuration = 1;
    set_interface = 1;
    device_status = 0;
    ep0_status = 0;
    ep_bulk_in_status = 0;
    ep_bulk_out_status = 0;
    
        UD_PWR = UD_PWR_DEFAULT;
        /* EP0 */
	UD_INDEX = UD_INDEX_EP0;       
	UD_MAXP = UD_MAXP_8; // 8 byte
	UD_INDEX = UD_INDEX_EP0;
	UD_ICSR1 = EP0_CSR_SOPKTRDY | EP0_CSR_SSE;
        /* EP2 */
	UD_INDEX = UD_INDEX_EP2;           
	UD_MAXP = UD_MAXP_64; // 64 byte
	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR1 = UD_ICSR1_FFLUSH | UD_ICSR1_CLRDT; // fifo flush, data toggle
	UD_INDEX = UD_INDEX_EP2;
	UD_ICSR2 = UD_ICSR2_MODEIN | UD_ICSR2_DMAIEN; // input mode, IN_PKT_RDY dis 
#ifdef USE_USBD_DMA
	UD_ICSR2 &= ~UD_ICSR2_DMAIEN;
#endif

	/* EP1 */
	UD_INDEX = UD_INDEX_EP1;            
	UD_MAXP = UD_MAXP_64; // 64 byte
	UD_INDEX = UD_INDEX_EP1;
	UD_ICSR1 = UD_ICSR1_FFLUSH | UD_ICSR1_CLRDT; // fifo flush, data toggle
	UD_INDEX = UD_INDEX_EP1;
	UD_ICSR2 = 0x0; // output mode
	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 = UD_OCSR1_FFLUSH | UD_OCSR1_CLRDT; // fifo flush
	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR2 = UD_OCSR2_DMAIEN; // OUT_PKT_RDY interrupt disable
#ifdef USE_USBD_DMA
	UD_OCSR2 &= ~UD_OCSR2_DMAIEN; // OUT_PKT_RDY interrupt disable
#endif

	UD_INTE = UD_INTE_EP0 | UD_INTE_EP2 | UD_INTE_EP1;
	UD_USBINTE = UD_USBINTE_RESET | UD_USBINTE_SUSPND;

        initialize_descriptors();
  	bINTCTL(oINTMSK) &= ~(INT_USBD);
}
    

static void
udc_int_hndlr(int irq, void *dev_id, struct pt_regs *regs)
{
  	__u8 saveIdx = UD_INDEX;
    	__u8 usb_status = UD_USBINT;
  	__u8 usbd_status = UD_INT;
	static int sb_debug_cnt = 1;	

	LOG("usb_status = 0x%02x, usbd_status = 0x%02x\n",
			usb_status, usbd_status);

	if ( usb_status & UD_USBINT_RESET ) {

	    LOG("\n[%d]RESET interrupt\n",sb_debug_cnt++);
	    if( usbctl_next_state_on_event(kEvReset) != kError ) {
		LOG("%s Resetting\n",pszMe);

		ep0_reset();
		ep1_reset();/* output */
		ep2_reset();/* input */

	    }

	   // reset_usbd();
	    reconfig_usbd();
	    UD_USBINT = UD_USBINT_RESET; //RESET_INT should be cleared after reconfig_usbd().- by samsung src
	    ep0_state = EP0_STATE_IDLE;

	}


	/* RESume Interrupt Request */
	if ( usb_status & UD_USBINT_RESUM ) {
 		LOG("[%d]RESUME interrupt\n", sb_debug_cnt++);
		UD_USBINT = UD_USBINT_RESUM;/* clear */
		usbctl_next_state_on_event( kEvResume );

	}


	/* SUSpend Interrupt Request */
	if ( usb_status & UD_USBINT_SUSPND ) { 
		LOG("[%d]SUSPEND interrupt\n", sb_debug_cnt++);
		UD_USBINT = UD_USBINT_SUSPND; /* clear */
		usbctl_next_state_on_event( kEvSuspend );

	}

	if ( usbd_status & UD_INT_EP0 ) {
		LOG("\n[%d]EP0 interrupt\n",sb_debug_cnt++);
		UD_INT = UD_INT_EP0; /* clear */
		ep0_int_hndlr();

	}

         /* output */
	if ( usbd_status & UD_INT_EP1 ) { 
		LOG("[%d]EP1 interrupt\n", sb_debug_cnt++);
		UD_INT = UD_INT_EP1;/* clear */
		ep1_int_hndlr(usbd_status);

	}
        /* input */
	if ( usbd_status & UD_INT_EP2 ) {
		LOG("[%d]EP2 interrupt\n", sb_debug_cnt++); 
		UD_INT = UD_INT_EP2; /* clear */
		ep2_int_hndlr(usbd_status);
		
	}

	if(usbd_status & UD_INT_EP3) UD_INT = UD_INT_EP3;
	if(usbd_status & UD_INT_EP4) UD_INT = UD_INT_EP4;
	
  	Clear_pending(INT_USBD);
	UD_INDEX= saveIdx;
	
}



//////////////////////////////////////////////////////////////////////////////
// Public Interface
//////////////////////////////////////////////////////////////////////////////

/* Open S3C2410 usb core on behalf of a client, but don't start running */

int
s3c2410_usb_open( const char * client )
{

	LOG("\n");

	 if ( usbd_info.client_name != NULL )
		  return -EBUSY;

	 usbd_info.client_name = (char*) client;
	 memset(&usbd_info.stats, 0, sizeof(struct usb_stats_t));
	 memset(string_desc_array, 0, sizeof(string_desc_array));

	 /* hack to start in zombie suspended state */
#if 0
	 sm_state = kStateZombieSuspend;
	 usbd_info.state = USB_STATE_SUSPENDED;
#endif

	 /* create descriptors for enumeration */
	 initialize_descriptors();

	 printk( "%sOpened for %s\n", pszMe, client );
	 return 0;
}

/* Start running. Must have called usb_open (above) first */
int
s3c2410_usb_start( void )
{
	unsigned long tmp;

	LOG("\n");

	 if ( usbd_info.client_name == NULL ) {
		  printk( "%s%s - no client registered\n",
				  pszMe, __FUNCTION__ );
		  return -EPERM;
	 }

	 /* start UDC internal machinery running */
	 udelay( 100 );

	 /* clear stall - receiver seems to start stalled? */
	UD_INDEX = UD_INDEX_EP2; // EP2 input 
	tmp = UD_ICSR1; 
	tmp &= ~(UD_ICSR1_SENTSTL | UD_ICSR1_FFLUSH | UD_ICSR1_UNDRUN);
        tmp &= ~(UD_ICSR1_PKTRDY | UD_ICSR1_SENDSTL);
	UD_ICSR1 = tmp;

	UD_INDEX = UD_INDEX_EP1; // EP1 output
	tmp = UD_OCSR1;
	tmp &= ~(UD_OCSR1_SENTSTL | UD_OCSR1_FFLUSH | UD_OCSR1_OVRRUN);
	tmp &= ~(UD_OCSR1_PKTRDY | UD_OCSR1_SENDSTL);
	UD_OCSR1 = tmp;


	/* flush DMA and fire through some -EAGAINs */
	ep2_init( usbd_info.dmach_tx );
	ep1_init( usbd_info.dmach_rx );


	/* clear all top-level sources */
	UD_INT = UD_INT_EP0 | UD_INT_EP1 | UD_INT_EP2;
	UD_USBINT = UD_USBINT_RESET | UD_USBINT_RESUM | UD_USBINT_SUSPND;

	printk( "%sStarted for %s\n", pszMe, usbd_info.client_name );
	return 0;
}

/* Stop USB core from running */
int
s3c2410_usb_stop( void )
{
	LOG("name=%s\n", usbd_info.client_name ? usbd_info.client_name : "NULL");
	 if ( usbd_info.client_name == NULL ) {
		  printk( "%s%s - no client registered\n",
				  pszMe, __FUNCTION__ );
		  return -EPERM;
	 }
#if 0
	 /* It may be default value of S3C2410 USBD and makes only RESET be enalble*/
	 UD_INTM = 0x13f;
#endif
	 ep1_reset(); 
	 ep2_reset();
	 printk( "%sStopped \n", pszMe );
	 return 0;
}

/* Tell S3C2410 core client is through using it */
int
s3c2410_usb_close( void )
{
	 if ( usbd_info.client_name == NULL ) {
		  printk( "%s%s - no client registered\n",
				  pszMe, __FUNCTION__ );
		  return -EPERM;
	 }
	 usbd_info.client_name = NULL;
	 return 0;
}

/* set a proc to be called when device is configured */
usb_notify_t s3c2410_set_configured_callback( usb_notify_t func )
{
	 usb_notify_t retval = configured_callback;

	LOG("\n");

	 configured_callback = func;
	 return retval;
}

/*====================================================
 * Descriptor Manipulation.
 * Use these between open() and start() above to setup
 * the descriptors for your device.
 *
 */

/* get pointer to static default descriptor */
desc_t *
s3c2410_usb_get_descriptor_ptr( void ) { return &desc; }

/* optional: set a string descriptor */
int
s3c2410_usb_set_string_descriptor( int i, string_desc_t * p )
{
	 int retval;

	 LOG("\n");

	 if ( i < MAX_STRING_DESC ) {
		  string_desc_array[i] = p;
		  retval = 0;
	 } else {
		  retval = -EINVAL;
	 }
	 return retval;
}

/* optional: get a previously set string descriptor */
string_desc_t *
s3c2410_usb_get_string_descriptor( int i )
{
	LOG("\n");

	 return ( i < MAX_STRING_DESC )
		    ? string_desc_array[i]
		    : NULL;
}


/* optional: kmalloc and unicode up a string descriptor */
string_desc_t *
s3c2410_usb_kmalloc_string_descriptor( const char * p )
{
	 string_desc_t * pResult = NULL;

	 LOG("\n");

	 if ( p ) {
		  int len = strlen( p );
		  int uni_len = len * sizeof( __u16 );
		  pResult = (string_desc_t*) kmalloc( uni_len + 2, GFP_KERNEL ); /* ugh! */
		  if ( pResult != NULL ) {
			   int i;
			   pResult->bLength = uni_len + 2;
			   pResult->bDescriptorType = USB_DESC_STRING;
			   for( i = 0; i < len ; i++ ) {
					pResult->bString[i] = make_word( (__u16) p[i] );
			   }
		  }
	 }
	 return pResult;
}

//////////////////////////////////////////////////////////////////////////////
// Exports to rest of driver
//////////////////////////////////////////////////////////////////////////////

/* called by the int handler here and the two endpoint files when interesting
   .."events" happen */

int
usbctl_next_state_on_event( int event )
{
	int next_state = device_state_machine[ sm_state ][ event ];

	LOG("\n");

	if ( next_state != kError )
	{
		int next_device_state = sm_state_to_device_state[ next_state ];
		printk( "%s%s --> [%s] --> %s. Device in %s state.\n",
				pszMe, state_names[ sm_state ], event_names[ event ],
				state_names[ next_state ], device_state_names[ next_device_state ] );

		sm_state = next_state;
		if ( usbd_info.state != next_device_state )
		{
			if ( configured_callback != NULL
				 &&
				 next_device_state == USB_STATE_CONFIGURED
				 &&
				 usbd_info.state != USB_STATE_SUSPENDED
			   ) {
			  configured_callback();
			}
			usbd_info.state = next_device_state;
		}
	}
#if 0
	else
		printk( "%s%s --> [%s] --> ??? is an error.\n",
				pszMe, state_names[ sm_state ], event_names[ event ] );
#endif
	return next_state;
}

//////////////////////////////////////////////////////////////////////////////
// Private Helpers
//////////////////////////////////////////////////////////////////////////////

//* setup default descriptors */

static void
initialize_descriptors(void)
{
	LOG("\n");

	desc.dev.bLength               = sizeof( device_desc_t );
	desc.dev.bDescriptorType       = USB_DESC_DEVICE;
	desc.dev.bcdUSB                = 0x100; /* 1.1 */
	desc.dev.bDeviceClass          = 0xFF;	/* vendor specific */
	desc.dev.bDeviceSubClass       = 0x0;
	desc.dev.bDeviceProtocol       = 0x0;
	desc.dev.bMaxPacketSize0       = 0x8;	/* ep0 max fifo size in s3c2410*/
	desc.dev.idVendor              = 0x49f; /* vendor ID undefined */
	desc.dev.idProduct             = 0x505a; /* product */
	desc.dev.bcdDevice             = 0x01 ; /* vendor assigned device release num */
	desc.dev.iManufacturer         = 0;	/* index of manufacturer string */
	desc.dev.iProduct              = 0; /* index of product description string */
	desc.dev.iSerialNumber         = 0;	/* index of string holding product s/n */
	desc.dev.bNumConfigurations    = 0x1;

	desc.b.cfg.bLength             = sizeof( config_desc_t );
	desc.b.cfg.bDescriptorType     = USB_DESC_CONFIG;
	desc.b.cfg.wTotalLength        = make_word_c( sizeof(struct cdb) );
	desc.b.cfg.bNumInterfaces      = 1;
	desc.b.cfg.bConfigurationValue = 1;
	desc.b.cfg.iConfiguration      = 0;
	desc.b.cfg.bmAttributes        = USB_CONFIG_BUSPOWERED;
	desc.b.cfg.MaxPower            = USB_POWER( 500 );

	desc.b.intf.bLength            = sizeof( intf_desc_t );
	desc.b.intf.bDescriptorType    = USB_DESC_INTERFACE;
	desc.b.intf.bInterfaceNumber   = 0x0; /* unique intf index*/
	desc.b.intf.bAlternateSetting  = 0x0;
	desc.b.intf.bNumEndpoints      = 2; /* endpoint number excluding ep0 */
	desc.b.intf.bInterfaceClass    = 0xFF; /* vendor specific */
	desc.b.intf.bInterfaceSubClass = 0x0;
	desc.b.intf.bInterfaceProtocol = 0x0;
	desc.b.intf.iInterface         = 0x0;

	desc.b.ep1.bLength             = sizeof( ep_desc_t );
        desc.b.ep1.bDescriptorType     = USB_DESC_ENDPOINT;
        desc.b.ep1.bEndpointAddress    = USB_EP_ADDRESS( 1, USB_OUT );
        desc.b.ep1.bmAttributes        = USB_EP_BULK;
        desc.b.ep1.wMaxPacketSize      = make_word_c( 64 );
        desc.b.ep1.bInterval	       = 0x0;		
	
	desc.b.ep2.bLength             = sizeof( ep_desc_t );
	desc.b.ep2.bDescriptorType     = USB_DESC_ENDPOINT;
	desc.b.ep2.bEndpointAddress    = USB_EP_ADDRESS( 2, USB_IN );
	desc.b.ep2.bmAttributes        = USB_EP_BULK;
	desc.b.ep2.wMaxPacketSize      = make_word_c( 64 );
	desc.b.ep2.bInterval           = 0x0;

	/* set language */
	/* See: http://www.usb.org/developers/data/USB_LANGIDs.pdf */
	sd_zero.bDescriptorType = USB_DESC_STRING;
	sd_zero.bLength         = sizeof( string_desc_t );
	sd_zero.bString[0]      = make_word_c( 0x409 ); /* American English */
	s3c2410_usb_set_string_descriptor( 0, &sd_zero );
}



//////////////////////////////////////////////////////////////////////////////
// Proc Filesystem Support
//////////////////////////////////////////////////////////////////////////////

#if CONFIG_PROC_FS
#define SAY( fmt, args... )  p += sprintf(p, fmt, ## args )
#define SAYV(  num )         p += sprintf(p, num_fmt, "Value", num )
#define SAYC( label, yn )    p += sprintf(p, yn_fmt, label, yn )
#define SAYS( label, v )     p += sprintf(p, cnt_fmt, label, v )

static int usbctl_read_proc(char *page, char **start, off_t off,
			    int count, int *eof, void *data)
{
	const char * num_fmt   = "%25.25s: %8.8lX\n";
	const char * cnt_fmt   = "%25.25s: %lu\n";
	const char * yn_fmt    = "%25.25s: %s\n";
	const char * yes       = "YES";
	const char * no        = "NO";
	const char * mask      = "MASK";
	const char * enable    = "ENABLE";
	unsigned long v;
	unsigned long backup;
	char * p = page;
	int len;

	SAY( "S3C2410 USB Controller Core\n" );
	SAY( "USB state: %s (%s) %d\n",
		device_state_names[ sm_state_to_device_state[ sm_state ] ],
		state_names[ sm_state ],
		sm_state );

	SAYS( "ep0 bytes read", usbd_info.stats.ep0_bytes_read );
	SAYS( "ep0 bytes written", usbd_info.stats.ep0_bytes_written );
	SAYS( "ep0 FIFO read failures", usbd_info.stats.ep0_fifo_write_failures );
	SAYS( "ep0 FIFO write failures", usbd_info.stats.ep0_fifo_write_failures );
	 
	v = UD_FIFO0;
	SAY( "%25.25s: 0x%8.8lX - %ld\n", "EP_FIFO0", v, v );

	 
	SAY( "\n" );

	v = UD_FUNC;
	SAY( "%25.25s: 0x%8.8lX - %ld\n", "Address Register", v, v );

	backup = UD_INDEX;

	UD_INDEX = UD_INDEX_EP0;
	v = UD_MAXP;
	SAY( "%25.25s: %ld (%8.8lX)\n", "EP0 size(EP0)", v, v );
	 
	UD_INDEX = UD_INDEX_EP2;
	v = UD_MAXP;
	SAY( "%25.25s: %ld (%8.8lX)\n", "IN  max packet size(EP2)", v, v );
	UD_INDEX = UD_INDEX_EP1;
	v = UD_MAXP;
	SAY( "%25.25s: %ld (%8.8lX)\n", "OUT max packet size(EP1)", v, v );

	UD_INDEX = backup;


	v = UD_PWR;
	SAY( "\nUDC POWER Management Register\n" );
	SAYV( v );
	SAYC( "ISO Update(R)",    ( v & UD_PWR_ISOUP )     ? yes : no );
	SAYC( "USB Reset(R)",   ( v & UD_PWR_RESET ) ? yes : no );
	SAYC( "MCU Resume(RW)",  ( v & UD_PWR_RESUME  ) ? yes : no );
	SAYC( "Suspend Mode(R)",( v & UD_PWR_SUSPND ) ? yes : no );
	SAYC( "Suspend Mode enable bit(RW)",  ( v & UD_PWR_ENSUSPND ) ? yes : no );
		 
	 
	v = UD_INT;
	SAY( "\nUDC Interrupt Register\n" );
	SAYV( v );
	SAYC( "EP4 pending",        ( v & UD_INT_EP4 )     ? yes : no );
	SAYC( "EP3 pending",        ( v & UD_INT_EP3 )     ? yes : no );
	SAYC( "EP2 pending",        ( v & UD_INT_EP2 )     ? yes : no );
	SAYC( "EP1 pending",        ( v & UD_INT_EP1 )     ? yes : no );
	SAYC( "EP0 pending",        ( v & UD_INT_EP0 )     ? yes : no );

	v = UD_USBINT;
	SAY( "\nUSB Interrupt Register\n" );
	SAYV( v );
	SAYC( "Reset",        ( v & UD_USBINT_RESET )     ? yes : no );
	SAYC( "Resume",        ( v & UD_USBINT_RESUM )     ? yes : no );
	SAYC( "Suspend",        ( v & UD_USBINT_SUSPND )     ? yes : no );

	v = UD_INTE;
	SAY( "\nUDC Interrupt Enable Register\n" );
	SAYV( v );
	SAYC( "EP4",                !( v & UD_INTE_EP4 )    ? mask : enable );
	SAYC( "EP3",                !( v & UD_INTE_EP3 )    ? mask : enable );
	SAYC( "EP2",                !( v & UD_INTE_EP2 )    ? mask : enable );
	SAYC( "EP1",                !( v & UD_INTE_EP1 )    ? mask : enable );
	SAYC( "EP0",                !( v & UD_INTE_EP0 )    ? mask : enable );

	v = UD_USBINTE;
	SAY( "\nUSB Interrupt Enable Register\n" );
	SAYV( v );
	SAYC( "Reset",              !( v & UD_USBINTE_RESET )    ? mask : enable );
	SAYC( "Suspend",            !( v & UD_USBINTE_SUSPND )  ? mask : enable );

	len = ( p - page ) - off;
	if ( len < 0 )
	     len = 0;
	*eof = ( len <=count ) ? 1 : 0;
	*start = page + off;
	return len;
}
#endif  /* CONFIG_PROC_FS */

#if defined(CONFIG_PM) && defined(CONFIG_MIZI)
void usbctl_suspend(void)
{
	/* TODO: FIXME: */
}

void usbctl_resume(void)
{
	/* TODO: FIXME: */
}
#endif


//////////////////////////////////////////////////////////////////////////////
// Module Initialization and Shutdown
//////////////////////////////////////////////////////////////////////////////
/*
 * usbctl_init()
 * Module load time. Allocate dma and interrupt resources. Setup /proc fs
 * entry. Leave UDC disabled.
 */
int __init usbctl_init( void )
{
	int retval = 0;

#if CONFIG_MAX_ROOT_PORTS > 1
	printk("check your kernel config.\n");
	return -ENODEV;
#endif

	memset( &usbd_info, 0, sizeof( usbd_info ) );

	usbd_info.dmach_tx = DMA_CH3; // ep1
	usbd_info.dmach_rx = DMA_CH0; // ep2

#if CONFIG_PROC_FS
	create_proc_read_entry ( PROC_NODE_NAME, 0, NULL, usbctl_read_proc, NULL);
#endif

#ifdef USE_USBD_DMA
	lsdkjflsdkjsdlkjflksdjf
	/* setup dma */
#if 1
	retval = s3c2410_request_dma("USB", DMA_CH0, 
								 ep1_dma_callback, NULL);
	if (retval) {
		printk("%sunable to register for dma rc=%d\n", pszMe, retval);
		goto err_dma;
	}

	retval = s3c2410_request_dma("USB", DMA_CH2, 
								 NULL, ep2_dma_callback);
	if (retval) {
		printk("%sunable to register for dma rc=%d\n", pszMe, retval);
		goto err_dma;
	}
#else
	retval = s3c2410_request_dma("USB", DMA_CH0, 
								 ep1_dma_callback, ep2_dma_callback);
	if (retval) {
		printk("%sunable to register for dma rc=%d\n", pszMe, retval);
		goto err_dma;
	}
#endif
#endif // USE_USBD_DMA
	
	/* now allocate the IRQ. */
	retval = request_irq(IRQ_USBD, udc_int_hndlr, SA_INTERRUPT,
			  "S3C2410 USB core", NULL);
	if (retval) {
		printk("%sCouldn't request USB irq rc=%d\n",pszMe, retval);
		goto err_irq;
	}

	/* MISC. register */
	MISCCR &= ~MISCCR_USBPAD;
	/* UPLLCON */
	UPLLCON = FInsrt(0x78, fPLL_MDIV) | FInsrt(0x02, fPLL_PDIV) 
				| FInsrt(0x03, fPLL_SDIV);
	/* CLKCON */
	CLKCON |= CLKCON_USBD;
    	Clear_pending(INT_USBD);

	printk( "S3C2410 USB Controller Core Initialized\n");
	return 0;

err_irq:

#ifdef USE_USBD_DMA
	s3c2410_free_dma(DMA_CH0);
	s3c2410_free_dma(DMA_CH3);
#endif 

err_dma:
	usbd_info.dmach_tx = 0;
	usbd_info.dmach_rx = 0;
	return retval;
}
/*
 * usbctl_exit()
 * Release DMA and interrupt resources
 */
void __exit usbctl_exit( void )
{
    printk("Unloading S3C2410 USB Controller\n");

	CLKCON &= ~CLKCON_USBD;

#ifdef CONFIG_PROC_FS
    remove_proc_entry ( PROC_NODE_NAME, NULL);
#endif

#ifdef USE_USBD_DMA
    s3c2410_free_dma(usbd_info.dmach_rx);
    s3c2410_free_dma(usbd_info.dmach_tx);
#endif
    free_irq(IRQ_USBD, NULL);
}

EXPORT_SYMBOL( s3c2410_usb_open );
EXPORT_SYMBOL( s3c2410_usb_start );
EXPORT_SYMBOL( s3c2410_usb_stop );
EXPORT_SYMBOL( s3c2410_usb_close );


EXPORT_SYMBOL( s3c2410_usb_get_descriptor_ptr );
EXPORT_SYMBOL( s3c2410_usb_set_string_descriptor );
EXPORT_SYMBOL( s3c2410_usb_get_string_descriptor );
EXPORT_SYMBOL( s3c2410_usb_kmalloc_string_descriptor );

#if defined(CONFIG_PM) && defined(CONFIG_MIZI)
EXPORT_SYMBOL( usbctl_resume );
EXPORT_SYMBOL( usbctl_suspend );
#endif

module_init( usbctl_init );
module_exit( usbctl_exit );

