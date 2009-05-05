/*
PVI PM1
 */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dio.h>
#include <linux/slab.h>                         /* kmalloc() */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/ioctl.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/pm.h>



#include <asm/arch/S3C2410.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>

//#include <asm/hwtest.h>                           /* hwreg_present() */
//#include <asm/io.h>                               /* readb() */


#define CMD_INIT 10000
#define CMD_Test 0
#define CMD_USB_PC 1
#define CMD_USB_eBR 2
#define CMD_USB_RESET 3

#define CMD_LED1 100
#define CMD_LED2 101
#define CMD_LED4 102
#define CMD_GET_PINMODE 103

#define CMD_KEYPAD 		106
#define CMD_POWER  		107
#define CMD_USB_PW		108
#define CMD_AC_IN		109
#define CMD_SW_OFF		110
#define CMD_PWR_APLO		111
#define CMD_PWR			112
#define CMD_SET_KEYPAD_MODE	113
#define CMD_SET_EINT_MODE	114
#define CMD_INIT_APLO	115
#define CMD_MPWR		116
#define CMD_SD_IN		117
#define CMD_APLO_CONF_INPUT_MOD	118
#define CMD_APLO_CONF_OUTPUT_MOD 119
#define CMD_USB_DISCONNECT 120
#define CMD_USB_CONNECT 121

#define Not_Use_RS232 0

// global data

devfs_handle_t	pvi_devfs_handle = 0;
devfs_handle_t	devfs_handlepow=0;

void register_pm(void);

struct pm_dev *pviio_pm_dev;

int wake_up=0;


#define BYTE unsigned char
#define bool int

//#define PIO_MAJOR			133				//include kernel
//#define PIO_DEVICE_NAME	"pvi_io"			//include kernel

#define  PIO_MAJOR	155
#define PIO_DEVICE_NAME	"pvi_ioc"



//#define PIO_MAJORPOW			156			//jeremy 2
//#define PIO_DEVICE_NAMEPOW	"pvi_iocpow"	// jeremy 2


#define GPACON1 0x56000000
#define GPADAT1 0x56000004

#define GPBCON1 0x56000010
#define GPBDAT1 0x56000014
#define GPBUP1  0x56000018

#define GPCCON1 0x56000020
#define GPCDAT1 0x56000024
#define GPCUP1  0x56000028

#define GPDCON1 0x56000030
#define GPDDAT1 0x56000034
#define GPDUP1  0x56000038

#define GPECON1 0x56000040
#define GPEDAT1 0x56000044
#define GPEUP1  0x56000048

#define GPFCON1 0x56000050
#define GPFDAT1 0x56000054
#define GPFUP1  0x56000058

#define GPGCON1 0x56000060
#define GPGDAT1 0x56000064
#define GPGUP1  0x56000068

#define GPHCON1 0x56000070
#define GPHDAT1 0x56000074
#define GPHUP1  0x56000078



#define TimeOutValue 0x1000000


//*************************************************************//
//***					  		B Port						***//
//*************************************************************//
typedef struct 
{
	int bit0:1;
	int bit1_11:11;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
	int bit16:1;
	int bit17:1;
	int bit18:1;
	int bit19:1;
	int bit20:1;
	int bit21:1;
	int bit22:1;
} PORTA_DATAREG;

typedef struct 
{
	int bit0:1;
	int bit1_11:11;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
	int bit16:1;
	int bit17:1;
	int bit18:1;
	int bit19:1;
	int bit20:1;
	int bit21:1;
	int bit22:1;
} PORTA_CONREG;

//*************************************************************//
//***					  LED struct	(B Port)			***//
//*************************************************************//
typedef struct 
{
	int bit0:1;		//gp_b0
	int bit1:1;		//gp_b1
	int bit2:1;		//gp_b2
	int bit3:1;		//gp_b3
	int bit4:1;		//gp_b4
	int bit5:1;		//gp_b5
	int bit6:1;		//gp_b6
	int wave:1;		//gp_b7
	int led1:1;		//gp_b8
	int bit9:1;		//gp_b9
	int test:1;		//gp_b10
}LED_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int wave:2;
	int led1:2;
	int bit9:2;
	int test:2;
}LED_CONREG;

typedef struct
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int wave:1;
	int led1:1;
	int bit9:1;
	int test:1;
}LED_UPREG;

//*************************************************************//
//***					  Aplo Bus struct	(C Port)		***//
//*************************************************************//
typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int h_d0:1;
	int h_d1:1;
	int h_d2:1;
	int h_d3:1;
	int h_d4:1;
	int h_d5:1;
	int h_d6:1;
	int h_d7:1;
}APLO_BUS_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int h_d0:2;
	int h_d1:2;
	int h_d2:2;
	int h_d3:2;
	int h_d4:2;
	int h_d5:2;
	int h_d6:2;
	int h_d7:2;
}APLO_BUS_CONREG;

typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int h_d0:1;
	int h_d1:1;
	int h_d2:1;
	int h_d3:1;
	int h_d4:1;
	int h_d5:1;
	int h_d6:1;
	int h_d7:1;
}APLO_BUS_UPREG;

//*************************************************************//
//***			    	Power  struct(D port)				***//
//*************************************************************//
typedef struct 
{
	int h_cd:1;		//gp_d0
	int h_wup:1;	//gp_d1
	int h_nrst:1;	//gp_d2
	int h_rw:1;		//gp_d3
	int h_ds:1;		//gp_d4
	int h_ack:1;	//gp_d5
	int bit6:1;		//gp_d6	
	int pwr_aplo:1;	//gp_d7
	int mpwr:1;		//gp_d8	(USB Power)
	int bit9:1;		//gp_d9
	int pwr:1;		//gp_d10
	int usb_reset:1;//gp_d11	
	int usb_sel:1;	//gp_d12
	int bit13:1;	//gp_d13
	int bit14:1;	//gp_d14
	int bit15:1;	//gp_d15
}PWR_USB_DATAREG;

typedef struct 
{
	int h_cd:2;		//gp_d0
	int h_wup:2;	//gp_d1
	int h_nrst:2;	//gp_d2
	int h_rw:2;		//gp_d3
	int h_ds:2;		//gp_d4
	int h_ack:2;	//gp_d5
	int bit6:2;		//gp_d6	
	int pwr_aplo:2;	//gp_d7
	int mpwr:2;		//gp_d8	(USB Power)
	int bit9:2;		//gp_d9
	int pwr:2;		//gp_d10
	int usb_reset:2;//gp_d11	
	int usb_sel:2;	//gp_d12
	int bit13:2;	//gp_d13
	int bit14:2;	//gp_d14
	int bit15:2;	//gp_d15
}PWR_USB_CONREG;

typedef struct 
{
	int h_cd:1;		//gp_d0
	int h_wup:1;	//gp_d1
	int h_nrst:1;	//gp_d2
	int h_rw:1;		//gp_d3
	int h_ds:1;		//gp_d4
	int h_ack:1;	//gp_d5
	int bit6:1;		//gp_d6	
	int pwr_aplo:1;	//gp_d7
	int mpwr:1;		//gp_d8	(USB Power)
	int bit9:1;		//gp_d9
	int pwr:1;		//gp_d10
	int usb_reset:1;//gp_d11	
	int usb_sel:1;	//gp_d12
	int bit13:1;	//gp_d13
	int bit14:1;	//gp_d14
	int bit15:1;	//gp_d15
}PWR_USB_UPREG;

//*************************************************************//
//***			    	Audio code(E port)					***//
//*************************************************************//
typedef struct 
{
	int bit0_4:5;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
	int bit11:1;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
} PORTE_DATAREG;

typedef struct 
{
	int bit0_4:10;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
	int bit11:2;
	int bit12:2;
	int bit13:2;
	int bit14:2;
	int bit15:2;
} PORTE_CONREG;

typedef struct 
{
	int bit0_4:5;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
	int bit11:1;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
} PORTE_UPREG;

//*************************************************************//
//***			   Keypad & Usb Pw struct	(F Port)		***//
//*************************************************************//
typedef struct
{
	int up:1;		//gp_f0
	int usb_pw:1;	//gp_f1
	int dn:1;		//gp_f2
	int music:1;	//gp_f3
	int menu:1;		//gp_f4
	int ret:1;		//gp_f5
	int ent:1;		//gp_f6
	int sw_off:1;	//gp_f7	
}KEYPAD_DATAREG;


typedef struct
{
	int up:2;
	int usb_pw:2;
	int dn:2;
	int music:2;
	int menu:2;
	int ret:2;
	int ent:2;
	int sw_off:2;
}KEYPAD_CONREG;

typedef struct
{
	int up:1;
	int usb_pw:1;
	int dn:1;
	int music:1;
	int menu:1;
	int ret:1;
	int ent:1;
	int sw_off:1;
}KEYPAD_UPREG;

