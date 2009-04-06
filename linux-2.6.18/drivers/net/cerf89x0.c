/* cs89x0.c: A Crystal Semiconductor (Now Cirrus Logic) CS89[02]0
 *  driver for linux.
 */

/*
	Written 1996 by Russell Nelson, with reference to skeleton.c
	written 1993-1994 by Donald Becker.

	This software may be used and distributed according to the terms
	of the GNU Public License, incorporated herein by reference.

        The author may be reached at nelson@crynwr.com, Crynwr
        Software, 521 Pleasant Valley Rd., Potsdam, NY 13676

  Changelog:

  Mike Cruse        : mcruse@cti-ltd.com
  Russ Nelson       : Jul 13 1998.  Added RxOnly DMA support.
  Melody Lee        : Aug 10 1999.  Changes for Linux 2.2.5 compatibility. 
  Alan Cox          : Removed 1.2 support, added 2.1 extra counters.
  Andrew Morton     : andrewm@uow.edu.au
  Sangwook Lee      : hitchcar@sec.samsung.com
*/

static char *version =
"cerf89x0.c: (kernel 2.3.99) Russell Nelson, Andrew Morton\n";

/* ======================= end of configuration ======================= */

/* Always include 'config.h' first in case the user wants to turn on
   or override something. */
#ifdef MODULE
#include <linux/module.h>
#include <linux/version.h>
#else
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif

/*
 * Set this to zero to remove all the debug statements via
 * dead code elimination
 */
#define DEBUGGING	1

/*
  Sources:

	Crynwr packet driver epktisa.

	Crystal Semiconductor data sheets.

*/

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/spinlock.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <asm/irq.h>
#include <asm/hardware.h>
#define IRQ_LAN IRQ_EINT8_23

#include "cs89x0.h"

/* First, a few definitions that the brave might change. */
/* A zero-terminated list of I/O addresses to be probed. */
static unsigned int netcard_portlist[] __initdata =
   { (0xd0000000+0x00000300), 0};

#ifdef  DEBUGGING
static unsigned int net_debug = 4;
#else
#define net_debug 0	/* gcc will remove all the debug code for us */
#endif

/* The number of low I/O ports used by the ethercard. */
#define NETCARD_IO_EXTENT	16

/* Information that need to be kept for each board. */
struct net_local {
	struct net_device_stats stats;
	int chip_type;		/* one of: CS8900, CS8920, CS8920M */
	char chip_revision;	/* revision letter of the chip ('A'...) */
	int send_cmd;		/* the proper send command: TX_NOW, TX_AFTER_381, or TX_AFTER_ALL */
	int auto_neg_cnf;	/* auto-negotiation word from EEPROM */
	int adapter_cnf;	/* adapter configuration from EEPROM */
	int isa_config;		/* ISA configuration from EEPROM */
	int irq_map;		/* IRQ map from EEPROM */
	int rx_mode;		/* what mode are we in? 0, RX_MULTCAST_ACCEPT, or RX_ALL_ACCEPT */
	int curr_rx_cfg;	/* a copy of PP_RxCFG */
	int linectl;		/* either 0 or LOW_RX_SQUELCH, depending on configuration. */
	int send_underrun;	/* keep track of how many underruns in a row we get */
        struct sk_buff * Gskb;	/* pinnter to TX buff waiting for Ready4Tx */
	spinlock_t lock;
};

/* Index to functions, as function prototypes. */

extern int cerf89x0_probe(struct net_device *dev);
static int S3C2410_HWinit(void);
static int cerf89x0_probe1(struct net_device *dev, int ioaddr);
static int net_open(struct net_device *dev);
static int net_send_packet(struct sk_buff *skb, struct net_device *dev);
static void net_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static void set_multicast_list(struct net_device *dev);
static void net_timeout(struct net_device *dev);
static void net_rx(struct net_device *dev);
static int net_close(struct net_device *dev);
static struct net_device_stats *net_get_stats(struct net_device *dev);
static void reset_chip(struct net_device *dev);
static int set_mac_address(struct net_device *dev, void *addr);
static void count_rx_errors(int status, struct net_local *lp);

/* Example routines you must write ;->. */
#define tx_done(dev) 1


/* Check for a network chip of this type, and return '0' if one exists.
 * Return ENODEV on Failure.
 */

#ifdef CONFIG_S3C2410_SMDK

