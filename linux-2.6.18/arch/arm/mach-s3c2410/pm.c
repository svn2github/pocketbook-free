#include <linux/config.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif



#include <asm/arch/S3C2410.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>

/*
 * Debug macros
 */
#undef DEBUG_PM
#ifdef DEBUG_PM
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

extern void s3c2410_cpu_suspend(void);
extern void s3c2410_cpu_resume(void);

#define SAVE(x)		sleep_save[SLEEP_SAVE_##x] = x
#define RESTORE(x)	x = sleep_save[SLEEP_SAVE_##x]

static u_int pwbt_num = 0;
static int pwbt_edge = 0;
static int pm_rtc_on = 0;

int
register_wakeup_src(u_int eint_num, int edge, int rtc_on)
{
	if (eint_num < 0 && eint_num > 23) {
		return -EINVAL;
	}

	pwbt_num = eint_num;
	pwbt_edge = edge;
	pm_rtc_on = rtc_on;

	return 0;
}



static void
setup_wakeup_src(void)
{
	if (pwbt_num < 4) {
		EXTINT0 = (pwbt_edge << (4 * pwbt_num));

		SRCPND |= (1 << pwbt_num);
		INTPND |= (1 << pwbt_num);

		INTMSK &= ~(1 << pwbt_num);
	} else {
		if (pwbt_num < 8) {
			EXTINT0 = (pwbt_edge << (4 * pwbt_num));
		} else if (pwbt_num < 16) {
			EXTINT1 = (pwbt_edge << (4 * (pwbt_num - 8)));
		} else {
			EXTINT2 = (pwbt_edge << (4 * (pwbt_num - 16)));
		}

		EINTPEND |= (1 << pwbt_num);
		
		EINTMASK &= ~(1 << pwbt_num);
	}

	if (pm_rtc_on) {
		/* Enable RTC */
		
		printk("enable RTC\n");	
		
		SRCPND |= INT_RTC;
		INTPND |= INT_RTC;
		INTMSK &= ~INT_RTC;
	}
}

/*
 * List of global S3C2410 peripheral registers to preserve.
 * More ones like CP and general purpose register values are preserved
 * with the stack location in sleep.S.
 */
enum {	SLEEP_SAVE_START = 0,

	SLEEP_SAVE_INTMSK, SLEEP_SAVE_INTPND, SLEEP_SAVE_SRCPND,
	SLEEP_SAVE_INTSUBMSK, SLEEP_SAVE_SUBSRCPND,
	SLEEP_SAVE_INTMOD,

	SLEEP_SAVE_CLKCON,

	SLEEP_SAVE_UBRDIV0, SLEEP_SAVE_ULCON0, SLEEP_SAVE_UFCON0,
	SLEEP_SAVE_UMCON0, SLEEP_SAVE_UCON0,

	SLEEP_SAVE_GPACON, SLEEP_SAVE_GPADAT,
	SLEEP_SAVE_GPBCON, SLEEP_SAVE_GPBDAT, SLEEP_SAVE_GPBUP,
	SLEEP_SAVE_GPCCON, SLEEP_SAVE_GPCDAT, SLEEP_SAVE_GPCUP,
	SLEEP_SAVE_GPDCON, SLEEP_SAVE_GPDDAT, SLEEP_SAVE_GPDUP,
	SLEEP_SAVE_GPECON, SLEEP_SAVE_GPEDAT, SLEEP_SAVE_GPEUP,
	SLEEP_SAVE_GPFCON, SLEEP_SAVE_GPFDAT, SLEEP_SAVE_GPFUP,
	SLEEP_SAVE_GPGCON, SLEEP_SAVE_GPGDAT, SLEEP_SAVE_GPGUP,

	SLEEP_SAVE_MISCCR, SLEEP_SAVE_DCLKCON,
	SLEEP_SAVE_EXTINT0, SLEEP_SAVE_EXTINT1, SLEEP_SAVE_EXTINT2,
	SLEEP_SAVE_EINTFLT0, SLEEP_SAVE_EINTFLT1, SLEEP_SAVE_EINTFLT2, 
	SLEEP_SAVE_EINTFLT3, SLEEP_SAVE_EINTMASK,

	SLEEP_SAVE_TCFG0, SLEEP_SAVE_TCFG1, SLEEP_SAVE_TCNTB4,
	SLEEP_SAVE_TCON,

	SLEEP_SAVE_SIZE
};

