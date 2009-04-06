/* TODO:

   (C) 2002, Mizi Research Inc.
   Author: Hwang, Chideok <hwang@mizi.co.kr>
*/

#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/errno.h>

#define BON_MAJOR 97

#define MAJOR_NR BON_MAJOR
#define DEVICE_NAME "bon"
#define DEVICE_REQUEST do_bon_request
#define DEVICE_NR(device) (device)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define DEVICE_NO_RANDOM
#include <linux/blk.h>

#include <linux/devfs_fs_kernel.h>

static int PARTITION_OFFSET  = (~0);

#define MAX_RETRY 5
#define MAX_MTD_PART 2

#define MAX_PART 5

typedef struct {
    unsigned long offset;
    unsigned long size;
    unsigned long flag;
    devfs_handle_t devfs_rw_handle;
    unsigned short *bad_blocks;
} partition_t;

static struct {
    struct mtd_info *mtd;
    struct mtd_info *shadow_mtd[MAX_MTD_PART];
    int num_part;
	int num_mtd_part;
    devfs_handle_t devfs_dir_handle;
    partition_t parts[MAX_PART];
	struct mtd_partition mtd_parts[MAX_MTD_PART];
} bon;

static const char BON_MAGIC[8] = {'M', 0, 0, 'I', 0, 'Z', 'I', 0};
static int bon_sizes[MAX_PART];
static int bon_blksizes[MAX_PART];

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
static struct proc_dir_entry *proc_bon;

static inline 
int bon_proc_info (char *buf, int i)
{
	partition_t *this = (partition_t *)&bon.parts[i];

	if (!this)
		return 0;

	return sprintf(buf, "bon%d: %8.8lx-%8.8lx (%8.8lx) %8.8lx\n", i, 
			this->offset, this->offset + this->size, this->size, this->flag);
}

static int 
bon_read_proc ( char *page, char **start, off_t off,int count 
		,int *eof, void *data_unused)
{
	int len, l, i;
	off_t   begin = 0;

	len = sprintf(page, "      position          size       flag\n");

	for (i=0; i< bon.num_part; i++) { 
		l = bon_proc_info(page + len, i);
		len += l;
		if (len+begin > off+count)
			goto done;
		if (len+begin < off) {
			begin += len;
			len = 0;
		}
	}
	*eof = 1;

done:
	if (off >= len+begin)
		return 0;
	*start = page + (off-begin);
	return ((count < begin+len-off) ? count : begin+len-off);
}
#endif // CONFIG_PROC_FS

static int
bon_open(struct inode *inode, struct file *file)
{
    int minor = MINOR(inode->i_rdev);
    partition_t *part = &bon.parts[minor];
    if (minor >= bon.num_part || !part->size) return -ENODEV;

    get_mtd_device(bon.mtd, -1);

    return 0;
}

static int
bon_close(struct inode *inode, struct file *file)
{
//    int minor = MINOR(inode->i_rdev);
    struct mtd_info *mtd;
//    partition_t *part = &bon.parts[minor];

    mtd = bon.mtd;

#if LINUX_VERSION_CODE != KERNEL_VERSION(2,4,18)
    invalidate_device(inode->i_rdev, 1);
#endif
    if (mtd->sync) mtd->sync(mtd);

    put_mtd_device(mtd);
    return 0;
}

static int
bon_ioctl(struct inode *inode, struct file *file,
    u_int cmd, u_long arg)
{
    int minor = MINOR(inode->i_rdev);
    partition_t *part = &bon.parts[minor];

    switch(cmd) {
	case BLKGETSIZE:
	    return put_user((part->size>>9), (long *)arg);
#ifdef BLKGETSIZE64
	case BLKGETSIZE64:
	    return put_user((u64)(part->size>>9), (long *)arg);
#endif
	case BLKFLSBUF:
	    if (!capable(CAP_SYS_ADMIN)) return -EACCES;
	    fsync_dev(inode->i_rdev);
	    invalidate_buffers(inode->i_rdev);
	    if (bon.mtd->sync) bon.mtd->sync(bon.mtd);
	    return 0;
	default:
	    return -ENOTTY;
    }
}

static struct block_device_operations bon_blk_fops = {
    owner: THIS_MODULE,
    open: bon_open,
    release: bon_close,
    ioctl: bon_ioctl
};

