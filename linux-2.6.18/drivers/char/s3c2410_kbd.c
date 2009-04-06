/*
 * Copyright (C) 2002 SAMSUNG ELECTRONICS 
 *                    SW.LEE <hitchcar@sec.samsung.com>
 *
 *  This keyboard driver drives a PS/2 keyboard and mouse connected
 *  to the KMI interfaces.  The KMI interfaces are nothing more than
 *  a uart; there is no inteligence in them to do keycode translation.
 *  We leave all that up to the keyboard itself.
 *
 * Author: SW.LEE <hitchcar@sec.samsung.com>
 * Date  : $Date: 2004/02/04 12:56:04 $ 
 *
 * $Revision: 1.1.1.1 $

   unknown         SW.LEE <hitchcar@sec.samsung.com>
   - initial

   Wed Aug 14 2002 Yong-iL Joh <tolkien@mizi.com>
   - new irq scheme

   Mon Aug 19 2002 Yong-iL Joh <tolkien@mizi.com>
   - rearrange smdk's keyboard mapping.
   - TODO: add kbd_setkeycode/kbd_getkeycode
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>	/* for in_interrupt */
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/delay.h>	/* for udelay */
#include <linux/kbd_kern.h>	/* for keyboard_tasklet */
#include <linux/kbd_ll.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/keyboard.h>

/*
 * Responses : SPI STATUS Reigster SPSTA
 */
#define SPI_COLLISION_ERROR     0x4
#define SPI_MULTI_MASTER_ERROR  0x2
#define SPI_RX_TX_READY         0x1

#define UNSET_OUTPUT()	write_gpio_bit(GPIO_KBD_nSS, 0)
#define SET_OUTPUT()	write_gpio_bit(GPIO_KBD_nSS, 1)
#define SPI_WRITE(c)	(SPTDAT1  = c)
#define SPI_READ(c)	(c = SPRDAT1 )

/*
 * key map
 */
static char kbmap[128] = {
KK_NONE, KK_LALT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_AGR,  KK_BSLH, KK_TAB,  KK_Z,    KK_A,    KK_X,    KK_NONE,
KK_NONE, KK_NONE, KK_LSFT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_LCTL, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_21,   KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_ESC,  KK_DEL,  KK_Q,    KK_CAPS, KK_S,    KK_C,    KK_3,
KK_NONE, KK_1,    KK_NONE, KK_W,    KK_NONE, KK_D,    KK_V,    KK_4,
KK_NONE, KK_2,    KK_T,    KK_E,    KK_NONE, KK_F,    KK_B,    KK_5, 
KK_NONE, KK_9,    KK_Y,    KK_R,    KK_K,    KK_G,    KK_N,    KK_6,
KK_NONE, KK_0,    KK_U,    KK_O,    KK_L,    KK_H,    KK_M,    KK_7,
KK_NONE, KK_MINS, KK_I,    KK_P,    KK_SEMI, KK_J,    KK_COMA, KK_8,
KK_NONE, KK_EQLS, KK_ENTR, KK_LSBK, KK_SQOT, KK_FSLH, KK_DOT,  KK_NONE,
KK_NONE, KK_NONE, KK_RSFT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_BKSP, KK_DOWN, KK_RSBK, KK_UP,   KK_LEFT, KK_SPCE, KK_RGHT,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE};

static char kbmapFN[128] = {
KK_NONE, KK_LALT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_LSFT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_LCTL, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_21,   KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_F3,
KK_NONE, KK_F1,   KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_F4,
KK_NONE, KK_F2,   KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_F5,
KK_NONE, KK_F9,   KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_F6,
KK_NONE, KK_F10,  KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_F7,
KK_NONE, KK_NUML, KK_NONE, KK_INS,  KK_PRNT, KK_NONE, KK_NONE, KK_F8,
KK_NONE, KK_BRK,  KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_RSFT, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_PGDN, KK_SCRL, KK_PGUP, KK_HOME, KK_NONE, KK_END,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE};

static char kbmapNL[128] = {
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KP_9,    KK_NONE, KK_NONE, KP_2,    KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KP_STR,  KP_4,    KP_6,    KP_3,    KK_NONE, KP_0,    KP_7,
KK_NONE, KK_NONE, KP_5,    KP_MNS , KP_PLS,  KP_1,    KK_NONE, KP_8,
KK_NONE, KK_NONE, KP_ENT,  KK_NONE, KK_NONE, KP_SLH,  KP_DOT,  KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE,
KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE, KK_NONE};

/****************************************************/

/*
 * The "program" we send to the keyboard to set it up how we want it:
 *  - default typematic delays
 *  - scancode set 1
 */
