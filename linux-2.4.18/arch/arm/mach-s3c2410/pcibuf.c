/*
 *  linux/arch/arm/mach-s3c2410/pcibuf.c
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/pci.h>

#include "pcipool.h"

//#define PCI_BUF_DEBUG 1

/*
 * simple buffer allocator for copying of unsafe to safe buffers
 * uses __alloc/__free for actual buffers
 * keeps track of safe buffers we've allocated so we can recover the
 * unsafe buffers.
 */

#define MAX_SAFE	32
#define SIZE_SMALL	1024
#define SIZE_LARGE	(16*1024)

static long mapped_alloc_size;
static char *safe_buffers[MAX_SAFE][2];


static struct pci_pool *small_buffer_cache, *large_buffer_cache;

static int
init_safe_buffers(struct pci_dev *dev)
{
	small_buffer_cache = pci_pool_create("pci_small_buffer",
					    dev,
					    SIZE_SMALL,
					    0 /* byte alignment */,
					    0 /* no page-crossing issues */,
					    GFP_KERNEL | GFP_DMA);

	if (small_buffer_cache == 0)
		return -1;

	large_buffer_cache = pci_pool_create("pci_large_buffer",
					    dev,
					    SIZE_LARGE,
					    0 /* byte alignment */,
					    0 /* no page-crossing issues */,
					    GFP_KERNEL | GFP_DMA);
	if (large_buffer_cache == 0)
		return -1;

	return 0;
}

/* allocate a 'safe' buffer and keep track of it */
static char *
alloc_safe_buffer(char *unsafe, int size, dma_addr_t *pbus)
{
	char *safe;
	dma_addr_t busptr;
	struct pci_pool *pool;
	int i;

#ifdef PCI_BUF_DEBUG
	printk("alloc_safe_buffer(size=%d)\n", size);
#endif

	if (size <= SIZE_SMALL)
		pool = small_buffer_cache;
	else
		if (size < SIZE_LARGE)
			pool = large_buffer_cache;
				else
					return 0;

	safe = pci_pool_alloc(pool, SLAB_ATOMIC, &busptr);
	if (safe == 0)
		return 0;

	for (i = 0; i < MAX_SAFE; i++)
		if (safe_buffers[i][0] == 0) {
			break;
		}

	if (i == MAX_SAFE) {
		panic(__FILE__ ": exceeded MAX_SAFE buffers");
	}

	/* place the size index and the old buffer ptr in the first 8 bytes
	 * and return a ptr + 12 to caller
	 */
	((int *)safe)[0] = i;
	((char **)safe)[1] = (char *)pool;
	((char **)safe)[2] = unsafe;

	busptr += sizeof(int) + sizeof(char *) + sizeof(char *);

	safe_buffers[i][0] = (void *)busptr;
	safe_buffers[i][1] = (void *)safe;

	safe += sizeof(int) + sizeof(char *) + sizeof(char *);

	*pbus = busptr;
	return safe;
}

/* determine if a buffer is from our "safe" pool */
static char *
find_safe_buffer(char *busptr, char **unsafe)
{
	int i;
	char *buf;

	for (i = 0; i < MAX_SAFE; i++) {
		if (safe_buffers[i][0] == busptr) {
#ifdef PCI_BUF_DEBUG
			printk("find_safe_buffer(%p) found @ %d\n", busptr, i);
#endif
			buf = safe_buffers[i][1];
			*unsafe = ((char **)buf)[2];
			return buf + sizeof(int) + sizeof(char *) + sizeof(char *);
		}
	}

	return (char *)0;
}

static void
free_safe_buffer(char *buf)
{
	int index;
	struct pci_pool *pool;
	char *dma;
  
#ifdef PCI_BUF_DEBUG
	printk("free_safe_buffer(buf=%p)\n", buf);
#endif

	/* retrieve the buffer size index */
	buf -= sizeof(int) + sizeof(char*) + sizeof(char*);
	index = ((int *)buf)[0];
	pool = (struct pci_pool *)((char **)buf)[1];

#ifdef PCI_BUF_DEBUG
	printk("free_safe_buffer(%p) index %d\n", buf, index);
#endif

	if (index < 0 || index >= MAX_SAFE) {
		printk(__FILE__ ": free_safe_buffer() corrupt buffer\n");
		return;
	}

	dma = safe_buffers[index][0];
	safe_buffers[index][0] = 0;

	pci_pool_free(pool, buf, (u32)dma);
}

