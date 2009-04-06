/*
 * linuette_kernel.h
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:38 $ 
 *
 * $Revision: 1.1.1.1 $
 *
   Fri May 10 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial
   - kernel vs app. API spec (draft) v1.33

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include "linuette_ioctl.h"

#ifndef _INCLUDE_LINUETTE_KERNEL_H_
#define _INCLUDE_LINUETTE_KERNEL_H_
#if !defined(__ASSEMBLY__) && defined(__KERNEL__)
#include <linux/fb.h>
#include <linux/pm.h>

struct mz_pm_ops_t {
  /* machine dependent stuff when do_sleep/wakeup */
  int (*machine_pm)(pm_request_t req);
  /* get battery status */
  int (*get_power_status)(BATTERY_RET *bat_dev);
  /* for brightness control */
  int (*fb_ioctl)(struct inode *inode, struct file *file, unsigned int cmd,
		  unsigned long arg, int con, struct fb_info *info);
  /* for brightness on/off */
  void (*blank_helper)(int blank);
};
extern struct mz_pm_ops_t mz_pm_ops;
#define MZ_BLANK_ON	1
#define MZ_BLANK_OFF	0

#define MZ_EVENT_BUF_SIZE	16
struct mz_event_queue_t {
  int buf[MZ_EVENT_BUF_SIZE];
  int head, tail;
};
extern void event_notify(int x_evt);

extern int mz_sys_state;
#define MZ_SYS_UNKNOWN	EVENT_UNKNOWN
#define MZ_SYS_SLEEP	SYSTEM_SLEEP
#define MZ_SYS_WAKEUP	SYSTEM_WAKEUP
static __inline__ int mz_sleep_p(void) {
    return (mz_sys_state == MZ_SYS_SLEEP);
}
static __inline__ int mz_wakeup_p(void) {
    return (mz_sys_state == MZ_SYS_WAKEUP);
}

#endif	/* !defined(__ASSEMBLY__) && defined(__KERNEL__) */
#endif /* _INCLUDE_LINUETTE_KERNEL_H_ */

/*
 | $Id: linuette_kernel.h,v 1.1.1.1 2004/02/04 12:57:38 laputa Exp $
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