//*************************************************************//
//***			   Keypad & AC & SD struct	(G Port)  		***//
//*************************************************************//
typedef struct 
{
	int ac_in:1;	//gp_g0 
	int bit1:1;		//gp_g1
	int ac_off:1;   //gp_g2 				
	int lf:1;		//gp_g3
	int sd_in:1;	//gp_g4
	int sd_ncd:1;	//gp_g5
	int del:1;		//gp_g6
	int rt:1;		//gp_g7	
	int vol_n:1;	//gp_g8	(vol-)	
	int vol_p:1; 	//gp_g9	(vol+)
	int bit10:1;	//gp_g10
	int bit11:1;	//gp_g11
	int bit12:1;	//gp_g12	
	int bit13:1;	//gp_g13
	int bit14:1;	//gp_g14
	int bit15:1;	//gp_g15
}KEYPAD_AC_DATAREG;

typedef struct 
{
	int ac_in:2;	//gp_g0 
	int bit1:2;		//gp_g1
	int ac_off:2;   //gp_g2 				
	int lf:2;		//gp_g3
	int sd_in:2;	//gp_g4
	int sd_ncd:2;	//gp_g5
	int del:2;		//gp_g6
	int rt:2;		//gp_g7	
	int vol_n:2;	//gp_g8	(vol-)	
	int vol_p:2; 	//gp_g9	(vol+)
	int bit10:2;	//gp_g10
	int bit11:2;	//gp_g11
	int bit12:2;	//gp_g12	
	int bit13:2;	//gp_g13
	int bit14:2;	//gp_g14
	int bit15:2;	//gp_g15
}KEYPAD_AC_CONREG;

typedef struct 
{
	int ac_in:1;	//gp_g0 
	int bit1:1;		//gp_g1
	int ac_off:1;   //gp_g2 				
	int lf:1;		//gp_g3
	int sd_in:1;	//gp_g4
	int sd_ncd:1;	//gp_g5
	int del:1;		//gp_g6
	int rt:1;		//gp_g7	
	int vol_n:1;	//gp_g8	(vol-)	
	int vol_p:1; 	//gp_g9	(vol+)
	int bit10:1;	//gp_g10
	int bit11:1;	//gp_g11
	int bit12:1;	//gp_g12	
	int bit13:1;	//gp_g13
	int bit14:1;	//gp_g14
	int bit15:1;	//gp_g15
}KEYPAD_AC_UPREG;

//*************************************************************//
//***					RS232 struct	(G Port) 			***//
//*************************************************************//
typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
} PORTH_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
} PORTH_CONREG;

typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
} PORTH_UPREG;



#define PA_OUT 	0x00
#define PD_IN  	0x00
#define PD_OUT 	0x01
#define PD_EINT 	0x02

#define PDDATA_IN  0x00
#define PDDATA_OUT 0x5555


//Port A
PORTA_CONREG	porta_conreg;
PORTA_DATAREG 	porta_datareg;

volatile PORTA_CONREG 	*porta_conregp;
volatile PORTA_DATAREG 	*porta_dataregp;

//Port B
LED_CONREG				led_conreg;
LED_DATAREG 			led_datareg;
LED_UPREG				led_upreg;

volatile LED_CONREG 		*led_conregp;
volatile LED_DATAREG 		*led_dataregp;
volatile LED_UPREG 		*led_upregp;

//Port C
APLO_BUS_CONREG	aplo_bus_conreg;
APLO_BUS_DATAREG	aplo_bus_datareg;
APLO_BUS_UPREG		aplo_bus_upreg;

volatile APLO_BUS_CONREG		*aplo_bus_conregp;
volatile APLO_BUS_DATAREG	*aplo_bus_dataregp;
volatile APLO_BUS_UPREG		*aplo_bus_upregp;

//Port D
PWR_USB_CONREG			pwr_usb_conreg;
PWR_USB_DATAREG			pwr_usb_datareg;
PWR_USB_UPREG				pwr_usb_upreg;

volatile PWR_USB_CONREG		*pwr_usb_conregp;
volatile PWR_USB_DATAREG		*pwr_usb_dataregp;
volatile PWR_USB_UPREG		*pwr_usb_upregp;


//Port E
PORTE_CONREG	porte_conreg;
PORTE_DATAREG 	porte_datareg;
PORTE_UPREG 	porte_upreg;

volatile PORTE_CONREG 	*porte_conregp;
volatile PORTE_DATAREG 	*porte_dataregp;
volatile PORTE_UPREG 	*porte_upregp;
	

//Port F
KEYPAD_CONREG 			pvi_keypad_conreg;
KEYPAD_DATAREG 		keypad_datareg;
KEYPAD_UPREG 			keypad_upreg;

volatile KEYPAD_CONREG 	*pvi_keypad_conregp;
volatile KEYPAD_DATAREG 	*keypad_dataregp;
volatile KEYPAD_UPREG 	*keypad_upregp;

//Port G
KEYPAD_AC_CONREG		pvi_keypad_ac_conreg;
KEYPAD_AC_DATAREG 		keypad_ac_datareg;
KEYPAD_AC_UPREG		keypad_ac_upreg;

volatile KEYPAD_AC_CONREG		*pvi_keypad_ac_conregp;
volatile KEYPAD_AC_DATAREG 		*keypad_ac_dataregp;
volatile KEYPAD_AC_UPREG			*keypad_ac_upregp;

//Port H
PORTH_CONREG	porth_conreg;
PORTH_DATAREG 	porth_datareg;
PORTH_UPREG		porth_upreg;

volatile PORTH_CONREG 	*porth_conregp;
volatile PORTH_DATAREG 	*porth_dataregp;
volatile PORTH_UPREG	*porth_upregp;

//Port A
void init_porta(void)
{
	porta_conreg=*porta_conregp;
	porta_conreg.bit0=PA_OUT;
	porta_conreg.bit12=PA_OUT;
	porta_conreg.bit13=PA_OUT;
	porta_conreg.bit14=PA_OUT;
	porta_conreg.bit15=PA_OUT;
	porta_conreg.bit16=PA_OUT;
	porta_conreg.bit17=PA_OUT;
	porta_conreg.bit18=PA_OUT;
	porta_conreg.bit19=PA_OUT;
	porta_conreg.bit20=PA_OUT;
	porta_conreg.bit22=PA_OUT;
	*porta_conregp=porta_conreg;
	
	porta_datareg=*porta_dataregp;
	porta_datareg.bit0=1;
	porta_datareg.bit12=1;
	porta_datareg.bit13=1;
	porta_datareg.bit14=1;
	porta_datareg.bit15=1;
	porta_datareg.bit16=1;
	porta_datareg.bit17=1;
	porta_datareg.bit18=1;
	porta_datareg.bit19=1;
	porta_datareg.bit20=1;
	porta_datareg.bit22=1;	
	*porta_dataregp=porta_datareg;		
}


void init_led(void) //port B
{
	led_conreg=*led_conregp;
	led_conreg.bit0=PD_OUT;
	led_conreg.bit1=PD_OUT;
	led_conreg.bit5=PD_OUT;
	led_conreg.bit6=PD_OUT;
	led_conreg.bit9=PD_OUT;
	led_conreg.led1=PD_OUT;	
	led_conreg.test=PD_IN;	
	led_conreg.wave=PD_IN;
	*led_conregp=led_conreg; 
	
	led_datareg=*led_dataregp;
	led_datareg.bit0=1;
	led_datareg.bit1=1;
	led_datareg.bit5=1;	
	led_datareg.bit6=1;
	led_datareg.bit9=1;	
	led_datareg.led1=0;	
	*led_dataregp=led_datareg;		
}

void setLED1(int value)
{
	led_datareg=*led_dataregp;			
	led_datareg.led1=value;	
	*led_dataregp=led_datareg;				
}
#if(0)
void setLED2(int value)
{
	led_datareg=*led_dataregp;			
	led_datareg.led2=value;	
	*led_dataregp=led_datareg;				
}

