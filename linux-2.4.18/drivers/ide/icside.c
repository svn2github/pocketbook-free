/*
 * linux/drivers/ide/icside.c
 *
 * Copyright (c) 1996,1997 Russell King.
 *
 * Changelog:
 *  08-Jun-1996	RMK	Created
 *  12-Sep-1997	RMK	Added interrupt enable/disable
 *  17-Apr-1999	RMK	Added support for V6 EASI
 *  22-May-1999	RMK	Added support for V6 DMA
 */

#include <linux/config.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <asm/dma.h>
#include <asm/ecard.h>
#include <asm/io.h>

extern char *ide_xfer_verbose (byte xfer_rate);
extern char *ide_dmafunc_verbose(ide_dma_action_t dmafunc);

/*
 * Maximum number of interfaces per card
 */
#define MAX_IFS	2

#define ICS_IDENT_OFFSET		0x8a0

#define ICS_ARCIN_V5_INTRSTAT		0x000
#define ICS_ARCIN_V5_INTROFFSET		0x001
#define ICS_ARCIN_V5_IDEOFFSET		0xa00
#define ICS_ARCIN_V5_IDEALTOFFSET	0xae0
#define ICS_ARCIN_V5_IDESTEPPING	4

#define ICS_ARCIN_V6_IDEOFFSET_1	0x800
#define ICS_ARCIN_V6_INTROFFSET_1	0x880
#define ICS_ARCIN_V6_INTRSTAT_1		0x8a4
#define ICS_ARCIN_V6_IDEALTOFFSET_1	0x8e0
#define ICS_ARCIN_V6_IDEOFFSET_2	0xc00
#define ICS_ARCIN_V6_INTROFFSET_2	0xc80
#define ICS_ARCIN_V6_INTRSTAT_2		0xca4
#define ICS_ARCIN_V6_IDEALTOFFSET_2	0xce0
#define ICS_ARCIN_V6_IDESTEPPING	4

struct cardinfo {
	unsigned int dataoffset;
	unsigned int ctrloffset;
	unsigned int stepping;
};

static struct cardinfo icside_cardinfo_v5 = {
	ICS_ARCIN_V5_IDEOFFSET,
	ICS_ARCIN_V5_IDEALTOFFSET,
	ICS_ARCIN_V5_IDESTEPPING
};

static struct cardinfo icside_cardinfo_v6_1 = {
	ICS_ARCIN_V6_IDEOFFSET_1,
	ICS_ARCIN_V6_IDEALTOFFSET_1,
	ICS_ARCIN_V6_IDESTEPPING
};

static struct cardinfo icside_cardinfo_v6_2 = {
	ICS_ARCIN_V6_IDEOFFSET_2,
	ICS_ARCIN_V6_IDEALTOFFSET_2,
	ICS_ARCIN_V6_IDESTEPPING
};

struct icside_state {
	unsigned int channel;
	unsigned int enabled;
	unsigned int irq_port;
};

static const card_ids icside_cids[] = {
	{ MANU_ICS,  PROD_ICS_IDE  },
	{ MANU_ICS2, PROD_ICS2_IDE },
	{ 0xffff, 0xffff }
};

typedef enum {
	ics_if_unknown,
	ics_if_arcin_v5,
	ics_if_arcin_v6
} iftype_t;

/* ---------------- Version 5 PCB Support Functions --------------------- */
/* Prototype: icside_irqenable_arcin_v5 (struct expansion_card *ec, int irqnr)
 * Purpose  : enable interrupts from card
 */
static void icside_irqenable_arcin_v5 (struct expansion_card *ec, int irqnr)
{
	unsigned int memc_port = (unsigned int)ec->irq_data;
	outb (0, memc_port + ICS_ARCIN_V5_INTROFFSET);
}

/* Prototype: icside_irqdisable_arcin_v5 (struct expansion_card *ec, int irqnr)
 * Purpose  : disable interrupts from card
 */
static void icside_irqdisable_arcin_v5 (struct expansion_card *ec, int irqnr)
{
	unsigned int memc_port = (unsigned int)ec->irq_data;
	inb (memc_port + ICS_ARCIN_V5_INTROFFSET);
}

