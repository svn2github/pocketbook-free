/*
 *  linux/drivers/misc/mcp-pxa.c
 *
 *  2002-01-10 Jeff Sutherland <jeffs@accelent.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * NOTE: This is a quick hack to gain access to the aclink codec's
 *       touch screen facility.  Its audio is handled by a separate
 *       (non-mcp) driver at the present time.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/ac97_codec.h>

#include "mcp.h"


extern int pxa_ac97_get(struct ac97_codec **codec);
extern void pxa_ac97_put(void);


struct mcp *mcp_get(void)
{
	struct ac97_codec *codec;
	if (pxa_ac97_get(&codec) < 0)
		return NULL;
	return (struct mcp *)codec;
}

void mcp_reg_write(struct mcp *mcp, unsigned int reg, unsigned int val)
{
	struct ac97_codec *codec = (struct ac97_codec *)mcp;
	codec->codec_write(codec, reg, val);
}

unsigned int mcp_reg_read(struct mcp *mcp, unsigned int reg)
{
	struct ac97_codec *codec = (struct ac97_codec *)mcp;
	return codec->codec_read(codec, reg);
}

void mcp_enable(struct mcp *mcp)
{
	/* 
	 * Should we do something here to make sure the aclink
	 * codec is alive???
	 * A: not for now  --NP
	*/
}

void mcp_disable(struct mcp *mcp)
{
}
