#ifndef _PIO_H
#define _PIO_H

#include "pio_ioctl.h"

#define PIO0_BASE_ADDRESS		(0x500600)
#define PIO1_BASE_ADDRESS		(0x500A00)

#define PIO_INT_REG_OFFSET		(0x00)
#define PIO_DATA_REG_OFFSET		(0x04)
#define PIO_DIR_REG_OFFSET		(0x08)
#define PIO_POL_REG_OFFSET		(0x0C)
#define PIO_INTEN_REG_OFFSET	(0x10)

#define PIO0_INT_REG			((volatile unsigned int *)(PIO0_BASE_ADDRESS + PIO_INT_REG_OFFSET))
#define PIO0_DATA_REG			((volatile unsigned int *)(PIO0_BASE_ADDRESS + PIO_DATA_REG_OFFSET))
#define PIO0_DIR_REG			((volatile unsigned int *)(PIO0_BASE_ADDRESS + PIO_DIR_REG_OFFSET))
#define PIO0_POL_REG			((volatile unsigned int *)(PIO0_BASE_ADDRESS + PIO_POL_REG_OFFSET))
#define PIO0_INTEN_REG			((volatile unsigned int *)(PIO0_BASE_ADDRESS + PIO_INTEN_REG_OFFSET))

#define PIO1_INT_REG			((volatile unsigned int *)(PIO1_BASE_ADDRESS + PIO_INT_REG_OFFSET))
#define PIO1_DATA_REG			((volatile unsigned int *)(PIO1_BASE_ADDRESS + PIO_DATA_REG_OFFSET))
#define PIO1_DIR_REG			((volatile unsigned int *)(PIO1_BASE_ADDRESS + PIO_DIR_REG_OFFSET))
#define PIO1_POL_REG			((volatile unsigned int *)(PIO1_BASE_ADDRESS + PIO_POL_REG_OFFSET))
#define PIO1_INTEN_REG			((volatile unsigned int *)(PIO1_BASE_ADDRESS + PIO_INTEN_REG_OFFSET))

#define MAKE_PIOREG(x,n)		((n << x) | (0x10000 << x))

#define PIO(n)					(1<<n)

#define INTENA_REG (*(volatile int *)0x00500224)
#define INTPOL_REG (*(volatile int *)0x00500220)

static inline void disable_pio_irqs ()
{
	int reg;
	reg = INTENA_REG;
	reg &= ~((1<<5) | (1<<6));
	INTENA_REG = reg;	
}

static inline void enable_pio_irqs ()
{
	int reg;
	reg = INTENA_REG;
	reg |= ((1<<5) | (1<<6));
	INTENA_REG = reg;	
}

#define PIO_LOCK	disable_pio_irqs
#define PIO_UNLOCK	enable_pio_irqs

#define PIO_EVENT_FIFO_LENGTH	10

			 
void pio0_interrupt_handler (PIO_DEVICE *pPioDevice);
void pio1_interrupt_handler (PIO_DEVICE *pPioDevice);

void pio0_enable_interrupt (unsigned long mask);
void pio1_enable_interrupt (unsigned long mask);

void pio0_configure (unsigned long mask);
void pio1_configure (unsigned long mask);

void pio0_polarity (unsigned long mask);
void pio1_polarity (unsigned long mask);

int  pio_get_event (PIO_DEVICE *pPioDevice, PIO_EVENT *pio_event);

void pio0_write (int pio, int level);
void pio1_write (int pio, int level);
int  pio0_read (int pio);
int  pio1_read (int pio);

int  pio_status (PIO_DEVICE *pPioDevice, PIO_STATUS *ppiostatus);

void battery_write_register (unsigned char reg, unsigned char data);
unsigned char battery_read_register (unsigned char reg);

void pio_timer_expiry (unsigned long context);

#endif
