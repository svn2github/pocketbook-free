/*
 * linux/include/asm-arm/arch-pxa/ide.h
 *
 * Author:	George Davis
 * Created:	Jan 10, 2002
 * Copyright:	MontaVista Software Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Originally based upon linux/include/asm-arm/arch-sa1100/ide.h
 *
 */

#include <linux/config.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>


/*
 * Set up a hw structure for a specified data port, control port and IRQ.
 * This should follow whatever the default interface uses.
 *
 * We fixup the IDE register offsets to use the non-overlapping locations
 * to facilitate successful implementation of the 16-bit I/O read fixups
 * required to work around erratum #86 for PXA250 rev B0 and earlier
 * since the IDE error reg can't be accessed from offset 1 by using
 * non-byte access.
 */
static __inline__ void
ide_init_hwif_ports(hw_regs_t *hw, int data_port, int ctrl_port, int *irq)
{
	ide_ioreg_t reg;

	memset(hw, 0, sizeof(*hw));

	reg = (ide_ioreg_t)data_port;

	hw->io_ports[IDE_DATA_OFFSET] =  reg + 8;
	hw->io_ports[IDE_ERROR_OFFSET] = reg + 13;
	hw->io_ports[IDE_NSECTOR_OFFSET] = reg + 2;
	hw->io_ports[IDE_SECTOR_OFFSET] = reg + 3;
	hw->io_ports[IDE_LCYL_OFFSET] = reg + 4;
	hw->io_ports[IDE_HCYL_OFFSET] = reg + 5;
	hw->io_ports[IDE_SELECT_OFFSET] = reg + 6;
	hw->io_ports[IDE_STATUS_OFFSET] = reg + 7;

	hw->io_ports[IDE_CONTROL_OFFSET] = (ide_ioreg_t) ctrl_port;

	if (irq)
		*irq = 0;
}


/*
 * Register the standard ports for this architecture with the IDE driver.
 */
static __inline__ void
ide_init_default_hwifs(void)
{
	/* Nothing to declare... */
}