static const expansioncard_ops_t icside_ops_arcin_v5 = {
	icside_irqenable_arcin_v5,
	icside_irqdisable_arcin_v5,
	NULL,
	NULL,
	NULL,
	NULL
};


/* ---------------- Version 6 PCB Support Functions --------------------- */
/* Prototype: icside_irqenable_arcin_v6 (struct expansion_card *ec, int irqnr)
 * Purpose  : enable interrupts from card
 */
static void icside_irqenable_arcin_v6 (struct expansion_card *ec, int irqnr)
{
	struct icside_state *state = ec->irq_data;
	unsigned int base = state->irq_port;

	state->enabled = 1;

	switch (state->channel) {
	case 0:
		outb(0, base + ICS_ARCIN_V6_INTROFFSET_1);
		inb(base + ICS_ARCIN_V6_INTROFFSET_2);
		break;
	case 1:
		outb(0, base + ICS_ARCIN_V6_INTROFFSET_2);
		inb(base + ICS_ARCIN_V6_INTROFFSET_1);
		break;
	}
}

/* Prototype: icside_irqdisable_arcin_v6 (struct expansion_card *ec, int irqnr)
 * Purpose  : disable interrupts from card
 */
static void icside_irqdisable_arcin_v6 (struct expansion_card *ec, int irqnr)
{
	struct icside_state *state = ec->irq_data;

	state->enabled = 0;

	inb (state->irq_port + ICS_ARCIN_V6_INTROFFSET_1);
	inb (state->irq_port + ICS_ARCIN_V6_INTROFFSET_2);
}

/* Prototype: icside_irqprobe(struct expansion_card *ec)
 * Purpose  : detect an active interrupt from card
 */
static int icside_irqpending_arcin_v6(struct expansion_card *ec)
{
	struct icside_state *state = ec->irq_data;

	return inb(state->irq_port + ICS_ARCIN_V6_INTRSTAT_1) & 1 ||
	       inb(state->irq_port + ICS_ARCIN_V6_INTRSTAT_2) & 1;
}

static const expansioncard_ops_t icside_ops_arcin_v6 = {
	icside_irqenable_arcin_v6,
	icside_irqdisable_arcin_v6,
	icside_irqpending_arcin_v6,
	NULL,
	NULL,
	NULL
};

/* Prototype: icside_identifyif (struct expansion_card *ec)
 * Purpose  : identify IDE interface type
 * Notes    : checks the description string
 */
static iftype_t __init icside_identifyif (struct expansion_card *ec)
{
	unsigned int addr;
	iftype_t iftype;
	int id = 0;

	iftype = ics_if_unknown;

	addr = ecard_address (ec, ECARD_IOC, ECARD_FAST) + ICS_IDENT_OFFSET;

	id = inb (addr) & 1;
	id |= (inb (addr + 1) & 1) << 1;
	id |= (inb (addr + 2) & 1) << 2;
	id |= (inb (addr + 3) & 1) << 3;

	switch (id) {
	case 0: /* A3IN */
		printk("icside: A3IN unsupported\n");
		break;

	case 1: /* A3USER */
		printk("icside: A3USER unsupported\n");
		break;

	case 3:	/* ARCIN V6 */
		printk(KERN_DEBUG "icside: detected ARCIN V6 in slot %d\n", ec->slot_no);
		iftype = ics_if_arcin_v6;
		break;

	case 15:/* ARCIN V5 (no id) */
		printk(KERN_DEBUG "icside: detected ARCIN V5 in slot %d\n", ec->slot_no);
		iftype = ics_if_arcin_v5;
		break;

	default:/* we don't know - complain very loudly */
		printk("icside: ***********************************\n");
		printk("icside: *** UNKNOWN ICS INTERFACE id=%d ***\n", id);
		printk("icside: ***********************************\n");
		printk("icside: please report this to linux@arm.linux.org.uk\n");
		printk("icside: defaulting to ARCIN V5\n");
		iftype = ics_if_arcin_v5;
		break;
	}

	return iftype;
}

