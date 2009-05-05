// pio kernel module for uCLinux
// specifically designed for DataPlay/EM8500 reference board
// Copyright (c) Sigma Designs, Inc. 2003
//
// uCLinux kernel module setup routines

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/ioctl.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/slab.h>

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/irq.h>

#include <asm/semaphore.h>
#include <asm/arch/S3C2410.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>

#define CMD_READ_INIT 0
#define CMD_READ 	1
#define CMD_ADC		2	
#define CMD_ABSOLUTE	3
// global data

devfs_handle_t	devfs_handle = 0;

#define BYTE unsigned char
#define bool int

#define PIO_MAJOR			132
#define PIO_DEVICE_NAME		"ebr_adc"


#define START_ADC_AIN(x) \
	{ \
		ADCCON = PRESCALE_EN | PRSCVL(255) | ADC_INPUT((x)) ; \
		ADCCON |= ADC_START; \
	}
int adc_read_init(int ain)
{
	int ret = 0;

	printk("ADCCON1= 0x%x\n", ADCCON);


	START_ADC_AIN(ain);
	
	printk("ADCCON2= 0x%x\n", ADCCON);

}

int adc_read(int ain)
{
	int ret = 0;
	
	ret = ADCDAT0 ;
	
	printk("ADCDAT0= 0x%x\n", ADCDAT0);
	printk("ADCDAT1= 0x%x\n", ADCDAT1);
	
	
	printk("AIN[%d] = 0x%04x, %d\n", ain, ret, ADCCON & 0x80 ? 1:0);
	return (ret & 0x3ff);
}


int adc_init(void)
{
	/* normal ADC */
	ADCTSC = 0; //XP_PST(NOP_MODE);


}


void init_adc()
{	
}

static int adc_open (struct inode * inode, struct file * file)
{
	printk("open adc module\n");
	return 0;
}

static int adc_release (struct inode * inode, struct file * file)
{
	printk("release adc module\n");
	return 0;
}




static int adc_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{	
	int ret;
	unsigned int i, j; 
	printk("adc ioctl \n");		
	
	switch (cmd)
	{		case CMD_READ_INIT:
				printk("CMD_READ_INIT\n");
				
				adc_read_init(arg);
				break;			
	
			case CMD_READ:
				printk("CMD_Read\n");
				ret=adc_read(arg);
				break;
			case CMD_ABSOLUTE:
				printk("CMD_ABSOLUTE\n"); 
				adc_read_init(arg); 
				ret=adc_read(arg); 
				break; 	
			case CMD_ADC:
				printk("CMD_ADC\n");
				adc_read_init(arg);
				ret = adc_read(arg);  
				printk("ADC Value: %d \n", ret);
				//if (ret > 618)
					//ret=5;
				//else if(ret > 604)
					//ret=4;
				//else if(ret > 590)
					//ret=3;
				//else if(ret > 576)
					//ret=2;
				//else if(ret > 562)
					//ret=1;
				//else 
					//ret=0;
				break;
	
	}
	
	
	return ret;
}

static int adc_check_media_change (void)
{
	return 0;
}

static int adc_revalidate (void)
{
	return 0;
}

static struct block_device_operations pio_fops = 
{
	owner:				THIS_MODULE,
	open:				adc_open,
	release:				adc_release,
	ioctl:				adc_ioctl,	
	check_media_change:	adc_check_media_change,
	revalidate:			adc_revalidate
};

int adc_init_module ()
{
	
	unsigned int maj, min, status, pio6;
	
	printk("init_adc\n");

	// register block driver.
	// if kernel doesnt support devfs, devfs_register returns NULL
	
	if ((devfs_handle = devfs_register(NULL, PIO_DEVICE_NAME,
			DEVFS_FL_DEFAULT, PIO_MAJOR, 0,
			S_IFBLK | S_IRUGO | S_IWUGO,
			&pio_fops, NULL)))
	{
		devfs_get_maj_min (devfs_handle, &maj, &min);
	}
	if (devfs_handle == 0)
	{
		return -1;
	}

	init_adc();
	printk("init_adc:end\n");


	return 0;
}

void adc_cleanup_module()
{
	devfs_unregister (devfs_handle);
	devfs_unregister_blkdev (PIO_MAJOR, PIO_DEVICE_NAME);
}

module_init(adc_init_module);						//include kernel           
module_exit(adc_cleanup_module);					//include kernel 
