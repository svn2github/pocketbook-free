/*
 * cs8900a.c: A Crystal Semiconductor (Now Cirrus Logic) CS8900A
		driver for SMDK-s3c2410 (based on cs89x0.c)
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:56:28 $ 
 *
 * $Revision: 1.1.1.1 $

   Wed Aug 14 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial, based on cs89x0.c

   Wed Aug 16 2002 Yong-iL Joh <tolkien@mizi.com>
   - working!

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>

#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>

#define IRQ_LAN        IRQ_CS8900
#include "cs89x0.h"

/*
 * Set this to zero to remove all the debug statements via
 * dead code elimination
 */
#undef DEBUGGING	4
#if DEBUGGING
#define DPRINTK(n, args...)	\
	if (n <= DEBUGGING) {	\
		printk(args);	\
	}
#else
#define DPRINTK(n, args...)
#endif

#if DEBUGGING
static char version[] __initdata =
"cs89x0.c: v2.4.3-pre1 Russell Nelson <nelson@crynwr.com>, Andrew Morton <andrewm@uow.edu.au>\n";
#endif

/* First, a few definitions that the brave might change.
   A zero-terminated list of I/O addresses to be probed. Some special flags..
      Addr & 1 = Read back the address port, look for signature and reset
                 the page window before probing 
      Addr & 3 = Reset the page window and probe 
   The CLPS eval board has the Cirrus chip at 0x80090300, in ARM IO space,
   but it is possible that a Cirrus board could be plugged into the ISA
   slots. */
static unsigned int netcard_portlist[] __initdata =
   { vCS8900_BASE + 0x300, 0};

/* The number of low I/O ports used by the ethercard. */
#define NETCARD_IO_EXTENT	0xfff

/* we allow the user to override various values normally set in the EEPROM */
#define FORCE_RJ45	0x0001    /* pick one of these three */
#define FORCE_AUI	0x0002
#define FORCE_BNC	0x0004

#define FORCE_AUTO	0x0010    /* pick one of these three */
#define FORCE_HALF	0x0020
#define FORCE_FULL	0x0030

/* Information that need to be kept for each board. */
struct net_local {
	struct net_device_stats stats;
	int chip_type;		/* one of: CS8900, CS8920, CS8920M */
	char chip_revision;	/* revision letter of the chip ('A'...) */
	int send_cmd;		/* the proper send command:
				   TX_NOW, TX_AFTER_381, or TX_AFTER_ALL */
	int auto_neg_cnf;	/* auto-negotiation word from EEPROM */
	int adapter_cnf;	/* adapter configuration from EEPROM */
	int isa_config;		/* ISA configuration from EEPROM */
	int irq_map;		/* IRQ map from EEPROM */
	int rx_mode;		/* what mode are we in?
				   0, RX_MULTCAST_ACCEPT, or RX_ALL_ACCEPT */
	int curr_rx_cfg;	/* a copy of PP_RxCFG */
	int linectl;		/* either 0 or LOW_RX_SQUELCH,
				   depending on configuration. */
	int send_underrun;	/* keep track of how many underruns
				   in a row we get */
	int force;		/* force various values; see FORCE* above. */
	spinlock_t lock;
};

/* Index to functions, as function prototypes. */

extern int cs89x0_probe(struct net_device *dev);
static int cs89x0_probe1(struct net_device *dev, int ioaddr);
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
#define tx_done(dev)	1

/* Check for a network adaptor of this type, and return '0' iff one exists.
   If dev->base_addr == 0, probe all likely locations.
   If dev->base_addr == 1, always return failure.
   If dev->base_addr == 2, allocate space for the device and return success
   (detachable devices only).
   Return 0 on success.
   */
int __init cs89x0_probe(struct net_device *dev) {
    int i;

    SET_MODULE_OWNER(dev);
    DPRINTK(1, "cs89x0:cs89x0_probe(0x%x)\n", base_addr);

    BWSCON = (BWSCON & ~(BWSCON_ST3 | BWSCON_WS3 | BWSCON_DW3)) |
      (BWSCON_ST3 | BWSCON_WS3 | BWSCON_DW(3, BWSCON_DW_16));
    BANKCON3= BANKCON_Tacs0 | BANKCON_Tcos4 | BANKCON_Tacc14 |
      BANKCON_Toch1 | BANKCON_Tcah4 | BANKCON_Tacp6 | BANKCON_PMC16;

    set_external_irq(IRQ_CS8900, EXT_RISING_EDGE, GPIO_PULLUP_DIS);

    for (i = 0; netcard_portlist[i]; i++) {
      if (cs89x0_probe1(dev, netcard_portlist[i]) == 0)
	return 0;
    }
    printk(KERN_WARNING "cs89x0: no cs8900 or cs8920 detected."
	   "Be sure to disable PnP with SETUP\n");
    return -ENODEV;
}

