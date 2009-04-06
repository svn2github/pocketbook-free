/*
 * event.c
 *
 * a device for sending from kernel to appl.
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:55:26 $ 
 *
 * $Revision: 1.1.1.1 $

   Tue 15 Jan 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial, see "kernel vs Application API spec (0.5, draft)"

   Fri May 10 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.33

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <asm/hardware.h>

int mz_sys_state = MZ_SYS_UNKNOWN;

DECLARE_WAIT_QUEUE_HEAD(mz_event_queue);
struct mz_event_queue_t mz_event_q;
#define INCBUF(x,mod) (((x)+1) & ((mod) - 1))

static void mz_event_task_handler(void *data) {
    wake_up_interruptible(&mz_event_queue);
}

static struct tq_struct mz_event_task = {
    routine: mz_event_task_handler
};

void event_notify(int x_evt) {
    if (x_evt == SYSTEM_SLEEP)
      mz_sys_state = MZ_SYS_SLEEP;
    else if (x_evt == SYSTEM_WAKEUP)
      mz_sys_state = MZ_SYS_WAKEUP;

    mz_event_q.buf[mz_event_q.head] = x_evt;
    mz_event_q.head = INCBUF(mz_event_q.head, MZ_EVENT_BUF_SIZE);
    if (mz_event_q.head == mz_event_q.tail)
      mz_event_q.tail = INCBUF(mz_event_q.tail, MZ_EVENT_BUF_SIZE);

    schedule_task(&mz_event_task);
}

static int __init event_init(void)
{
    init_waitqueue_head(&mz_event_queue);
    mz_event_q.head = mz_event_q.tail = 0;
    return 0;
}

module_init(event_init);

MODULE_AUTHOR("Yong-iL Joh");
MODULE_DESCRIPTION("a device for sending from kernel to appl.");

EXPORT_SYMBOL(event_notify);
EXPORT_SYMBOL(mz_event_queue);
EXPORT_SYMBOL(mz_event_q);
EXPORT_SYMBOL(mz_sys_state);
/*
 | $Id: event.c,v 1.1.1.1 2004/02/04 12:55:26 laputa Exp $
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