void setLED4(int value)
{
	led_datareg=*led_dataregp;			
	led_datareg.led4=value;	
	*led_dataregp=led_datareg;				
}
#endif
void init_aplo_bus(void) //port C
{
	//Port C
	aplo_bus_conreg=*aplo_bus_conregp;
	aplo_bus_conreg.bit0=PD_OUT;
	aplo_bus_conreg.bit1=PD_OUT;
	aplo_bus_conreg.bit2=PD_OUT;
	aplo_bus_conreg.bit3=PD_OUT;
	aplo_bus_conreg.bit4=PD_OUT;
	aplo_bus_conreg.bit5=PD_OUT;
	aplo_bus_conreg.bit6=PD_OUT;
	aplo_bus_conreg.bit7=PD_OUT;
	aplo_bus_conreg.h_d0=PD_IN;
	aplo_bus_conreg.h_d1=PD_IN;
	aplo_bus_conreg.h_d2=PD_IN;
	aplo_bus_conreg.h_d3=PD_IN;
	aplo_bus_conreg.h_d4=PD_IN;
	aplo_bus_conreg.h_d5=PD_IN;
	aplo_bus_conreg.h_d6=PD_IN;
	aplo_bus_conreg.h_d7=PD_IN;
	*aplo_bus_conregp=aplo_bus_conreg;
	
	aplo_bus_datareg=*aplo_bus_dataregp;
	aplo_bus_datareg.bit0=1;
	aplo_bus_datareg.bit1=1;
	aplo_bus_datareg.bit2=1;
	aplo_bus_datareg.bit3=1;
	aplo_bus_datareg.bit4=1;
	aplo_bus_datareg.bit5=1;
	aplo_bus_datareg.bit6=1;
	aplo_bus_datareg.bit7=1;
	*aplo_bus_dataregp = aplo_bus_datareg;
	
	//Port D
	/*pwr_usb_conreg=*pwr_usb_conregp;
	pwr_usb_conreg.h_cd=PD_OUT;
	pwr_usb_conreg.h_wup=PD_OUT;
	pwr_usb_conreg.h_nrst=PD_OUT;
	pwr_usb_conreg.h_rw=PD_OUT;
	pwr_usb_conreg.h_ds=PD_OUT;
	pwr_usb_conreg.h_ack=PD_IN;
	*pwr_usb_conregp=pwr_usb_conreg;

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.h_ds=1;
	pwr_usb_datareg.h_wup=0;
	pwr_usb_datareg.h_nrst=1;
	pwr_usb_datareg.pwr_appollo=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
	
	pwr_usb_datareg.h_nrst=0;
 	*pwr_usb_dataregp=pwr_usb_datareg;
	pwr_usb_datareg.h_nrst=1;
	*pwr_usb_dataregp=pwr_usb_datareg;*/
	

	/*printk("aplo_bus_conreg.h_d0  		%x\n", aplo_bus_conreg.h_d0);
	printk("aplo_bus_conreg.h_d1		%x\n",aplo_bus_conreg.h_d1);
	printk("aplo_bus_conreg.h_d2		%x\n",aplo_bus_conreg.h_d2);
	printk("aplo_bus_conreg.h_d3		%x\n",aplo_bus_conreg.h_d3);
	printk("aplo_bus_conreg.h_d4		%x\n",aplo_bus_conreg.h_d4);
	printk("aplo_bus_conreg.h_d5		%x\n",aplo_bus_conreg.h_d5);
	printk("aplo_bus_conreg.h_d6		%x\n",aplo_bus_conreg.h_d6);
	printk("aplo_bus_conreg.h_d7		%x\n",aplo_bus_conreg.h_d7);

	printk("pwr_usb_conreg.h_cd		%x\n",pwr_usb_conreg.h_cd);
	printk("pwr_usb_conreg.h_wup		%x\n",pwr_usb_conreg.h_wup);
	printk("pwr_usb_conreg.h_nrst		%x\n",pwr_usb_conreg.h_nrst);
	printk("pwr_usb_conreg.h_rw		%x\n",pwr_usb_conreg.h_rw);
	printk("pwr_usb_conreg.h_dsT		%x\n",pwr_usb_conreg.h_ds);
	printk("pwr_usb_conreg.h_ack		%x\n",pwr_usb_conreg.h_ack);*/
}


void init_usb_Aplo(void) //Port D
{
	pwr_usb_conreg=*pwr_usb_conregp;
	pwr_usb_conreg.h_cd=PD_OUT;
	pwr_usb_conreg.h_wup=PD_OUT;
	pwr_usb_conreg.h_nrst=PD_OUT;
	pwr_usb_conreg.h_rw=PD_OUT;
	pwr_usb_conreg.h_ds=PD_OUT;
	pwr_usb_conreg.h_ack=PD_IN;
	pwr_usb_conreg.bit6=PD_OUT;
	pwr_usb_conreg.pwr_aplo=PD_OUT;
	pwr_usb_conreg.mpwr=PD_OUT;		//(usb power)
	pwr_usb_conreg.bit9=PD_OUT;
	pwr_usb_conreg.pwr=PD_OUT;
	pwr_usb_conreg.usb_reset=PD_OUT;
	pwr_usb_conreg.usb_sel=PD_OUT;
	pwr_usb_conreg.bit15=PD_OUT;
	*pwr_usb_conregp=pwr_usb_conreg;

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.bit6 =1;
	pwr_usb_datareg.bit9 =1;
	pwr_usb_datareg.bit15 =1;
	pwr_usb_datareg.usb_sel=1;
	pwr_usb_datareg.usb_reset=1;
	pwr_usb_datareg.h_ds=1;
	pwr_usb_datareg.h_wup=0;
	pwr_usb_datareg.h_nrst=1;
	pwr_usb_datareg.pwr=1;
	pwr_usb_datareg.mpwr=1;
	pwr_usb_datareg.pwr_aplo=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
	
	pwr_usb_datareg.h_nrst=0;
 	*pwr_usb_dataregp=pwr_usb_datareg;
	pwr_usb_datareg.h_nrst=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
}

void aplo_conf_input_mod(void)
{
	aplo_bus_conreg=*aplo_bus_conregp;
	aplo_bus_conreg.h_d0=PD_IN;
	aplo_bus_conreg.h_d1=PD_IN;
	aplo_bus_conreg.h_d2=PD_IN;
	aplo_bus_conreg.h_d3=PD_IN;
	aplo_bus_conreg.h_d4=PD_IN;
	aplo_bus_conreg.h_d5=PD_IN;
	aplo_bus_conreg.h_d6=PD_IN;
	aplo_bus_conreg.h_d7=PD_IN;
	*aplo_bus_conregp=aplo_bus_conreg;

	
	pwr_usb_conreg=*pwr_usb_conregp;
	pwr_usb_conreg.h_cd=PD_IN;
	pwr_usb_conreg.h_wup=PD_IN;
	pwr_usb_conreg.h_nrst=PD_IN;
	pwr_usb_conreg.h_rw=PD_IN;
	pwr_usb_conreg.h_ds=PD_IN;
	pwr_usb_conreg.h_ack=PD_IN;
	*pwr_usb_conregp=pwr_usb_conreg;
	
	/*aplo_bus_upreg=*aplo_bus_upregp;
	aplo_bus_upreg.h_d0=1;
	aplo_bus_upreg.h_d1=1;
	aplo_bus_upreg.h_d2=1;
	aplo_bus_upreg.h_d3=1;
	aplo_bus_upreg.h_d4=1;
	aplo_bus_upreg.h_d5=1;
	aplo_bus_upreg.h_d6=1;
	aplo_bus_upreg.h_d7=1;
	*aplo_bus_upregp=aplo_bus_upreg;*/
	
	/*pwr_usb_upreg=*pwr_usb_upregp;
	pwr_usb_upreg.h_cd=1;
	pwr_usb_upreg.h_wup=1;
	pwr_usb_upreg.h_nrst=1;
	pwr_usb_upreg.h_rw=1;
	pwr_usb_upreg.h_ds=1;
	pwr_usb_upreg.h_ack=1;
	*pwr_usb_upregp=pwr_usb_upreg;*/

/*	printk("aplo_bus_conreg.h_d0  		%x\n", aplo_bus_conreg.h_d0);
	printk("aplo_bus_conreg.h_d1		%x\n",aplo_bus_conreg.h_d1);
	printk("aplo_bus_conreg.h_d2		%x\n",aplo_bus_conreg.h_d2);
	printk("aplo_bus_conreg.h_d3		%x\n",aplo_bus_conreg.h_d3);
	printk("aplo_bus_conreg.h_d4		%x\n",aplo_bus_conreg.h_d4);
	printk("aplo_bus_conreg.h_d5		%x\n",aplo_bus_conreg.h_d5);
	printk("aplo_bus_conreg.h_d6		%x\n",aplo_bus_conreg.h_d6);
	printk("aplo_bus_conreg.h_d7		%x\n",aplo_bus_conreg.h_d7);

	printk("pwr_usb_conreg.h_cd		%x\n",pwr_usb_conreg.h_cd);
	printk("pwr_usb_conreg.h_wup		%x\n",pwr_usb_conreg.h_wup);
	printk("pwr_usb_conreg.h_nrst		%x\n",pwr_usb_conreg.h_nrst);
	printk("pwr_usb_conreg.h_rw		%x\n",pwr_usb_conreg.h_rw);
	printk("pwr_usb_conreg.h_ds		%x\n",pwr_usb_conreg.h_ds);
	printk("pwr_usb_conreg.h_ack		%x\n",pwr_usb_conreg.h_ack);*/
}