#define CS8900_Tacs    (0x0)   // 0clk
#define CS8900_Tcos    (0x3)   // 4clk
#define CS8900_Tacc    (0x7)   // 14clk
#define CS8900_Tcoh    (0x1)   // 1clk
#define CS8900_Tah (0x3)   // 4clk
#define CS8900_Tacp    (0x3)   // 6clk
#define CS8900_PMC (0x0)   // normal(1data)

int __init S3C2410_HWinit()
{
	// initialize CS8900
    BWSCON  = (BWSCON&~(0xf<<12))|(0xd<<12);  /*  nWAIT */
    //  rGSTATUS0;      /* Read Only */
	GPGCON &= ~(3 << 2);
    GPGCON |= (2 << 2); /* GPG1 set to EINT9 */
    BANKCON3=((CS8900_Tacs<<13)+(CS8900_Tcos<<11)+(CS8900_Tacc<<8)
        +(CS8900_Tcoh<<6)+(CS8900_Tah<<4)+(CS8900_Tacp<<2)+(CS8900_PMC));
	EXTINT1 &= ~(7 << 4);
    EXTINT1 |= (4 << 4); /* EINT9 rising edge */
	return 0;
}
#endif

int __init cerf89x0_probe(struct net_device *dev)
{
	static int initialised = 0;
	int i;

	if (initialised)
		return -ENODEV;
	initialised = 1;
#ifdef CONFIG_S3C2410_SMDK
	S3C2410_HWinit();
#endif
	if (net_debug)
		printk("cerf89x0:cerf89x0_probe()\n");
	
#ifdef CONFIG_S3C2410_SMDK
    GPGUP  |= (1 << 1); /* GPG1 pull-up disable */
    GPGCON &= ~(3 << 2);
    GPGCON |= (2 << 2); /* GPG1 set to EINT9 */
#endif
	
	for (i = 0; netcard_portlist[i]; i++) {
		int ioaddr = netcard_portlist[i];
		if (cerf89x0_probe1(dev, ioaddr) == 0)
			return 0;
	}
	printk(KERN_WARNING "cerf89x0: no cs8900 detected.\n");
	return ENODEV;
}

extern u8 inline
raw_readb(u32 addr)
{
	return (*(volatile u8 *)(addr));
}

extern u16 inline
raw_readw(u32 addr)
{
	return (*(volatile u16 *)(addr));
}

extern void inline
raw_writeb(u8 value, u32 addr)
{
	(*(volatile u8 *)(addr)) = value;
}

extern void inline
raw_writew(u16 value, u32 addr)
{
	(*(volatile u16 *)(addr)) = value;
}

extern u16 inline
readreg(struct net_device *dev, int portno)
{
	raw_writew(portno, dev->base_addr + ADD_PORT);
	return raw_readw(dev->base_addr + DATA_PORT);
}

extern void inline
writereg(struct net_device *dev, int portno, u16 value)
{
	raw_writew(portno, dev->base_addr + ADD_PORT);
	raw_writew(value,  dev->base_addr + DATA_PORT);
}

extern u16 inline
readword(struct net_device *dev, int portno)
{
	return raw_readw(dev->base_addr + portno);
}

extern void inline
writeword(struct net_device *dev, int portno, u16 value)
{
	raw_writew(value,  dev->base_addr + portno);
}

static void writeblock(struct net_device *dev, char *pData, int Length)
{
	int i;
	
	for (i = 0 ; i < (Length / 2); i++)
	{   
		writeword(dev, TX_FRAME_PORT, *(u16 *)pData );
		pData += 2;
	}
   
	if (Length % 2) 
	{
		u16 OddWordValue = *pData;
		writeword(dev, TX_FRAME_PORT, OddWordValue);
	}
}

