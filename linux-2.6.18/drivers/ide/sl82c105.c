/*
 * linux/drivers/ide/sl82c105.c
 *
 * SL82C105/Winbond 553 IDE driver
 *
 * Maintainer unknown.
 *
 * Changelog:
 *
 * 15/11/1998	RMK	Drive tuning added from Rebel.com's kernel
 *			sources
 * 30/03/2002	RMK	Add fixes specified in W83C553F errata.
 *			(with special thanks to Todd Inglett)
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/pci.h>
#include <linux/ide.h>

#include <asm/io.h>
#include <asm/dma.h>

#include "ide_modes.h"

extern char *ide_xfer_verbose (byte xfer_rate);

/*
 * SL82C105 PCI config register 0x40 bits.
 */
#define CTRL_IDE_IRQB	(1 << 30)
#define CTRL_IDE_IRQA	(1 << 28)
#define CTRL_LEGIRQ	(1 << 11)
#define CTRL_P1F16	(1 << 5)
#define CTRL_P1EN	(1 << 4)
#define CTRL_P0F16	(1 << 1)
#define	CTRL_P0EN	(1 << 0)

/*
 * Convert a PIO mode and cycle time to the required on/off
 * times for the interface.  This has protection against run-away
 * timings.
 */
static unsigned int get_timing_sl82c105(ide_pio_data_t *p)
{
	unsigned int cmd_on;
	unsigned int cmd_off;

	cmd_on = (ide_pio_timings[p->pio_mode].active_time + 29) / 30;
	cmd_off = (p->cycle_time - 30 * cmd_on + 29) / 30;

	if (cmd_on > 32)
		cmd_on = 32;
	if (cmd_on == 0)
		cmd_on = 1;

	if (cmd_off > 32)
		cmd_off = 32;
	if (cmd_off == 0)
		cmd_off = 1;

	return (cmd_on - 1) << 8 | (cmd_off - 1) | (p->use_iordy ? 0x40 : 0x00);
}

/*
 * Configure the drive and chipset for PIO
 */
static void config_for_pio(ide_drive_t *drive, int pio, int report)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct pci_dev *dev = hwif->pci_dev;
	ide_pio_data_t p;
	unsigned short drv_ctrl = 0x909;
	unsigned int xfer_mode, reg;

	reg = (hwif->channel ? 0x4c : 0x44) + (drive->select.b.unit ? 4 : 0);

	pio = ide_get_best_pio_mode(drive, pio, 5, &p);

	switch (pio) {
	default:
	case 0:		xfer_mode = XFER_PIO_0;		break;
	case 1:		xfer_mode = XFER_PIO_1;		break;
	case 2:		xfer_mode = XFER_PIO_2;		break;
	case 3:		xfer_mode = XFER_PIO_3;		break;
	case 4:		xfer_mode = XFER_PIO_4;		break;
	}

	if (ide_config_drive_speed(drive, xfer_mode) == 0)
		drv_ctrl = get_timing_sl82c105(&p);

	if (drive->using_dma == 0) {
		/*
		 * If we are actually using MW DMA, then we can not
		 * reprogram the interface drive control register.
		 */
		pci_write_config_word(dev, reg, drv_ctrl);
		pci_read_config_word(dev, reg, &drv_ctrl);

		if (report) {
			printk("%s: selected %s (%dns) (%04X)\n", drive->name,
			       ide_xfer_verbose(xfer_mode), p.cycle_time, drv_ctrl);
		}
	}
}

/*
 * Configure the drive and the chipset for DMA
 */
static int config_for_dma(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct pci_dev *dev = hwif->pci_dev;
	unsigned short drv_ctrl = 0x909;
	unsigned int reg;

	reg = (hwif->channel ? 0x4c : 0x44) + (drive->select.b.unit ? 4 : 0);

	if (ide_config_drive_speed(drive, XFER_MW_DMA_2) == 0)
		drv_ctrl = 0x0240;

	pci_write_config_word(dev, reg, drv_ctrl);

	return 0;
}


/*
 * Check to see if the drive and
 * chipset is capable of DMA mode
 */
static int sl82c105_check_drive(ide_drive_t *drive)
{
	ide_dma_action_t dma_func = ide_dma_off_quietly;

	do {
		struct hd_driveid *id = drive->id;
		ide_hwif_t *hwif = HWIF(drive);

		if (!hwif->autodma)
			break;

		if (!id || !(id->capability & 1))
			break;

		/* Consult the list of known "bad" drives */
		if (ide_dmaproc(ide_dma_bad_drive, drive)) {
			dma_func = ide_dma_off;
			break;
		}

		if (id->field_valid & 2) {
			if  (id->dma_mword & 7 || id->dma_1word & 7)
				dma_func = ide_dma_on;
			break;
		}

		if (ide_dmaproc(ide_dma_good_drive, drive)) {
			dma_func = ide_dma_on;
			break;
		}
	} while (0);

	return HWIF(drive)->dmaproc(dma_func, drive);
}

/*
 * The SL82C105 holds off all IDE interrupts while in DMA mode until
 * all DMA activity is completed.  Sometimes this causes problems (eg,
 * when the drive wants to report an error condition).
 *
 * 0x7e is a "chip testing" register.  Bit 2 resets the DMA controller
 * state machine.  We need to kick this to work around various bugs.
 */
static inline void sl82c105_reset_host(struct pci_dev *dev)
{
	u16 val;

	pci_read_config_word(dev, 0x7e, &val);
	pci_write_config_word(dev, 0x7e, val | (1 << 2));
	pci_write_config_word(dev, 0x7e, val & ~(1 << 2));
}