void aplo_conf_output_mod(void)
{
	aplo_bus_conreg=*aplo_bus_conregp;
	aplo_bus_conreg.h_d0=PD_OUT;
	aplo_bus_conreg.h_d1=PD_OUT;
	aplo_bus_conreg.h_d2=PD_OUT;
	aplo_bus_conreg.h_d3=PD_OUT;
	aplo_bus_conreg.h_d4=PD_OUT;
	aplo_bus_conreg.h_d5=PD_OUT;
	aplo_bus_conreg.h_d6=PD_OUT;
	aplo_bus_conreg.h_d7=PD_OUT;
	*aplo_bus_conregp=aplo_bus_conreg;
	
	pwr_usb_conreg=*pwr_usb_conregp;
	pwr_usb_conreg.h_cd=PD_OUT;
	pwr_usb_conreg.h_wup=PD_OUT;
	pwr_usb_conreg.h_nrst=PD_OUT;
	pwr_usb_conreg.h_rw=PD_OUT;
	pwr_usb_conreg.h_ds=PD_OUT;
	pwr_usb_conreg.h_ack=PD_OUT;
	*pwr_usb_conregp=pwr_usb_conreg;
	

	/*printk("aplo_bus_conreg.h_d0  		%x\n", aplo_bus_conreg.h_d0);
	printk("aplo_bus_conreg.h_d1		%x\n",aplo_bus_conreg.h_d1);
	printk("aplo_bus_conreg.h_d2		%x\n",aplo_bus_conreg.h_d2);
	printk("aplo_bus_conreg.h_d3		%x\n",aplo_bus_conreg.h_d3);
	printk("aplo_bus_conreg.h_d4		%x\n",aplo_bus_conreg.h_d4);
	printk("aplo_bus_conreg.h_d5		%x\n",aplo_bus_conreg.h_d5);
	printk("aplo_bus_conreg.h_d6		%x\n",aplo_bus_conreg.h_d6);
	printk("aplo_bus_conreg.h_d7		%x\n",aplo_bus_conreg.h_d7);

	printk("pwr_usb_conreg.h_cd		%x\n",pwr_usb_conreg.h_cd);
	printk("pwr_usb_conreg.h_wup		%x\n",pwr_usb_conreg.h_wup);
	printk("pwr_usb_conreg.h_nrst		%x\n",pwr_usb_conreg.h_nrst);
	printk("pwr_usb_conreg.h_rw		%x\n",pwr_usb_conreg.h_rw);
	printk("pwr_usb_conreg.h_ds		%x\n",pwr_usb_conreg.h_ds);
	printk("pwr_usb_conreg.h_ack		%x\n",pwr_usb_conreg.h_ack);*/
	
	
	aplo_bus_datareg=*aplo_bus_dataregp;
	aplo_bus_datareg.h_d0=0;
	aplo_bus_datareg.h_d1=0;
	aplo_bus_datareg.h_d2=0;
	aplo_bus_datareg.h_d3=0;
	aplo_bus_datareg.h_d4=0;
	aplo_bus_datareg.h_d5=0;
	aplo_bus_datareg.h_d6=0;
	aplo_bus_datareg.h_d7=0;
	*aplo_bus_dataregp=aplo_bus_datareg;
		
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.h_cd=0;
	pwr_usb_datareg.h_wup=0;
	pwr_usb_datareg.h_nrst=0;
	pwr_usb_datareg.h_rw=0;
	pwr_usb_datareg.h_ds=0;
	pwr_usb_datareg.h_ack=0;
	*pwr_usb_dataregp=pwr_usb_datareg;
}

//Port E
void init_porte(void)
{
	porte_conreg=*porte_conregp;
	porte_conreg.bit5=PD_OUT;
	porte_conreg.bit6=PD_OUT;
	porte_conreg.bit7=PD_OUT;
	porte_conreg.bit8=PD_OUT;
	porte_conreg.bit9=PD_OUT;
	porte_conreg.bit10=PD_OUT;
	porte_conreg.bit11=PD_OUT;
	porte_conreg.bit12=PD_OUT;	
	porte_conreg.bit13=PD_OUT;	
	porte_conreg.bit14=PD_OUT;	
	porte_conreg.bit15=PD_OUT;	
	*porte_conregp=porte_conreg;
	
	porte_datareg=*porte_dataregp;
	porte_datareg.bit5=1;
	porte_datareg.bit6=1;
	porte_datareg.bit7=1;
	porte_datareg.bit8=1;
	porte_datareg.bit9=1;	
	porte_datareg.bit10=1;	
	porte_datareg.bit11=1;	
	porte_datareg.bit12=1;	
	porte_datareg.bit13=1;	
	porte_datareg.bit14=1;	
	porte_datareg.bit15=1;	
	*porte_dataregp=porte_datareg;		
}



void init_keypad(void) //port F¡BG
{
	//	printk("driver layer :init_keypad()..........\n");	
	pvi_keypad_conreg=*pvi_keypad_conregp;
	pvi_keypad_conreg.up=PD_IN;
	pvi_keypad_conreg.usb_pw =PD_IN;
	pvi_keypad_conreg.dn=PD_IN;
	pvi_keypad_conreg.music=PD_IN;
	pvi_keypad_conreg.menu=PD_IN;
	pvi_keypad_conreg.ret=PD_IN;
	pvi_keypad_conreg.ent=PD_IN;
	pvi_keypad_conreg.sw_off=PD_IN;
	*pvi_keypad_conregp=pvi_keypad_conreg;	

	pvi_keypad_ac_conreg=*pvi_keypad_ac_conregp;
	pvi_keypad_ac_conreg.ac_in=PD_IN;
	pvi_keypad_ac_conreg.bit1=PD_OUT;
	pvi_keypad_ac_conreg.ac_off=PD_IN;
	pvi_keypad_ac_conreg.lf=PD_IN;
	pvi_keypad_ac_conreg.sd_in=PD_IN;
	pvi_keypad_ac_conreg.sd_ncd=PD_IN;
	pvi_keypad_ac_conreg.del=PD_IN;
	pvi_keypad_ac_conreg.rt=PD_IN;
	pvi_keypad_ac_conreg.vol_p=PD_IN;
	pvi_keypad_ac_conreg.vol_n=PD_IN;
	pvi_keypad_ac_conreg.bit10=PD_OUT;
	pvi_keypad_ac_conreg.bit11=PD_OUT;
	pvi_keypad_ac_conreg.bit12=PD_OUT;
	pvi_keypad_ac_conreg.bit13=PD_OUT;
	pvi_keypad_ac_conreg.bit14=PD_OUT;
	pvi_keypad_ac_conreg.bit15=PD_OUT;
	*pvi_keypad_ac_conregp=pvi_keypad_ac_conreg;

	/*keypad_upreg=*keypad_upregp;
	keypad_upreg.up=1;
	keypad_upreg.usb_pw=1;
	keypad_upreg.dn=1;
	keypad_upreg.music=1;
	keypad_upreg.menu=1;
	keypad_upreg.ret=1;
	keypad_upreg.ent=1;
	keypad_upreg.sw_off=1;
	*keypad_upregp=keypad_upreg;*/

	keypad_ac_datareg=*keypad_ac_dataregp;
	keypad_ac_datareg.bit1=1;
	keypad_ac_datareg.bit10=1;
	keypad_ac_datareg.bit11=1;
	keypad_ac_datareg.bit12=1;
	keypad_ac_datareg.bit13=1;
	keypad_ac_datareg.bit14=1;
	keypad_ac_datareg.bit15=1;
	*keypad_ac_dataregp=keypad_ac_datareg;
	
	/*keypad_ac_upreg=*keypad_ac_upregp;
	keypad_ac_upreg.ac_in=1;
	keypad_ac_upreg.ac_off=1;
	keypad_ac_upreg.lf=1;
	keypad_ac_upreg.sd_in=1;
	keypad_ac_upreg.sd_ncd=1;
	keypad_ac_upreg.del=1;
	keypad_ac_upreg.rt=1;
	keypad_ac_upreg.vol_p=1;
	keypad_ac_upreg.vol_n=1;
	*keypad_ac_upregp=keypad_ac_upreg;*/
}

//Port H
void init_porth(void)
{
	porth_conreg=*porth_conregp;
	porth_conreg.bit0=PD_OUT;
	porth_conreg.bit1=PD_OUT;
	porth_conreg.bit2=PD_OUT;
	porth_conreg.bit3=PD_OUT;
	porth_conreg.bit4=PD_OUT;
	porth_conreg.bit5=PD_OUT;
	porth_conreg.bit6=PD_OUT;
	porth_conreg.bit7=PD_OUT;	
	porth_conreg.bit8=PD_OUT;	
	*porth_conregp=porth_conreg;
	
	porth_datareg=*porth_dataregp;
	porth_datareg.bit0=1;
	porth_datareg.bit1=1;
	porth_datareg.bit2=1;
	porth_datareg.bit3=1;
	porth_datareg.bit4=1;	
	porth_datareg.bit5=1;	
	porth_datareg.bit6=1;	
	porth_datareg.bit7=1;	
	porth_datareg.bit8=1;	
	*porth_dataregp=porth_datareg;		
}
void get_pinmode (int *inputvalue)
{
	int i;
	i = 0;
	led_datareg = *led_dataregp;	
	*inputvalue = 0x00;
	if(led_datareg.test == 0)
	{
		i |= 0x01;
	}
	if(led_datareg.wave == 0)
	{	
		i |= 0x10;
	}
	*inputvalue = i; 
}
void get_keypad (int *inputvalue )
{
		
//	printk("driver layer :get_keypad()...............................................................\n");
	if(wake_up)
	{
//		printk("wake-up: First Get Key \n");	
		wake_up=0;	
	}
	else
	{	
		keypad_datareg=*keypad_dataregp;
		keypad_ac_datareg=*keypad_ac_dataregp;
//	led_datareg=*led_dataregp;
	}
	*inputvalue=0x0;
	if(keypad_datareg.ret==0)
	*inputvalue|=1;
	if(keypad_ac_datareg.del==0)
	*inputvalue|=(1<<1);
	if(keypad_ac_datareg.lf==0)
	*inputvalue|=(1<<2);
	if(keypad_datareg.menu==0)
	*inputvalue|=(1<<3);
	if(keypad_datareg.music==0)
	*inputvalue|=(1<<4);
	if(keypad_datareg.ent==0)
	*inputvalue|=(1<<5);
	if(keypad_ac_datareg.rt==0)		
	*inputvalue|=(1<<6);
	if(keypad_ac_datareg.vol_n==0)
	*inputvalue|=(1<<7);
	if(keypad_ac_datareg.vol_p==0)
	*inputvalue|=(1<<8);
	if(keypad_datareg.dn==0)
	*inputvalue|=(1<<9);
	if(keypad_datareg.up==0)
	*inputvalue|=(1<<10);
}

