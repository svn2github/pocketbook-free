/*
 * S3C2410 register monitor & controller
 *
 * derived from 
 *   SA1110:registers.c - Sukjae Cho <sjcho@east.isi.edu>
 *
 * $Id: registers.c,v 1.1.1.1 2004/02/04 12:55:27 laputa Exp $
 *
	
   Thr Jan  2 2003 SeonKon Choi <bushi@mizi.com>
   - happy new year!
   - initial

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
 	
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>               /* because we are a module */
#include <linux/init.h>                 /* for the __init macros */
#include <linux/proc_fs.h>              /* all the /proc functions */
#include <linux/ioport.h>
#include <asm/uaccess.h>                /* to copy to/from userspace */
#include <asm/arch/hardware.h>

#define MODULE_NAME "regmon"
#define CPU_DIRNAME "cpu"
#define REG_DIRNAME "registers"

static ssize_t proc_read_reg(struct file * file, char * buf,
		size_t nbytes, loff_t *ppos);
static ssize_t proc_write_reg(struct file * file, const char * buffer,
		size_t count, loff_t *ppos);

static struct file_operations proc_reg_operations = {
	read:	proc_read_reg,
	write:	proc_write_reg
};

typedef struct s3c2410_reg_entry {
	u32 phyaddr;
	char* name;
	unsigned short low_ino;
} s3c2410_reg_entry_t;

static s3c2410_reg_entry_t s3c2410_regs[] =
{
/*	{ phyaddr,    name } */
	{0x48000000,   "BWSCON" },
	{0x48000004, "BANKCON0"},
	{0x48000008, "BANKCON1"},
	{0x4800000C, "BANKCON2"},
	{0x48000010, "BANKCON3"},
	{0x48000014, "BANKCON4"},
	{0x48000018, "BANKCON5"},
	{0x4800001C, "BANKCON6"},
	{0x48000020, "BANKCON7"},
	{0x48000024, "REFRESH"},
	{0x48000028, "BANKSIZE"},
	{0x4800002C, "MRSRB6"},
	{0x4800002C, "MRSRB7"},
	{0x56000000, "GPACON"},
	{0x56000010, "GPBCON"},
	{0x56000020, "GPCCON"},
	{0x56000030, "GPDCON"},
	{0x56000040, "GPECON"},
	{0x56000050, "GPFCON"},
	{0x56000060, "GPGCON"},
	{0x56000070, "GPHCON"},
	{0x56000004, "GPADAT"},
	{0x56000014, "GPBDAT"},
	{0x56000024, "GPCDAT"},
	{0x56000034, "GPDDAT"},
	{0x56000044, "GPEDAT"},
	{0x56000054, "GPFDAT"},
	{0x56000064, "GPGDAT"},
	{0x56000074, "GPHDAT"},
	{0x56000018, "GPBUP"},
	{0x56000028, "GPCUP"},
	{0x56000038, "GPDUP"},
	{0x56000048, "GPEUP"},
	{0x56000058, "GPFUP"},
	{0x56000068, "GPGUP"},
	{0x56000078, "GPHUP"},
	{0x56000080, "MISCCR"},
	{0x56000088, "EXTINT0"},
	{0x5600008c, "EXTINT1"},
	{0x56000090, "EXTINT3"},
};

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof((x)[0]))

static int proc_read_reg(struct file * file, char * buf,
		size_t nbytes, loff_t *ppos)
{
	int i_ino = (file->f_dentry->d_inode)->i_ino;
	char outputbuf[15];
	int count;
	int i;
	s3c2410_reg_entry_t* current_reg=NULL;
	if (*ppos>0) /* Assume reading completed in previous read*/
		return 0;
	for (i=0;i<ARRAY_SIZE(s3c2410_regs);i++) {
		if (s3c2410_regs[i].low_ino==i_ino) {
			current_reg = &s3c2410_regs[i];
			break;
		}
	}
	if (current_reg==NULL)
		return -EINVAL;

	count = sprintf(outputbuf, "0x%08lx\n",
			*((volatile unsigned long *) io_p2v(current_reg->phyaddr)));
	*ppos+=count;
	if (count>nbytes)  /* Assume output can be read at one time */
		return -EINVAL;
	if (copy_to_user(buf, outputbuf, count))
		return -EFAULT;
	return count;
}

static ssize_t proc_write_reg(struct file * file, const char * buffer,
		size_t count, loff_t *ppos)
{
	int i_ino = (file->f_dentry->d_inode)->i_ino;
	s3c2410_reg_entry_t* current_reg=NULL;
	int i;
	unsigned long newRegValue;
	char *endp;

	for (i=0;i<ARRAY_SIZE(s3c2410_regs);i++) {
		if (s3c2410_regs[i].low_ino==i_ino) {
			current_reg = &s3c2410_regs[i];
			break;
		}
	}
	if (current_reg==NULL)
		return -EINVAL;

	newRegValue = simple_strtoul(buffer,&endp,0);
	*((volatile unsigned long *) io_p2v(current_reg->phyaddr))=newRegValue;
	return (count+endp-buffer);
}

static struct proc_dir_entry *regdir;
static struct proc_dir_entry *cpudir;

static int __init init_reg_monitor(void)
{
	struct proc_dir_entry *entry;
	int i;

	cpudir = proc_mkdir(CPU_DIRNAME, &proc_root);
	if (cpudir == NULL) {
		printk(KERN_ERR MODULE_NAME": can't create /proc/" CPU_DIRNAME "\n");
		return(-ENOMEM);
	}

	regdir = proc_mkdir(REG_DIRNAME, cpudir);
	if (regdir == NULL) {
		printk(KERN_ERR MODULE_NAME": can't create /proc/" CPU_DIRNAME "/" REG_DIRNAME "\n");
		return(-ENOMEM);
	}

	for(i=0;i<ARRAY_SIZE(s3c2410_regs);i++) {
		entry = create_proc_entry(s3c2410_regs[i].name,
				S_IWUSR |S_IRUSR | S_IRGRP | S_IROTH,
				regdir);
		if(entry) {
			s3c2410_regs[i].low_ino = entry->low_ino;
			entry->proc_fops = &proc_reg_operations;
		} else {
			printk( KERN_ERR MODULE_NAME
				": can't create /proc/" REG_DIRNAME
				"/%s\n", s3c2410_regs[i].name);
			return(-ENOMEM);
		}
	}
	return (0);
}

static void __exit cleanup_reg_monitor(void)
{
	int i;
	for(i=0;i<ARRAY_SIZE(s3c2410_regs);i++)
		remove_proc_entry(s3c2410_regs[i].name,regdir);
	remove_proc_entry(REG_DIRNAME, cpudir);
	remove_proc_entry(CPU_DIRNAME, &proc_root);
}

module_init(init_reg_monitor);
module_exit(cleanup_reg_monitor);

MODULE_LICENSE("GPL");

EXPORT_NO_SYMBOLS;