int pm_do_suspend(void)
{
	unsigned long sleep_save[SLEEP_SAVE_SIZE];

	DPRINTK("I am pm_do_suspend\n");
	printk("dz_pm_do_suspend\n");


	cli();


	/* save vital registers */
	SAVE(UBRDIV0);
	SAVE(ULCON0);
	SAVE(UFCON0);
	SAVE(UMCON0);
	SAVE(UCON0);

	SAVE(GPACON); SAVE(GPADAT);
	SAVE(GPBCON); SAVE(GPBDAT); SAVE(GPBUP);
	SAVE(GPCCON); SAVE(GPCDAT); SAVE(GPCUP);
	SAVE(GPDCON); SAVE(GPDDAT); SAVE(GPDUP);
	SAVE(GPECON); SAVE(GPEDAT); SAVE(GPEUP);
	SAVE(GPFCON); SAVE(GPFDAT); SAVE(GPFUP);
	SAVE(GPGCON); SAVE(GPGDAT); SAVE(GPGUP);

	SAVE(MISCCR); SAVE(DCLKCON);
	SAVE(EXTINT0); SAVE(EXTINT1); SAVE(EXTINT2);
	SAVE(EINTFLT0); SAVE(EINTFLT1); SAVE(EINTFLT2); SAVE(EINTFLT3);
	SAVE(EINTMASK);

	SAVE(INTMOD); SAVE(INTMSK); SAVE(INTSUBMSK);


	SAVE(TCFG0); SAVE(TCNTB4); SAVE(TCON);

	/* temporary.. */
	GPFDAT |= 0xf0;  
	GPGDAT &= ~(1 << 4);

	PMCTL1 |= (USBSPD1 | USBSPD0);
	PMCTL1 |= (0x3);

	/* Clear previous reset status */
	PMST = (PMST_HWR | PMST_WDR | PMST_SMR);

	/* set resume return address */
	PMSR0 = virt_to_phys(s3c2410_cpu_resume);

	setup_wakeup_src();

	/* go zzz */
	s3c2410_cpu_suspend();

	/* ensure not to come back here if it wasn't intended */
	PMSR0 = 0;

	PMCTL1 &= ~(USBSPD1 | USBSPD0);

	RESTORE(TCFG0);
	RESTORE(TCNTB4);
	TCON = (TCON_4_AUTO | TCON_4_UPDATE | COUNT_4_OFF);
	TCON = (TCON_4_AUTO | COUNT_4_ON);



	/* restore registers */
	RESTORE(GPACON); RESTORE(GPADAT);
	RESTORE(GPBCON); RESTORE(GPBDAT); RESTORE(GPBUP);
	RESTORE(GPCCON); RESTORE(GPCDAT); RESTORE(GPCUP);
	RESTORE(GPDCON); RESTORE(GPDDAT); RESTORE(GPDUP);
	RESTORE(GPECON); RESTORE(GPEDAT); RESTORE(GPEUP);
	RESTORE(GPFCON); RESTORE(GPFDAT); RESTORE(GPFUP);
	RESTORE(GPGCON); RESTORE(GPGDAT); RESTORE(GPGUP);

	RESTORE(MISCCR); RESTORE(DCLKCON);
	RESTORE(EXTINT0); RESTORE(EXTINT1); RESTORE(EXTINT2);
	RESTORE(EINTFLT0); RESTORE(EINTFLT1); RESTORE(EINTFLT2); RESTORE(EINTFLT3);
	RESTORE(EINTMASK);

	RESTORE(INTMOD); RESTORE(INTMSK); RESTORE(INTSUBMSK);

	/* Clear interrupts */
	EINTPEND = EINTPEND;
	LCDSRCPND = LCDSRCPND;
	LCDINTPND = LCDINTPND;
	SUBSRCPND = SUBSRCPND;
	SRCPND = SRCPND;
	INTPND = INTPND;


	/* temporary.. reset UART */
	RESTORE(ULCON0); RESTORE(UCON0); RESTORE(UFCON0);
	RESTORE(UMCON0); RESTORE(UBRDIV0);
	GPFCON &= ~(0xff00);
	GPFCON |= 0x5500;
	GPFUP |= 0xf0;
	GPFDAT &= ~(0xf0);  
	GPFDAT |= 0xa0;



	sti();


	DPRINTK("I am still alive\n");
	printk("dz_pm_do_suspend: wake up\n");

	return 0;
}

