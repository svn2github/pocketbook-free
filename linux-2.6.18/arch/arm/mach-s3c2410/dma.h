/*
 * linux/arch/arm/mach-s3c2410/dma.h
 *
 * Copyright (C) 2002 MIZI Research, Inc
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 12:55:27 $
 *
 * $Revision: 1.1.1.1 $
 *
 * History:
 *
 * 2002-05-28: Janghoon Lyu <nandy@mizi.com>
 *    - copy from linux/arch/arm/mach-s3c2400
 *
 */


/* DMA buffer struct */
typedef struct dma_buf_s {
	int size;		/* buffer size */
	dma_addr_t dma_start;	/* starting DMA address */
	int ref;		/* number of DMA references */
	void *id;		/* to identify buffer from outside */
	int write;		/* 1: buf to write , 0: but to read  */
	struct dma_buf_s *next;	/* next buf to process */
} dma_buf_t;

/* DMA control register structure */
typedef struct {
	volatile u_long DISRC;
	volatile u_long DISRCC;
	volatile u_long DIDST;
	volatile u_long DIDSTC;
	volatile u_long DCON;
	volatile u_long DSTAT;
	volatile u_long DCSRC;
	volatile u_long DCDST;
	volatile u_long DMASKTRIG;
} dma_regs_t;

/* DMA device structre */
typedef struct {
	dma_callback_t callback;
	u_long dst;
	u_long src;
	u_long ctl;
	u_long dst_ctl;
	u_long src_ctl;
} dma_device_t;

/* DMA channel structure */
typedef struct {
	dmach_t channel;
	unsigned int in_use;	/* Device is allocated */
	const char *device_id;	/* Device name */
	dma_buf_t *head;	/* where to insert buffers */
	dma_buf_t *tail;	/* where to remove buffers */
	dma_buf_t *curr;	/* buffer currently DMA'ed */
	unsigned long queue_count;	/* number of buffers in the queue */
	int active;		/* 1 if DMA is actually processing data */
	dma_regs_t *regs;	/* points to appropriate DMA registers */
	int irq;		/* IRQ used by the channel */
	dma_device_t write;	/* to write */
	dma_device_t read;	/* to read */
} s3c2410_dma_t;

s3c2410_dma_t dma_chan[MAX_S3C2410_DMA_CHANNELS];


typedef struct {
	const char *name;
	u_long write_src;
	u_long write_dst;
	u_long write_ctl;
	u_long write_src_ctl;
	u_long write_dst_ctl;
	u_long read_src;
	u_long read_dst;
	u_long read_ctl;
	u_long read_src_ctl;
	u_long read_dst_ctl;
} dma_type_t;

#define DMA_UNDEF		0xffffffff	/* not available */
#define BUF_ON_MEM		(ON_AHB | ADDR_INC)
#define BUF_ON_APB		(ON_APB	| ADDR_FIX)

#define UART0_MEM		0x0
#define UART0_CTL		(HS_MODE | SYNC_PCLK | INT_MODE | TSZ_UNIT | \
				 SINGLE_SERVICE | HWSRC(CH0_UART0) | DMA_SRC_HW | \
				 CLR_ATRELOAD | DSZ(DSZ_BYTE) | TX_CNT(0))
#define UART0_WR_SRC		UART0_MEM
#define UART0_WR_DST		0x50000020	/* UTXH0 */
#define UART0_WR_CTL		UART0_CTL
#define UART0_RD_SRC		0x50000024	/* URXH0 */
#define UART0_RD_DST		UART0_MEM
#define UART0_RD_CTL		UART0_CTL
#define UART0_WR_SRC_CTL	BUF_ON_MEM
#define UART0_WR_DST_CTL	BUF_ON_APB
#define UART0_RD_SRC_CTL	BUF_ON_APB
#define UART0_RD_DST_CTL	BUF_ON_MEM

#define UART1_WR_SRC		DMA_UNDEF
#define UART1_WR_DST		DMA_UNDEF
#define UART1_WR_CTL		DMA_UNDEF
#define UART1_RD_SRC		DMA_UNDEF
#define UART1_RD_DST		DMA_UNDEF
#define UART1_RD_CTL		DMA_UNDEF
#define UART1_WR_SRC_CTL	DMA_UNDEF		
#define UART1_WR_DST_CTL	DMA_UNDEF
#define UART1_RD_SRC_CTL	DMA_UNDEF
#define UART1_RD_DST_CTL	DMA_UNDEF

