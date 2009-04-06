/*
 * (C) Copyright 2000-2001 MIZI Research, Inc.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  usb-char.c
 *
 *  Miscellaneous character device interface for S3C2410 USB function
 *	driver.
 */

#include <linux/module.h>
#include <linux/config.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/cache.h>
#include <linux/poll.h>
#include <linux/circ_buf.h>
#include <linux/timer.h>

#include <asm/io.h>
#include <asm/semaphore.h>
#include <asm/proc/page.h>
#include <asm/mach-types.h>

#include "usb-char.h"
#include "s3c2410_usb.h"

#ifdef CONFIG_MIZI
#include <asm/irq.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
static struct pm_dev *usb_char_pm_dev;
#endif
#endif


//////////////////////////////////////////////////////////////////////////////
// Driver  Options
//////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_MIZI
#define VERSION		"0.5"
#define LINUETTE_VENDOR_ID	0x49f
#define LINUETTE_PRODUCT_ID	0x505A
#define LINUETTE_REV_NUM 1
 
#define USB_CHAR_MANUFACTURE	"MIZI Research Inc."
#define USB_CHAR_FOR_WHAT	"linu@usb"
#define USB_CHAR_BUILD_DATE	"20020115"
#define USB_CHAR_INTERFACE	"Bulk Transfer"
#else
#define VERSION		"0.4"
#endif


//#define VERBOSITY 1

#if VERBOSITY
# define	PRINTK(a...)	printk(__FILE__":"__FUNCTION__"(): " ##a)
#else
# define	PRINTK(a...)	(void)(0)
#endif

//////////////////////////////////////////////////////////////////////////////
// Globals - Macros - Enums - Structures
//////////////////////////////////////////////////////////////////////////////
#ifndef MIN
#define MIN( a, b ) ((a)<(b)?(a):(b))
#endif

typedef int bool; enum { false = 0, true = 1 };

static const char pszMe[] = "usbchr: ";

static wait_queue_head_t wq_read;
static wait_queue_head_t wq_write;
static wait_queue_head_t wq_poll;

/* Serialze multiple writers onto the transmit hardware
.. since we sleep the writer during transmit to stay in
.. sync. (Multiple writers don't make much sense, but..) */
static DECLARE_MUTEX( xmit_sem );

// size of usb DATA0/1 packets. 64 is standard maximum
// for bulk transport, though most hosts seem to be able
// to handle larger.
#define TX_PACKET_SIZE 64
#define RX_PACKET_SIZE 64
#define RBUF_SIZE  (4*PAGE_SIZE)

static struct wcirc_buf {
  char *buf;
  int in;
  int out;
} rx_ring = { NULL, 0, 0 };

static struct {
	 unsigned long  cnt_rx_complete;
	 unsigned long  cnt_rx_errors;
	 unsigned long  bytes_rx;
	 unsigned long  cnt_tx_timeouts;
	 unsigned long  cnt_tx_errors;
	 unsigned long  bytes_tx;
} charstats;


static char * tx_buf = NULL;
static char * packet_buffer = NULL;
static int sending = 0;
static int usb_ref_count = 0;
static int last_tx_result = 0;
static int last_rx_result = 0;
static int last_tx_size = 0;
static struct timer_list tx_timer;

//////////////////////////////////////////////////////////////////////////////
// Prototypes
//////////////////////////////////////////////////////////////////////////////
#ifndef CONFIG_MIZI
static char * 	what_the_f( int e );
#endif
static void 	free_txrx_buffers( void );
static void     twiddle_descriptors( void );
static void     free_string_descriptors( void ) ;
static int      usbc_open( struct inode *pInode, struct file *pFile );
static void     rx_done_callback_packet_buffer( int flag, int size );

static void     tx_timeout( unsigned long );
static void     tx_done_callback( int flag, int size );

static ssize_t  usbc_read( struct file *, char *, size_t, loff_t * );
static ssize_t  usbc_write( struct file *, const char *, size_t, loff_t * );
static unsigned int usbc_poll( struct file *pFile, poll_table * pWait );
static int usbc_ioctl( struct inode *pInode, struct file *pFile,
                       unsigned int nCmd, unsigned long argument );