/* void init_power()
{
//	printk("driver layer :init_power()..........\n");	
	pwr_usb_conreg=*pwr_usb_conregp;
	pwr_usb_conreg.pwr_aplo=PD_OUT;
	pwr_usb_conreg.pwr=PD_OUT;
	pwr_usb_conreg.mpwr=PD_OUT;		//(usb power)
	*pwr_usb_conregp=pwr_usb_conreg;

	pvi_keypad_conreg=*pvi_keypad_conregp;
	pvi_keypad_conreg.usb_pw=PD_IN;		//(Usb In)
	pvi_keypad_conreg.sw_off=PD_IN;
	*pvi_keypad_conregp=pvi_keypad_conreg;

	pvi_keypad_ac_conreg=*pvi_keypad_ac_conregp;
	pvi_keypad_ac_conreg.ac_in=PD_IN;
	pvi_keypad_ac_conreg.ac_off=PD_IN;
	pvi_keypad_ac_conreg.sd_in=PD_IN;
	pvi_keypad_ac_conreg.sd_ncd=PD_IN;
	*pvi_keypad_ac_conregp=pvi_keypad_ac_conreg;

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_aplo=1;
	pwr_usb_datareg.pwr=1;
	pwr_usb_datareg.mpwr=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
}*/

void get_sd_in(int *inputvalue)
{
	/*int temp_pwr_appollo=0;
	int temp_pwr=0;
	int temp_usb_pwr=0;

	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_aplo;
	temp_pwr=pwr_usb_datareg.pwr;
	temp_usb_pwr=pwr_usb_datareg.mpwr;
	*pwr_usb_dataregp=pwr_usb_datareg;

	init_power();

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_aplo=temp_pwr_appollo;
	pwr_usb_datareg.pwr=temp_pwr;
	pwr_usb_datareg.mpwr=temp_usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;*/


	keypad_ac_datareg=*keypad_ac_dataregp;

	*inputvalue=0x0;

	//	printk("driver layer :get_power()..........\n");
	if(keypad_ac_datareg.sd_in!=0)
	{
//		printk("SD Card       plug-in   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
		*inputvalue=1;
	}
	else
	{
//		printk("SD Card       remove usb ------   LOW	!!!!!!!!!!!!!!!!!\n");	
		*inputvalue=0;
	}
}

void get_usb_power(int *inputvalue) //USB Plug_IN
{
	/*int temp_pwr_appollo=0;
	int temp_pwr=0;
	int temp_usb_pwr=0;

	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_appollo;
	temp_pwr=pwr_usb_datareg.pwr;
	temp_usb_pwr=pwr_usb_datareg.usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;


	init_power();


	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_appollo=temp_pwr_appollo;
	pwr_usb_datareg.pwr=temp_pwr;
	pwr_usb_datareg.usb_pwr=temp_usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;*/

	
	keypad_datareg=*keypad_dataregp;
	*inputvalue=0x0;

//	printk("driver layer :get_power()..........\n");
	if(keypad_datareg.usb_pw==0)
	{
//		printk("usb       plug-in   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
		*inputvalue=1;
	}
	else
	{
//		printk("usb       plug-in -------   LOW	!!!!!!!!!!!!!!!!!\n");	
		*inputvalue=0;
	}
}



void get_sw_off(int *inputvalue)
{

	/*int temp_pwr_appollo=0;
	int temp_pwr=0;
	int temp_usb_pwr=0;

	
	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_appollo;
	temp_pwr=pwr_usb_datareg.pwr;
	temp_usb_pwr=pwr_usb_datareg.usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;


	init_power();


	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_appollo=temp_pwr_appollo;
	pwr_usb_datareg.pwr=temp_pwr;
	pwr_usb_datareg.usb_pwr=temp_usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg; */

	keypad_datareg=*keypad_dataregp;

	*inputvalue=0x0;

//	printk("driver layer :get_sw_off()..........\n");
	if(keypad_datareg.sw_off!=0)
	{
//		printk("sw_on   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
		*inputvalue=1;
	}
	else
	{
//		printk("sw_off   -------   LOW	!!!!!!!!!!!!!!!!!\n");	
		*inputvalue=0;
	}
}


void get_ac_in(int *inputvalue)
{
	/*int temp_pwr_appollo=0;
	int temp_pwr=0;
	int temp_usb_pwr=0;

	
	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_appollo;
	temp_pwr=pwr_usb_datareg.pwr;
	temp_usb_pwr=pwr_usb_datareg.usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;

	init_power();

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_appollo=temp_pwr_appollo;
	pwr_usb_datareg.pwr=temp_pwr;
	pwr_usb_datareg.usb_pwr=temp_usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;*/

	keypad_ac_datareg=*keypad_ac_dataregp;

	*inputvalue=0x0;

//	printk("driver layer :get_ac_in()..........\n");
	if(keypad_ac_datareg.ac_off!=0)
	{
//		printk("ac_in   -------   LOW	!!!!!!!!!!!!!!!!!\n");
		*inputvalue=0;
	}
	else
	{
//		printk("ac_in   -------   HIGH	!!!!!!!!!!!!!!!!!\n");	
		*inputvalue=1;
	}


	
}

void set_pwr_aplo(int inputvalue)
{
/*	int temp_pwr=0;
	int temp_usb_pwr=0;

	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr=pwr_usb_datareg.pwr;
	temp_usb_pwr=pwr_usb_datareg.usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;

	init_power();

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr=temp_pwr;
	pwr_usb_datareg.usb_pwr=temp_usb_pwr;*/
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_aplo=inputvalue;

//	printk("driver layer :set_pwr_aplo()..........\n");
	if(pwr_usb_datareg.pwr_aplo!=0)
	{
//		printk("sw_on   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
//		*inputvalue=1;
	}
	else
	{
//		printk("sw_off   -------   LOW	!!!!!!!!!!!!!!!!!\n");	
//		*inputvalue=0;
	}
	*pwr_usb_dataregp=pwr_usb_datareg;
}

void set_pwr(int inputvalue)
{
	/*int temp_pwr_appollo=0;
	int temp_usb_pwr=0;

	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_appollo;
	temp_usb_pwr=pwr_usb_datareg.usb_pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;
	
	init_power();


	pwr_usb_datareg=*pwr_usb_dataregp;

	pwr_usb_datareg.pwr_appollo=temp_pwr_appollo;
	pwr_usb_datareg.usb_pwr=temp_usb_pwr;*/
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr=inputvalue;

//	printk("driver layer :set_pwr()..........\n");
	if(pwr_usb_datareg.pwr!=0)
	{
//		printk("pwr_on   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
//		*inputvalue=1;
	}
	else
	{
//		printk("pwr_off   -------   LOW	!!!!!!!!!!!!!!!!!\n");	
//		*inputvalue=0;
	}

	*pwr_usb_dataregp=pwr_usb_datareg;
	

}

void init_pvi_io(void)
{	
	init_porta();		//port A
	init_led(); 		//port B
	init_keypad();		//port F¡BG
	init_usb_Aplo();	//Port D
	init_aplo_bus();  	//port C
	init_porte();
	#if Not_Use_RS232
		init_porth();		//port H
	#endif	
}

void init_keypad_mode(void)
{
	init_keypad();
//	printk("driver layer :init_keypad_mode()..........\n");	
}