#define UART2_WR_SRC		DMA_UNDEF
#define UART2_WR_DST		DMA_UNDEF
#define UART2_WR_CTL		DMA_UNDEF
#define UART2_RD_SRC		DMA_UNDEF
#define UART2_RD_DST		DMA_UNDEF
#define UART2_RD_CTL		DMA_UNDEF
#define UART2_WR_SRC_CTL	DMA_UNDEF		
#define UART2_WR_DST_CTL	DMA_UNDEF
#define UART2_RD_SRC_CTL	DMA_UNDEF
#define UART2_RD_DST_CTL	DMA_UNDEF

#define I2SSDO_CTL		(HS_MODE | SYNC_PCLK | INT_MODE | TSZ_UNIT |  \
				SINGLE_SERVICE | HWSRC(CH2_I2SSDO) | DMA_SRC_HW | \
				CLR_ATRELOAD | DSZ(DSZ_HALFWORD) | TX_CNT(0))

#define I2SSDO_WR_SRC		0x0
#define I2SSDO_WR_DST		0x55000010
#define I2SSDO_WR_CTL		I2SSDO_CTL
#define I2SSDO_RD_SRC		DMA_UNDEF
#define I2SSDO_RD_DST		DMA_UNDEF
#define I2SSDO_RD_CTL		DMA_UNDEF
#define I2SSDO_WR_SRC_CTL	BUF_ON_MEM	
#define I2SSDO_WR_DST_CTL	BUF_ON_APB
#define I2SSDO_RD_SRC_CTL	DMA_UNDEF
#define I2SSDO_RD_DST_CTL	DMA_UNDEF

#define I2SSDI_WR_SRC		DMA_UNDEF
#define I2SSDI_WR_SRC		DMA_UNDEF
#define I2SSDI_WR_DST		DMA_UNDEF
#define I2SSDI_WR_CTL		DMA_UNDEF
#define I2SSDI_RD_SRC		DMA_UNDEF
#define I2SSDI_RD_DST		DMA_UNDEF
#define I2SSDI_RD_CTL		DMA_UNDEF
#define I2SSDI_WR_SRC_CTL	DMA_UNDEF		
#define I2SSDI_WR_DST_CTL	DMA_UNDEF
#define I2SSDI_RD_SRC_CTL	DMA_UNDEF
#define I2SSDI_RD_DST_CTL	DMA_UNDEF

#define USB_WR_SRC		DMA_UNDEF
#define USB_WR_DST		DMA_UNDEF
#define USB_WR_CTL		DMA_UNDEF
#define USB_RD_SRC		DMA_UNDEF
#define USB_RD_DST		DMA_UNDEF
#define USB_RD_CTL		DMA_UNDEF
#define USB_WR_SRC_CTL		DMA_UNDEF		
#define USB_WR_DST_CTL		DMA_UNDEF
#define USB_RD_SRC_CTL		DMA_UNDEF
#define USB_RD_DST_CTL		DMA_UNDEF

#define MMC_WR_SRC		DMA_UNDEF
#define MMC_WR_DST		DMA_UNDEF
#define MMC_WR_CTL		DMA_UNDEF
#define MMC_RD_SRC		DMA_UNDEF
#define MMC_RD_DST		DMA_UNDEF
#define MMC_RD_CTL		DMA_UNDEF
#define MMC_WR_SRC_CTL		DMA_UNDEF		
#define MMC_WR_DST_CTL		DMA_UNDEF
#define MMC_RD_SRC_CTL		DMA_UNDEF
#define MMC_RD_DST_CTL		DMA_UNDEF

#define SPI_WR_SRC		DMA_UNDEF
#define SPI_WR_DST		DMA_UNDEF
#define SPI_WR_CTL		DMA_UNDEF
#define SPI_RD_SRC		DMA_UNDEF
#define SPI_RD_DST		DMA_UNDEF
#define SPI_RD_CTL		DMA_UNDEF
#define SPI_WR_SRC_CTL		DMA_UNDEF		
#define SPI_WR_DST_CTL		DMA_UNDEF
#define SPI_RD_SRC_CTL		DMA_UNDEF
#define SPI_RD_DST_CTL		DMA_UNDEF

#define TIMER_WR_SRC		DMA_UNDEF
#define TIMER_WR_DST		DMA_UNDEF
#define TIMER_WR_CTL		DMA_UNDEF
#define TIMER_RD_SRC		DMA_UNDEF
#define TIMER_RD_DST		DMA_UNDEF
#define TIMER_RD_CTL		DMA_UNDEF
#define TIMER_WR_SRC_CTL	DMA_UNDEF		
#define TIMER_WR_DST_CTL	DMA_UNDEF
#define TIMER_RD_SRC_CTL	DMA_UNDEF
#define TIMER_RD_DST_CTL	DMA_UNDEF