unsigned long sleep_phys_sp(void *sp) 
{
	return virt_to_phys(sp);
}


#ifdef CONFIG_SYSCTL
/*
 * ARGH! ACPI people defined CTL_ACPI in linux/acpi.h rather than
 * linux/sysctl.h
 *
 * This means our interface here won't survice long - it needs a new
 * interface. Quick hact to et this working - use sysctl id 9999.
 */
#warning ACPI broke the kernel, this interface needs to be fixed up.
#define CTL_ACPI 9999
#define ACPI_S1_SLP_TYP	19

/*
 * Send us to sleep.
 */
 
#define GPBCON1 0x56000010
#define GPBDAT1 0x56000014
#define GPBUP1  0x56000018

#define GPCCON1 0x56000020
#define GPCDAT1 0x56000024
#define GPCUP1  0x56000028

#define GPDCON1 0x56000030
#define GPDDAT1 0x56000034
#define GPDUP1  0x56000038

#define GPFCON1 0x56000050
#define GPFDAT1 0x56000054
#define GPFUP1  0x56000058

#define GPGCON1 0x56000060
#define GPGDAT1 0x56000064
#define GPGUP1  0x56000068


#define PD_EINT 	0x02
 
 
typedef struct
{
	int up:2;
	int bit1:2;
	int dn:2;
	int music:2;
	int menu:2;
	int ret:2;
	int ent:2;
	int sw_off:2;
}KEYPAD_CONREG;

typedef struct 
{
	int ac_in:2;
	int bit1:2;
	int bit2:2;
//	int rtkey:2;					//jeremy 0419
	int lf:2;
	int bit4:2;
	int bit5:2;
	int bit6:2;
	int eintrt:2;
	int volplus:2;
	int volnegative:2;
	int del:2;
	int rt:2;
	int bit12:2;
	int bit13:2;
	int bit14:2;
	int bit15:2;
}KEYPAD_AC_CONREG;
 
 
KEYPAD_CONREG 			keypad_conreg;
volatile KEYPAD_CONREG 	*keypad_conregp;
KEYPAD_AC_CONREG		keypad_ac_conreg;
volatile KEYPAD_AC_CONREG		*keypad_ac_conregp; 


void init_eint()
{
#if 0	
	//KEYPAD   AND SW-OFF
	keypad_conregp=(volatile KEYPAD_CONREG *)GPFCON1;

	//KEYPAD AND AC-IN
	keypad_ac_conregp=(volatile KEYPAD_AC_CONREG*)GPGCON1;
#else	
	
	//KEYPAD   AND SW-OFF
	keypad_conregp=(volatile KEYPAD_CONREG *)ioremap_nocache(GPFCON1,4);

	//KEYPAD AND AC-IN
	keypad_ac_conregp=(volatile KEYPAD_AC_CONREG*)ioremap_nocache(GPGCON1,4);
#endif	
	
}
 
void eint_mode()
{


	keypad_conreg=*keypad_conregp;

	keypad_conreg.up=PD_EINT;
	keypad_conreg.dn=PD_EINT;
	keypad_conreg.music=PD_EINT;
	keypad_conreg.menu=PD_EINT;
	keypad_conreg.ret=PD_EINT;
	keypad_conreg.ent=PD_EINT;
	
	*keypad_conregp=keypad_conreg;


	keypad_ac_conreg=*keypad_ac_conregp;
		
	keypad_ac_conreg.lf=PD_EINT;
	keypad_ac_conreg.del=PD_EINT;
	keypad_ac_conreg.rt=PD_EINT;				//jeremy 0419
//	keypad_ac_conreg.rtkey=PD_EINT;			//jeremy 0419
	keypad_ac_conreg.eintrt=PD_EINT;			//jeremy 0419

	*keypad_ac_conregp=keypad_ac_conreg;	
}
 
 
static int
sysctl_pm_do_suspend(void)
{
	int ret;
	
	
//	printk("dz_sysctl_pm_do_suspend \n");

//	eint_mode();


	ret = pm_send_all(PM_SUSPEND, (void *)3);

//	printk("dz: after pm_send_all ret=%d \n",ret);	

	if (ret == 0) {
		
//		printk("dz: ENTER pm_do_suspend \n");
		
		
		ret = pm_do_suspend();

//		keypad_resume();

		pm_send_all(PM_RESUME, (void *)0);
	}


#if 0
	if(ret==0)
		printk("!!!!!!!! ret=0 !!!!!!!!!!");
	else
		printk("!!!!!!!! ret!=0 !!!!!!!!!!");
#endif

	return ret;
}