static void readblock(struct net_device *dev, char *pData, int Length)
{
#if 0
	u16 wOddWord;
	int i;
   	
	u16 StartOffset = PP_RxFrame | AUTOINCREMENT;

   	writeword(dev, ADD_PORT, StartOffset);

	if ((u32) pData % 2)
	{
		for (i=0; i < (Length/2); i++) 
		{
			wOddWord = readword(dev, DATA_PORT);
               
			*(u8*)pData++ = (u8) wOddWord & 0xFF;
			*(u8*)pData++ = (u8) (wOddWord >> 8) & 0xFF;
		}
	}
	else
	{
		for (i=0; i < (Length/2); i++) 
		{
			*(u16*) pData = readword(dev, DATA_PORT);
			pData += 2;
		}
	}

	switch (Length % 2) 
   	{
		case 1:
			*pData = (u8) (readword(dev, DATA_PORT) & 0xff);
	}
#endif
	u16 InputWord;
	int i;
   	
	for (i=0; i < (Length>>1); i++) 
	{
		InputWord = readword(dev, RX_FRAME_PORT);
		*(u8*)pData++ = (u8) InputWord & 0xFF;
		*(u8*)pData++ = (u8) (InputWord >> 8) & 0xFF;
	}

	switch (Length & 0x1) 
   	{
		case 1:
			*pData = (u8) (readword(dev, RX_FRAME_PORT) & 0xff);
	}
}


/* This is the real probe routine.  Linux has a history of friendly device
   probes on the ISA bus.  A good device probes avoids doing writes, and
   verifies that the correct device exists and functions.
   Return 0 on success.
 */
char chip_version(int PID)
{
  switch(PID) {
  case 0x7:
    return 'B';
  case 0x8:
    return 'C';
  case 0x9:
    return 'D';
  default :
    return '?';
 }

}

static int __init
cerf89x0_probe1(struct net_device *dev, int ioaddr)
{
	struct net_local *lp;
	static unsigned version_printed = 0;
	int i;
	unsigned rev_type = 0;
	u16 MacAddr[3] = {0,0,0};
	int retval;
	unsigned short wData;

	/* Initialize the device structure. */
	if (dev->priv == NULL)
	{
		dev->priv = kmalloc(sizeof(struct net_local), GFP_KERNEL);
		if (dev->priv == 0)
		{
			retval = ENOMEM;
			goto out;
		}
		memset(dev->priv, 0, sizeof(struct net_local));
	}
	lp = (struct net_local *)dev->priv;

	/*  Bus Reset Consideration  */
	wData = raw_readw(ioaddr + ADD_PORT);

	if ((wData & ADD_MASK) != ADD_SIG )
	{
	  //	if (net_debug)
			printk("cerf89x0:cerf89x0_probe1() 0x%08X\n", wData);
			//			printk(" BWSCON 0x%08X\n", rBWSCON);
			
		retval = ENODEV;
		goto out1;
	}


	/* Fill in the 'dev' fields. */
	dev->base_addr = ioaddr;

	/* get the chip type */
	rev_type = readreg(dev, PRODUCT_ID_ADD);
	lp->chip_type = rev_type &~ REVISON_BITS;
	lp->chip_revision = chip_version((rev_type & REVISON_BITS) >> 8);
	

	/* Check the chip type and revision in order to set the correct send command
	   CS8920 revision C and CS8900 revision F can use the faster send.
	*/
/*  	lp->send_cmd = TX_AFTER_381; */
  	lp->send_cmd = TX_AFTER_ALL; 


/*
	if (lp->chip_type == CS8900 && lp->chip_revision >= 'F')
		lp->send_cmd = TX_NOW;
	if (lp->chip_type != CS8900 && lp->chip_revision >= 'C')
		lp->send_cmd = TX_NOW;

*/	
	reset_chip(dev);

	if (net_debug  &&  version_printed++ == 0)
		printk(version);

	printk(KERN_INFO "%s: cs89%c0%s rev %c Base %x",
	       dev->name,
	       lp->chip_type==CS8900 ? '0' : '2',
	       lp->chip_type==CS8920M ? "M" : "",
	       lp->chip_revision,
	       (unsigned int )dev->base_addr);


	MacAddr[0] = 0x0000;
	MacAddr[1] = 0x24f0;
	MacAddr[2] = 0x5b05;
   
	for (i = 0; i < ETH_ALEN/2; i++)
	{
		dev->dev_addr[i*2]   = MacAddr[i];
		dev->dev_addr[i*2+1] = MacAddr[i] >> 8;
	}

	dev->irq = IRQ_LAN;
	printk(KERN_INFO ", IRQ %d", dev->irq);

	/* print the ethernet address. */
	printk(", MAC ");
	for (i = 0; i < ETH_ALEN; i++)
	{
		printk("%s%02X", i ? ":" : "", dev->dev_addr[i]);
	}
	printk("\n");

	dev->open		= net_open;
	dev->stop		= net_close;
	dev->tx_timeout		= net_timeout;
	dev->watchdog_timeo	= 3 * HZ ;
	dev->hard_start_xmit 	= net_send_packet;
	dev->get_stats		= net_get_stats;
	dev->set_multicast_list = set_multicast_list;
	dev->set_mac_address 	= set_mac_address;

	/* Fill in the fields of the device structure with ethernet values. */
	ether_setup(dev);

	return 0;
out1:
	kfree(dev->priv);
	dev->priv = 0;
out:
	return retval;
}