/*
 * Handle routing of interrupts.  This is called before
 * we write a command to the drive.
 */
static void icside_maskproc(ide_drive_t *drive, int mask)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct icside_state *state = hwif->hw.priv;
	unsigned long flags;

	local_irq_save(flags);

	state->channel = hwif->channel;

	if (state->enabled && !mask) {
		switch (hwif->channel) {
		case 0:
			outb(0, state->irq_port + ICS_ARCIN_V6_INTROFFSET_1);
			inb(state->irq_port + ICS_ARCIN_V6_INTROFFSET_2);
			break;
		case 1:
			outb(0, state->irq_port + ICS_ARCIN_V6_INTROFFSET_2);
			inb(state->irq_port + ICS_ARCIN_V6_INTROFFSET_1);
			break;
		}
	} else {
		inb(state->irq_port + ICS_ARCIN_V6_INTROFFSET_2);
		inb(state->irq_port + ICS_ARCIN_V6_INTROFFSET_1);
	}

	local_irq_restore(flags);
}

#ifdef CONFIG_BLK_DEV_IDEDMA_ICS

static int icside_dmaproc(ide_dma_action_t func, ide_drive_t *drive);

/*
 * SG-DMA support.
 *
 * Similar to the BM-DMA, but we use the RiscPCs IOMD DMA controllers.
 * There is only one DMA controller per card, which means that only
 * one drive can be accessed at one time.  NOTE! We do not enforce that
 * here, but we rely on the main IDE driver spotting that both
 * interfaces use the same IRQ, which should guarantee this.
 */
#define NR_ENTRIES 256
#define TABLE_SIZE (NR_ENTRIES * 8)

static int icside_build_dmatable(ide_drive_t *drive, int dma_mode)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct request *rq = HWGROUP(drive)->rq;
	struct buffer_head *bh;
	struct scatterlist *sg = hwif->sg_table;
	int nents = 0;

	bh = rq->bh;
	do {
		char *virt_addr;

		memset(sg, 0, sizeof(*sg));
		sg->address = virt_addr = bh->b_data;

		do {
			virt_addr += bh->b_size;

			bh = bh->b_reqnext;
			if (bh == NULL)
				break;

		} while (virt_addr == bh->b_data);

		sg->length = virt_addr - (char *)sg->address;
		sg++;
		nents++;
	} while (bh != NULL);

	if (dma_mode == DMA_MODE_READ)
		hwif->sg_dma_direction = PCI_DMA_FROMDEVICE;
	else
		hwif->sg_dma_direction = PCI_DMA_TODEVICE;

	hwif->sg_nents = pci_map_sg(NULL, hwif->sg_table, nents,
				    hwif->sg_dma_direction);

	return hwif->sg_nents;
}

/* Teardown mappings after DMA has completed.  */
static inline void icside_destroy_dmatable(ide_hwif_t *hwif)
{
	pci_unmap_sg(NULL, hwif->sg_table, hwif->sg_nents,
		     hwif->sg_dma_direction);
}

/*
 * Configure the IOMD to give the appropriate timings for the transfer
 * mode being requested.  We take the advice of the ATA standards, and
 * calculate the cycle time based on the transfer mode, and the EIDE
 * MW DMA specs that the drive provides in the IDENTIFY command.
 *
 * We have the following IOMD DMA modes to choose from:
 *
 *	Type	Active		Recovery	Cycle
 *	A	250 (250)	312 (550)	562 (800)
 *	B	187		250		437
 *	C	125 (125)	125 (375)	250 (500)
 *	D	62		125		187
 *
 * (figures in brackets are actual measured timings)
 *
 * However, we also need to take care of the read/write active and
 * recovery timings:
 *
 *			Read	Write
 *  	Mode	Active	-- Recovery --	Cycle	IOMD type
 *	MW0	215	50	215	480	A
 *	MW1	80	50	50	150	C
 *	MW2	70	25	25	120	C
 */