inline int readreg(struct net_device *dev, int portno) {
	outw(portno, dev->base_addr + ADD_PORT);
	return inw(dev->base_addr + DATA_PORT);
}

inline void writereg(struct net_device *dev, int portno, int value) {
	outw(portno, dev->base_addr + ADD_PORT);
	outw(value, dev->base_addr + DATA_PORT);
}

inline int readword(struct net_device *dev, int portno) {
	return inw(dev->base_addr + portno);
}

inline void writeword(struct net_device *dev, int portno, int value) {
	outw(value, dev->base_addr + portno);
}

inline void writeblock(struct net_device *dev, char *pData, int Length) {
    int i;

    for (i = 0 ; i < (Length/2); i++) {
        writeword(dev, TX_FRAME_PORT, *(u16 *)pData );
        pData += 2;
    }

    if (Length % 2) {
        u16 OddWordValue = *pData;
        writeword(dev, TX_FRAME_PORT, OddWordValue);
    }
}

inline void readblock(struct net_device *dev, char *pData, int Length) {
    u16 InputWord;
    int i;

    for (i=0; i < (Length/2); i++) {
        InputWord = readword(dev, RX_FRAME_PORT);
        *(u8*)pData++ = (u8) InputWord & 0xFF;
        *(u8*)pData++ = (u8) (InputWord >> 8) & 0xFF;
    }

    if (Length & 0x1)
      *pData = (u8) (readword(dev, RX_FRAME_PORT) & 0xff);
}

/* This is the real probe routine.  Linux has a history of friendly device
   probes on the ISA bus.  A good device probes avoids doing writes, and
   verifies that the correct device exists and functions.
   Return 0 on success.
 */

