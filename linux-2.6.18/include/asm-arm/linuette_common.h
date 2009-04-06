/*
 * include/asm-arm/arch-sa1100/linuette_common.h
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2004/02/04 12:57:38 $ 
 *
 * $Revision: 1.1.1.1 $
 *
   Tue Jun 13 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial

   Fri Feb 22 2002 Yong-iL Joh <tolkien@mizi.com>
   - apply "Kernel vs Application API", 1.27

   Tue May  7 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.31

   Fri May 10 2002 Yong-iL Joh <tolkien@mizi.com>
   - kernel vs app. API spec (draft) v1.33

 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#ifndef _INCLUDE_LINUETTE_COMMON_H_
#define _INCLUDE_LINUETTE_COMMON_H_
#ifndef __ASSEMBLY__

/*
 * Definition of Generic Key Scancode
 */
#define SCANCODE_LEFT		0x69
#define SCANCODE_RIGHT		0x6a
#define SCANCODE_UP		0x67
#define SCANCODE_DOWN		0x6c
#define SCANCODE_ENTER		0x1c
#define SCANCODE_SLIDE_UP	0x68	/* Page Up */
#define SCANCODE_SLIDE_DOWN	0x6d	/* Page Down */
#define SCANCODE_SLIDE_CENTER	0x60	/* PAD Enter */
#define SCANCODE_POWER		0x3f	/* F5 */
#define SCANCODE_RECORD		0x40	/* F6 */
#define SCANCODE_ACTION		SCANCODE_ENTER

#define KEY_RELEASED	0
#define KEY_PRESSED	1

/*
 * key definition with iPAQ
 */
#define SCANCODE_MENU		0x58	/* F12 */
#define SCANCODE_HOME		0x7a
#define SCANCODE_CONTACT	0x7b
#define SCANCODE_CALENDAR	0x7c
#define SCANCODE_TASK		0x7d

/*
 * Key PAD
 */
#define SCANCODE_PAD_0		0x52
#define SCANCODE_PAD_1		0x4f
#define SCANCODE_PAD_2		0x50
#define SCANCODE_PAD_3		0x51
#define SCANCODE_PAD_4		0x4b
#define SCANCODE_PAD_5		0x4c
#define SCANCODE_PAD_6		0x4d
#define SCANCODE_PAD_7		0x47
#define SCANCODE_PAD_8		0x48
#define SCANCODE_PAD_9		0x49
#define SCANCODE_PAD_MINUS	0x4a
#define SCANCODE_PAD_PLUS	0x4e
#define SCANCODE_PAD_ENTER	0x60
#define SCANCODE_PAD_PERIOD	0x53
#define SCANCODE_PAD_SLASH	0x62
#define SCANCODE_PAD_ASTERISK	0x37

/*
 * Phone Key
 */
#define SCANCODE_ASTERISK	SCANCODE_PAD_PLUS
#define SCANCODE_SHARP		SCANCODE_PAD_MINUS
#define SCANCODE_SEND		0x7e
#define SCANCODE_END		0x7f
#define SCANCODE_FN_LEFT	0x63	/* PrintScreen */
#define SCANCODE_EMAIL		0x46	/* ScrollLock */
#define SCANCODE_FN_RIGHT	0x77	/* Break */

/*
 * Undefined Region
 */
#define SCANCODE_F7		0x41
#define SCANCODE_F8		0x42
#define SCANCODE_F9		0x43
#define SCANCODE_F10		0x44
#define SCANCODE_F11		0x57
#define SCANCODE_U1		0x78	/* Unknown */
#define SCANCODE_U2		0x79	/* Unknown */
#define SCANCODE_U3		0x70	/* Unknown */
#define SCANCODE_U4		0x71	/* Unknown */
#define SCANCODE_U5		0x72	/* Unknown */
#define SCANCODE_U6		0x73	/* Unknown */
#define SCANCODE_U7		0x74	/* Unknown */
#define SCANCODE_U8		0x75	/* Unknown */
#define SCANCODE_U9		0x76	/* Unknown */

#endif	/* __ASSEMBLY__ */
#endif /* _INCLUDE_LINUETTE_COMMON_H_ */

/*
 | $Id: linuette_common.h,v 1.1.1.1 2004/02/04 12:57:38 laputa Exp $
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