static int icside_speedproc(ide_drive_t *drive, byte xfer_mode)
{
	int func = ide_dma_off;
	int cycle_time = 0, use_dma_info = 0;

	/*
	 * Limit the transfer speed to MW_DMA_2.
	 */
	if (xfer_mode > XFER_MW_DMA_2)
		xfer_mode = XFER_MW_DMA_2;

	switch (xfer_mode) {
	case XFER_MW_DMA_2:
		cycle_time = 250;
		use_dma_info = 1;
		break;
	case XFER_MW_DMA_1:
		cycle_time = 250;
		use_dma_info = 1;
		break;
	case XFER_MW_DMA_0:
		cycle_time = 480;
		break;

	case XFER_SW_DMA_2:
	case XFER_SW_DMA_1:
	case XFER_SW_DMA_0:
		cycle_time = 480;
		break;

	default:
		func = ide_dma_off;
		break;
	}

	/*
	 * If we're going to be doing MW_DMA_1 or MW_DMA_2, we should
	 * take care to note the values in the ID...
	 */
	if (use_dma_info && drive->id->eide_dma_time > cycle_time)
		cycle_time = drive->id->eide_dma_time;

	drive->drive_data = cycle_time;

	if (cycle_time && ide_config_drive_speed(drive, xfer_mode) == 0)
		func = ide_dma_on;
	else
		drive->drive_data = 480;

	if (!drive->init_speed)
		drive->init_speed = xfer_mode;
	else if (cycle_time)
		printk("%s: %s selected (peak %dMB/s)\n", drive->name,
			ide_xfer_verbose(xfer_mode), 2000 / drive->drive_data);

	drive->current_speed = xfer_mode;

	return func;
}

/*
 * dma_intr() is the handler for disk read/write DMA interrupts
 */
static ide_startstop_t icside_dmaintr(ide_drive_t *drive)
{
	byte stat, dma_stat;

	dma_stat = icside_dmaproc(ide_dma_end, drive);

	/*
	 * Verify the drive status
	 */
	stat = GET_STAT();
	if (OK_STAT(stat,DRIVE_READY,drive->bad_wstat|DRQ_STAT)) {
		if (!dma_stat) {
			struct request *rq = HWGROUP(drive)->rq;
			int i;

			for (i = rq->nr_sectors; i > 0;) {
				i -= rq->current_nr_sectors;
				ide_end_request(1, HWGROUP(drive));
			}

			return ide_stopped;
		}
		printk("%s: dma_intr: bad DMA status (dma_stat=%x)\n", 
		       drive->name, dma_stat);
	}
	return ide_error(drive, "dma_intr", stat);
}

/*
 * The following is a sick duplication from ide-dma.c ;(
 *
 * This should be defined in one place only.
 */
struct drive_list_entry {
	char * id_model;
	char * id_firmware;
};

static struct drive_list_entry drive_whitelist [] = {
	{ "Micropolis 2112A",			"ALL"		},
	{ "CONNER CTMA 4000",			"ALL"		},
	{ "CONNER CTT8000-A",			"ALL"		},
	{ "ST34342A",				"ALL"		},
	{ NULL,					0		}
};

static struct drive_list_entry drive_blacklist [] = {
	{ "WDC AC11000H",			"ALL"		},
	{ "WDC AC22100H",			"ALL"		},
	{ "WDC AC32500H",			"ALL"		},
	{ "WDC AC33100H",			"ALL"		},
	{ "WDC AC31600H",			"ALL"		},
	{ "WDC AC32100H",			"24.09P07"	},
	{ "WDC AC23200L",			"21.10N21"	},
	{ "Compaq CRD-8241B",			"ALL"		},
	{ "CRD-8400B",				"ALL"		},
	{ "CRD-8480B",				"ALL"		},
	{ "CRD-8480C",				"ALL"		},
	{ "CRD-8482B",				"ALL"		},
 	{ "CRD-84",				"ALL"		},
	{ "SanDisk SDP3B",			"ALL"		},
	{ "SanDisk SDP3B-64",			"ALL"		},
	{ "SANYO CD-ROM CRD",			"ALL"		},
	{ "HITACHI CDR-8",			"ALL"		},
	{ "HITACHI CDR-8335",			"ALL"		},
	{ "HITACHI CDR-8435",			"ALL"		},
	{ "Toshiba CD-ROM XM-6202B",		"ALL"		},
	{ "CD-532E-A",				"ALL"		},
	{ "E-IDE CD-ROM CR-840",		"ALL"		},
	{ "CD-ROM Drive/F5A",			"ALL"		},
	{ "RICOH CD-R/RW MP7083A",		"ALL"		},
	{ "WPI CDD-820",			"ALL"		},
	{ "SAMSUNG CD-ROM SC-148C",		"ALL"		},
	{ "SAMSUNG CD-ROM SC-148F",		"ALL"		},
	{ "SAMSUNG CD-ROM SC",			"ALL"		},
	{ "SanDisk SDP3B-64",			"ALL"		},
	{ "SAMSUNG CD-ROM SN-124",		"ALL"		},
	{ "PLEXTOR CD-R PX-W8432T",		"ALL"		},
	{ "ATAPI CD-ROM DRIVE 40X MAXIMUM",	"ALL"		},
	{ "_NEC DV5800A",			"ALL"		},
	{ NULL,					0		}
};