void init_eint_mode(void)
{
	//gpio_power_saving();
	//init_keypad();
	aplo_bus_conreg=*aplo_bus_conregp;
	aplo_bus_conreg.h_d0=PD_OUT;
	aplo_bus_conreg.h_d1=PD_OUT;
	aplo_bus_conreg.h_d2=PD_OUT;
	aplo_bus_conreg.h_d3=PD_OUT;
	aplo_bus_conreg.h_d4=PD_OUT;
	aplo_bus_conreg.h_d5=PD_OUT;
	aplo_bus_conreg.h_d6=PD_OUT;
	aplo_bus_conreg.h_d7=PD_OUT;
	*aplo_bus_conregp=aplo_bus_conreg;
	
	aplo_bus_datareg=*aplo_bus_dataregp;
	aplo_bus_datareg.h_d0=1;
	aplo_bus_datareg.h_d1=1;
	aplo_bus_datareg.h_d2=1;
	aplo_bus_datareg.h_d3=1;
	aplo_bus_datareg.h_d4=1;
	aplo_bus_datareg.h_d5=1;
	aplo_bus_datareg.h_d6=1;
	aplo_bus_datareg.h_d7=1;
	*aplo_bus_dataregp = aplo_bus_datareg;
	
	pvi_keypad_conreg=*pvi_keypad_conregp;
	pvi_keypad_conreg.up=PD_EINT;
	pvi_keypad_conreg.usb_pw=PD_EINT;
	pvi_keypad_conreg.dn=PD_EINT;
	pvi_keypad_conreg.music=PD_EINT;
	pvi_keypad_conreg.menu=PD_EINT;
	pvi_keypad_conreg.ret=PD_EINT;
	pvi_keypad_conreg.ent=PD_EINT;
	pvi_keypad_conreg.sw_off=PD_EINT;
	*pvi_keypad_conregp=pvi_keypad_conreg;

#if 0
	printk("pvi_keypad_conreg.up		%x\n",pvi_keypad_conreg.up);
	printk("pvi_keypad_conreg.dn		%x\n",pvi_keypad_conreg.dn);
	printk("pvi_keypad_conreg.music	%x\n",pvi_keypad_conreg.music);
	printk("pvi_keypad_conreg.menu		%x\n",pvi_keypad_conreg.menu);
	printk("pvi_keypad_conreg.ret 		%x\n",pvi_keypad_conreg.ret);
	printk("pvi_keypad_conreg.ent 		%x\n",pvi_keypad_conreg.ent);
#endif

	pvi_keypad_ac_conreg=*pvi_keypad_ac_conregp;
	keypad_ac_datareg=*keypad_ac_dataregp;
	if(keypad_ac_datareg.ac_off==0)
		pvi_keypad_ac_conreg.ac_in=PD_EINT;
	else	
		pvi_keypad_ac_conreg.ac_off=PD_EINT;

	if(keypad_ac_datareg.sd_in==0)
		pvi_keypad_ac_conreg.sd_ncd=PD_EINT;
	else	
		pvi_keypad_ac_conreg.sd_in=PD_EINT;
	pvi_keypad_ac_conreg.lf=PD_EINT;
	pvi_keypad_ac_conreg.del=PD_EINT;
	pvi_keypad_ac_conreg.rt=PD_EINT;				
	*pvi_keypad_ac_conregp=pvi_keypad_ac_conreg;
	
#if 0
	printk("pvi_keypad_ac_conreg.lf			%x\n",pvi_keypad_ac_conreg.lf);
	printk("pvi_keypad_ac_conreg.del		%x\n",pvi_keypad_ac_conreg.del);
	printk("pvi_keypad_ac_conreg.rt			%x\n",pvi_keypad_ac_conreg.rt);
//	printk("pvi_keypad_ac_conreg.eintrt		%x\n",pvi_keypad_ac_conreg.eintrt);	
#endif

	
}


void set_keypad_mode(void)
{
	init_keypad_mode();
}

void set_eint_mode(void)
{
	init_eint_mode();
}


void set_usb_pwr(int inputvalue)
{
	/*int temp_pwr_appollo=0;
	int temp_pwr=0;
	
	pwr_usb_datareg=*pwr_usb_dataregp;
	temp_pwr_appollo=pwr_usb_datareg.pwr_appollo;
	temp_pwr=pwr_usb_datareg.pwr;
	*pwr_usb_dataregp=pwr_usb_datareg;
	
	printk("init power before.......\n");
	printk("pwr_usb_datareg.pwr_appollo: %d",pwr_usb_datareg.pwr_appollo);
	printk("pwr_usb_datareg.pwr		: %d",pwr_usb_datareg.pwr);
	printk("----------------\n");
	
	init_power();

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.pwr_appollo=temp_pwr_appollo;
	pwr_usb_datareg.pwr=temp_pwr;
//	*pwr_usb_dataregp=pwr_usb_datareg;

	printk("init power after.......\n");
	printk("pwr_usb_datareg.pwr_appollo: %d",pwr_usb_datareg.pwr_appollo);
	printk("pwr_usb_datareg.pwr		: %d",pwr_usb_datareg.pwr);
	printk("----------------\n");*/

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.mpwr=inputvalue;

	printk("driver layer :set_usb_pwr()..........\n");
	if(pwr_usb_datareg.mpwr!=0)
	{
//		printk("usb_pwr_on   -------   HIGH	!!!!!!!!!!!!!!!!!\n");
//		*inputvalue=1;
	}
	else
	{
//		printk("usb_pwr_off   -------   LOW	!!!!!!!!!!!!!!!!!\n");	
//		*inputvalue=0;
	}
	*pwr_usb_dataregp=pwr_usb_datareg;
}



void USB_RESET(void)
{
	int i;

	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.usb_reset=0;
	*pwr_usb_dataregp=pwr_usb_datareg;
	
	printk(" USB Reset \n");
	
	for(i=0;i<65536;i++)
	{
	}
	
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.usb_reset=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
}

void set_USB_PC(void)
{
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.usb_sel=0;
	*pwr_usb_dataregp=pwr_usb_datareg;
	USB_RESET();
}

void set_USB_eBR(void)
{
	pwr_usb_datareg=*pwr_usb_dataregp;
	pwr_usb_datareg.usb_sel=1;
	*pwr_usb_dataregp=pwr_usb_datareg;
	USB_RESET();
}



static int pio_open (struct inode * inode, struct file * file)
{
//	printk("open module\n");
	return 0;
}

static int pio_release (struct inode * inode, struct file * file)
{
//	printk("Release module\n");
	return 0;
}




static int pio_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
//	int value;
	int *p = (int *)arg;
	switch (cmd)
	{		case CMD_Test:
			
				break;

			case CMD_INIT:
							
				init_pvi_io();
				break;
				
			case CMD_USB_PC:				
				set_USB_PC();
				break;
				
				
			case CMD_USB_eBR:
				set_USB_eBR();	
				break;
			
			case CMD_USB_RESET:
				USB_RESET();	
				break;
				
			case CMD_LED1:
//				#ifdef DEBUGIOINFO	//add by wanghp
//				printk("LED1 %d\n",(int)arg);
//				#endif
				setLED1(arg);
				break;				

			case CMD_LED2:
//				#ifdef DEBUGIOINFO
//				printk("LED2 %d\n",(int)arg);
//				#endif
				//setLED2(arg);
				break;

			case CMD_LED4:
//				#ifdef DEBUGIOINFO
//				printk("LED4 %d\n",(int)arg);
//				#endif
				//setLED4(arg);
				break;
			case CMD_GET_PINMODE:
				get_pinmode (p); 
				break; 
			case CMD_KEYPAD:
//				printk("KEYPAD...... %d  pio_ioctl () \n",arg);
				get_keypad (p);			
				break;
				
/*			case CMD_POWER:
				printk("POWER........%d  pio_ioctl() \n");
				get_power();
*/
			/* begin 0201 jeremy */
			case CMD_USB_PW:

				get_usb_power(p);
				break;
			
			case CMD_SD_IN:

				get_sd_in(p);
				break;

			case CMD_AC_IN:
//				#ifdef DEBUGIOINFO
//				printk("AC_IN    ......  pio_ioctl()\n");
//				#endif
				get_ac_in(p);				
				break;

			case CMD_SW_OFF:
//				#ifdef DEBUGIOINFO
//				printk("SW_OFF ......  pio_ioctl()\n");
//				#endif
				get_sw_off(p);
				break;

			case CMD_PWR_APLO:
//				#ifdef DEBUGIOINFO
//				printk("PWR_APLO ........ pio_ioctl()\n");
//				#endif
				set_pwr_aplo(arg);
				break;

			case CMD_PWR:
//				#ifdef DEBUGIOINFO
//				printk("PWR ........ pio_ioctl()\n");
//				#endif
				set_pwr(arg);
				break;

			case CMD_SET_KEYPAD_MODE:
//				printk("SET_KEYPAD_MODE ........ pio_ioctl()\n");
				set_keypad_mode();
				break;

			case CMD_SET_EINT_MODE:
//				#ifdef DEBUGIOINFO
//				printk("SET_EINT_MODE     ......... pio_ioctl()\n");
//				#endif
				eint_mode();
				break;
				
			case CMD_INIT_APLO:
//				printk("INIT APLO ...................... pio_ioctl()\n");
				init_aplo_bus();
				break;
				
			case CMD_APLO_CONF_INPUT_MOD:					
//				printk("APLO_CONF_INPUT_MOD ............ pio_ioctl()\n");
				aplo_conf_input_mod();
				break;
				
			case CMD_APLO_CONF_OUTPUT_MOD:
//				printk("APLO_CONF_OUTPUT_MOD ............ pio_ioctl()\n");
				aplo_conf_output_mod();
				break;	
				
			case CMD_MPWR:
//				#ifdef DEBUGIOINFO
//				printk("USB power off ................. pio_ioctl()\n");
//				#endif
				set_usb_pwr(arg);
				break;
			case CMD_USB_DISCONNECT:
				//ohci_suspend();
				//usb_clk_off();
			break;
			case CMD_USB_CONNECT:
				//usb_clk_on();			  
				//ohci_resume();
				//USB_RESET();
			break;	
			/* end    0201 jeremy */				

	}
	return 1;
}

static int pio_check_media_change (void)
{
	return 0;
}

