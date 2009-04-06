/* $Id: scatterlist.h,v 1.1.1.1 2004/02/04 12:57:51 laputa Exp $ */
#ifndef _SPARC64_SCATTERLIST_H
#define _SPARC64_SCATTERLIST_H

#include <asm/page.h>

struct scatterlist {
	/* This will disappear in 2.5.x */
	char *address;

	/* These two are only valid if ADDRESS member of this
	 * struct is NULL.
	 */
	struct page *page;
	unsigned int offset;

	unsigned int length;

	dma_addr_t dma_address;
	__u32 dma_length;
};

#define sg_dma_address(sg)	((sg)->dma_address)
#define sg_dma_len(sg)     	((sg)->dma_length)

#define ISA_DMA_THRESHOLD	(~0UL)

#endif /* !(_SPARC64_SCATTERLIST_H) */