static int
in_drive_list(struct hd_driveid *id, struct drive_list_entry *drive_table)
{
	for ( ; drive_table->id_model ; drive_table++)
		if ((!strcmp(drive_table->id_model, id->model)) &&
		    ((!strstr(drive_table->id_firmware, id->fw_rev)) ||
		     (!strcmp(drive_table->id_firmware, "ALL"))))
			return 1;
	return 0;
}

static int icside_check_drive_lists(ide_drive_t *drive, ide_dma_action_t func)
{
	struct hd_driveid *id = drive->id;
	int found;

	if (func == ide_dma_good_drive) {
		found = in_drive_list(id, drive_whitelist);
	} else {
		found = in_drive_list(id, drive_blacklist);
		if (found)
			printk("%s: Disabling DMA for %s (blacklisted)\n",
				drive->name, id->model);
	}
	return found;
}

static int icside_dma_check(ide_drive_t *drive)
{
	struct hd_driveid *id = drive->id;
	ide_hwif_t *hwif = HWIF(drive);
	int autodma = hwif->autodma;
	int xfer_mode = XFER_PIO_2;
	int func = ide_dma_off_quietly;

	if (!id || !(id->capability & 1) || !autodma)
		goto out;

	/*
	 * Consult the list of known "bad" drives
	 */
	if (icside_check_drive_lists(drive, ide_dma_bad_drive))
		goto out;

	/*
	 * Enable DMA on any drive that has multiword DMA
	 */
	if (id->field_valid & 2) {
		if (id->dma_mword & 4) {
			xfer_mode = XFER_MW_DMA_2;
			func = ide_dma_on;
		} else if (id->dma_mword & 2) {
			xfer_mode = XFER_MW_DMA_1;
			func = ide_dma_on;
		} else if (id->dma_mword & 1) {
			xfer_mode = XFER_MW_DMA_0;
			func = ide_dma_on;
		}
		goto out;
	}

	/*
	 * Consult the list of known "good" drives
	 */
	if (icside_check_drive_lists(drive, ide_dma_good_drive)) {
		if (id->eide_dma_time > 150)
			goto out;
		xfer_mode = XFER_MW_DMA_1;
		func = ide_dma_on;
	}

out:
	func = icside_speedproc(drive, xfer_mode);

	return icside_dmaproc(func, drive);
}

static int icside_dma_verbose(ide_drive_t *drive)
{
	printk(", %s (peak %dMB/s)",
		ide_xfer_verbose(drive->current_speed),
		2000 / drive->drive_data);
	return 1;
}