static int pio_revalidate (void)
{
	return 0;
}



/*static struct block_device_operations pio_fopspow = 
{
	owner:				THIS_MODULE,
	open:				pio_open,
	release:			pio_release,
	ioctl:				pio_ioctl,
	check_media_change:	pio_check_media_change,
	revalidate:			pio_revalidate
};*/

static struct block_device_operations pio_fops = 
{
	owner:				THIS_MODULE,
	open:				pio_open,
	release:			pio_release,
	ioctl:				pio_ioctl,
	check_media_change:	pio_check_media_change,
	revalidate:			pio_revalidate
};

int init_module (void)
{
	unsigned int maj, min;
	//unsigned int majpow, minpow;
	
//	printk("init_pvi_io\n");
	
	
	//init_gpio();

	// register block driver.
	// if kernel doesnt support devfs, devfs_register returns NULL
	
	//Port A
	porta_conregp=(volatile PORTA_CONREG *)ioremap_nocache(GPACON1,4);
	porta_dataregp=(volatile PORTA_DATAREG *)ioremap_nocache(GPADAT1,4);

		
	//LED
	led_conregp=(volatile LED_CONREG *)ioremap_nocache(GPBCON1,4);
	led_dataregp=(volatile LED_DATAREG *)ioremap_nocache(GPBDAT1,4);
	led_upregp=(volatile LED_UPREG *)ioremap_nocache(GPBUP1,4);

	//KEYPAD   AND SW-OFF Port F
	pvi_keypad_conregp=(volatile KEYPAD_CONREG *)ioremap_nocache(GPFCON1,4);
	keypad_dataregp=(volatile KEYPAD_DATAREG *)ioremap_nocache(GPFDAT1,4);
	keypad_upregp=(volatile KEYPAD_UPREG *)ioremap_nocache(GPFUP1,4);

	//KEYPAD AND AC-IN Port G
	pvi_keypad_ac_conregp=(volatile KEYPAD_AC_CONREG*)ioremap_nocache(GPGCON1,4);
	keypad_ac_dataregp=(volatile KEYPAD_AC_DATAREG*)ioremap_nocache(GPGDAT1,4);
	keypad_ac_upregp=(volatile KEYPAD_AC_UPREG*)ioremap_nocache(GPGUP1,4);

	//PWR  .PWR_appllo .USB-in .SDCard-in .USB-sel .USB-reset .aplo-bus
	
	pwr_usb_conregp=(volatile PWR_USB_CONREG*)ioremap_nocache(GPDCON1,4);
	pwr_usb_dataregp=(volatile PWR_USB_DATAREG*)ioremap_nocache(GPDDAT1,4);
	pwr_usb_upregp=(volatile PWR_USB_UPREG*)ioremap_nocache(GPDUP1,4);


	//Aplo-bus
	aplo_bus_conregp=(volatile APLO_BUS_CONREG*)ioremap_nocache(GPCCON1,4);
	aplo_bus_dataregp=(volatile APLO_BUS_DATAREG*)ioremap_nocache(GPCDAT1,4);
	aplo_bus_upregp=(volatile APLO_BUS_UPREG*)ioremap_nocache(GPCUP1,4);
	
	//Port E
	porte_conregp=(volatile PORTE_CONREG *)ioremap_nocache(GPECON1,4);
	porte_dataregp=(volatile PORTE_DATAREG *)ioremap_nocache(GPEDAT1,4);
	porte_upregp=(volatile PORTE_UPREG*)ioremap_nocache(GPEUP1,4);
	
	//Port H
	porth_conregp=(volatile PORTH_CONREG *)ioremap_nocache(GPHCON1,4);
	porth_dataregp=(volatile PORTH_DATAREG *)ioremap_nocache(GPHDAT1,4);
	porth_upregp=(volatile PORTH_UPREG*)ioremap_nocache(GPHUP1,4);
	
	
	if ((pvi_devfs_handle = devfs_register(NULL, PIO_DEVICE_NAME,
			DEVFS_FL_DEFAULT, PIO_MAJOR, 0,
			S_IFBLK | S_IRUGO | S_IWUGO,
			&pio_fops, NULL)))
	{
		devfs_get_maj_min (pvi_devfs_handle, &maj, &min);
	}
	if (pvi_devfs_handle == 0)
	{
		return -1;
	}

	init_pvi_io();
		

	/* add */
	/*if ((devfs_handlepow = devfs_register(NULL, PIO_DEVICE_NAMEPOW,
			DEVFS_FL_DEFAULT, PIO_MAJORPOW, 0,
			S_IFBLK | S_IRUGO | S_IWUGO,
			&pio_fopspow, NULL)))
	{
		devfs_get_maj_min (devfs_handlepow, &majpow, &minpow);
	}
	if (devfs_handlepow== 0)
	{
		return -1;
	}*/


//	about_pw_conregp=(volatile ABOUT_PW_CONREG *)ioremap_nocache(GPDCON,4);
//	about_pw_dataregp=(volatile ABOUT_PW_DATAREG *)ioremap_nocache(GPDDAT,4);
//	init_power();
	
//	printk("init_pvi_io:end\n");
	register_pm();
	return 0;
}



void cleanup_module (void)
{
	devfs_unregister (pvi_devfs_handle);
	devfs_unregister_blkdev (PIO_MAJOR, PIO_DEVICE_NAME);


	//devfs_unregister (devfs_handlepow);
	//devfs_unregister_blkdev (PIO_MAJORPOW, PIO_DEVICE_NAMEPOW);
}

void keypad_resume(void)
{
//		printk("dz:pviio resume\n");
		init_keypad();
		
		keypad_datareg=*keypad_dataregp;
		keypad_ac_datareg=*keypad_ac_dataregp;
		wake_up=1;	
}

int pviio_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{

	if (req == PM_SUSPEND) 
	{		
		init_eint_mode();	
//		printk("dz:pviio suspend\n");	
		
	} else if (req == PM_RESUME) 
	{		
		keypad_resume();	
	}	

	return 0;
}



void register_pm(void)
{
//	printk("Register PM\n");
	pviio_pm_dev = pm_register(PM_USER_DEV, 0,
					 pviio_pm_callback);

}


//************************************************************************************************************************
// Power Saving IO
//
// GPA17, GPA18, GPA19, GPA20, GPA22
// GPC0, GPC1, GPC2, GPC3, GPC4, GPC5, GPC6, GPC7
// GPE5, GPE6, GPE7, GPE8, GPE9, GPE10, GPE11, GPE12, GPE13, GPE14, GPE15
// GPG4, GPG5, GPG6, GPG7, GPG12, GPG13, GPG14, GPG15
// GPH0, GPH1, GPH2, GPH3, GPH4, GPH5, GPH6, GPH7, GPH8
//************************************************************************************************************************

// GPA17, GPA18, GPA19, GPA20, GPA22