static int __init cs89x0_probe1(struct net_device *dev, int ioaddr) {
    struct net_local *lp;
#if DEBUGGING
    static unsigned version_printed;
#endif
    unsigned rev_type = 0;
    int ret;

    /* Initialize the device structure. */
    if (dev->priv == NULL) {
      dev->priv = kmalloc(sizeof(struct net_local), GFP_KERNEL);
      if (dev->priv == 0) {
	ret = -ENOMEM;
	goto before_kmalloc;
      }
      lp = (struct net_local *)dev->priv;
      memset(lp, 0, sizeof(*lp));
      spin_lock_init(&lp->lock);
    }
    lp = (struct net_local *)dev->priv;

    /* Fill in the 'dev' fields. */
    dev->base_addr = ioaddr;

    /*  Bus Reset Consideration  */
    ret = readword(dev, ADD_PORT);
    if ((ret & ADD_MASK) != ADD_SIG ) {
      DPRINTK(1, __FUNCTION__ " 0x%08X\n", ret);
      ret = -ENODEV;
      goto after_kmalloc;
    }

    /* get the chip type */
    rev_type = readreg(dev, PRODUCT_ID_ADD);
    lp->chip_type = rev_type &~ REVISON_BITS;
    lp->chip_revision = ((rev_type & REVISON_BITS) >> 8) + 'A';

#if DEBUGGING
    if (version_printed++ == 0)
      printk(version);
#endif

    printk(KERN_INFO "%s: cs89%c0%s rev %c(%s) found at %#3lx\n",
	   dev->name,
	   lp->chip_type==CS8900 ? '0' : '2',
	   lp->chip_type==CS8920M ? "M" : "",
	   lp->chip_revision,
	   readreg(dev, PP_SelfST) & ACTIVE_33V ? "3.3 Volts" : "5 Volts",
	   dev->base_addr);
    if (lp->chip_type != CS8900) {
      printk(__FILE__ ": wrong device driver!\n");
      ret = -ENODEV;
      goto after_kmalloc;
    }

    /* Check the chip type and revision in order to
       set the correct send command
       CS8900 revision F can use the faster send. */
    lp->send_cmd = TX_AFTER_ALL;
    if (lp->chip_type == CS8900 && lp->chip_revision >= 'F')
      lp->send_cmd = TX_NOW;

    reset_chip(dev);

    lp->adapter_cnf = A_CNF_10B_T | A_CNF_MEDIA_10B_T;
    lp->auto_neg_cnf = EE_AUTO_NEG_ENABLE;

    printk(KERN_INFO "cs89x0 media %s%s",
	   (lp->adapter_cnf & A_CNF_10B_T)?"RJ-45":"",
	   (lp->adapter_cnf & A_CNF_AUI)?"AUI":"");

    dev->dev_addr[0] = 0x00;
    dev->dev_addr[1] = 0x00;
    dev->dev_addr[2] = 0xc0;
    dev->dev_addr[3] = 0xff;
    dev->dev_addr[4] = 0xee;
    dev->dev_addr[5] = 0x08;
    set_mac_address(dev, dev->dev_addr);

    dev->irq = IRQ_LAN;
    printk(", IRQ %d", dev->irq);

    dev->open		= net_open;
    dev->stop		= net_close;
    dev->tx_timeout	= net_timeout;
    dev->watchdog_timeo	= 3 * HZ;
    dev->hard_start_xmit	= net_send_packet;
    dev->get_stats		= net_get_stats;
    dev->set_multicast_list	= set_multicast_list;
    dev->set_mac_address 	= set_mac_address;

    /* Fill in the fields of the device structure with ethernet values. */
    ether_setup(dev);

    printk("\n");
    DPRINTK(1, "cs89x0_probe1() successful\n");
    return 0;

 after_kmalloc:
    kfree(dev->priv);
 before_kmalloc:
    return ret;
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
static int net_open(struct net_device *dev)
{
    struct net_local *lp = (struct net_local *)dev->priv;
    int ret;

    /* Prevent the crystal chip from generating interrupts */
    writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) & ~ENABLE_IRQ);
    ret = request_irq(dev->irq, &net_interrupt, SA_SHIRQ, "cs89x0", dev);
    if (ret) {
      printk("%s: request_irq(%d) failed\n", dev->name, dev->irq);
      goto bad_out;
    }
    /* Set up the IRQ - Apparently magic */
    if (lp->chip_type == CS8900)
      writereg(dev, PP_CS8900_ISAINT, 0);
    else
      writereg(dev, PP_CS8920_ISAINT, 0);

    /* while we're testing the interface, leave interrupts disabled */
    writereg(dev, PP_BusCTL, MEMORY_ON);

    /* Set the LineCTL quintuplet */
    lp->linectl = 0;

    /* Turn on both receive and transmit operations */
    writereg(dev, PP_LineCTL,
	     readreg(dev, PP_LineCTL) | SERIAL_RX_ON | SERIAL_TX_ON);

    /* Receive only error free packets addressed to this card */
    lp->rx_mode = 0;
    writereg(dev, PP_RxCTL, DEF_RX_ACCEPT);

    lp->curr_rx_cfg = RX_OK_ENBL | RX_CRC_ERROR_ENBL;

    if (lp->isa_config & STREAM_TRANSFER)
      lp->curr_rx_cfg |= RX_STREAM_ENBL;
    writereg(dev, PP_RxCFG, lp->curr_rx_cfg);

    writereg(dev, PP_TxCFG,
	     TX_LOST_CRS_ENBL | TX_SQE_ERROR_ENBL | TX_OK_ENBL |
	     TX_LATE_COL_ENBL | TX_JBR_ENBL |
	     TX_ANY_COL_ENBL | TX_16_COL_ENBL);

    writereg(dev, PP_BufCFG,
	     READY_FOR_TX_ENBL | RX_MISS_COUNT_OVRFLOW_ENBL |
	     TX_COL_COUNT_OVRFLOW_ENBL | TX_UNDERRUN_ENBL);

    /* now that we've got our act together, enable everything */
    writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) | ENABLE_IRQ);
    enable_irq(dev->irq);

    netif_start_queue(dev);
    DPRINTK(1, "cs89x0: net_open() succeeded\n");
    return 0;

 bad_out:
    return ret;
}

static void net_timeout(struct net_device *dev)
{
  /* If we get here, some higher level has decided we are broken.
     There should really be a "kick me" function call instead. */
    DPRINTK(1, "%s: transmit timed out, %s?\n", dev->name,
	    tx_done(dev) ? "IRQ conflict ?" : "network cable problem");
    /* Try to restart the adaptor. */
    //netif_wake_queue(dev);
    net_close(dev);
    writereg(dev, PP_SelfCTL, readreg(dev, PP_SelfCTL) | POWER_ON_RESET);
    net_open(dev);
}

