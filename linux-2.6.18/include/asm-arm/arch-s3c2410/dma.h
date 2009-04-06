/*
 * linux/include/asm-arm/arch-s3c2410/dma.h
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * History 
 *
 * 2001-12-20: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-01-30: Janghoon Lyu <nandy@mizi.com>
 *    - Add a S3C2400_DMA_CHANNELS defininition
 *
 * 2002-04-24: Janghoon Lyu <nandy@mizi.com>
 *    - dma_device_t 
 *    - DMA read/write
 *
 * 2002-05-28: Janghoon Lyu <nandy@mizi.com>
 *    - S3C2400 
 */

#ifndef __ASM_ARCH_DMA_H__
#define __ASM_ARCH_DMA_H__

#include "hardware.h"

#define MAX_DMA_ADDRESS	0xffffffff

/*
 * NB: By nandy
 * If MAX_DMA_CHANNELS is zero, It means that this architecuture not use 
 * the regular generic DMA interface provided by kernel.
 * Why? I don't know. I will investigate S3C2410 DMA model and generic
 * DMA interface. But not yet.
 */
#define MAX_DMA_CHANNELS	0

/* The S3C2410 has four internal DMA channels. */
#define S3C2410_DMA_CHANNELS	4

#define MAX_S3C2410_DMA_CHANNELS	S3C2410_DMA_CHANNELS

#define DMA_CH0			0
#define DMA_CH1			1
#define DMA_CH2			2
#define DMA_CH3			3

#define DMA_BUF_WR		1
#define DMA_BUF_RD		0

typedef void (*dma_callback_t)(void *buf_id, int size);

/* S3C2410 DMA API */
extern int s3c2410_request_dma(const char *device_id, dmach_t channel,
				dma_callback_t write_cb, dma_callback_t read_cb); 
extern int s3c2410_dma_queue_buffer(dmach_t channel, void *buf_id, 
					dma_addr_t data, int size, int write);
extern int s3c2410_dma_flush_all(dmach_t channel);
extern void s3c2410_free_dma(dmach_t channel);
extern int s3c2410_dma_get_current(dmach_t channel, void **buf_id, dma_addr_t *addr);
extern int s3c2410_dma_stop(dmach_t channel);
    
#endif /* __ASM_ARCH_DMA_H__ */