static int      usbc_close( struct inode *pInode, struct file *pFile );

#ifdef CONFIG_S3C2410_EXTENEX1
static void     extenex_configured_notify_proc( void );
#endif
#ifdef CONFIG_MIZI
static int usbc_activate(void);
static void usbc_deactivate(void);
#endif
//////////////////////////////////////////////////////////////////////////////
// Private Helpers
//////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_PM) && defined(CONFIG_MIZI)
extern void usbctl_suspend(void);
extern void usbctl_resume(void);
static int
usbchar_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{
	PRINTK("pm_callback: %d\n", req);

	switch (req) {
	case PM_RESUME:    
		usbctl_resume();
		usbc_activate();
		
		printk("PM_RESUME");
		PRINTK("usb-char wakeup...");
		break;
	case PM_SUSPEND:   
		usbc_deactivate();
		usbctl_suspend();
		printk("PM_SUSPEND");
		PRINTK("usb-char sleep...");
		break;
	}
	PRINTK("done\n");
 
	return 0;
}
#endif

#ifndef CONFIG_MIZI
static char * what_the_f( int e )
{
	 char * p;
	 switch( e ) {
	 case 0:
		  p = "noErr";
		  break;
	 case -ENODEV:
		  p = "ENODEV - usb not in config state";
		  break;
	 case -EBUSY:
		  p = "EBUSY - another request on the hardware";
		  break;
	 case -EAGAIN:
		  p = "EAGAIN";
		  break;
	 case -EINTR:
		  p = "EINTR - interrupted\n";
		  break;
	 case -EPIPE:
		  p = "EPIPE - zero length xfer\n";
		  break;
	 default:
		  p = "????";
		  break;
	 }
	 return p;
}
#endif

static void free_txrx_buffers( void )
{
	PRINTK("\n");
	 if ( rx_ring.buf != NULL ) {
		  kfree( rx_ring.buf );
		  rx_ring.buf = NULL;
	 }
	 if ( packet_buffer != NULL ) {
		  kfree( packet_buffer );
		  packet_buffer = NULL;
	 }
	 if ( tx_buf != NULL ) {
		  kfree( tx_buf );
		  tx_buf = NULL;
	 }
}

/* twiddle_descriptors()
 * It is between open() and start(). Setup descriptors.
 */
static void twiddle_descriptors( void )
{
	 desc_t * pDesc = s3c2410_usb_get_descriptor_ptr();
	 string_desc_t * pString;

	PRINTK("\n");

	 pDesc->b.ep1.wMaxPacketSize = make_word_c( RX_PACKET_SIZE );
	 pDesc->b.ep1.bmAttributes   = USB_EP_BULK;
	 pDesc->b.ep2.wMaxPacketSize = make_word_c( TX_PACKET_SIZE );
	 pDesc->b.ep2.bmAttributes   = USB_EP_BULK;

#ifdef CONFIG_MIZI
	pDesc->dev.idVendor = LINUETTE_VENDOR_ID;
	pDesc->dev.idProduct = LINUETTE_PRODUCT_ID;
	pDesc->dev.bcdDevice = LINUETTE_REV_NUM;
	pDesc->b.cfg.bmAttributes = USB_CONFIG_SELFPOWERED;
	pDesc->b.cfg.MaxPower = 0;

	if (s3c2410_usb_get_string_descriptor(4)) return;

	pString = s3c2410_usb_kmalloc_string_descriptor( USB_CHAR_MANUFACTURE);
	if ( pString ) {
		s3c2410_usb_set_string_descriptor( 1, pString );
		pDesc->dev.iManufacturer = 1;
	}

	pString = s3c2410_usb_kmalloc_string_descriptor( USB_CHAR_FOR_WHAT );
	if ( pString ) {
		s3c2410_usb_set_string_descriptor( 2, pString );
		pDesc->dev.iProduct = 2;
	}

	pString = s3c2410_usb_kmalloc_string_descriptor( USB_CHAR_BUILD_DATE );
	if ( pString ) {
		s3c2410_usb_set_string_descriptor( 3, pString );
		pDesc->dev.iSerialNumber = 3;
	}

	pString = s3c2410_usb_kmalloc_string_descriptor( USB_CHAR_INTERFACE );
	if ( pString ) {
		s3c2410_usb_set_string_descriptor( 4, pString );
		pDesc->b.intf.iInterface = 4;
	}
#endif
}