#define KBCTL_NODATA	-1
#define KBCTL_AGAIN	-2
static void key_interrupt(int nr, void *devid, struct pt_regs *regs)
{
    static int lastc = -1;
    static int key_fn_down = 0;
    static int key_numl_down = 0;
    u_int val = 0; 
    int x = 0 ,y = 0;
    int   ret = 0;

    kbd_pt_regs = regs;
    UNSET_OUTPUT();
    while (!((SPSTA1) & SPSTA_READY));
    SPI_WRITE(0x0);
    while (!((SPSTA1) & SPSTA_READY));
    SPI_READ(val);
    SET_OUTPUT();

    x = val & 0xff;	/* get char by sending one */
    if (x == lastc || x == 0 || x == 0xff) {
      ret =  KBCTL_NODATA;
      tasklet_schedule(&keyboard_tasklet);
    }
    lastc = x;

    if ( x & 0x80) {		/* key up */
      x &= 0x7f;
      if (x == 0x21) {		/* fn key up */
	key_fn_down = 0;
	ret = KBCTL_AGAIN;
      } else {
	if (key_fn_down) { 	/* this is a fn modified key */
	  y = kbmapFN[x];
	  if (y == KK_NUML)
	    ret = KBCTL_AGAIN;
	} else { 
	  if (key_numl_down) 	/* this is a numlock modified key */
	    y = kbmapNL[x];
	  else
	    y = kbmap[x];
	}
      }
      ret =  (y | 0x80);
    } else {			/* key down */
      if (x == 0x21) {		/* fn key down */
	key_fn_down = 1;
	ret = KBCTL_AGAIN;
      } else {
	if (key_fn_down) {	/* this is a fn modified key */
	  y = kbmapFN[x];
	  if (y == KK_NUML) {	/* toggle local numlock */
	    key_numl_down = !key_numl_down;
	    ret = KBCTL_AGAIN;
	  }
	} else {
	  if (key_numl_down)	/* this is a numlock modified key */
	    y = kbmapNL[x];
	  else 
	    y = kbmap[x];	
	}
      }
      ret = y;
    }

    if ( ret != KBCTL_NODATA ) {
      if (  ret != KBCTL_AGAIN && ret != KK_NONE)
	handle_scancode(ret, !(ret & 0x80));
      tasklet_schedule(&keyboard_tasklet);
    } 
}


/*
 * You must see SPIcoder 06 UR5HCSPI-06 manual and Schematic of S3C2410  
 */
static int __init HW_kbd_init(void)
{
    int ret = -ENODEV;
    int delay ;

        set_external_irq(IRQ_KBD, EXT_FALLING_EDGE, GPIO_PULLUP_DIS);
        set_gpio_ctrl(GPIO_KBD_SPIMISO);
        set_gpio_ctrl(GPIO_KBD_SPIMOSI);
        set_gpio_ctrl(GPIO_KBD_SPICLK);
        set_gpio_ctrl(GPIO_KBD_nSS);
        set_gpio_ctrl(GPIO_KBD_PWR);

        write_gpio_bit(GPIO_KBD_PWR, 0);

    // Setup SPI registers
    /* Interrupt mode, prescaler enable, master mode,
       active high clock, format B, normal mode */
    SPCON1 = (SPCON_SMOD_INT | SPCON_ENSCK | SPCON_MSTR |
	      SPCON_CPOL_HIGH | SPCON_CPHA_FMTB);
    /*  Developer MUST change the value of prescaler properly
	whenever value of PCLK is changed. */
    SPPRE1 = 255;
    // 99.121K = 203M/4/2/(255+1) PCLK=50.75Mhz FCLK=203Mhz SPICLK=99.121Khz
 
    for (delay=0; delay < 10; delay++)
      SPI_WRITE(0xff);
	
    /*
     * Claim the appropriate interrupts
     */
    ret = request_irq(IRQ_KBD, key_interrupt, SA_INTERRUPT,
		      "keyboard", key_interrupt);
    if (ret) {
      printk(KERN_INFO "request IRQ failed (%d)\n", IRQ_KBD);
      return ret;
    }

    return ret;
}

static int s3c_kbd_translate(u_char scancode, u_char *keycode, char raw_mode)
{
//    *keycode = scancode & 0x7f;
    *keycode = scancode;
    return 1;
}

int __init s3c2410_kbd_init(void) {
    int  ret = -ENODEV;

    ret = HW_kbd_init();
    if (ret)
      return ret;

    /* include/asm/keyboard.h , we have not key_ops_struct */
    k_translate		= s3c_kbd_translate;

    return 0;
}
/*
 | $Id: s3c2410_kbd.c,v 1.1.1.1 2004/02/04 12:56:04 laputa Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
