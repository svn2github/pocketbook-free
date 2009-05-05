/*
 *	$Id: io_dc.c,v 1.1.1.1 2004/02/04 12:55:48 laputa Exp $
 *	I/O routines for SEGA Dreamcast
 */

#include <asm/io.h>
#include <asm/machvec.h>

unsigned long dreamcast_isa_port2addr(unsigned long offset)
{
	return offset + 0xa0000000;
}