/*typedef struct 
{
	int bit0_16:17;
	int bit17:1;
	int bit18:1;
	int bit19:1;
	int bit20:1;
	int bit21:1;
	int bit22:1;
} PORTA_DATAREG;

typedef struct 
{
	int bit0_16:17;
	int bit17:1;
	int bit18:1;
	int bit19:1;
	int bit20:1;
	int bit21:1;
	int bit22:1;
} PORTA_CONREG;

volatile PORTA_CONREG 	*porta_conregp;
volatile PORTA_DATAREG 	*porta_dataregp;

PORTA_CONREG	porta_conreg;
PORTA_DATAREG 	porta_datareg;

void porta_init()
{
	porta_conreg=*porta_conregp;
	porta_conreg.bit17=PA_OUT;
	porta_conreg.bit18=PA_OUT;
	porta_conreg.bit19=PA_OUT;
	porta_conreg.bit20=PA_OUT;
	porta_conreg.bit22=PA_OUT;
	*porta_conregp=porta_conreg;
	
	porta_datareg=*porta_dataregp;
	porta_datareg.bit17=1;
	porta_datareg.bit18=1;
	porta_datareg.bit19=1;
	porta_datareg.bit20=1;
	porta_datareg.bit22=1;	
	*porta_dataregp=porta_datareg;		
}

// GPB0, GPB5, GPB6, GPB9

typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
} PORTB_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
} PORTB_CONREG;

volatile PORTB_CONREG 	*portb_conregp;
volatile PORTB_DATAREG 	*portb_dataregp;

PORTB_CONREG	portb_conreg;
PORTB_DATAREG 	portb_datareg;

void portb_init()
{
	portb_conreg=*portb_conregp;
	portb_conreg.bit0=PD_OUT;
	portb_conreg.bit5=PD_OUT;
	portb_conreg.bit6=PD_OUT;
	portb_conreg.bit9=PD_OUT;
	*portb_conregp=portb_conreg;
	
	portb_datareg=*portb_dataregp;
	portb_datareg.bit0=1;
	portb_datareg.bit5=1;	
	portb_datareg.bit6=1;	
	portb_datareg.bit9=1;
	*portb_dataregp=portb_datareg;		
}


// GPC0, GPC1, GPC2, GPC3, GPC4, GPC5, GPC6, GPC7

typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
	int bit11:1;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
} PORTC_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
	int bit11:2;
	int bit12:2;
	int bit13:2;
	int bit14:2;
	int bit15:2;
} PORTC_CONREG;

volatile PORTC_CONREG 	*portc_conregp;
volatile PORTC_DATAREG 	*portc_dataregp;

PORTC_CONREG	portc_conreg;
PORTC_DATAREG 	portc_datareg;

void portc_init()
{
	portc_conreg=*portc_conregp;
	portc_conreg.bit0=PD_OUT;
	portc_conreg.bit1=PD_OUT;
	portc_conreg.bit2=PD_OUT;
	portc_conreg.bit3=PD_OUT;
	portc_conreg.bit4=PD_OUT;
	portc_conreg.bit5=PD_OUT;
	portc_conreg.bit6=PD_OUT;
	portc_conreg.bit7=PD_OUT;

	*portc_conregp=portc_conreg;
	
	portc_datareg=*portc_dataregp;
	portc_datareg.bit0=1;
	portc_datareg.bit1=1;
	portc_datareg.bit2=1;
	portc_datareg.bit3=1;
	portc_datareg.bit4=1;	
	portc_datareg.bit5=1;	
	portc_datareg.bit6=1;	
	portc_datareg.bit7=1;	

	*portc_dataregp=portc_datareg;		
}


void portc_init2()
{
	portc_conreg=*portc_conregp;
#if 1
	portc_conreg.bit8=PD_OUT;
	portc_conreg.bit9=PD_OUT;
	portc_conreg.bit10=PD_OUT;
	portc_conreg.bit11=PD_OUT;
	portc_conreg.bit12=PD_OUT;
	portc_conreg.bit13=PD_OUT;
	portc_conreg.bit14=PD_OUT;
	portc_conreg.bit15=PD_OUT;	
#endif		

	*portc_conregp=portc_conreg;
	
	portc_datareg=*portc_dataregp;
	
#if 1	
	portc_datareg.bit8=1;
	portc_datareg.bit9=1;
	portc_datareg.bit10=1;
	portc_datareg.bit11=1;
	portc_datareg.bit12=1;	
	portc_datareg.bit13=1;	
	portc_datareg.bit14=1;	
	portc_datareg.bit15=1;		
#endif	

	*portc_dataregp=portc_datareg;		
}



// GPE5, GPE6, GPE7, GPE8, GPE9, GPE10, GPE11, GPE12, GPE13, GPE14, GPE15


typedef struct 
{
	int bit0_4:5;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
	int bit11:1;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
} PORTE_DATAREG;

typedef struct 
{
	int bit0_4:10;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
	int bit11:2;
	int bit12:2;
	int bit13:2;
	int bit14:2;
	int bit15:2;
} PORTE_CONREG;

volatile PORTE_CONREG 	*porte_conregp;
volatile PORTE_DATAREG 	*porte_dataregp;

PORTE_CONREG	porte_conreg;
PORTE_DATAREG 	porte_datareg;

void porte_init()
{
	porte_conreg=*porte_conregp;
	porte_conreg.bit5=PD_OUT;
	porte_conreg.bit6=PD_OUT;
	porte_conreg.bit7=PD_OUT;
	porte_conreg.bit8=PD_OUT;
	porte_conreg.bit9=PD_OUT;
	porte_conreg.bit10=PD_OUT;
	porte_conreg.bit11=PD_OUT;
	porte_conreg.bit12=PD_OUT;	
	porte_conreg.bit13=PD_OUT;	
	porte_conreg.bit14=PD_OUT;	
	porte_conreg.bit15=PD_OUT;	
	*porte_conregp=porte_conreg;
	
	porte_datareg=*porte_dataregp;
	porte_datareg.bit5=1;
	porte_datareg.bit6=1;
	porte_datareg.bit7=1;
	porte_datareg.bit8=1;
	porte_datareg.bit9=1;	
	porte_datareg.bit10=1;	
	porte_datareg.bit11=1;	
	porte_datareg.bit12=1;	
	porte_datareg.bit13=1;	
	porte_datareg.bit14=1;	
	porte_datareg.bit15=1;	
	*porte_dataregp=porte_datareg;		
}

// GPG4, GPG5, GPG6, GPG7, GPG12, GPG13, GPG14, GPG15

typedef struct 
{
	int bit0_3:4;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8_11:4;
	int bit12:1;
	int bit13:1;
	int bit14:1;
	int bit15:1;
} PORTG_DATAREG;

typedef struct 
{
	int bit0_3:8;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8_11:8;
	int bit12:2;
	int bit13:2;
	int bit14:2;
	int bit15:2;
} PORTG_CONREG;

volatile PORTG_CONREG 	*portg_conregp;
volatile PORTG_DATAREG 	*portg_dataregp;

PORTG_CONREG	portg_conreg;
PORTG_DATAREG 	portg_datareg;

void portg_init()
{
	portg_conreg=*portg_conregp;
	portg_conreg.bit4=PD_OUT;
	portg_conreg.bit5=PD_OUT;
	portg_conreg.bit6=PD_OUT;
	portg_conreg.bit7=PD_OUT;
	portg_conreg.bit12=PD_OUT;	
	portg_conreg.bit13=PD_OUT;	
	portg_conreg.bit14=PD_OUT;	
	portg_conreg.bit15=PD_OUT;	
	*portg_conregp=portg_conreg;
	
	portg_datareg=*portg_dataregp;
	portg_datareg.bit4=1;
	portg_datareg.bit5=1;
	portg_datareg.bit6=1;
	portg_datareg.bit7=1;
	portg_datareg.bit12=1;	
	portg_datareg.bit13=1;	
	portg_datareg.bit14=1;	
	portg_datareg.bit15=1;	
	*portg_dataregp=portg_datareg;		
}

// GPH0, GPH1, GPH2, GPH3, GPH4, GPH5, GPH6, GPH7, GPH8

typedef struct 
{
	int bit0:1;
	int bit1:1;
	int bit2:1;
	int bit3:1;
	int bit4:1;
	int bit5:1;
	int bit6:1;
	int bit7:1;
	int bit8:1;
	int bit9:1;
	int bit10:1;
} PORTH_DATAREG;

typedef struct 
{
	int bit0:2;
	int bit1:2;
	int bit2:2;
	int bit3:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int bit7:2;
	int bit8:2;
	int bit9:2;
	int bit10:2;
} PORTH_CONREG;

volatile PORTH_CONREG 	*porth_conregp;
volatile PORTH_DATAREG 	*porth_dataregp;

PORTH_CONREG	porth_conreg;
PORTH_DATAREG 	porth_datareg;

void porth_init()
{
	porth_conreg=*porth_conregp;
	porth_conreg.bit0=PD_OUT;
	porth_conreg.bit1=PD_OUT;
	porth_conreg.bit2=PD_OUT;
	porth_conreg.bit3=PD_OUT;
	porth_conreg.bit4=PD_OUT;
	porth_conreg.bit5=PD_OUT;
	porth_conreg.bit6=PD_OUT;
	porth_conreg.bit7=PD_OUT;	
	porth_conreg.bit8=PD_OUT;	
	*porth_conregp=porth_conreg;
	
	porth_datareg=*porth_dataregp;
	porth_datareg.bit0=1;
	porth_datareg.bit1=1;
	porth_datareg.bit2=1;
	porth_datareg.bit3=1;
	porth_datareg.bit4=1;	
	porth_datareg.bit5=1;	
	porth_datareg.bit6=1;	
	porth_datareg.bit7=1;	
	porth_datareg.bit8=1;	
	*porth_dataregp=porth_datareg;		
}

void init_gpio(void)
{
	porta_conregp=(volatile PORTA_CONREG *)ioremap_nocache(GPACON1,4);
	porta_dataregp=(volatile PORTA_DATAREG *)ioremap_nocache(GPADAT1,4);

	portb_conregp=(volatile PORTB_CONREG *)ioremap_nocache(GPBCON1,4);
	portb_dataregp=(volatile PORTB_DATAREG *)ioremap_nocache(GPBDAT1,4);

	portc_conregp=(volatile PORTC_CONREG *)ioremap_nocache(GPCCON1,4);
	portc_dataregp=(volatile PORTC_DATAREG *)ioremap_nocache(GPCDAT1,4);
	
	porte_conregp=(volatile PORTE_CONREG *)ioremap_nocache(GPECON1,4);
	porte_dataregp=(volatile PORTE_DATAREG *)ioremap_nocache(GPEDAT1,4);
	
	portg_conregp=(volatile PORTG_CONREG *)ioremap_nocache(GPGCON1,4);
	portg_dataregp=(volatile PORTG_DATAREG *)ioremap_nocache(GPGDAT1,4);

	porth_conregp=(volatile PORTH_CONREG *)ioremap_nocache(GPHCON1,4);
	porth_dataregp=(volatile PORTH_DATAREG *)ioremap_nocache(GPHDAT1,4);
	
}


void gpio_power_saving(void)
{
	porta_init();
	portb_init();
	portc_init();	
//	portc_init2();
	porte_init();
	portg_init();
	
//	porth_init();	//UART Port
}*/


module_init(init_module);						//include kernel           
module_exit(cleanup_module);					//include kernel 
