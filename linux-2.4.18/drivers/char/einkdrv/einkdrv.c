// pio kernel module for uCLinux
// specifically designed for DataPlay/EM8500 reference board
// Copyright (c) Sigma Designs, Inc. 2003
//
// uCLinux kernel module setup routines


#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/pm.h>



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/ioctl.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/unistd.h>



#include <asm/arch/S3C2410.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/hardware.h>

#include "pio_ioctl.h"
#include "EPDConstants.h"
#include "einkcmd.h"

// global data

static spinlock_t einklock;
devfs_handle_t	eink_devfs_handle = 0;

#define BYTE unsigned char
#define bool int

#define PIO_MAJOR			129
#define PIO_DEVICE_NAME		"einkdrv"


#define GPDCON 0x56000030
#define GPDDAT 0x56000034
#define GPDUP  0x56000038

#define GPCCON 0x56000020
#define GPCDAT 0x56000024
#define GPCUP  0x56000028


#define TimeOutValue1 0x1000000
#define TimeOutValue 0x1000000





typedef struct 
{
	int CD:1;
	int WUP:1;
	int NRST:1;
	int RW:1;
	int DS:1;
	int ACK:1;
	int bit7:1;
	int PWR_APLO:1;
}EINK_DATAREG;

typedef struct 
{
	int CD:2;
	int WUP:2;
	int NRST:2;
	int RW:2;	
	int DS:2;
	int ACK:2;
	int bit7:2;
	int PWR_APLO:2;
}EINK_CONREG;

typedef struct 
{
	int bit0_7:8;
	unsigned int DATA:8;	
}EINK_DATAREG2;

typedef struct 
{
	int bit0_7:16;	
	unsigned int DATA:16;	
}EINK_CONREG2;


#define PD_IN  0x0
#define PD_OUT 0x01

#define PDDATA_IN  0x00
#define PDDATA_OUT 0x5555

EINK_CONREG eink_conreg;
EINK_DATAREG eink_datareg;
EINK_CONREG2 eink_conreg2;
EINK_DATAREG2 eink_datareg2;

volatile EINK_CONREG *eink_conregp;
volatile EINK_DATAREG *eink_dataregp;
volatile EINK_CONREG2 *eink_conregp2;
volatile EINK_DATAREG2 *eink_dataregp2;




void init_EINK()
{	
	eink_conregp=(volatile EINK_CONREG *)ioremap_nocache(GPDCON,4);
	eink_dataregp=(volatile EINK_DATAREG *)ioremap_nocache(GPDDAT,4);
	eink_conregp2=(volatile EINK_CONREG2 *)ioremap_nocache(GPCCON,4);
	eink_dataregp2=(volatile EINK_DATAREG2 *)ioremap_nocache(GPCDAT,4);

	eink_conreg=*eink_conregp;	
	eink_conreg.CD=PD_OUT;
	eink_conreg.RW=PD_OUT;
	eink_conreg.DS=PD_OUT;
	eink_conreg.ACK=PD_IN;	
	eink_conreg.NRST=PD_OUT;
	eink_conreg.WUP=PD_OUT;		
	eink_conreg.PWR_APLO=PD_OUT;		
	*eink_conregp=eink_conreg;


	eink_conreg2=*eink_conregp2;	
	eink_conreg2.DATA=PDDATA_IN;	
	*eink_conregp2=eink_conreg2;
	

	eink_datareg=*eink_dataregp;
	eink_datareg.PWR_APLO=1;			// DS is High by default
	eink_datareg.DS=1;				// DS is High by default
	eink_datareg.WUP=0;		
	eink_datareg.NRST=1;	
	*eink_dataregp=eink_datareg;
	
		
	eink_datareg.NRST=0;
	*eink_dataregp=eink_datareg;		
	eink_datareg.NRST=1;		
	*eink_dataregp=eink_datareg;	
}



