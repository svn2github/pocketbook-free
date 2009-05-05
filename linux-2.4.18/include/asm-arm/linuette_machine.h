/*
 * linuette_machine.h
 *
 * vendor/machine specifice ioctl function
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:38 $ 
 *
 * $Revision: 1.1.1.1 $
 */

/* extended ioctl for MTD
 * change the permission of MTDPART
 */
#define MEMCHMODPART		_IOWR('M', 0xf0, struct mtd_info_user)

/*
 * for SmartMedia Card
 * 'h' is Charon filesystem <zapman@interlan.net>
 * see Documentation/ioctl-number.txt
 */
#define SM_FORMAT		_IO ('h', 0x90)
#define SM_FORMAT_LOW		_IO ('h', 0x91)

/*
 | $Id: linuette_machine.h,v 1.1.1.1 2004/02/04 12:57:38 laputa Exp $
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