static struct ctl_table pm_table[] =
{
	{ACPI_S1_SLP_TYP, "suspend", NULL, 0, 0600, NULL, (proc_handler *)&sysctl_pm_do_suspend},
	{0}
};

static struct ctl_table pm_dir_table[] =
{
	{CTL_ACPI, "pm", NULL, 0, 0555, pm_table},
	{0}
};
#endif

#ifdef CONFIG_PROC_FS
static int
pminfo_proc_output(char *buf)
{
	char *p;
	struct pm_dev *dev;

	p = buf;
	p += sprintf(p, "type \t\t id \t\t stat \t prev_state \n");
	p += sprintf(p, "----------------------------------------------\n");

	dev = NULL;

	while ((dev = pm_find(PM_UNKNOWN_DEV, dev)) != NULL) {
		switch (dev->type) {
		case PM_SYS_DEV:
			p += sprintf(p, "PM_SYS_DEV \t ");
			break;
		case PM_PCI_DEV:
			p += sprintf(p, "PM_PCI_DEV \t ");
			break;
		case PM_USB_DEV:
			p += sprintf(p, "PM_USB_DEV \t ");
			break;
		case PM_SCSI_DEV:
			p += sprintf(p, "PM_SCSI_DEV \t ");
			break;
		case PM_ISA_DEV:
			p += sprintf(p, "PM_ISA_DEV \t ");
			break;
		case PM_MTD_DEV:
			p += sprintf(p, "PM_MTD_DEV \t ");
			break;
		case PM_ILLUMINATION_DEV:
			p += sprintf(p, "PM_ILLUMINATION_DEV \t ");
			break;
		case PM_USER_DEV:
			p += sprintf(p, "PM_USER_DEV \t ");
			break;
		case PM_DEBUG_DEV:
			p += sprintf(p, "PM_DEBUG_DEV \t ");
			break;
		case PM_GP_DEV:
			p += sprintf(p, "PM_GP_DEV \t ");
			break;
		default:
			p += sprintf(p, "PM_UNKONWN_DEV \t ");
			break;
		}

		switch (dev->id) {
		case PM_SYS_KBC:
			p += sprintf(p, "PM_SYS_KBC \t ");
			break;
		case PM_SYS_COM:
			p += sprintf(p, "PM_SYS_COM \t ");
			break;
		case PM_SYS_IRDA:
			p += sprintf(p, "PM_SYS_IRDA \t ");
			break;
		case PM_SYS_FDC:
			p += sprintf(p, "PM_SYS_FDC \t ");
			break;
		case PM_SYS_VGA:
			p += sprintf(p, "PM_SYS_VGA \t ");
			break;
		case PM_SYS_PCMCIA:
			p += sprintf(p, "PM_SYS_PCMCIA \t ");
			break;
		case PM_USER_LCD:
			p += sprintf(p, "PM_USER_LCD \t ");
			break;
		case PM_USER_LIGHT:
			p += sprintf(p, "PM_USER_LIGHT \t ");
			break;
		case PM_USER_INPUT:
			p += sprintf(p, "PM_USER_INPUT \t ");
			break;
		case PM_DEBUG_0:
			p += sprintf(p, "PM_DEBUG_0 \t ");
			break;
		case PM_DEBUG_1:
			p += sprintf(p, "PM_DEBUG_1 \t ");
			break;
		case PM_DEBUG_2:
			p += sprintf(p, "PM_DEBUG_2 \t ");
			break;
		case PM_SYS_MISC:
			p += sprintf(p, "PM_SYS_MISC \t ");
			break;
		default:
			p += sprintf(p, "PM_UNKONWN_DEV \t ");
			break;
		}
		p += sprintf(p, "%d \t %d\n", (int)dev->state, (int)dev->prev_state);
	}

	return p - buf;

}

static int
pminfo_read_proc(char *page, char **start, off_t off,
                 int count, int *eof, void *data)
{
	int len = pminfo_proc_output(page);
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif

/*
 * Initialize power interface
 */
static int __init
pm_init(void)
{
#ifdef CONFIG_SYSCTL
	register_sysctl_table(pm_dir_table, 1);
#endif    
#ifdef CONFIG_PROC_FS
	create_proc_read_entry("pminfo", 0, 0, pminfo_read_proc, NULL);
#endif
	return 0;
}

__initcall(pm_init);