static int
do_read(partition_t *part, char *buf, unsigned long pos, size_t size)
{
    int ret, retlen;
    int i;
    while(size > 0) {
	unsigned long block = pos / bon.mtd->erasesize;
	unsigned long start, start_in_block;
	size_t this_size;

	if (part->bad_blocks) {
	    unsigned short *bad = part->bad_blocks;
	    while(*bad++ <= block) {
		block++;
	    }
	}
	start_in_block = pos % bon.mtd->erasesize;
	start = block * bon.mtd->erasesize + start_in_block;
	this_size = bon.mtd->erasesize - start_in_block;
	if (this_size > size) this_size = size;

	ret = MTD_READ(bon.mtd, part->offset + start, this_size, &retlen, buf);
	if (ret) return ret;
	if (this_size != retlen) return -EIO;

	/* ecc */
	for(i=0;i<this_size;i++) {
	    unsigned char oobbuf[16];
	    unsigned char *ecc1 = oobbuf + 8, *ecc2 = ecc1 + 3;
	    unsigned char calc_ecc[3];
	    size_t retlen;
	    int ecc_result;
	    MTD_READOOB(bon.mtd, part->offset + start + i, 16, &retlen, oobbuf);

	    nand_calculate_ecc(buf,  calc_ecc);
	    ecc_result = nand_correct_data(buf, ecc1, calc_ecc);
	    if (ecc_result == -1) {
		printk("bon: ecc error, page = 0x%08lx\n", (part->offset + start + i) >> 9);
		return -EIO;
	    }
	    nand_calculate_ecc(buf + 256,  calc_ecc);
	    ecc_result = nand_correct_data(buf + 256, ecc2, calc_ecc);
	    if (ecc_result == -1) {
		printk("bon: ecc error, page = 0x%08lx\n", (part->offset + start + i) >> 9);
		return -EIO;
	    }
	    i += 512;
	}
	size -= this_size;
	buf += this_size;
	pos += this_size;
    }
    return 0;
}

static void
do_bon_request(request_queue_t *q)
{
    struct request *req;
//    struct mtd_info *mtd = bon.mtd;
    partition_t *part;
    int res;

    while (1) {
	INIT_REQUEST;
	req = CURRENT;
	spin_unlock_irq(&io_request_lock);
	res = 0;
	if (MINOR(req->rq_dev) >= bon.num_part) {
	    printk("bon: Unsupported devices\n");
	    goto end_req;
	}
	part = &bon.parts[MINOR(req->rq_dev)];
	if (req->current_nr_sectors << 9 > part->size) {
	    printk("bon: attempt to read past end of device!\n");
	    goto end_req;
	}
	switch(req->cmd) {
	    int err;
	    case READ:
		err = do_read(part, req->buffer, 
		    req->sector << 9, req->current_nr_sectors << 9);
		if (!err) res = 1;
		break;
	    case WRITE:
		break;
	}
end_req:
	spin_lock_irq(&io_request_lock);
	end_request(res);
    }
}

static int
read_partition_info(struct mtd_info *mtd)
{
    unsigned long offset = PARTITION_OFFSET;
    int i;
    char buf[512];
    unsigned char oobbuf[16];
    unsigned int *s;
    int retlen, k;
    int retry_count = MAX_RETRY;

    if (offset > mtd->size - mtd->erasesize) 
	offset = mtd->size - mtd->erasesize;

    while(retry_count-- > 0) {
	if (MTD_READOOB(mtd, offset, 8, &retlen, oobbuf) < 0) {
	    goto next_block;
	}
	if (oobbuf[5] != 0xff) {
	    goto next_block;
	}
	if (MTD_READ(mtd, offset, 512, &retlen, buf) < 0) {
	    goto next_block;
	}
	if (strncmp(buf, BON_MAGIC, 8) == 0) break;
	printk("bon:cannot find partition table\n");
	return -1;
next_block:
        offset -= mtd->erasesize;
    }

    if (retry_count <= 0) {
	printk("bon:cannot find partition table\n");
	return -1;
    }

    s = (unsigned int *)(buf + 8);
    bon.num_part = min_t(unsigned long, *s++, MAX_PART);
	bon.num_mtd_part = 0;

    bon.mtd = mtd;

    bon.devfs_dir_handle = devfs_mk_dir(NULL, "bon", NULL);
    // for each partition, make 
    for(i=0;i < bon.num_part; i++) {
	char name[8];
//	int num_block;
	bon.parts[i].offset = *s++;
	bon.parts[i].size = *s++;
	bon.parts[i].flag = *s++;


	printk("bon%d: %8.8lx-%8.8lx (%8.8lx) %8.8lx\n", i, 
			bon.parts[i].offset, bon.parts[i].offset + bon.parts[i].size, bon.parts[i].size, bon.parts[i].flag);


	if (bon.parts[i].flag & 0x01) {
#if 0 // bushi
		struct mtd_partition *mtd_part;
		if (bon.num_mtd_part >= MAX_MTD_PART)
			continue;
	   	mtd_part = &(bon.mtd_parts[bon.num_mtd_part]);
	    mtd_part->mask_flags = 0;
	    mtd_part->size = bon.parts[i].size;
	    mtd_part->name = "nandflash  (bon  )";
		sprintf(mtd_part->name, "nandflash%d (bon%d)", bon.num_mtd_part, i);
	    mtd_part->offset = bon.parts[i].offset;
		mtd_part->mtdp = &bon.shadow_mtd[bon.num_mtd_part];
	    bon.num_mtd_part++;
#endif
	} else {
	    sprintf(name, "%d", i);
	    bon.parts[i].devfs_rw_handle = devfs_register(bon.devfs_dir_handle,
		name, DEVFS_FL_DEFAULT, BON_MAJOR, i, 
		S_IFBLK | S_IRUGO, // | S_IWUGO, /* forbid writing */
		&bon_blk_fops, NULL);
	}
    }
    for(i=0;i<bon.num_part;i++) {
	unsigned int num_bad_block = *s++;
	if (num_bad_block == 0) continue;
	bon.parts[i].bad_blocks = kmalloc((1+num_bad_block) * sizeof(unsigned short), GFP_KERNEL);
	for(k=0;k<num_bad_block;k++) {
	    bon.parts[i].bad_blocks[k] = *s++;
	}
	bon.parts[i].bad_blocks[k] = ~0;
    }

#if 0 // bushi
	printk("add mtd part! - start\n");
	if (bon.num_mtd_part) {
		add_mtd_partitions(mtd, &bon.mtd_parts[0], bon.num_mtd_part);
	}
	printk("add mtd part! - end\n");
#endif

    return 0;
}