static int net_send_packet(struct sk_buff *skb, struct net_device *dev)
{
    struct net_local *lp = (struct net_local *)dev->priv;

    writereg(dev, PP_BusCTL, 0x0);
    writereg(dev, PP_BusCTL, readreg(dev, PP_BusCTL) | ENABLE_IRQ);

    DPRINTK(3, "%s: sent %d byte packet of type %x\n",
	    dev->name, skb->len,
	    (skb->data[ETH_ALEN+ETH_ALEN] << 8) |
	    (skb->data[ETH_ALEN+ETH_ALEN+1]));

    /* keep the upload from being interrupted, since we
       ask the chip to start transmitting before the
       whole packet has been completely uploaded. */

    spin_lock_irq(&lp->lock);
    netif_stop_queue(dev);

    /* initiate a transmit sequence */
    writeword(dev, TX_CMD_PORT, lp->send_cmd);
    writeword(dev, TX_LEN_PORT, skb->len);

    /* Test to see if the chip has allocated memory for the packet */
    if ((readreg(dev, PP_BusST) & READY_FOR_TX_NOW) == 0) {
      /*
       * Gasp!  It hasn't.  But that shouldn't happen since
       * we're waiting for TxOk, so return 1 and requeue this packet.
       */
		
      spin_unlock_irq(&lp->lock);
      DPRINTK(1, "cs89x0: Tx buffer not free!\n");
      return 1;
    }
    /* Write the contents of the packet */
    writeblock(dev, skb->data, skb->len);

    spin_unlock_irq(&lp->lock);
    dev->trans_start = jiffies;
    dev_kfree_skb (skb);

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

    ioaddr = dev->base_addr;
    lp = (struct net_local *)dev->priv;

    /* we MUST read all the events out of the ISQ, otherwise we'll never
       get interrupted again.  As a consequence, we can't have any limit
       on the number of times we loop in the interrupt handler.  The
       hardware guarantees that eventually we'll run out of events.  Of
       course, if you're on a slow machine, and packets are arriving
       faster than you can read them off, you're screwed.  Hasta la
       vista, baby!  */
    while ((status = readword(dev, ISQ_PORT))) {
      DPRINTK(4, "%s: event=%04x\n", dev->name, status);
      switch(status & ISQ_EVENT_MASK) {
      case ISQ_RECEIVER_EVENT:
	/* Got a packet(s). */
	net_rx(dev);
	break;
      case ISQ_TRANSMITTER_EVENT:
	lp->stats.tx_packets++;
	netif_wake_queue(dev);	/* Inform upper layers. */
	if ((status & (	TX_OK |
			TX_LOST_CRS | TX_SQE_ERROR |
			TX_LATE_COL | TX_16_COL)) != TX_OK) {
	  if ((status & TX_OK) == 0) lp->stats.tx_errors++;
	  if (status & TX_LOST_CRS) lp->stats.tx_carrier_errors++;
	  if (status & TX_SQE_ERROR) lp->stats.tx_heartbeat_errors++;
	  if (status & TX_LATE_COL) lp->stats.tx_window_errors++;
	  if (status & TX_16_COL) lp->stats.tx_aborted_errors++;
	}
	break;
      case ISQ_BUFFER_EVENT:
	if (status & READY_FOR_TX) {
	  /* we tried to transmit a packet earlier,
	     but inexplicably ran out of buffers.
	     That shouldn't happen since we only ever
	     load one packet.  Shrug.  Do the right
	     thing anyway. */
	  netif_wake_queue(dev);	/* Inform upper layers. */
	}
	if (status & TX_UNDERRUN) {
	  DPRINTK(1, "%s: transmit underrun\n", dev->name);
	  lp->send_underrun++;
	  if (lp->send_underrun == 3)		lp->send_cmd = TX_AFTER_381;
	  else if (lp->send_underrun == 6)	lp->send_cmd = TX_AFTER_ALL;
	  /* transmit cycle is done, although
	     frame wasn't transmitted - this
	     avoids having to wait for the upper
	     layers to timeout on us, in the
	     event of a tx underrun */
	  netif_wake_queue(dev);	/* Inform upper layers. */
	}
	break;
      case ISQ_RX_MISS_EVENT:
	lp->stats.rx_missed_errors += (status >>6);
	break;
      case ISQ_TX_COL_EVENT:
	lp->stats.collisions += (status >>6);
	break;
      }
    }
}