static void free_string_descriptors( void )
{
	PRINTK("\n");
	 if ( machine_is_extenex1() ) {
		  string_desc_t * pString;
		  int i;
		  for( i = 1 ; i <= 4 ; i++ ) {
			   pString = s3c2410_usb_get_string_descriptor( i );
			   if ( pString )
					kfree( pString );
		  }
	 }
#ifdef CONFIG_MIZI
	{
		string_desc_t * pString;
		int i;
		for( i = 1 ; i <= 4 ; i++ ) {
			pString = s3c2410_usb_get_string_descriptor( i );
			if ( pString )
				kfree( pString );
			s3c2410_usb_set_string_descriptor( i , NULL);
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
// ASYNCHRONOUS
//////////////////////////////////////////////////////////////////////////////
static  void kick_start_rx( void )
{
	PRINTK("\n");
#if defined(CONFIG_MIZI)
 	 {
#else
	    // PRINTK("i'm not  MIZI - %d\n", usb_ref_count);
	 if ( usb_ref_count ) {
#endif
		  int total_space  = CIRC_SPACE( rx_ring.in, rx_ring.out, RBUF_SIZE );
//	     		PRINTK("total_space = %d, %d\n", total_space, RX_PACKET_SIZE);
		  if ( total_space >= RX_PACKET_SIZE ) {
			   s3c2410_usb_recv( packet_buffer,
								RX_PACKET_SIZE,
								rx_done_callback_packet_buffer
						      );
		  }
	 }
}
/*
 * rx_done_callback_packet_buffer()
 * We have completed a DMA xfer into the temp packet buffer.
 * Move to ring.
 *
 * flag values:
 * on init,  -EAGAIN
 * on reset, -EINTR
 * on RPE, -EIO
 * on short packet -EPIPE
 */
static void
rx_done_callback_packet_buffer( int flag, int size )
{
	 charstats.cnt_rx_complete++;

	PRINTK("\n");
	 if ( flag == 0 || flag == -EPIPE ) {
		  size_t n;

		  charstats.bytes_rx += size;

		  n = CIRC_SPACE_TO_END( rx_ring.in, rx_ring.out, RBUF_SIZE );
		  n = MIN( n, size );
		  size -= n;

		  memcpy( &rx_ring.buf[ rx_ring.in ], packet_buffer, n );
		  rx_ring.in = (rx_ring.in + n) & (RBUF_SIZE-1);
		  memcpy( &rx_ring.buf[ rx_ring.in ], packet_buffer + n, size );
		  rx_ring.in = (rx_ring.in + size) & (RBUF_SIZE-1);

		  wake_up_interruptible( &wq_read );
		  wake_up_interruptible( &wq_poll );

		  last_rx_result = 0;

		  kick_start_rx();

	 } else if ( flag != -EAGAIN ) {
		  charstats.cnt_rx_errors++;
		  last_rx_result = flag;
		  wake_up_interruptible( &wq_read );
		  wake_up_interruptible( &wq_poll );
	 }
	 else  /* init, start a read */
		  kick_start_rx();
}


static void tx_timeout( unsigned long unused )
{
	PRINTK( "%stx timeout\n", pszMe );
	s3c2410_usb_send_reset();
	charstats.cnt_tx_timeouts++;
}


// on init, -EAGAIN
// on reset, -EINTR
// on TPE, -EIO
static void tx_done_callback( int flags, int size )
{
	PRINTK("\n");
	 if ( flags == 0 )
		  charstats.bytes_tx += size;
	 else
		  charstats.cnt_tx_errors++;
	 last_tx_size = size;
	 last_tx_result = flags;
	 sending = 0;
	 wake_up_interruptible( &wq_write );
	 wake_up_interruptible( &wq_poll );
}


//////////////////////////////////////////////////////////////////////////////
// Workers
//////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_MIZI
static void usbc_alloc_mem(void)
{
	PRINTK("\n");
	tx_buf = (char*) kmalloc( TX_PACKET_SIZE, GFP_KERNEL | GFP_DMA );
	if ( tx_buf == NULL ) {
		PRINTK( "%sARGHH! COULD NOT ALLOCATE TX BUFFER\n", pszMe );
	}

	rx_ring.buf = (char*) kmalloc( RBUF_SIZE, GFP_KERNEL );
	if ( rx_ring.buf == NULL ) {
		PRINTK( "%sARGHH! COULD NOT ALLOCATE RX BUFFER\n", pszMe );
	}

	packet_buffer = (char*) kmalloc( RX_PACKET_SIZE, GFP_KERNEL | GFP_DMA );
	if ( packet_buffer == NULL ) {
		PRINTK( "%sARGHH! COULD NOT ALLOCATE RX PACKET BUFFER\n",pszMe);
	}
	//usbc_ioctl(0, 0, USBC_IOC_FLUSH_ALL, 0);
}
#endif

static int usbc_open( struct inode *pInode, struct file *pFile )
{
	 MOD_INC_USE_COUNT;
	 kick_start_rx();
	 return 0;
}

/*
 * Read endpoint. Note that you can issue a read to an
 * unconfigured endpoint. Eventually, the host may come along
 * and configure underneath this module and data will appear.
 */
static ssize_t usbc_read( struct file *pFile, char *pUserBuffer,
                        size_t stCount, loff_t *pPos )
{
	 ssize_t retval;
	 int flags;
	 DECLARE_WAITQUEUE( wait, current );

	// PRINTK( "count=%d", stCount);

	 local_irq_save( flags );
	 if ( last_rx_result == 0 ) {
		  local_irq_restore( flags );
	 } else {  /* an error happended and receiver is paused */
		  local_irq_restore( flags );
		  last_rx_result = 0;
		  kick_start_rx();
	 }

	 add_wait_queue( &wq_read, &wait );
	 while( 1 ) {
		  ssize_t bytes_avail;
		  ssize_t bytes_to_end;

		  set_current_state( TASK_INTERRUPTIBLE );

		  /* snap ring buf state */
		  local_irq_save( flags );
		  bytes_avail  = CIRC_CNT( rx_ring.in, rx_ring.out, RBUF_SIZE );
		  bytes_to_end = CIRC_CNT_TO_END( rx_ring.in, rx_ring.out, RBUF_SIZE );
		  local_irq_restore( flags );

		  if ( bytes_avail != 0 ) {
			   ssize_t bytes_to_move = MIN( stCount, bytes_avail );
			   retval = 0;		// will be bytes transfered
			   if ( bytes_to_move != 0 ) {
					size_t n = MIN( bytes_to_end, bytes_to_move );
					if ( copy_to_user( pUserBuffer,
									   &rx_ring.buf[ rx_ring.out ],
									   n ) ) {
						 retval = -EFAULT;
						 break;
					}
					bytes_to_move -= n;
 					retval += n;
					// might go 1 char off end, so wrap
					rx_ring.out = ( rx_ring.out + n ) & (RBUF_SIZE-1);
					if ( copy_to_user( pUserBuffer + n,
									   &rx_ring.buf[ rx_ring.out ],
									   bytes_to_move )
						 ) {
						 retval = -EFAULT;
						 break;
					}
					rx_ring.out += bytes_to_move;		// cannot wrap
					retval += bytes_to_move;
					kick_start_rx();
			   }
			   break;
		  }
		  else if ( last_rx_result ) {
			   retval = last_rx_result;
			   break;
		  }
		  else if ( pFile->f_flags & O_NONBLOCK ) {  // no data, can't sleep
			   retval = -EAGAIN;
			   break;
		  }
		  else if ( signal_pending( current ) ) {   // no data, can sleep, but signal
			   retval = -ERESTARTSYS;
			   break;
		  }
		  schedule();					   			// no data, can sleep
	 }
	 set_current_state( TASK_RUNNING );
	 remove_wait_queue( &wq_read, &wait );

#ifndef CONFIG_MIZI
	 if ( retval < 0 )
		  printk( "%sread error %d - %s\n", pszMe, retval, what_the_f( retval ) );
#endif
	 return retval;
}

/*
 * Write endpoint. This routine attempts to break the passed in buffer
 * into usb DATA0/1 packet size chunks and send them to the host.
 * (The lower-level driver tries to do this too, but easier for us
 * to manage things here.)
 *
 * We are at the mercy of the host here, in that it must send an IN
 * token to us to pull this data back, so hopefully some higher level
 * protocol is expecting traffic to flow in that direction so the host
 * is actually polling us. To guard against hangs, a 5 second timeout
 * is used.
 *
 * This routine takes some care to only report bytes sent that have
 * actually made it across the wire. Thus we try to stay in lockstep
 * with the completion routine and only have one packet on the xmit
 * hardware at a time. Multiple simultaneous writers will get
 * "undefined" results.
 *
  */
static ssize_t  usbc_write( struct file *pFile, const char * pUserBuffer,
							 size_t stCount, loff_t *pPos )
{
	 ssize_t retval = 0;
	 ssize_t stSent = 0;

	 DECLARE_WAITQUEUE( wait, current );

	 PRINTK( KERN_DEBUG "%swrite() %d bytes\n", pszMe, stCount );

	 down( &xmit_sem );  // only one thread onto the hardware at a time

	 while( stCount != 0 && retval == 0 ) {
		  int nThisTime  = MIN( TX_PACKET_SIZE, stCount );
		  copy_from_user( tx_buf, pUserBuffer, nThisTime );
		  sending = nThisTime;
 		  retval = s3c2410_usb_send( tx_buf, nThisTime, tx_done_callback );
		  if ( retval < 0 ) {
#ifndef CONFIG_MIZI
			   char * p = what_the_f( retval );
			   printk( "%sCould not queue xmission. rc=%d - %s\n",
					   pszMe, retval, p );
#endif
			   sending = 0;
			   break;
		  }
		  /* now have something on the diving board */
		  add_wait_queue( &wq_write, &wait );
		  tx_timer.expires = jiffies + ( HZ * 5 );
		  add_timer( &tx_timer );
		  while( 1 ) {
			   set_current_state( TASK_INTERRUPTIBLE );
			   if ( sending == 0 ) {  /* it jumped into the pool */
					del_timer( &tx_timer );
					retval = last_tx_result;
					if ( retval == 0 ) {
						 stSent		 += last_tx_size;
						 pUserBuffer += last_tx_size;
						 stCount     -= last_tx_size;
					}
#ifndef CONFIG_MIZI
					else
						 printk( "%sxmission error rc=%d - %s\n",
								 pszMe, retval, what_the_f(retval) );
#endif
					break;
			   }
			   else if ( signal_pending( current ) ) {
					del_timer( &tx_timer );
					PRINTK( "%ssignal\n", pszMe  );
					retval = -ERESTARTSYS;
					break;
			   }
			   schedule();
		  }
		  set_current_state( TASK_RUNNING );
		  remove_wait_queue( &wq_write, &wait );
	 }

	 up( &xmit_sem );

	 if ( 0 == retval )
		  retval = stSent;
	 return retval;
}

static unsigned int usbc_poll( struct file *pFile, poll_table * pWait )
{
	 unsigned int retval = 0;

	 PRINTK( KERN_DEBUG "%poll()\n", pszMe );

	 poll_wait( pFile, &wq_poll, pWait );

	 if ( CIRC_CNT( rx_ring.in, rx_ring.out, RBUF_SIZE ) )
		  retval |= POLLIN | POLLRDNORM;
	 if ( s3c2410_usb_xmitter_avail() )
		  retval |= POLLOUT | POLLWRNORM;
	 return retval;
}

static int usbc_ioctl( struct inode *pInode, struct file *pFile,
                       unsigned int nCmd, unsigned long argument )
{
	 int retval = 0;

	 switch( nCmd ) {

	 case USBC_IOC_FLUSH_RECEIVER:
		  s3c2410_usb_recv_reset();
		  rx_ring.in = rx_ring.out = 0;
		  break;

	 case USBC_IOC_FLUSH_TRANSMITTER:
		  s3c2410_usb_send_reset();
		  break;

	 case USBC_IOC_FLUSH_ALL:
		  s3c2410_usb_recv_reset();
		  rx_ring.in = rx_ring.out = 0;
		  s3c2410_usb_send_reset();
		  break;

	 default:
		  retval = -ENOIOCTLCMD;
		  break;

	 }
	 return retval;
}


#ifdef CONFIG_MIZI
static int
usbc_activate(void)
{
	int retval = 0;

	PRINTK("\n");
	/* start usb core */
	retval = s3c2410_usb_open( "usb-char" );

	twiddle_descriptors();

	retval = s3c2410_usb_start();
	if ( retval ) {
		PRINTK( "%sAGHH! Could not USB core\n", pszMe );
		free_txrx_buffers();
		return retval;
	}

	usb_ref_count++;   /* must do _before_ kick_start() */
	kick_start_rx();
	return 0;
}

static void
usbc_deactivate(void)
{
	PRINTK("\n");
	down( &xmit_sem );
	s3c2410_usb_stop();
	del_timer( &tx_timer );
	s3c2410_usb_close();
	up( &xmit_sem );
}
#endif

static int usbc_close( struct inode *pInode, struct file * pFile )
{
	PRINTK( KERN_DEBUG "%sclose()\n", pszMe );
#ifdef CONFIG_MIZI
	--usb_ref_count;

#else
	if ( --usb_ref_count == 0 ) {
		 down( &xmit_sem );
		 s3c2410_usb_stop();
		 free_txrx_buffers();
		 free_string_descriptors();
		 del_timer( &tx_timer );
		 s3c2410_usb_close();
		 up( &xmit_sem );
	}
#endif
    MOD_DEC_USE_COUNT;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////////

static struct file_operations usbc_fops = {
		owner:      THIS_MODULE,
		open:		usbc_open,
		read:		usbc_read,
		write:		usbc_write,
		poll:		usbc_poll,
		ioctl:		usbc_ioctl,
		release:	usbc_close,
};

static struct miscdevice usbc_misc_device = {
    USBC_MINOR, "usb_char", &usbc_fops
};

/*
 * usbc_init()
 */

int __init usbc_init( void )
{
	 int rc;

#if !defined( CONFIG_ARCH_S3C2410 )
	 return -ENODEV;
#endif

	 if ( (rc = misc_register( &usbc_misc_device )) != 0 ) {
		  printk( KERN_WARNING "%sCould not register device 10, "
				  "%d. (%d)\n", pszMe, USBC_MINOR, rc );
		  return -EBUSY;
	 }

	 // initialize wait queues
	 init_waitqueue_head( &wq_read );
	 init_waitqueue_head( &wq_write );
	 init_waitqueue_head( &wq_poll );

	 // initialize tx timeout timer
	 init_timer( &tx_timer );
	 tx_timer.function = tx_timeout;

#ifdef CONFIG_MIZI
	memset( &charstats, 0, sizeof( charstats ) );
	sending = 0;
	last_tx_result = 0;
	last_tx_size = 0;
#endif
	printk( KERN_INFO "USB Function Character Driver Interface"
		" - %s, (C) 2001, Extenex Corp.\n", VERSION);
 
#ifdef CONFIG_MIZI
#ifdef CONFIG_PM
	usb_char_pm_dev = pm_register(PM_SYS_DEV, PM_SYS_MISC,
				      usbchar_pm_callback);
#endif
	usbc_alloc_mem();
	usbc_activate();
#endif
	 return rc;
}

void __exit usbc_exit( void )
{
#ifdef CONFIG_MIZI
	usbc_deactivate();
	free_txrx_buffers();
	free_string_descriptors();
#ifdef CONFIG_PM
	if (usb_char_pm_dev)
		pm_unregister(usb_char_pm_dev);
#endif 
#endif
	misc_deregister( &usbc_misc_device );
}

EXPORT_NO_SYMBOLS;

module_init(usbc_init);
module_exit(usbc_exit);



// end: usb-char.c