void  __init reset_chip(struct net_device *dev)
{
	int reset_start_time;

	writereg(dev, PP_SelfCTL, readreg(dev, PP_SelfCTL) | POWER_ON_RESET);

	/* wait 30 ms */
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(30*HZ/1000);

	/* Wait until the chip is reset */
	reset_start_time = jiffies;
	while( (readreg(dev, PP_SelfST) & INIT_DONE) == 0 &&
		jiffies - reset_start_time < 4)
		;
}

/* Open/initialize the board.  This is called (in the current kernel)
   sometime after booting when the 'ifconfig' program is run.

   This routine should set everything up anew at each open, even
   registers that "should" only need to be set once at boot, so that
   there is non-reboot way to recover if something goes wrong.
   */

/* AKPM: do we need to do any locking here? */

static int
net_open(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *)dev->priv;
	int i;

#ifdef CONFIG_S3C2410_SMDK
    EINTPEND |= (1 << 9); /* External Interrupt Pending clear */

    SRCPND |= (1 << 5); /* EINT8_23 source pending clear */
    INTPND |= (1 << 5); /* EINT8_23 interrupt pending clear */

    INTMOD &= ~(1 << 5); /* IRQ interrupt mode */
    INTMSK &= ~(1 << 5); /* interrupt service available */

    EXTINT1 &= ~(7 << 4);
    EXTINT1 |= (4 << 4); /* EINT9 rising edge */

    EINTMASK &= ~(1 << 9); /* External Interrupt Enable */
#endif

	/* Prevent the crystal chip from generating interrupts */
	writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) & ~ENABLE_IRQ);
        enable_irq(IRQ_EINT8_23);
	/* Grab the interrupt */
	if (request_irq(dev->irq, &net_interrupt, SA_SHIRQ, "cs89x0", dev))
	{
		if (net_debug)
			printk("cerf89x0: request_irq(%d) failed\n", dev->irq);
		return -EAGAIN;
	}

	/* Set up the IRQ - Apparently magic */
	if (lp->chip_type == CS8900)
		writereg(dev, PP_CS8900_ISAINT, 0);
	else
		writereg(dev, PP_CS8920_ISAINT, 0);
	
	/* set the Ethernet address */
	for (i=0; i < ETH_ALEN/2; i++)
		writereg(dev, PP_IA+i*2, dev->dev_addr[i*2] | (dev->dev_addr[i*2+1] << 8));

	/* Receive only error free packets addressed to this card */
	lp->rx_mode = 0;//RX_OK_ACCEPT | RX_IA_ACCEPT;
	lp->curr_rx_cfg = RX_OK_ENBL | RX_CRC_ERROR_ENBL;
	
	writereg(dev, PP_RxCTL, DEF_RX_ACCEPT);

 	writereg(dev, PP_RxCFG, lp->curr_rx_cfg);

	writereg(dev, PP_TxCFG,
		TX_LOST_CRS_ENBL |
		TX_SQE_ERROR_ENBL |
		TX_OK_ENBL |
		TX_LATE_COL_ENBL |
		TX_JBR_ENBL |
		TX_ANY_COL_ENBL |
		TX_16_COL_ENBL);

	writereg(dev, PP_BufCFG,
		READY_FOR_TX_ENBL |
		RX_MISS_COUNT_OVRFLOW_ENBL |
		TX_COL_COUNT_OVRFLOW_ENBL |
		TX_UNDERRUN_ENBL);

	/* Turn on both receive and transmit operations */
	writereg(dev, PP_LineCTL, readreg(dev, PP_LineCTL) |
			SERIAL_RX_ON |
			SERIAL_TX_ON);

	/* now that we've got our act together, enable everything */
	writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) | IO_CHANNEL_READY_ON);
	writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) | ENABLE_IRQ);

	enable_irq(dev->irq);

	MOD_INC_USE_COUNT;
	netif_start_queue(dev);

	return 0;
}