/*
  NOTE:
  replace pci_map/unmap_single with local routines which will
  do buffer copies if buffer is above 1mb...
*/

/*
 * see if a buffer address is in an 'unsafe' range.  if it is
 * allocate a 'safe' buffer and copy the unsafe buffer into it.
 * substitute the safe buffer for the unsafe one.
 * (basically move the buffer from an unsafe area to a safe one)
 *
 * we assume calls to map_single are symmetric with calls to unmap_single...
 */
dma_addr_t
s3c2410_map_single(struct pci_dev *hwdev, void *virtptr,
	       size_t size, int direction)
{
	dma_addr_t busptr;

	mapped_alloc_size += size;

#ifdef PCI_BUF_DEBUG
	printk("pci_map_single(hwdev=%p,ptr=%p,size=%d,dir=%x) "
		      "alloced=%ld\n",
		      hwdev, virtptr, size, direction, mapped_alloc_size);
#endif

	busptr = virt_to_bus(virtptr);

	/* we assume here that a buffer will never be >=64k */
	if ( (((unsigned long)busptr) & 0x100000) ||
	     ((((unsigned long)busptr)+size) & 0x100000) )
	{
		char *safe;

		safe = alloc_safe_buffer(virtptr, size, &busptr);
		if (safe == 0) {
			printk("unable to map unsafe buffer %p!\n", virtptr);
			return 0;
		}

#ifdef PCI_BUF_DEBUG
		printk("unsafe buffer %p (phy=%p) mapped to %p (phy=%p)\n",
			      virtptr, (void *)virt_to_bus(virtptr),
			      safe, (void *)busptr);
#endif

		memcpy(safe, virtptr, size);
		consistent_sync(safe, size, direction);

		return busptr;
	}

	consistent_sync(virtptr, size, direction);
	return busptr;
}

/*
 * see if a mapped address was really a "safe" buffer and if so,
 * copy the data from the safe buffer back to the unsafe buffer
 * and free up the safe buffer.
 * (basically return things back to the way they should be)
 */
void
s3c2410_unmap_single(struct pci_dev *hwdev, dma_addr_t dma_addr,
		 size_t size, int direction)
{
	char *safe, *unsafe;
	void *buf;

	/* hack; usb-ohci.c never sends hwdev==NULL, all others do */
	if (hwdev == NULL) {
		return;
	}

	mapped_alloc_size -= size;

#ifdef PCI_BUF_DEBUG
	printk("pci_unmap_single(hwdev=%p,ptr=%p,size=%d,dir=%x) "
		      "alloced=%ld\n",
		      hwdev, (void *)dma_addr, size, direction,
		      mapped_alloc_size);
#endif

	if ((safe = find_safe_buffer((void *)dma_addr, &unsafe))) {
#ifdef PCI_BUF_DEBUG
		printk("copyback unsafe %p, safe %p, size %d\n",
			      unsafe, safe, size);
#endif

		consistent_sync(safe, size, PCI_DMA_FROMDEVICE);
		memcpy(unsafe, safe, size);
		free_safe_buffer(safe);
	} else {
		/* assume this is normal memory */
		buf = bus_to_virt(dma_addr);
		consistent_sync(buf, size, PCI_DMA_FROMDEVICE);
	}
}

EXPORT_SYMBOL(s3c2410_map_single);
EXPORT_SYMBOL(s3c2410_unmap_single);

static int __init s3c2410_init_safe_buffers(void)
{
	printk("Initializing S3C2410 buffer pool for DMA workaround\n");
	init_safe_buffers(NULL);
	return 0;
}

static void free_safe_buffers(void)
{
	pci_pool_destroy(small_buffer_cache);
	pci_pool_destroy(large_buffer_cache);
}

module_init(s3c2410_init_safe_buffers);
module_exit(free_safe_buffers);