static void
bon_notify_add(struct mtd_info *mtd)
{
//    partition_t *part;
//    int dev;
    int i;

    if (!mtd->read_oob) return; 
    if (bon.num_part) return;
    if (read_partition_info(mtd)) return;
    for(i=0;i<bon.num_part;i++) {
	bon_sizes[i] = bon.parts[i].size/1024;
	bon_blksizes[i] = mtd->erasesize;
	if (bon_blksizes[i] > PAGE_SIZE) bon_blksizes[i] = PAGE_SIZE;
    }
}

static void
bon_notify_remove(struct mtd_info *mtd)
{
    int i;
    if (!bon.num_part || bon.mtd != mtd) return;
    devfs_unregister(bon.devfs_dir_handle);
    for(i=0;i<bon.num_part;i++) {
	devfs_unregister(bon.parts[i].devfs_rw_handle);
    }
    memset(&bon, 0, sizeof(bon));
}

static struct mtd_notifier bon_notifier = {
    add: bon_notify_add,
    remove: bon_notify_remove
};

static int
init_bon(void)
{
    int i;

    memset(&bon, 0, sizeof(bon));

    if (devfs_register_blkdev(BON_MAJOR, "bon", &bon_blk_fops)) {
	printk(KERN_WARNING "bon: unable to allocate major device num\n");
	return -EAGAIN;
    }

    for(i=0;i<MAX_PART;i++) {
        bon_blksizes[i] = 1024;
	bon_sizes[i] = 0;
    }

    blksize_size[BON_MAJOR] = bon_blksizes;
    blk_size[BON_MAJOR] = bon_sizes;

    blk_init_queue(BLK_DEFAULT_QUEUE(BON_MAJOR), &do_bon_request);
    register_mtd_user(&bon_notifier);

#ifdef CONFIG_PROC_FS
	if ((proc_bon = create_proc_entry( "bon", 0, 0 )))
		proc_bon->read_proc = bon_read_proc;
#endif

    return 0;
}

static void __exit cleanup_bon(void)
{
    unregister_mtd_user(&bon_notifier);
    devfs_unregister_blkdev(BON_MAJOR, "bon");
    blk_cleanup_queue(BLK_DEFAULT_QUEUE(BON_MAJOR));
    blksize_size[BON_MAJOR] = NULL;

#ifdef CONFIG_PROC_FS
	if (proc_bon)
		remove_proc_entry( "bon", 0);
#endif
}

#ifdef MODULE
MODULE_PARM(PARTITION_OFFSET, "i");
#else
int __init part_setup(char *options)
{
    if (!options || !*options) return 0;
    PARTITION_OFFSET = simple_strtoul(options, &options, 0);
    if (*options == 'k' || *options == 'K') {
	PARTITION_OFFSET *= 1024;
    } else if (*options == 'm' || *options == 'M') {
	PARTITION_OFFSET *= 1024;
    }
    return 0;
}
__setup("nand_part_offset=", part_setup);
#endif

module_init(init_bon);
module_exit(cleanup_bon);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hwang, Chideok <hwang@mizi.co.kr>");
MODULE_DESCRIPTION("Simple Block Device for Nand Flash");