static void net_timeout(struct net_device *dev)
{
	/* If we get here, some higher level has decided we are broken.
	   There should really be a "kick me" function call instead. */
        printk(KERN_INFO " CS8900A device is not stable or unplugged \n ");
	if (net_debug > 0)
		printk("%s: transmit timed out, %s?\n", dev->name,
	   	tx_done(dev) ? "IRQ conflict ?" : "network cable problem");
	/* Try to restart the adaptor. */
	netif_wake_queue(dev);
}

static int net_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct net_local *lp = (struct net_local *)dev->priv;
	writereg(dev, PP_BusCTL, 0x0);
	writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) | ENABLE_IRQ);

	if (net_debug > 4)
	{
		printk("%s: sent %d byte packet of type %x\n",
			dev->name, skb->len,
			(skb->data[ETH_ALEN+ETH_ALEN] << 8) | skb->data[ETH_ALEN+ETH_ALEN+1]);
	}

	/* keep the upload from being interrupted, since we
                  ask the chip to start transmitting before the
                  whole packet has been completely uploaded. */


	spin_lock_irq(&lp->lock);
	netif_stop_queue(dev);

	/* initiate a transmit sequence */

	writeword(dev, TX_CMD_PORT, lp->send_cmd); /* SW.LEE */
	writeword(dev, TX_LEN_PORT, skb->len); 


	/* Test to see if the chip has allocated memory for the packet */

	if ((readreg(dev, PP_BusST) & READY_FOR_TX_NOW) == 0)
	{
		/*
		 * Gasp!  It hasn't.  But that shouldn't happen since
		 * we're waiting for TxOk, so return 1 and requeue this packet.
		 */
	  lp->Gskb = skb;
	  spin_unlock_irq(&lp->lock);
	  return 0;
	/*  	if (net_debug) */
	  /*  			printk("cs89x0: Tx buffer not free!\n"); */
			
	  /*  		return 1; */
	}

	/* Write the contents of the packet */
	writeblock(dev, skb->data, skb->len);	
	
	spin_unlock_irq(&lp->lock);
	dev->trans_start = jiffies;

/*  	
	netif_start_queue(dev);
	It NETDEV WATCHDOG time not happens , It will stop 

*/
	dev_kfree_skb(skb);

	/*
	 * We DO NOT call netif_wake_queue() here.
	 * We also DO NOT call netif_start_queue().
	 *
	 * Either of these would cause another bottom half run through
	 * net_send_packet() before this packet has fully gone out.  That causes
	 * us to hit the "Gasp!" above and the send is rescheduled.  it runs like
	 * a dog.  We just return and wait for the Tx completion interrupt handler
	 * to restart the netdevice layer
	 */

	return 0;
}

/* The typical workload of the driver:
   Handle the network interface interrupts. */
   
static void net_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = dev_id;
	struct net_local *lp;
	int ioaddr, status;

#ifdef CONFIG_S3C2410_SMDK
    EINTPEND |= (1 << 9); /* External Interrupt Pending clear */

    SRCPND |= (1 << 5); /* EINT8_23 source pending clear */
    INTPND |= (1 << 5); /* EINT8_23 interrupt pending clear */