static void count_rx_errors(int status, struct net_local *lp) {
    lp->stats.rx_errors++;
    if (status & RX_RUNT) lp->stats.rx_length_errors++;
    if (status & RX_EXTRA_DATA) lp->stats.rx_length_errors++;
    if (status & RX_CRC_ERROR) if (!(status & (RX_EXTRA_DATA|RX_RUNT)))
      /* per str 172 */
      lp->stats.rx_crc_errors++;
    if (status & RX_DRIBBLE) lp->stats.rx_frame_errors++;
    return;
}

/* We have a good packet(s), get it/them out of the buffers. */
static void net_rx(struct net_device *dev) {
    struct net_local *lp = (struct net_local *)dev->priv;
    struct sk_buff *skb;
    int status, length;

    int ioaddr = dev->base_addr;

    status = inw(ioaddr + RX_FRAME_PORT);
    if ((status & RX_OK) == 0) {
      count_rx_errors(status, lp);
      return;
    }

    length = inw(ioaddr + RX_FRAME_PORT);

    /* Malloc up new buffer. */
    skb = dev_alloc_skb(length + 2);
    if (skb == NULL) {
      lp->stats.rx_dropped++;
      return;
    }
    skb_reserve(skb, 2);	/* longword align L3 header */
    skb->len = length;
    skb->dev = dev;
    readblock(dev, skb->data, skb->len);

    DPRINTK(3, "%s: received %d byte packet of type %x\n",
	    dev->name, length,
	    (skb->data[ETH_ALEN+ETH_ALEN] << 8) | skb->data[ETH_ALEN+ETH_ALEN+1]);

    skb->protocol=eth_type_trans(skb,dev);
    netif_rx(skb);
    dev->last_rx = jiffies;
    lp->stats.rx_packets++;
    lp->stats.rx_bytes += length;
}

/* The inverse routine to net_open(). */
static int net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	
	writereg(dev, PP_RxCFG, 0);
	writereg(dev, PP_TxCFG, 0);
	writereg(dev, PP_BufCFG, 0);
	writereg(dev, PP_BusCTL, 0);

	free_irq(dev->irq, dev);

	/* Update the statistics here. */
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
	if (dev->flags&IFF_PROMISC) {
		lp->rx_mode = RX_ALL_ACCEPT;
	} else if((dev->flags&IFF_ALLMULTI)||dev->mc_list) {
		/* The multicast-accept list is initialized to accept-all,
		   and we rely on higher-level filtering for now. */
		lp->rx_mode = RX_MULTCAST_ACCEPT;
	} else
		lp->rx_mode = 0;

	writereg(dev, PP_RxCTL, DEF_RX_ACCEPT | lp->rx_mode);

	/* in promiscuous mode, we accept errored packets,
	   so we have to enable interrupts on them also */
	writereg(dev, PP_RxCFG, lp->curr_rx_cfg |
		 (lp->rx_mode == RX_ALL_ACCEPT ?
		  (RX_CRC_ERROR_ENBL|RX_RUNT_ENBL|RX_EXTRA_DATA_ENBL) : 0));
	spin_unlock_irqrestore(&lp->lock, flags);
}


static int set_mac_address(struct net_device *dev, void *addr)
{
    int i;

    if (netif_running(dev))
      return -EBUSY;

    DPRINTK(1, "%s: Setting MAC address to ", dev->name);
    for (i = 0; i < 6; i++) {
      dev->dev_addr[i] = ((unsigned char *)addr)[i];
      DPRINTK(1, " %2.2x", dev->dev_addr[i]);
    }
    DPRINTK(1, ".\n");

    /* set the Ethernet address */
    for (i=0; i < ETH_ALEN/2; i++)
      writereg(dev, PP_IA+i*2, dev->dev_addr[i*2] | (dev->dev_addr[i*2+1] << 8));
    return 0;
}

#ifdef MODULE
static struct net_device dev_cs89x0 = {
        "",
        0, 0, 0, 0,
        0, 0,
        0, 0, 0, NULL, NULL };

/*
 * Support the 'debug' module parm even if we're compiled for non-debug to 
 * avoid breaking someone's startup scripts 
 */

