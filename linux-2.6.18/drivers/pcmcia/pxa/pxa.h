/*
 * linux/drivers/pcmcia/pxa/pxa.h
 *
 * Author:	George Davis
 * Created:	Jan 10, 2002
 * Copyright:	MontaVista Software Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Originally based upon linux/drivers/pcmcia/sa1100_generic.h
 *
 */

/*======================================================================

    Device driver for the PCMCIA control functionality of Intel
    PXA250/210 microprocessors.

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is John G. Dorsey
    <john+@cs.cmu.edu>.  Portions created by John G. Dorsey are
    Copyright (C) 1999 John G. Dorsey.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU Public License version 2 (the "GPL"), in which
    case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.
    
======================================================================*/

#if !defined(_PCMCIA_PXA_H)
# define _PCMCIA_PXA_H

#include <pcmcia/cs_types.h>
#include <pcmcia/ss.h>
#include <pcmcia/bulkmem.h>
#include <pcmcia/cistpl.h>
#include "../cs_internal.h"

#include <asm/arch/pcmcia.h>


/* MECR: Expansion Memory Configuration Register
 * (SA-1100 Developers Manual, p.10-13; SA-1110 Developers Manual, p.10-24)
 *
 * MECR layout is:  
 *
 *   FAST1 BSM1<4:0> BSA1<4:0> BSIO1<4:0> FAST0 BSM0<4:0> BSA0<4:0> BSIO0<4:0>
 *
 * (This layout is actually true only for the SA-1110; the FASTn bits are
 * reserved on the SA-1100.)
 */

#define MCXX_SETUP_MASK     (0x7f)
#define MCXX_ASST_MASK      (0x1f)
#define MCXX_HOLD_MASK      (0x3f)
#define MCXX_SETUP_SHIFT    (0)
#define MCXX_ASST_SHIFT     (7)
#define MCXX_HOLD_SHIFT     (14)


#define MECR_SET(mecr, sock, shift, mask, bs) \
((mecr)=((mecr)&~(((mask)<<(shift))<<\
                  ((sock)==0?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT)))|\
        (((bs)<<(shift))<<((sock)==0?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT)))

#define MECR_GET(mecr, sock, shift, mask) \
((((mecr)>>(((sock)==0)?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT))>>\
 (shift))&(mask))

#define MECR_BSIO_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSIO_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSIO_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSIO_SHIFT, MECR_BS_MASK)

#define MECR_BSA_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSA_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSA_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSA_SHIFT, MECR_BS_MASK)

#define MECR_BSM_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSM_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSM_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSM_SHIFT, MECR_BS_MASK)

#define MECR_FAST_SET(mecr, sock, fast) \
MECR_SET((mecr), (sock), MECR_FAST_SHIFT, MECR_FAST_MODE_MASK, (fast))

#define MECR_FAST_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_FAST_SHIFT, MECR_FAST_MODE_MASK)


/* This function implements the BS value calculation for setting the MECR
 * using integer arithmetic:
 */
static inline unsigned int pxa_pcmcia_mecr_bs(unsigned int pcmcia_cycle_ns,
						 unsigned int cpu_clock_khz){
  unsigned int t = ((pcmcia_cycle_ns * cpu_clock_khz) / 6) - 1000000;
  return (t / 1000000) + (((t % 1000000) == 0) ? 0 : 1);
}

static inline u_int pxa_mcxx_hold(u_int pcmcia_cycle_ns,
					    u_int mem_clk_10khz){
  u_int code = pcmcia_cycle_ns * mem_clk_10khz;
  return (code / 300000) + ((code % 300000) ? 1 : 0) - 1;
}

static inline u_int pxa_mcxx_asst(u_int pcmcia_cycle_ns,
					    u_int mem_clk_10khz){
  u_int code = pcmcia_cycle_ns * mem_clk_10khz;
  return (code / 300000) + ((code % 300000) ? 1 : 0) - 1;
}

static inline u_int pxa_mcxx_setup(u_int pcmcia_cycle_ns,
					    u_int mem_clk_10khz){
  u_int code = pcmcia_cycle_ns * mem_clk_10khz;
  return (code / 100000) + ((code % 100000) ? 1 : 0) - 1;
}

/* This function returns the (approxmiate) command assertion period, in
 * nanoseconds, for a given CPU clock frequency and MCXX_ASST value:
 */

static inline u_int pxa_pcmcia_cmd_time(u_int mem_clk_10khz,
					   u_int pcmcia_mcxx_asst){
  return (300000 * (pcmcia_mcxx_asst + 1) / mem_clk_10khz);
}


/* SA-1100 PCMCIA Memory and I/O timing
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * The SA-1110 Developer's Manual, section 10.2.5, says the following:
 *
 *  "To calculate the recommended BS_xx value for each address space:
 *   divide the command width time (the greater of twIOWR and twIORD,
 *   or the greater of twWE and twOE) by processor cycle time; divide
 *   by 2; divide again by 3 (number of BCLK's per command assertion);
 *   round up to the next whole number; and subtract 1."
 *
 * The PC Card Standard, Release 7, section 4.13.4, says that twIORD
 * has a minimum value of 165ns. Section 4.13.5 says that twIOWR has
 * a minimum value of 165ns, as well. Section 4.7.2 (describing
 * common and attribute memory write timing) says that twWE has a
 * minimum value of 150ns for a 250ns cycle time (for 5V operation;
 * see section 4.7.4), or 300ns for a 600ns cycle time (for 3.3V
 * operation, also section 4.7.4). Section 4.7.3 says that taOE
 * has a maximum value of 150ns for a 300ns cycle time (for 5V
 * operation), or 300ns for a 600ns cycle time (for 3.3V operation).
 *
 * When configuring memory maps, Card Services appears to adopt the policy
 * that a memory access time of "0" means "use the default." The default
 * PCMCIA I/O command width time is 165ns. The default PCMCIA 5V attribute
 * and memory command width time is 150ns; the PCMCIA 3.3V attribute and
 * memory command width time is 300ns.
 */
#define PXA_PCMCIA_IO_ACCESS      (165)
#define PXA_PCMCIA_5V_MEM_ACCESS  (150)
#define PXA_PCMCIA_3V_MEM_ACCESS  (300)


/* The socket driver actually works nicely in interrupt-driven form,
 * so the (relatively infrequent) polling is "just to be sure."
 */
#define PXA_PCMCIA_POLL_PERIOD    (2*HZ)


/* This structure encapsulates per-socket state which we might need to
 * use when responding to a Card Services query of some kind.
 */
struct pxa_pcmcia_socket {
  socket_state_t        cs_state;
  struct pcmcia_state   k_state;
  unsigned int          irq;
  void                  (*handler)(void *, unsigned int);
  void                  *handler_info;
  pccard_io_map         io_map[MAX_IO_WIN];
  pccard_mem_map        mem_map[MAX_WIN];
  ioaddr_t              virt_io, phys_attr, phys_mem;
  unsigned short        speed_io, speed_attr, speed_mem;
};


/* I/O pins replacing memory pins
 * (PCMCIA System Architecture, 2nd ed., by Don Anderson, p.75)
 *
 * These signals change meaning when going from memory-only to 
 * memory-or-I/O interface:
 */
#define iostschg bvd1
#define iospkr   bvd2


/*
 * Declaration for all implementation specific low_level operations.
 */
extern struct pcmcia_low_level lubbock_pcmcia_ops;
extern struct pcmcia_low_level pxa_idp_pcmcia_ops;
extern struct pcmcia_low_level cerf_pcmcia_ops;

#endif  /* !defined(_PCMCIA_PXA_H) */