#endif

	ioaddr = dev->base_addr;
	lp = (struct net_local *)dev->priv;

	/* we MUST read all the events out of the ISQ, otherwise we'll never
		get interrupted again.  As a consequence, we can't have any limit
		on the number of times we loop in the interrupt handler.  The
		hardware guarantees that eventually we'll run out of events.  Of
		course, if you're on a slow machine, and packets are arriving
		faster than you can read them off, you're screwed.  Hasta la
		vista, baby!
	*/
	while ((status = readword(dev, ISQ_PORT)))
	{
		if (net_debug > 4)
			printk("%s: event=%04x\n", dev->name, status);
			
		switch(status & ISQ_EVENT_MASK)
		{
			case ISQ_RECEIVER_EVENT:
				/* Got a packet(s). */
				net_rx(dev);
				break;
			
			case ISQ_TRANSMITTER_EVENT:
				lp->stats.tx_packets++;
				netif_wake_queue(dev);	/* Inform upper layers. */
				if ((status & (	TX_OK |
						TX_LOST_CRS |
						TX_SQE_ERROR |
						TX_LATE_COL |
						TX_16_COL)) != TX_OK)
				{
					if ((status & TX_OK) == 0) lp->stats.tx_errors++;
					if (status & TX_LOST_CRS) lp->stats.tx_carrier_errors++;
					if (status & TX_SQE_ERROR) lp->stats.tx_heartbeat_errors++;
					if (status & TX_LATE_COL) lp->stats.tx_window_errors++;
					if (status & TX_16_COL) lp->stats.tx_aborted_errors++;
				}
				break;
			
			case ISQ_BUFFER_EVENT:
				if (status & READY_FOR_TX)
				{
	    /* we tried to transmit a packet earlier, but inexplicably ran out of buffers.
	     * That shouldn't happen since we only ever load one packet.
	     *	Shrug. Do the right thing anyway. 
	     */
				  if ( lp->Gskb ) {
				    writeblock(dev,lp->Gskb->data,lp->Gskb->len);
				    dev->trans_start = jiffies;
				    dev_kfree_skb(lp->Gskb);
				    lp->Gskb= NULL;
				/*      printk(" ------------> Gskb --------> \n"); */
				  } else {
				    			  
					netif_wake_queue(dev);	/* Inform upper layers. */
				  }
				}
				if (status & TX_UNDERRUN)
				{
					if (net_debug > 0)
						printk("%s: transmit underrun\n", dev->name);
					lp->send_underrun++;
					if (lp->send_underrun == 3)
						lp->send_cmd = TX_AFTER_381;
					else if (lp->send_underrun == 6)
						lp->send_cmd = TX_AFTER_ALL;
					/*
					 * transmit cycle is done, although	frame wasn't transmitted - this
				 	 * avoids having to wait for the upper	layers to timeout on us,
				 	 * in the event of a tx underrun
				 	 */
					netif_wake_queue(dev);	/* Inform upper layers. */
				}
				break;
			
			case ISQ_RX_MISS_EVENT:
				lp->stats.rx_missed_errors += (status >>6);
				break;
			
			case ISQ_TX_COL_EVENT:
				lp->stats.collisions += (status >>6);
				break;
			default:
				if (net_debug > 3)
					printk("%s: event=%04x\n", dev->name, status);
		}
	}

	writereg(dev, PP_BusCTL,0x0); /* DISABLE_IRQ */
	writereg(dev, PP_BusCTL,ENABLE_IRQ);
}

static void
count_rx_errors(int status, struct net_local *lp)
{
	lp->stats.rx_errors++;
	if (status & RX_RUNT) lp->stats.rx_length_errors++;
	if (status & RX_EXTRA_DATA) lp->stats.rx_length_errors++;
	if (status & RX_CRC_ERROR)
		if (!(status & (RX_EXTRA_DATA|RX_RUNT)))
			/* per str 172 */
			lp->stats.rx_crc_errors++;
	if (status & RX_DRIBBLE) lp->stats.rx_frame_errors++;
	return;
}

/* We have a good packet(s), get it/them out of the buffers. */
static void
net_rx(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *)dev->priv;
	struct sk_buff *skb;
	int status = 0, length = 0;
	
	/*  status = readreg(dev, PP_RxStatus); */
	status = readword(dev, RX_FRAME_PORT);
	if ((status & RX_OK) == 0) {
		count_rx_errors(status, lp);
		return;
	}

/*  	length = readreg(dev, PP_RxLength); */
	length = readword(dev, RX_FRAME_PORT);

	/* Malloc up new buffer. */
	skb = alloc_skb(length+2, GFP_ATOMIC);
	skb_reserve(skb, 2);

	if (skb == NULL)
	{
		lp->stats.rx_dropped++;
		return;
	}
	skb->len = length;
	skb->dev = dev;

	readblock(dev, skb->data, skb->len);

	if (net_debug > 4)
	{
		printk("%s: received %d byte packet of type %x\n",
			dev->name, length,
			(skb->data[ETH_ALEN+ETH_ALEN] << 8) | skb->data[ETH_ALEN+ETH_ALEN+1]);
	}

	skb->protocol=eth_type_trans(skb,dev);
	netif_rx(skb);
	lp->stats.rx_packets++;
	lp->stats.rx_bytes+=skb->len;
	return;
}

/* The inverse routine to net_open(). */
static int
net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	
	writereg(dev, PP_RxCFG, 0);
	writereg(dev, PP_TxCFG, 0);
	writereg(dev, PP_BufCFG, 0);
	writereg(dev, PP_BusCTL, 0);

	free_irq(dev->irq, dev);

	/* Update the statistics here. */
	MOD_DEC_USE_COUNT;
	return 0;
}

