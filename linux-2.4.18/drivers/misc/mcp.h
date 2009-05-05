/*
 *  linux/drivers/misc/mcp.h
 *
 *  Copyright (C) 2001 Russell King, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */
#ifndef MCP_H
#define MCP_H

#ifdef CONFIG_ARCH_SA1100
#include <asm/dma.h>
#endif

struct mcp {
	struct module	*owner;
	spinlock_t	lock;
	int		use_count;
	unsigned int	sclk_rate;
	unsigned int	rw_timeout;
#ifdef CONFIG_ARCH_SA1100
	dma_device_t	dma_audio_rd;
	dma_device_t	dma_audio_wr;
	dma_device_t	dma_telco_rd;
	dma_device_t	dma_telco_wr;
#endif
	void		(*set_telecom_divisor)(struct mcp *, unsigned int);
	void		(*set_audio_divisor)(struct mcp *, unsigned int);
	void		(*reg_write)(struct mcp *, unsigned int, unsigned int);
	unsigned int	(*reg_read)(struct mcp *, unsigned int);
	void		(*enable)(struct mcp *);
	void		(*disable)(struct mcp *);
};

void mcp_set_telecom_divisor(struct mcp *, unsigned int);
void mcp_set_audio_divisor(struct mcp *, unsigned int);
void mcp_reg_write(struct mcp *, unsigned int, unsigned int);
unsigned int mcp_reg_read(struct mcp *, unsigned int);
void mcp_enable(struct mcp *);
void mcp_disable(struct mcp *);

/* noddy implementation alert! */
struct mcp *mcp_get(void);
int mcp_register(struct mcp *);

#define mcp_get_sclk_rate(mcp)	((mcp)->sclk_rate)

#endif