#define XDREQ0_WR_SRC		DMA_UNDEF
#define XDREQ0_WR_DST		DMA_UNDEF
#define XDREQ0_WR_CTL		DMA_UNDEF
#define XDREQ0_RD_SRC		DMA_UNDEF
#define XDREQ0_RD_DST		DMA_UNDEF
#define XDREQ0_RD_CTL		DMA_UNDEF
#define XDREQ0_WR_SRC_CTL	DMA_UNDEF		
#define XDREQ0_WR_DST_CTL	DMA_UNDEF
#define XDREQ0_RD_SRC_CTL	DMA_UNDEF
#define XDREQ0_RD_DST_CTL	DMA_UNDEF

#define XDREQ1_WR_SRC		DMA_UNDEF
#define XDREQ1_WR_DST		DMA_UNDEF
#define XDREQ1_WR_CTL		DMA_UNDEF
#define XDREQ1_RD_SRC		DMA_UNDEF
#define XDREQ1_RD_DST		DMA_UNDEF
#define XDREQ1_RD_CTL		DMA_UNDEF
#define XDREQ1_WR_SRC_CTL	DMA_UNDEF		
#define XDREQ1_WR_DST_CTL	DMA_UNDEF
#define XDREQ1_RD_SRC_CTL	DMA_UNDEF
#define XDREQ1_RD_DST_CTL	DMA_UNDEF