/* Get the current statistics.	This may be called with the card open or
   closed. */
static struct net_device_stats *
net_get_stats(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *)dev->priv;
	unsigned long flags;

	spin_lock_irqsave(&lp->lock, flags);
	/* Update the statistics from the device registers. */
	lp->stats.rx_missed_errors += (readreg(dev, PP_RxMiss) >> 6);
	lp->stats.collisions += (readreg(dev, PP_TxCol) >> 6);
	spin_unlock_irqrestore(&lp->lock, flags);

	return &lp->stats;
}

static void set_multicast_list(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *)dev->priv;
	unsigned long flags;

	spin_lock_irqsave(&lp->lock, flags);
	if(dev->flags&IFF_PROMISC)
	{
		lp->rx_mode = RX_ALL_ACCEPT;
	}
	else if((dev->flags&IFF_ALLMULTI)||dev->mc_list)
	{
		/* The multicast-accept list is initialized to accept-all, and we
		   rely on higher-level filtering for now. */
		lp->rx_mode = RX_MULTCAST_ACCEPT;
	} 
	else
		lp->rx_mode = 0;

	writereg(dev, PP_RxCTL, DEF_RX_ACCEPT | lp->rx_mode);

	/* in promiscuous mode, we accept errored packets, so we have to enable interrupts on them also */
	writereg(dev, PP_RxCFG, lp->curr_rx_cfg |
	     (lp->rx_mode == RX_ALL_ACCEPT? (RX_CRC_ERROR_ENBL|RX_RUNT_ENBL|RX_EXTRA_DATA_ENBL) : 0));
	spin_unlock_irqrestore(&lp->lock, flags);
}


static int set_mac_address(struct net_device *dev, void *addr)
{
	int i;

	if (netif_running(dev))
		return -EBUSY;
	if (net_debug)
	{
		printk("%s: Setting MAC address to ", dev->name);
		for (i = 0; i < 6; i++)
			printk(" %2.2x", dev->dev_addr[i] = ((unsigned char *)addr)[i]);
		printk(".\n");
	}
	/* set the Ethernet address */
	for (i=0; i < ETH_ALEN/2; i++)
		writereg(dev, PP_IA+i*2, dev->dev_addr[i*2] | (dev->dev_addr[i*2+1] << 8));

	return 0;
}



static char namespace[16] = "";
static struct net_device dev_cs89x0 = {
        "",
        0, 0, 0, 0,
        0, 0,
        0, 0, 0, NULL, NULL };

/*
 * Support the 'debug' module parm even if we're compiled for non-debug to 
 * avoid breaking someone's startup scripts 
 */

static int debug = 1;
static char media[8];
static int duplex = 0;


int
init_s3c2410(void)
{
	struct net_local *lp;

#if DEBUGGING
	net_debug = debug;
#endif

	dev_cs89x0.init = cerf89x0_probe;
	dev_cs89x0.priv = kmalloc(sizeof(struct net_local), GFP_KERNEL);
	if (dev_cs89x0.priv == 0)
	{
		printk(KERN_ERR "cs89x0.c: Out of memory.\n");
		return -ENOMEM;
	}
	memset(dev_cs89x0.priv, 0, sizeof(struct net_local));
	lp = (struct net_local *)dev_cs89x0.priv;

	spin_lock_init(&lp->lock);

	if (register_netdev(&dev_cs89x0) != 0) {
		printk(KERN_ERR "cerf89x0.c: No chip found \n");
		return -ENXIO;
	}
    return 0;
}

void
cleanup_s3c2410(void) 
{
	writeword(&dev_cs89x0, ADD_PORT, PP_ChipID);
	if (dev_cs89x0.priv != NULL) {
		/* Free up the private structure, or leak memory :-)  */
		unregister_netdev(&dev_cs89x0);
		kfree(dev_cs89x0.priv);
		dev_cs89x0.priv = NULL;	/* gets re-allocated by cerf89x0_probe1 */
		/* If we don't do this, we can't re-insmod it later. */
		release_region(dev_cs89x0.base_addr, NETCARD_IO_EXTENT);
	}
}


module_init(init_s3c2410);
module_exit(cleanup_s3c2410);