int WriteData(BYTE Data )
{
   int Check;
   int  TimeOut;   
//   printk("W1\n");

	eink_datareg=*eink_dataregp;
	eink_datareg.PWR_APLO=1;			// DS is High by default	
	eink_datareg.CD=0;				//CD=0 for data
	eink_datareg.RW=0;				//RW=0 for Write		
	*eink_dataregp=eink_datareg;		
//   	printk("W2\n");
	
	eink_conreg2=*eink_conregp2;	
	eink_conreg2.DATA=PDDATA_OUT;	// Set data port as output
	*eink_conregp2=eink_conreg2;
//  	 printk("W3\n");
	
	
	eink_datareg2=*eink_dataregp2;	
	eink_datareg2.DATA=Data;			//Write Data
	*eink_dataregp2=eink_datareg2;		
	
//	  printk("W4\n");

	
	eink_datareg.DS=0;			//DS=0, for write
	*eink_dataregp=eink_datareg;		
	
//   	printk("W5\n");
	
	
	
   TimeOut = 0;
   while (((*eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
   {
      TimeOut++;
//     printk("W1");
   }
   Check = (TimeOut < TimeOutValue);
   
	eink_datareg.DS=1;			//DS=1
	*eink_dataregp=eink_datareg;		
	
//      printk("W6\n");

   
   if (Check)
   {
      //{ Wait until Ack is 1 }
      TimeOut = 0;
      while ((!(*eink_dataregp).ACK) && (TimeOut < TimeOutValue))
      {
        TimeOut++;
//        printk("W2");
      }
      Check = (TimeOut < TimeOutValue);
   }
   
	eink_conreg2.DATA=PDDATA_IN;	// Set data port as input
	*eink_conregp2=eink_conreg2;
	
	   //printk("W7\n");

   
   return Check;	
}



void Wake_UP(void)
{
   int Check;
   int  TimeOut;   

//	printk("Wake-up Start \n");

	eink_datareg=*eink_dataregp;
	eink_datareg.DS=0;				 
	*eink_dataregp=eink_datareg;
	eink_datareg.WUP=1;				// WUP set high to wake up CPU	
	*eink_dataregp=eink_datareg;

#if 0
   TimeOut = 0;
   
   while ((!(*eink_dataregp).ACK) && (TimeOut < 0x100))
   {
      TimeOut++;
   }
#endif	
	
   TimeOut = 0;
   
   while (((*eink_dataregp).ACK) && (TimeOut < 0x1000000))
   {
      TimeOut++;
   }

	eink_datareg.WUP=0;					
	*eink_dataregp=eink_datareg;
   
   eink_datareg.DS=1;    
	*eink_dataregp=eink_datareg;

//	printk("Wake-up End \n");	
   
   return Check;	
}



int WriteDataBuf(BYTE *Data,int len)
{
	EINK_DATAREG L_eink_datareg,L_eink_dataregDS1,L_eink_dataregDS0;
	EINK_DATAREG2 L_eink_datareg2;
	volatile EINK_DATAREG *L_eink_dataregp;
	volatile EINK_DATAREG2 *L_eink_dataregp2;
//	volatile char *L_eink_dataregp2_B;
	BYTE *DataMax=Data+len;
  	 int Check;
  	 int  TimeOut;   
  	 int i;
//   printk("W1\n");


//	spin_lock(&einklock);

	L_eink_dataregp=eink_dataregp;
	L_eink_dataregp2=eink_dataregp2;
//	L_eink_dataregp2_B=(char *)eink_dataregp2;

	L_eink_datareg=*L_eink_dataregp;
	L_eink_datareg.PWR_APLO=1;			// DS is High by default	
	L_eink_datareg.CD=0;				//CD=0 for data
	L_eink_datareg.RW=0;				//RW=0 for Write		
	*L_eink_dataregp=L_eink_datareg;		
	
  // printk("W2\n");
	
	eink_conreg2=*eink_conregp2;	
	eink_conreg2.DATA=PDDATA_OUT;	// Set data port as output
	*eink_conregp2=eink_conreg2;
   //printk("W3\n");
	

	L_eink_datareg2=*L_eink_dataregp2;	
	
	
	// First Byte
	{
		L_eink_datareg2.DATA=*Data++;			//Write Data
		*L_eink_dataregp2=L_eink_datareg2;		
	
		L_eink_datareg.DS=0;			//DS=0, for write
		L_eink_dataregDS0=L_eink_datareg;
		*L_eink_dataregp=L_eink_datareg;		

   		TimeOut = 0;
   		
   		while (((*L_eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
  	 	{
      		TimeOut++;
  //   		printk("W3");

   		}	
   
		L_eink_datareg.DS=1;			//DS=1
		L_eink_dataregDS1=L_eink_datareg;
		*L_eink_dataregp=L_eink_datareg;		
   		
	}
	
//	L_eink_datareg2.DATA=*Data++;
	
	while(Data<DataMax)
	{		
		L_eink_datareg2.DATA=*Data++;			//Write Data
		*L_eink_dataregp2=L_eink_datareg2;		

#if 1
      		//{ Wait until Ack is 1 }      		
 //  		TimeOut = 0;
   		while ((!(*L_eink_dataregp).ACK) && (TimeOut < TimeOutValue))
  		{
//      		TimeOut++;
//      		printk("W4");
   		}      		
   		
//   		printk("W4=%d",TimeOut);

   		
#endif   		
		
		*L_eink_dataregp=L_eink_dataregDS0;		//DS=0, for write   
		
		
//		L_eink_datareg2.DATA=*Data++;			//Write Data
		
#if 1
//   		TimeOut = 0;   		
   		while (((*L_eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
  	 	{
//      		TimeOut++;
//      		printk("W5");
   		}	      		
//   		printk("W5=%d",TimeOut);
   		
#endif   		 		
		
		*L_eink_dataregp=L_eink_dataregDS1;		//DS=1		
			
	}
	
	
   
	eink_conreg2.DATA=PDDATA_IN;	// Set data port as input
	*eink_conregp2=eink_conreg2;
	
	   //printk("W7\n");


//	spin_unlock(&einklock);

	Check=1;   
   return Check;	
}




int WriteCommand(BYTE Data )
{


   int Check;
   int  TimeOut;
	
	eink_datareg=*eink_dataregp;
	eink_datareg.PWR_APLO=1;			// DS is High by default
	
	eink_datareg.CD=1;				//CD=1 for command
	eink_datareg.RW=0;				//RW=0 for Write		
	*eink_dataregp=eink_datareg;		
	
	
	eink_conreg2=*eink_conregp2;
	eink_conreg2.DATA=PDDATA_OUT;	// Set data port as output
	*eink_conregp2=eink_conreg2;
	
	eink_datareg2=*eink_dataregp2;	
	eink_datareg2.DATA=Data;			//Write Data
	*eink_dataregp2=eink_datareg2;		
	
	
	eink_datareg.DS=0;			//DS=0, for write
	*eink_dataregp=eink_datareg;		

if(Data==dc_DisplayPartial || Data==dc_DisplayImage || Data==dc_Rotate)
{	
   TimeOut = 0;
   while (((*eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
   {
      TimeOut++;
//	    printk("x1");		
//		wait_ms(10);
//		usleep(10000);
		udelay(10000);

   }
//	printk("x1=%d \n",TimeOut);  //by wanghp
   
   
   Check = (TimeOut < TimeOutValue);
   
	eink_datareg.DS=1;			//DS=1
	*eink_dataregp=eink_datareg;		
   
   if (Check)
   {
      //{ Wait until Ack is 1 }
      TimeOut = 0;
      while ((!(*eink_dataregp).ACK) && (TimeOut < TimeOutValue))
      {
        TimeOut++;
//		printk("x2");
      }
      
//		printk("x2=%d \n",TimeOut);

      
      Check = (TimeOut < TimeOutValue);
   }
   
}
else
{
	
   TimeOut = 0;
   while (((*eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
   {
      TimeOut++;
//  		printk("x3");

   }
   
//	printk("x3=%d \n",TimeOut);

   Check = (TimeOut < TimeOutValue);
   
	eink_datareg.DS=1;			//DS=1
	*eink_dataregp=eink_datareg;		
   
   if (Check)
   {
      //{ Wait until Ack is 1 }
      TimeOut = 0;
      while ((!(*eink_dataregp).ACK) && (TimeOut < TimeOutValue))
      {
        TimeOut++;
        
//        printk("x4");
      }
      
//  		printk("x4=%d \n",TimeOut);

      
      Check = (TimeOut < TimeOutValue);
   }
	
	
	
}
   
   
	eink_conreg2.DATA=PDDATA_IN;	// Set data port as output
	*eink_conregp2=eink_conreg2;
	
	eink_datareg.CD=0;				//CD=1 for command
	*eink_dataregp=eink_datareg;			
   
   return Check;	
}



int ReadData(BYTE *Data )
{



   int Check;
   int  TimeOut;
   
   eink_conreg2=*eink_conregp2;
	eink_conreg2.DATA=PDDATA_IN;	// Set data port as input
	*eink_conregp2=eink_conreg2;
   
	
	eink_datareg=*eink_dataregp;
	eink_datareg.PWR_APLO=1;			// DS is High by default
	eink_datareg.CD=0;				//CD=0 for data
	eink_datareg.RW=1;				//RW=1 for Read		
	*eink_dataregp=eink_datareg;		
	
	
	eink_datareg.DS=0;			//DS=0, for write
	*eink_dataregp=eink_datareg;		
	
	
   TimeOut = 0;
   while (((*eink_dataregp).ACK) && (TimeOut < TimeOutValue1))
   {
      TimeOut++;
   }
   Check = (TimeOut < TimeOutValue);
   
   *Data=(*eink_dataregp2).DATA;
 
   
	eink_datareg.DS=1;			//DS=1
	*eink_dataregp=eink_datareg;		
   
   if (Check)
   {
      //{ Wait until Ack is 1 }
      TimeOut = 0;
      while ((!(*eink_dataregp).ACK) && (TimeOut < TimeOutValue))
      {
        TimeOut++;
      }
      Check = (TimeOut < TimeOutValue);
   }
   return Check;	
}


void test()
{
	int i;
	int j;
	
	
	for(i=0;i<0x100000;i++)
	{
		eink_datareg.CD=1;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.RW=1;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.WUP=1;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.NRST=1;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.DS=1;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		
		eink_datareg.CD=0;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.RW=0;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.WUP=0;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.NRST=0;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		eink_datareg.DS=0;
		*eink_dataregp=eink_datareg;				
		for(j=0;j<0x100;j++);
		
	}	
}










static int pio_open (struct inode * inode, struct file * file)
{
//	printk("open module\n");		//by wanghp
	return 0;
}

static int pio_release (struct inode * inode, struct file * file)
{
//	printk("release module\n");		//by wanghp
	return 0;
}

static void pio0_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
}

static void pio1_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
}



static int pio_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
		char *p=arg;
		int i;
		char data;
		int ret;
	
//		printk("ioctl \n");	
		
		
	
	switch (cmd)
	{		case CMD_Test:
//				printk("CMD_Test\n");		//by wanghp
				
				test();
				
				break;
				
			case CMD_TestReadData:	
				ReadData(&data);
				break;
			case CMD_TestWriteData:	
				for(i=0;i<256;i++)
				WriteData(i);
				
				break;
			
			case CMD_TestWriteCMD:	
				WriteCommand(0xAA);
				break;
			
			
			case CMD_WakeUp:
				Wake_UP();
				break;	
				
				
			case CMD_SendCommand:			
					
//		printk("Cmd ");
		
//					printk("C1");
					
		ret=SendCommand ((TDisplayCommand *)arg);
//					printk("C2");
		return ret;
				break;
				
	
	
	}
}

static int pio_check_media_change (void)
{
	return 0;
}

static int pio_revalidate (void)
{
	return 0;
}

static struct block_device_operations pio_fops = 
{
	owner:				THIS_MODULE,
	open:				pio_open,
	release:			pio_release,
	ioctl:				pio_ioctl,	
	check_media_change:	pio_check_media_change,
	revalidate:			pio_revalidate
};

#ifdef CONFIG_PM
struct pm_dev *eink_pm_dev;

bool SendCommand (TDisplayCommand *dCommand);

bool SetSleepCommand ()
{
	int Temp[5];	
	TDisplayCommand *DisplayCommand=(TDisplayCommand *)Temp;
	
   	DisplayCommand->Command      = dc_GoToSleep;
   	DisplayCommand->BytesToWrite = 0;
   	DisplayCommand->BytesToRead  = 0;
   	DisplayCommand->Owner        = 0;   
   	return SendCommand(DisplayCommand);
}

bool SetNormalCommand ()
{
	int Temp[5];	
	TDisplayCommand *DisplayCommand=(TDisplayCommand *)Temp;
	
   	DisplayCommand->Command      = dc_GoToNormal;
   	DisplayCommand->BytesToWrite = 0;
   	DisplayCommand->BytesToRead  = 0;
   	DisplayCommand->Owner        = 0;   
   	return SendCommand(DisplayCommand);
}


static int eink_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{

	if (req == PM_SUSPEND) {		
		printk("EINK suspend\n");								
		SetSleepCommand ();
		
	} else if (req == PM_RESUME) 
	{			  
		printk("EINK resume\n");				
		SetNormalCommand ();
		
	}
	return 0;
}

#endif 




int Eink_init_module ()
{
	
	unsigned int maj, min, status, pio6;
	
//	printk("Kernel Eink init (start)\n");

	// register block driver.
	// if kernel doesnt support devfs, devfs_register returns NULL
	
	if ((eink_devfs_handle = devfs_register(NULL, PIO_DEVICE_NAME,
			DEVFS_FL_DEFAULT, PIO_MAJOR, 0,
			S_IFBLK | S_IRUGO | S_IWUGO,
			&pio_fops, NULL)))
	{
		devfs_get_maj_min (eink_devfs_handle, &maj, &min);
	}
	if (eink_devfs_handle == 0)
	{
		return -1;
	}

	init_EINK();
//	printk("Kernel Eink init (end)\n");

#ifdef CONFIG_PM
//	printk("Register eink PM\n");
	eink_pm_dev = pm_register(PM_USER_DEV, 0,
					 eink_pm_callback);
#endif		

	return 0;
}


void Eink_cleanup_module ()
{
	devfs_unregister (eink_devfs_handle);
	devfs_unregister_blkdev (PIO_MAJOR, PIO_DEVICE_NAME);	
}



module_init(Eink_init_module);
module_exit(Eink_cleanup_module);