static int icside_dmaproc(ide_dma_action_t func, ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct icside_state *state = hwif->hw.priv;
	int count, dma_mode = DMA_MODE_READ;

	switch (func) {
	case ide_dma_off:
		printk("%s: DMA disabled\n", drive->name);
		/*FALLTHROUGH*/

	case ide_dma_off_quietly:
	case ide_dma_on:
		drive->using_dma = (func == ide_dma_on);
		return 0;

	case ide_dma_check:
		return icside_dma_check(drive);

	case ide_dma_write:
		dma_mode = DMA_MODE_WRITE;
	case ide_dma_read:
		count = icside_build_dmatable(drive, dma_mode);
		if (!count)
			return 1;
		disable_dma(hwif->hw.dma);

		/*
		 * Ensure that we have the right interrupt routed.
		 */
		icside_maskproc(drive, 0);

		/*
		 * Route the DMA signals to to the correct interface.
		 */
		outb(hwif->select_data, hwif->config_data);

		/*
		 * Select the correct timing for this drive.
		 */
		set_dma_speed(hwif->hw.dma, drive->drive_data);
		set_dma_sg(hwif->hw.dma, HWIF(drive)->sg_table, count);
		set_dma_mode(hwif->hw.dma, dma_mode);

		drive->waiting_for_dma = 1;
		if (drive->media != ide_disk)
			return 0;

		ide_set_handler(drive, &icside_dmaintr, WAIT_CMD, NULL);
		OUT_BYTE(dma_mode == DMA_MODE_READ ?
			 WIN_READDMA : WIN_WRITEDMA, IDE_COMMAND_REG);
		/*FALLTHROUGH*/

	case ide_dma_begin:
		enable_dma(hwif->hw.dma);
		return 0;

	case ide_dma_end:
		drive->waiting_for_dma = 0;
		disable_dma(hwif->hw.dma);
		icside_destroy_dmatable(hwif);
		return get_dma_residue(hwif->hw.dma) != 0;

	case ide_dma_test_irq:
		return inb(state->irq_port +
			(hwif->channel ?
				ICS_ARCIN_V6_INTRSTAT_2 :
				ICS_ARCIN_V6_INTRSTAT_1)) & 1;

	case ide_dma_bad_drive:
	case ide_dma_good_drive:
		return icside_check_drive_lists(drive, func);

	case ide_dma_verbose:
		return icside_dma_verbose(drive);

	case ide_dma_lostirq:
	case ide_dma_timeout:
	default:
		printk("icside_dmaproc: unsupported %s func: %d\n",
			ide_dmafunc_verbose(func), func);
	}
	return 1;
}

static int icside_setup_dma(ide_hwif_t *hwif, int autodma)
{
	hwif->sg_table = kmalloc(sizeof(struct scatterlist) * NR_ENTRIES,
				 GFP_KERNEL);
	if (!hwif->sg_table)
		goto failed;

	hwif->dmatable_cpu = NULL;
	hwif->dmatable_dma = 0;
	hwif->speedproc    = icside_speedproc;
	hwif->dmaproc      = icside_dmaproc;
	hwif->autodma      = autodma;

	return 1;

failed:
	printk(" -- ERROR, unable to allocate DMA table\n");
	return 0;
}

int ide_release_dma(ide_hwif_t *hwif)
{
	if (hwif->sg_table) {
		kfree(hwif->sg_table);
		hwif->sg_table = NULL;
	}
	return 1;
}
#endif

static ide_hwif_t *icside_find_hwif(unsigned long dataport)
{
	ide_hwif_t *hwif;
	int index;

	for (index = 0; index < MAX_HWIFS; ++index) {
		hwif = &ide_hwifs[index];
		if (hwif->io_ports[IDE_DATA_OFFSET] == (ide_ioreg_t)dataport)
			goto found;
	}

	for (index = 0; index < MAX_HWIFS; ++index) {
		hwif = &ide_hwifs[index];
		if (!hwif->io_ports[IDE_DATA_OFFSET])
			goto found;
	}

	hwif = NULL;
found:
	return hwif;
}

static ide_hwif_t *
icside_setup(unsigned long base, struct cardinfo *info, int irq)
{
	unsigned long port = base + info->dataoffset;
	ide_hwif_t *hwif;

	hwif = icside_find_hwif(base);
	if (hwif) {
		int i;

		memset(&hwif->hw, 0, sizeof(hw_regs_t));

		for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
			hwif->hw.io_ports[i] = (ide_ioreg_t)port;
			hwif->io_ports[i] = (ide_ioreg_t)port;
			port += 1 << info->stepping;
		}
		hwif->hw.io_ports[IDE_CONTROL_OFFSET] = base + info->ctrloffset;
		hwif->io_ports[IDE_CONTROL_OFFSET] = base + info->ctrloffset;
		hwif->hw.irq	 = irq;
		hwif->irq	 = irq;
		hwif->hw.dma	 = NO_DMA;
		hwif->noprobe	 = 0;
		hwif->chipset	 = ide_acorn;
	}

	return hwif;
}