/*
 * If we get an IRQ timeout, it might be that the DMA state machine
 * got confused.  Fix from Todd Inglett.  Details from Winbond.
 *
 * This function is called when the IDE timer expires, the drive
 * indicates that it is READY, and we were waiting for DMA to complete.
 */
static int sl82c105_lostirq(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct pci_dev *dev = hwif->pci_dev;
	u32 val, mask = hwif->channel ? CTRL_IDE_IRQB : CTRL_IDE_IRQA;
	unsigned long dma_base = hwif->dma_base;

	printk("sl82c105: lost IRQ: resetting host\n");

	/*
	 * Check the raw interrupt from the drive.
	 */
	pci_read_config_dword(dev, 0x40, &val);
	if (val & mask)
		printk("sl82c105: drive was requesting IRQ, but host lost it\n");

	/*
	 * Was DMA enabled?  If so, disable it - we're resetting the
	 * host.  The IDE layer will be handling the drive for us.
	 */
	val = inb(dma_base);
	if (val & 1) {
		outb(val & ~1, dma_base);
		printk("sl82c105: DMA was enabled\n");
	}

	sl82c105_reset_host(dev);

	/* ide_dmaproc would return 1, so we do as well */
	return 1;
}

/*
 * ATAPI devices can cause the SL82C105 DMA state machine to go gaga.
 * Winbond recommend that the DMA state machine is reset prior to
 * setting the bus master DMA enable bit.
 *
 * The generic IDE core will have disabled the BMEN bit before this
 * function is called.
 */
static void sl82c105_before_bm_enable(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct pci_dev *dev = hwif->pci_dev;

	sl82c105_reset_host(dev);
}

/*
 * Our very own dmaproc.  We need to intercept various calls
 * to fix up the SL82C105 specific behaviour.
 */
static int sl82c105_dmaproc(ide_dma_action_t func, ide_drive_t *drive)
{
	switch (func) {
	case ide_dma_check:
		return sl82c105_check_drive(drive);

	case ide_dma_on:
		if (config_for_dma(drive))
			func = ide_dma_off;
		/* fall through */

	case ide_dma_off_quietly:
	case ide_dma_off:
		config_for_pio(drive, 4, 0);
		break;

	case ide_dma_read:
	case ide_dma_write:
	case ide_dma_begin:
	case ide_dma_timeout:
		sl82c105_before_bm_enable(drive);
		break;

	case ide_dma_lostirq:
		return sl82c105_lostirq(drive);

	default:
		break;
	}
	return ide_dmaproc(func, drive);
}

/*
 * We only deal with PIO mode here - DMA mode 'using_dma' is not
 * initialised at the point that this function is called.
 */
static void tune_sl82c105(ide_drive_t *drive, byte pio)
{
	config_for_pio(drive, pio, 1);

	/*
	 * We support 32-bit I/O on this interface, and it
	 * doesn't have problems with interrupts.
	 */
	drive->io_32bit = 1;
	drive->unmask = 1;
}

/*
 * Return the revision of the Winbond bridge
 * which this function is part of.
 */
static unsigned int sl82c105_bridge_revision(struct pci_dev *dev)
{
	struct pci_dev *bridge;
	unsigned char rev;

	/*
	 * The bridge should be part of the same device, but function 0.
	 */
	bridge = pci_find_slot(dev->bus->number,
			       PCI_DEVFN(PCI_SLOT(dev->devfn), 0));
	if (!bridge)
		return -1;

	/*
	 * Make sure it is a Winbond 553 and is an ISA bridge.
	 */
	if (bridge->vendor != PCI_VENDOR_ID_WINBOND ||
	    bridge->device != PCI_DEVICE_ID_WINBOND_83C553 ||
	    bridge->class >> 8 != PCI_CLASS_BRIDGE_ISA)
		return -1;

	/*
	 * We need to find function 0's revision, not function 1
	 */
	pci_read_config_byte(bridge, PCI_REVISION_ID, &rev);

	return rev;
}

/*
 * Enable the PCI device
 */
unsigned int __init pci_init_sl82c105(struct pci_dev *dev, const char *msg)
{
	u32 val;

	pci_read_config_dword(dev, 0x40, &val);
	val |= CTRL_P0EN | CTRL_P0F16 | CTRL_P1EN | CTRL_P1F16;
	pci_write_config_dword(dev, 0x40, val);

	return dev->irq;
}

void __init dma_init_sl82c105(ide_hwif_t *hwif, unsigned long dma_base)
{
	unsigned int bridge_rev;
	byte dma_state;

	dma_state = inb(dma_base + 2);
	bridge_rev = sl82c105_bridge_revision(hwif->pci_dev);
	if (bridge_rev <= 5) {
		hwif->autodma = 0;
		hwif->drives[0].autotune = 1;
		hwif->drives[1].autotune = 1;
		printk("    %s: Winbond 553 bridge revision %d, BM-DMA disabled\n",
		       hwif->name, bridge_rev);
		dma_state &= ~0x60;
	} else {
		dma_state |= 0x60;
		hwif->autodma = 1;
	}
	outb(dma_state, dma_base + 2);

	ide_setup_dma(hwif, dma_base, 8);

	if (bridge_rev <= 5)
		hwif->dmaproc = NULL;
	else
		hwif->dmaproc = sl82c105_dmaproc;
}

/*
 * Initialise the chip
 */
void __init ide_init_sl82c105(ide_hwif_t *hwif)
{
	hwif->tuneproc = tune_sl82c105;
}