static dma_type_t dma_types[4][5] = {
{
	{ "XDREQ0", XDREQ0_WR_SRC, XDREQ0_WR_DST, XDREQ0_WR_CTL, \
		    XDREQ0_WR_SRC_CTL, XDREQ0_WR_DST_CTL, \
		    XDREQ0_RD_SRC, XDREQ0_RD_DST, XDREQ0_RD_CTL, \
		    XDREQ0_RD_SRC_CTL, XDREQ0_RD_DST_CTL },
	{ "UART0",  UART0_WR_SRC, UART0_WR_DST, UART0_WR_CTL, \
		    UART0_WR_SRC_CTL, UART0_WR_DST_CTL, \
		    UART0_RD_SRC, UART0_RD_DST, UART0_RD_CTL, \
		    UART0_RD_SRC_CTL, UART0_RD_DST_CTL },
	{ "MMC",    MMC_WR_SRC, MMC_WR_DST, MMC_WR_CTL, \
		    MMC_WR_SRC_CTL, MMC_WR_DST_CTL, \
		    MMC_RD_SRC, MMC_RD_SRC, MMC_RD_CTL, \
		    MMC_RD_SRC_CTL, MMC_RD_DST_CTL },
	{ "TIMER",  TIMER_WR_SRC, TIMER_WR_DST, TIMER_WR_CTL, \
		    TIMER_WR_SRC_CTL, TIMER_WR_DST_CTL, \
		    TIMER_RD_SRC, TIMER_RD_DST, TIMER_RD_CTL, \
		    TIMER_RD_SRC_CTL, TIMER_RD_DST_CTL },
	{ "USB",    USB_WR_SRC, USB_WR_DST, USB_WR_CTL, \
		    USB_WR_SRC_CTL, USB_WR_DST_CTL, \
		    USB_RD_SRC, USB_RD_DST, USB_RD_CTL, \
		    USB_RD_SRC_CTL, USB_RD_DST_CTL }
},
{
	{ "XDREQ1", XDREQ1_WR_SRC, XDREQ1_WR_DST, XDREQ1_WR_CTL, \
		    XDREQ1_WR_SRC_CTL, XDREQ1_WR_DST_CTL, \
		    XDREQ1_RD_SRC, XDREQ1_RD_DST, XDREQ1_RD_CTL, \
		    XDREQ1_RD_SRC_CTL, XDREQ1_RD_DST_CTL },
	{ "UART1",  UART1_WR_SRC, UART1_WR_DST, UART1_WR_CTL, \
		    UART1_WR_SRC_CTL, UART1_WR_DST_CTL, \
		    UART1_RD_SRC, UART1_RD_DST, UART1_RD_CTL, \
		    UART1_RD_SRC_CTL, UART1_RD_DST_CTL },
	{ "I2SSDI", I2SSDI_WR_SRC, I2SSDI_WR_DST, I2SSDI_WR_CTL, \
		    I2SSDI_WR_SRC_CTL, I2SSDI_WR_DST_CTL, \
		    I2SSDI_RD_SRC, I2SSDI_RD_DST, I2SSDI_RD_CTL, \
		    I2SSDI_RD_SRC_CTL, I2SSDI_RD_DST_CTL },
	{ "SPI",    SPI_WR_SRC, SPI_WR_DST, SPI_WR_CTL, \
		    SPI_WR_SRC_CTL, SPI_WR_DST_CTL, \
		    SPI_RD_SRC, SPI_RD_DST, SPI_WR_CTL, \
		    SPI_RD_SRC_CTL, SPI_RD_DST_CTL },
	{ "USB",    USB_WR_SRC, USB_WR_DST, USB_WR_CTL, \
		    USB_WR_SRC_CTL, USB_WR_DST_CTL, \
		    USB_RD_SRC, USB_RD_DST, USB_RD_CTL, \
		    USB_RD_SRC_CTL, USB_RD_DST_CTL }

},
{
	{ "I2SSDO", I2SSDO_WR_SRC, I2SSDO_WR_DST, I2SSDO_WR_CTL, \
		    I2SSDO_WR_SRC_CTL, I2SSDO_WR_DST_CTL, \
		    I2SSDO_RD_SRC, I2SSDO_RD_DST, I2SSDO_RD_CTL, \
		    I2SSDO_RD_SRC_CTL, I2SSDO_RD_DST_CTL },
	{ "I2SSDI", I2SSDI_WR_SRC, I2SSDI_WR_DST, I2SSDI_WR_CTL, \
		    I2SSDI_WR_SRC_CTL, I2SSDI_WR_DST_CTL, \
		    I2SSDI_RD_SRC, I2SSDI_RD_DST, I2SSDI_RD_CTL, \
		    I2SSDI_RD_SRC_CTL, I2SSDI_RD_DST_CTL },
	{ "MMC",    MMC_WR_SRC, MMC_WR_DST, MMC_WR_CTL, \
		    MMC_WR_SRC_CTL, MMC_WR_DST_CTL, \
		    MMC_RD_SRC, MMC_RD_SRC, MMC_RD_CTL, \
		    MMC_RD_SRC_CTL, MMC_RD_DST_CTL },
	{ "TIMER",  TIMER_WR_SRC, TIMER_WR_DST, TIMER_WR_CTL, \
		    TIMER_WR_SRC_CTL, TIMER_WR_DST_CTL, \
		    TIMER_RD_SRC, TIMER_RD_DST, TIMER_RD_CTL, \
		    TIMER_RD_SRC_CTL, TIMER_RD_DST_CTL },
	{ "USB",    USB_WR_SRC, USB_WR_DST, USB_WR_CTL, \
		    USB_WR_SRC_CTL, USB_WR_DST_CTL, \
		    USB_RD_SRC, USB_RD_DST, USB_RD_CTL, \
		    USB_RD_SRC_CTL, USB_RD_DST_CTL }
},
{
	{ "UART2",  UART2_WR_SRC, UART2_WR_DST, UART2_WR_CTL, \
		    UART2_WR_SRC_CTL, UART2_WR_DST_CTL, \
		    UART2_RD_SRC, UART2_RD_DST, UART2_RD_CTL, \
		    UART2_RD_SRC_CTL, UART2_RD_DST_CTL },
	{ "MMC",    MMC_WR_SRC, MMC_WR_DST, MMC_WR_CTL, \
		    MMC_WR_SRC_CTL, MMC_WR_DST_CTL, \
		    MMC_RD_SRC, MMC_RD_SRC, MMC_RD_CTL, \
		    MMC_RD_SRC_CTL, MMC_RD_DST_CTL },
	{ "SPI",    SPI_WR_SRC, SPI_WR_DST, SPI_WR_CTL, \
		    SPI_WR_SRC_CTL, SPI_WR_DST_CTL, \
		    SPI_RD_SRC, SPI_RD_DST, SPI_WR_CTL, \
		    SPI_RD_SRC_CTL, SPI_RD_DST_CTL },
	{ "TIMER",  TIMER_WR_SRC, TIMER_WR_DST, TIMER_WR_CTL, \
		    TIMER_WR_SRC_CTL, TIMER_WR_DST_CTL, \
		    TIMER_RD_SRC, TIMER_RD_DST, TIMER_RD_CTL, \
		    TIMER_RD_SRC_CTL, TIMER_RD_DST_CTL },
	{ "USB",    USB_WR_SRC, USB_WR_DST, USB_WR_CTL, \
		    USB_WR_SRC_CTL, USB_WR_DST_CTL, \
		    USB_RD_SRC, USB_RD_DST, USB_RD_CTL, \
		    USB_RD_SRC_CTL, USB_RD_DST_CTL }
}
};

//#define HOOK_LOST_INT
#ifdef HOOK_LOST_INT
#define stop_dma_timer()	(i{ TCON |= TIMER3_OFF; })
#define start_dma_timer() \
//	({ TCNTB3 = 15626; 	/* less than 10ms */  \
		({ TCNTB3 = 21093; 	/* less than 10ms */  \
	   TCON = (TCON | TIMER3_MANUP | TIMER3_OFF); \
	   TCON = (TCON | TIMER3_ATLOAD_ON | TIMER3_ON | TIMER3_NOP); })
#endif