static int __init icside_register_v5(struct expansion_card *ec, int autodma)
{
	unsigned long slot_port;
	ide_hwif_t *hwif;

	slot_port = ecard_address(ec, ECARD_MEMC, 0);

	ec->irqaddr  = (unsigned char *)ioaddr(slot_port + ICS_ARCIN_V5_INTRSTAT);
	ec->irqmask  = 1;
	ec->irq_data = (void *)slot_port;
	ec->ops      = (expansioncard_ops_t *)&icside_ops_arcin_v5;

	/*
	 * Be on the safe side - disable interrupts
	 */
	inb(slot_port + ICS_ARCIN_V5_INTROFFSET);

	hwif = icside_setup(slot_port, &icside_cardinfo_v5, ec->irq);

	return hwif ? 0 : -ENODEV;
}

static int __init icside_register_v6(struct expansion_card *ec, int autodma)
{
	unsigned long slot_port, port;
	struct icside_state *state;
	ide_hwif_t *hwif, *mate;
	unsigned int sel = 0;

	slot_port = ecard_address(ec, ECARD_IOC, ECARD_FAST);
	port      = ecard_address(ec, ECARD_EASI, ECARD_FAST);

	if (port == 0)
		port = slot_port;
	else
		sel = 1 << 5;

	outb(sel, slot_port);

	/*
	 * Be on the safe side - disable interrupts
	 */
	inb(port + ICS_ARCIN_V6_INTROFFSET_1);
	inb(port + ICS_ARCIN_V6_INTROFFSET_2);

	/*
	 * Find and register the interfaces.
	 */
	hwif = icside_setup(port, &icside_cardinfo_v6_1, ec->irq);
	mate = icside_setup(port, &icside_cardinfo_v6_2, ec->irq);

	if (!hwif || !mate)
		return -ENODEV;

	state = kmalloc(sizeof(struct icside_state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	state->channel    = 0;
	state->enabled    = 0;
	state->irq_port   = port;

	ec->irq_data      = state;
	ec->ops           = (expansioncard_ops_t *)&icside_ops_arcin_v6;

	hwif->maskproc    = icside_maskproc;
	hwif->channel     = 0;
	hwif->hw.priv     = state;
	hwif->mate        = mate;
	hwif->serialized  = 1;
	hwif->config_data = slot_port;
	hwif->select_data = sel;
	hwif->hw.dma      = ec->dma;

	mate->maskproc    = icside_maskproc;
	mate->channel     = 1;
	mate->hw.priv     = state;
	mate->mate        = hwif;
	mate->serialized  = 1;
	mate->config_data = slot_port;
	mate->select_data = sel | 1;
	mate->hw.dma      = ec->dma;

#ifdef CONFIG_BLK_DEV_IDEDMA_ICS
	if (ec->dma != NO_DMA && !request_dma(ec->dma, hwif->name)) {
		icside_setup_dma(hwif, autodma);
		icside_setup_dma(mate, autodma);
	}
#endif
	return 0;
}

int __init icside_init(void)
{
	int autodma = 0;

#ifdef CONFIG_IDEDMA_ICS_AUTO
	autodma = 1;
#endif

	ecard_startfind ();

	do {
		struct expansion_card *ec;
		int result;

		ec = ecard_find(0, icside_cids);
		if (ec == NULL)
			break;

		ecard_claim(ec);

		switch (icside_identifyif(ec)) {
		case ics_if_arcin_v5:
			result = icside_register_v5(ec, autodma);
			break;

		case ics_if_arcin_v6:
			result = icside_register_v6(ec, autodma);
			break;

		default:
			result = -1;
			break;
		}

		if (result)
			ecard_release(ec);
	} while (1);

	return 0;
}