static int io = 0xd0000300;
static int irq = IRQ_LAN;
static char media[8];
static int duplex= 0;
MODULE_PARM(io, "i");
MODULE_PARM(irq, "i");
MODULE_PARM(media, "c8");
MODULE_PARM(duplex, "i");
MODULE_PARM_DESC(io, "cs89x0 I/O base address");
MODULE_PARM_DESC(irq, "cs89x0 IRQ number");
MODULE_PARM_DESC(media, "Set cs89x0 adapter(s) media type(s) (rj45,bnc,aui)");
/* No other value than -1 for duplex seems to be currently interpreted */
MODULE_PARM_DESC(duplex, "(ignored)");

MODULE_AUTHOR("Mike Cruse, Russwll Nelson <nelson@crynwr.com>, Andrew Morton <andrewm@uow.edu.au>");
MODULE_LICENSE("GPL");


EXPORT_NO_SYMBOLS;

/*
* media=t             - specify media type
   or media=2
   or media=aui
   or medai=auto
* duplex=0            - specify forced half/full/autonegotiate duplex
* debug=#             - debug level


* Default Chip Configuration:
  * DMA Burst = enabled
  * IOCHRDY Enabled = enabled
    * UseSA = enabled
    * CS8900 defaults to half-duplex if not specified on command-line
    * CS8920 defaults to autoneg if not specified on command-line
    * Use reset defaults for other config parameters

* Assumptions:
  * media type specified is supported (circuitry is present)
  * if memory address is > 1MB, then required mem decode hw is present
  * if 10B-2, then agent other than driver will enable DC/DC converter
    (hw or software util)


*/

static int __init init_cs8900a_s3c2410(void) {
    struct net_local *lp;
    int ret = 0;

    dev_cs89x0.irq = irq;
    dev_cs89x0.base_addr = io;

    dev_cs89x0.init = cs89x0_probe;
    dev_cs89x0.priv = kmalloc(sizeof(struct net_local), GFP_KERNEL);
    if (dev_cs89x0.priv == 0) {
      printk(KERN_ERR "cs89x0.c: Out of memory.\n");
      return -ENOMEM;
    }
    memset(dev_cs89x0.priv, 0, sizeof(struct net_local));
    lp = (struct net_local *)dev_cs89x0.priv;

    request_region(dev_cs89x0.base_addr, NETCARD_IO_EXTENT, "cs8900a");

    spin_lock_init(&lp->lock);

    /* boy, they'd better get these right */
    if (!strcmp(media, "rj45"))
      lp->adapter_cnf = A_CNF_MEDIA_10B_T | A_CNF_10B_T;
    else if (!strcmp(media, "aui"))
      lp->adapter_cnf = A_CNF_MEDIA_AUI   | A_CNF_AUI;
    else if (!strcmp(media, "bnc"))
      lp->adapter_cnf = A_CNF_MEDIA_10B_2 | A_CNF_10B_2;
    else
      lp->adapter_cnf = A_CNF_MEDIA_10B_T | A_CNF_10B_T;

    if (duplex==-1)
      lp->auto_neg_cnf = AUTO_NEG_ENABLE;

    if (io == 0) {
      printk(KERN_ERR "cs89x0.c: Module autoprobing not allowed.\n");
      printk(KERN_ERR "cs89x0.c: Append io=0xNNN\n");
      ret = -EPERM;
      goto out;
    }

    if (register_netdev(&dev_cs89x0) != 0) {
      printk(KERN_ERR "cs89x0.c: No card found at 0x%x\n", io);
      ret = -ENXIO;
      goto out;
    }
out:
    if (ret)
      kfree(dev_cs89x0.priv);
    return ret;
}

static void __exit cleanup_cs8900a_s3c2410(void) {
    if (dev_cs89x0.priv != NULL) {
      /* Free up the private structure, or leak memory :-)  */
      unregister_netdev(&dev_cs89x0);
      outw(PP_ChipID, dev_cs89x0.base_addr + ADD_PORT);
      kfree(dev_cs89x0.priv);
      dev_cs89x0.priv = NULL;	/* gets re-allocated by cs89x0_probe1 */
      /* If we don't do this, we can't re-insmod it later. */
      release_region(dev_cs89x0.base_addr, NETCARD_IO_EXTENT);
    }
}

module_init(init_cs8900a_s3c2410);
module_exit(cleanup_cs8900a_s3c2410);
#endif

/*
 | $Id: cs8900a.c,v 1.1.1.1 2004/02/04 12:56:28 laputa Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
