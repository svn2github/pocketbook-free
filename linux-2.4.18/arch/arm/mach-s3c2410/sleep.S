/*
 * S3C2410 Assembler Sleep/WakeUp Management Routines
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 * 
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/hardware.h>

	.text

/*
 * s3c2410_cpu_suspend()
 */

ENTRY(s3c2410_cpu_suspend)
	stmfd	sp!, {r4 - r12, lr}		@ save registers on stack

	@ get coprocessor registers
	mrc	p15, 0, r4, c3, c0, 0		@ domain ID
	mrc	p15, 0, r5, c2, c0, 0		@ translation table base addr
	mrc	p15, 0, r6, c13, c0, 0		@ PID
	mrc	p15, 0, r7, c1, c0, 0		@ control reg

	@ store them plus current virtual stack ptr on stack
	mov	r8, sp
	stmfd	sp!, {r4 - r8}

	@ preserve phys address of stack
	mov	r0, sp
	bl	sleep_phys_sp
	ldr	r1, =sleep_save_sp
	str	r0, [r1]

	@ clean data cache and invalidate WB
	bl	cpu_arm920_cache_clean_invalidate_all

	@ go flash memory
	mov	r2, #0x34
	mrc	p15, 0, r1, c1, c0, 0		@ read ctrl register
	bic	r1, r1, #0x0001			@ MMU off
	mcr	p15, 0, r1, c1, c0, 0		@ write ctrl register
	mov	pc, r2
	nop
	nop 

/*
 * s3c2410_cpu_resume()
 * entry point from bootloader into kernel during resume
 *
 */

	.data
	.align 5
ENTRY(s3c2410_cpu_resume)
	mov	r0, #I_BIT | F_BIT | MODE_SVC	@ set SVC, irqs off
	msr	cpsr_c, r0

	ldr	r0, sleep_save_sp		@ stack phys addr
	ldr	r2, =resume_after_mmu		@ its aboslute virtual address
	ldmfd	r0!, {r4 - r7, sp}		@ CP regs + virt stack ptr

	mov	r1, #0
	mcr	p15, 0, r1, c7, c7		@ invalidate I,D caches
	mcr	p15, 0, r1, c7, c10, 4		@ drain write buffer
	mcr	p15, 0, r1, c8, c7		@ invalidate I,D TLBs

	mcr	p15, 0, r4, c3, c0, 0		@ domain ID
	mcr	p15, 0, r5, c2, c0, 0		@ translation table base addr
	mcr	p15, 0, r6, c13, c0, 0		@ PID

	mcr	p15, 0, r7, c1, c0, 0		@ turn on MMU, caches, etc.
	nop
	mov	pc, r2				@ jump to virtual addr
	nop
	nop
	nop

sleep_save_sp:
	.word	0				@ preserve stack phys ptr here

	.text
resume_after_mmu:
	ldmfd	sp!, {r4 - r12, pc}		@ return to caller
