/*
 * include/asm-arm/arch-sa1100/ming_ioctl.h
 *
 * ioctl's defintion.
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:38 $ 
 *
 * $Revision: 1.1.1.1 $
 *
   Wed Jun 13 2001 Yong-iL Joh <tolkien@mizi.com>
   - initial

   Mon Dec 17 2001 Lee, SooJin <soojin@mizi.com>
   - add radio's ioctl

   Wed Jan  9 2002 Lee, SooJin <soojin@mizi.com>

   Sat Jan 12 2002 Yong-iL Joh <tolkien@mizi.com>
   - rename as linuette_ioctl.h

   Tue May  7 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.31

   Fri May 10 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.33

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/ioctl.h>
#include "linuette_common.h"

#ifndef _INCLUDE_LINUETTE_IOCTL_H_
#define _INCLUDE_LINUETTE_IOCTL_H_
#ifndef __ASSEMBLY__

/* ioctls
 * 'F' is video subsystem's ioctl prefix
 * see Documentation/ioctl-number.txt
 */
#define BACKLIGHT	0x1
#define FRONTLIGHT	0x0
typedef struct {
  unsigned char mode;	/* backlight = 1, frontlight = 0 */
  unsigned char level;
} BRIGHTNESS_DEV;

#define GET_BRIGHTNESS		_IOR('F', 0x40, unsigned int)
#define SET_BRIGHTNESS		_IOW('F', 0x41, unsigned int)
#define PROBE_BRIGHTNESS	_IOR('F', 0x42, BRIGHTNESS_DEV)

typedef struct {
  unsigned short pressure;
  unsigned short x;
  unsigned short y;
  unsigned short pad;
} TS_RET;

typedef struct {
  int xscale;
  int xtrans;
  int yscale;
  int ytrans;
  int xyswap;
} TS_CAL;


#define AC_OFF_LINE	0x00
#define AC_ON_LINE	0x01
#define AC_BACKUP	0x02
#define AC_UNKNOWN	0xff

#define BATTERY_FULL	0x10
#define BATTERY_HIGH	0x00
#define BATTERY_LOW	0x01
#define BATTERY_CRIT	0x02
#define BATTERY_CHARGE	0x03
#define BATTERY_NONE	0x04
#define BATTERY_NOSYS	0x80
#define BATTERY_UNKNOWN	0xff
#define BATTERY_NORMAL	0x00

typedef struct {
  unsigned long sec;
  int level;
  unsigned char ac;
  unsigned char battery;
} BATTERY_RET;

/*
 * 'A' is for linux/apm_bios.h
 * see Documentation/ioctl-number.txt
 */
#define GET_BATTERY_STATUS	_IOR('A', 0x10, BATTERY_RET)
#define GET_JIFFIES		_IOR('A', 0x11, unsigned long)
#define GET_LCD_STATUS		_IOR('A', 0x13, unsigned int)
#define SET_INPUT_DEV		_IOW('A', 0x14, unsigned int)
#define GET_MZ_EVENT		_IOR('A', 0x25, unsigned long)
#define APM_LCD_OFF		_IO ('A', 0x50)	/* 0x50 == 80 */
#define APM_LCD_ON		_IO ('A', 0x51)
#define APM_DEV_LIST		_IO ('A', 0x52)	/* for debugging */
#define APM_DEV_ONOFF		_IOW('A', 0x53, struct pm_dev) 
#define APM_MZ_SLEEP		_IO ('A', 0x54) 

#define LCD_ON		0x01
#define LCD_OFF		0x00

#define INPUT_DEV_ON	0x01
#define INPUT_DEV_OFF	0x00

/* event */
#define BATTERY_IS_LOW	0x00000001
#define BATTERY_IS_FULL	0x00000002
#define ACLINE_ON	0x00000004
#define ACLINE_OFF	0x00000003

#define EVENT_UNKNOWN	0x00000010
#define PHONE_CALLED	0x00000020
#define DATA_RECEIVED	0x00000040
#define SYSTEM_NOMEM	0x00000060
#define SYSTEM_WAKEUP	0x00000070
#define SYSTEM_SLEEP	0x00000080
#define JFFS2_PANIC	0x00000090	/* JFFS2 panic(severely broken) */

#define CRADLE_INSERT	0x00000100	/* put/remove cradle */
#define CRADLE_REMOVE	0x00000200
#define USB_SL_INSERT	0x00000300	/* USB slave port insert/remove */
#define USB_SL_REMOVE	0x00000400
#define USB_MA_INSERT	0x00000500	/* some USB device de/attached */
#define USB_MA_REMOVE	0x00000600
#define UART_INSERT	0x00000700	/* UART port insert/remove */
#define UART_REMOVE	0x00000800

#define PCMCIA_INSERT	0x00001000	/* some PCMCIA device de/attached */
#define PCMCIA_REMOVE	0x00002000
#define CF_INSERT	PCMCIA_INSERT	/* some CF device de/attached */
#define CF_REMOVE	PCMCIA_REMOVE
#define EXT_DEV_INSERT	0x00003000	/* some removable device de/attached */
#define EXT_DEV_REMOVE	0x00004000

/*
 * for /dev/misc/led
 * 'h' is Charon filesystem <zapman@interlan.net>
 * see Documentation/ioctl-number.txt
 */
typedef struct {
  unsigned int index;		/* LED index to control */
  unsigned int stat;		/* control command or current status */
  unsigned int rate;		/* blinking rate */
  unsigned int info;		/* capable function */
} LED_RET;

#define GET_LED_NO		_IOR('h', 0x80, unsigned int)
#define GET_LED_STATUS		_IOR('h', 0x82, LED_RET)
#define SET_LED_STATUS		_IOW('h', 0x83, LED_RET)

#define MZ_LED_ON		0x01
#define MZ_LED_OFF		0x00
#define MZ_LED_BLINK		0x04
#define MZ_LED_BLINK_RATE	0x08
#define MZ_LED_READ_ONLY	0x80

/* ioctls for FB - extended 
 * see include/linux/fb.h
 */
typedef struct {
	int s_xoffset;
	int s_yoffset;
	int d_xoffset;
	int d_yoffset;
	int height;
	int width;
} HWBLT, *PHWBLT;
#define FBBITBLT		_IOW('F', 0xf0, HWBLT)  /* hardware bitblt */

#include "linuette_machine.h"
#endif	/* __ASSEMBLY__ */
#endif /* _INCLUDE_LINUETTE_IOCTL_H_ */

/*
 | $Id: linuette_ioctl.h,v 1.1.1.1 2004/02/04 12:57:38 laputa Exp $
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
