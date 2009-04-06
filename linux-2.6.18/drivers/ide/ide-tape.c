/*
 * linux/drivers/ide/ide-tape.c		Version 1.17a	Jan, 2001
 *
 * Copyright (C) 1995 - 1999 Gadi Oxman <gadio@netvision.net.il>
 *
 * $Header: /home/cvs/s3c2410_kernel2.4.18_rel/drivers/ide/ide-tape.c,v 1.1.1.1 2004/02/04 12:56:13 laputa Exp $
 *
 * This driver was constructed as a student project in the software laboratory
 * of the faculty of electrical engineering in the Technion - Israel's
 * Institute Of Technology, with the guide of Avner Lottem and Dr. Ilana David.
 *
 * It is hereby placed under the terms of the GNU general public license.
 * (See linux/COPYING).
 */
 
/*
 * IDE ATAPI streaming tape driver.
 *
 * This driver is a part of the Linux ide driver and works in co-operation
 * with linux/drivers/block/ide.c.
 *
 * The driver, in co-operation with ide.c, basically traverses the 
 * request-list for the block device interface. The character device
 * interface, on the other hand, creates new requests, adds them
 * to the request-list of the block device, and waits for their completion.
 *
 * Pipelined operation mode is now supported on both reads and writes.
 *
 * The block device major and minor numbers are determined from the
 * tape's relative position in the ide interfaces, as explained in ide.c.
 *
 * The character device interface consists of the following devices:
 *
 * ht0		major 37, minor 0	first  IDE tape, rewind on close.
 * ht1		major 37, minor 1	second IDE tape, rewind on close.
 * ...
 * nht0		major 37, minor 128	first  IDE tape, no rewind on close.
 * nht1		major 37, minor 129	second IDE tape, no rewind on close.
 * ...
 *
 * Run linux/scripts/MAKEDEV.ide to create the above entries.
 *
 * The general magnetic tape commands compatible interface, as defined by
 * include/linux/mtio.h, is accessible through the character device.
 *
 * General ide driver configuration options, such as the interrupt-unmask
 * flag, can be configured by issuing an ioctl to the block device interface,
 * as any other ide device.
 *
 * Our own ide-tape ioctl's can be issued to either the block device or
 * the character device interface.
 *
 * Maximal throughput with minimal bus load will usually be achieved in the
 * following scenario:
 *
 *	1.	ide-tape is operating in the pipelined operation mode.
 *	2.	No buffering is performed by the user backup program.
 *
 * Testing was done with a 2 GB CONNER CTMA 4000 IDE ATAPI Streaming Tape Drive.
 * 
 * Ver 0.1   Nov  1 95   Pre-working code :-)
 * Ver 0.2   Nov 23 95   A short backup (few megabytes) and restore procedure
 *                        was successful ! (Using tar cvf ... on the block
 *                        device interface).
 *                       A longer backup resulted in major swapping, bad
 *                        overall Linux performance and eventually failed as
 *                        we received non serial read-ahead requests from the
 *                        buffer cache.
 * Ver 0.3   Nov 28 95   Long backups are now possible, thanks to the
 *                        character device interface. Linux's responsiveness
 *                        and performance doesn't seem to be much affected
 *                        from the background backup procedure.
 *                       Some general mtio.h magnetic tape operations are
 *                        now supported by our character device. As a result,
 *                        popular tape utilities are starting to work with
 *                        ide tapes :-)
 *                       The following configurations were tested:
 *                       	1. An IDE ATAPI TAPE shares the same interface
 *                       	   and irq with an IDE ATAPI CDROM.
 *                        	2. An IDE ATAPI TAPE shares the same interface
 *                          	   and irq with a normal IDE disk.
 *                        Both configurations seemed to work just fine !
 *                        However, to be on the safe side, it is meanwhile
 *                        recommended to give the IDE TAPE its own interface
 *                        and irq.
 *                       The one thing which needs to be done here is to
 *                        add a "request postpone" feature to ide.c,
 *                        so that we won't have to wait for the tape to finish
 *                        performing a long media access (DSC) request (such
 *                        as a rewind) before we can access the other device
 *                        on the same interface. This effect doesn't disturb
 *                        normal operation most of the time because read/write
 *                        requests are relatively fast, and once we are
 *                        performing one tape r/w request, a lot of requests
 *                        from the other device can be queued and ide.c will
 *			  service all of them after this single tape request.
 * Ver 1.0   Dec 11 95   Integrated into Linux 1.3.46 development tree.
 *                       On each read / write request, we now ask the drive
 *                        if we can transfer a constant number of bytes
 *                        (a parameter of the drive) only to its buffers,
 *                        without causing actual media access. If we can't,
 *                        we just wait until we can by polling the DSC bit.
 *                        This ensures that while we are not transferring
 *                        more bytes than the constant referred to above, the
 *                        interrupt latency will not become too high and
 *                        we won't cause an interrupt timeout, as happened
 *                        occasionally in the previous version.
 *                       While polling for DSC, the current request is
 *                        postponed and ide.c is free to handle requests from
 *                        the other device. This is handled transparently to
 *                        ide.c. The hwgroup locking method which was used
 *                        in the previous version was removed.
 *                       Use of new general features which are provided by
 *                        ide.c for use with atapi devices.
 *                        (Programming done by Mark Lord)
 *                       Few potential bug fixes (Again, suggested by Mark)
 *                       Single character device data transfers are now
 *                        not limited in size, as they were before.
 *                       We are asking the tape about its recommended
 *                        transfer unit and send a larger data transfer
 *                        as several transfers of the above size.
 *                        For best results, use an integral number of this
 *                        basic unit (which is shown during driver
 *                        initialization). I will soon add an ioctl to get
 *                        this important parameter.
 *                       Our data transfer buffer is allocated on startup,
 *                        rather than before each data transfer. This should
 *                        ensure that we will indeed have a data buffer.
 * Ver 1.1   Dec 14 95   Fixed random problems which occurred when the tape
 *                        shared an interface with another device.
 *                        (poll_for_dsc was a complete mess).
 *                       Removed some old (non-active) code which had
 *                        to do with supporting buffer cache originated
 *                        requests.
 *                       The block device interface can now be opened, so
 *                        that general ide driver features like the unmask
 *                        interrupts flag can be selected with an ioctl.
 *                        This is the only use of the block device interface.
 *                       New fast pipelined operation mode (currently only on
 *                        writes). When using the pipelined mode, the
 *                        throughput can potentially reach the maximum
 *                        tape supported throughput, regardless of the
 *                        user backup program. On my tape drive, it sometimes
 *                        boosted performance by a factor of 2. Pipelined
 *                        mode is enabled by default, but since it has a few
 *                        downfalls as well, you may want to disable it.
 *                        A short explanation of the pipelined operation mode
 *                        is available below.
 * Ver 1.2   Jan  1 96   Eliminated pipelined mode race condition.
 *                       Added pipeline read mode. As a result, restores
 *                        are now as fast as backups.
 *                       Optimized shared interface behavior. The new behavior
 *                        typically results in better IDE bus efficiency and
 *                        higher tape throughput.
 *                       Pre-calculation of the expected read/write request
 *                        service time, based on the tape's parameters. In
 *                        the pipelined operation mode, this allows us to
 *                        adjust our polling frequency to a much lower value,
 *                        and thus to dramatically reduce our load on Linux,
 *                        without any decrease in performance.
 *                       Implemented additional mtio.h operations.
 *                       The recommended user block size is returned by
 *                        the MTIOCGET ioctl.
 *                       Additional minor changes.
 * Ver 1.3   Feb  9 96   Fixed pipelined read mode bug which prevented the
 *                        use of some block sizes during a restore procedure.
 *                       The character device interface will now present a
 *                        continuous view of the media - any mix of block sizes
 *                        during a backup/restore procedure is supported. The
 *                        driver will buffer the requests internally and
 *                        convert them to the tape's recommended transfer
 *                        unit, making performance almost independent of the
 *                        chosen user block size.
 *                       Some improvements in error recovery.
 *                       By cooperating with ide-dma.c, bus mastering DMA can
 *                        now sometimes be used with IDE tape drives as well.
 *                        Bus mastering DMA has the potential to dramatically
 *                        reduce the CPU's overhead when accessing the device,
 *                        and can be enabled by using hdparm -d1 on the tape's
 *                        block device interface. For more info, read the
 *                        comments in ide-dma.c.
 * Ver 1.4   Mar 13 96   Fixed serialize support.
 * Ver 1.5   Apr 12 96   Fixed shared interface operation, broken in 1.3.85.
 *                       Fixed pipelined read mode inefficiency.
 *                       Fixed nasty null dereferencing bug.
 * Ver 1.6   Aug 16 96   Fixed FPU usage in the driver.
 *                       Fixed end of media bug.
 * Ver 1.7   Sep 10 96   Minor changes for the CONNER CTT8000-A model.
 * Ver 1.8   Sep 26 96   Attempt to find a better balance between good
 *                        interactive response and high system throughput.
 * Ver 1.9   Nov  5 96   Automatically cross encountered filemarks rather
 *                        than requiring an explicit FSF command.
 *                       Abort pending requests at end of media.
 *                       MTTELL was sometimes returning incorrect results.
 *                       Return the real block size in the MTIOCGET ioctl.
 *                       Some error recovery bug fixes.
 * Ver 1.10  Nov  5 96   Major reorganization.
 *                       Reduced CPU overhead a bit by eliminating internal
 *                        bounce buffers.
 *                       Added module support.
 *                       Added multiple tape drives support.
 *                       Added partition support.
 *                       Rewrote DSC handling.
 *                       Some portability fixes.
 *                       Removed ide-tape.h.
 *                       Additional minor changes.
 * Ver 1.11  Dec  2 96   Bug fix in previous DSC timeout handling.
 *                       Use ide_stall_queue() for DSC overlap.
 *                       Use the maximum speed rather than the current speed
 *                        to compute the request service time.
 * Ver 1.12  Dec  7 97   Fix random memory overwriting and/or last block data
 *                        corruption, which could occur if the total number
 *                        of bytes written to the tape was not an integral
 *                        number of tape blocks.
 *                       Add support for INTERRUPT DRQ devices.
 * Ver 1.13  Jan  2 98   Add "speed == 0" work-around for HP COLORADO 5GB
 * Ver 1.14  Dec 30 98   Partial fixes for the Sony/AIWA tape drives.
 *                       Replace cli()/sti() with hwgroup spinlocks.
 * Ver 1.15  Mar 25 99   Fix SMP race condition by replacing hwgroup
 *                        spinlock with private per-tape spinlock.
 * Ver 1.16  Sep  1 99   Add OnStream tape support.
 *                       Abort read pipeline on EOD.
 *                       Wait for the tape to become ready in case it returns
 *                        "in the process of becoming ready" on open().
 *                       Fix zero padding of the last written block in
 *                        case the tape block size is larger than PAGE_SIZE.
 *                       Decrease the default disconnection time to tn.
 * Ver 1.16e Oct  3 99   Minor fixes.
 * Ver 1.16e1 Oct 13 99  Patches by Arnold Niessen,
 *                          niessen@iae.nl / arnold.niessen@philips.com
 *                   GO-1)  Undefined code in idetape_read_position
 *				according to Gadi's email
 *                   AJN-1) Minor fix asc == 11 should be asc == 0x11
 *                               in idetape_issue_packet_command (did effect
 *                               debugging output only)
 *                   AJN-2) Added more debugging output, and
 *                              added ide-tape: where missing. I would also
 *				like to add tape->name where possible
 *                   AJN-3) Added different debug_level's 
 *                              via /proc/ide/hdc/settings
 * 				"debug_level" determines amount of debugging output;
 * 				can be changed using /proc/ide/hdx/settings
 * 				0 : almost no debugging output
 * 				1 : 0+output errors only
 * 				2 : 1+output all sensekey/asc
 * 				3 : 2+follow all chrdev related procedures
 * 				4 : 3+follow all procedures
 * 				5 : 4+include pc_stack rq_stack info
 * 				6 : 5+USE_COUNT updates
 *                   AJN-4) Fixed timeout for retension in idetape_queue_pc_tail
 *				from 5 to 10 minutes
 *                   AJN-5) Changed maximum number of blocks to skip when
 *                              reading tapes with multiple consecutive write
 *                              errors from 100 to 1000 in idetape_get_logical_blk
 *                   Proposed changes to code:
 *                   1) output "logical_blk_num" via /proc
 *                   2) output "current_operation" via /proc
 *                   3) Either solve or document the fact that `mt rewind' is
 *                      required after reading from /dev/nhtx to be
 *			able to rmmod the idetape module;
 *			Also, sometimes an application finishes but the
 *			device remains `busy' for some time. Same cause ?
 *                   Proposed changes to release-notes:
 *		     4) write a simple `quickstart' section in the
 *                      release notes; I volunteer if you don't want to
 * 		     5) include a pointer to video4linux in the doc
 *                      to stimulate video applications
 *                   6) release notes lines 331 and 362: explain what happens
 *			if the application data rate is higher than 1100 KB/s; 
 *			similar approach to lower-than-500 kB/s ?
 *		     7) 6.6 Comparison; wouldn't it be better to allow different 
 *			strategies for read and write ?
 *			Wouldn't it be better to control the tape buffer
 *			contents instead of the bandwidth ?
 *		     8) line 536: replace will by would (if I understand
 *			this section correctly, a hypothetical and unwanted situation
 *			 is being described)
 * Ver 1.16f Dec 15 99   Change place of the secondary OnStream header frames.
 * Ver 1.17  Nov 2000 / Jan 2001  Marcel Mol, marcel@mesa.nl
 *			- Add idetape_onstream_mode_sense_tape_parameter_page
 *			  function to get tape capacity in frames: tape->capacity.
 *			- Add support for DI-50 drives( or any DI- drive).
 *			- 'workaround' for read error/blank block arround block 3000.
 *			- Implement Early warning for end of media for Onstream.
 *			- Cosmetic code changes for readability.
 *			- Idetape_position_tape should not use SKIP bit during
 *			  Onstream read recovery.
 *			- Add capacity, logical_blk_num and first/last_frame_position
 *			  to /proc/ide/hd?/settings.
 *			- Module use count was gone in the Linux 2.4 driver.
 * Ver 1.17a Apr 2001 Willem Riede osst@riede.org
 * 			- Get drive's actual block size from mode sense block descriptor
 * 			- Limit size of pipeline
 *
 * Here are some words from the first releases of hd.c, which are quoted
 * in ide.c and apply here as well:
 *
 * | Special care is recommended.  Have Fun!
 *
 */

/*
 * An overview of the pipelined operation mode.
 *
 * In the pipelined write mode, we will usually just add requests to our
 * pipeline and return immediately, before we even start to service them. The
 * user program will then have enough time to prepare the next request while
 * we are still busy servicing previous requests. In the pipelined read mode,
 * the situation is similar - we add read-ahead requests into the pipeline,
 * before the user even requested them.
 *
 * The pipeline can be viewed as a "safety net" which will be activated when
 * the system load is high and prevents the user backup program from keeping up
 * with the current tape speed. At this point, the pipeline will get
 * shorter and shorter but the tape will still be streaming at the same speed.
 * Assuming we have enough pipeline stages, the system load will hopefully
 * decrease before the pipeline is completely empty, and the backup program
 * will be able to "catch up" and refill the pipeline again.
 * 
 * When using the pipelined mode, it would be best to disable any type of
 * buffering done by the user program, as ide-tape already provides all the
 * benefits in the kernel, where it can be done in a more efficient way.
 * As we will usually not block the user program on a request, the most
 * efficient user code will then be a simple read-write-read-... cycle.
 * Any additional logic will usually just slow down the backup process.
 *
 * Using the pipelined mode, I get a constant over 400 KBps throughput,
 * which seems to be the maximum throughput supported by my tape.
 *
 * However, there are some downfalls:
 *
 *	1.	We use memory (for data buffers) in proportional to the number
 *		of pipeline stages (each stage is about 26 KB with my tape).
 *	2.	In the pipelined write mode, we cheat and postpone error codes
 *		to the user task. In read mode, the actual tape position
 *		will be a bit further than the last requested block.
 *
 * Concerning (1):
 *
 *	1.	We allocate stages dynamically only when we need them. When
 *		we don't need them, we don't consume additional memory. In
 *		case we can't allocate stages, we just manage without them
 *		(at the expense of decreased throughput) so when Linux is
 *		tight in memory, we will not pose additional difficulties.
 *
 *	2.	The maximum number of stages (which is, in fact, the maximum
 *		amount of memory) which we allocate is limited by the compile
 *		time parameter IDETAPE_MAX_PIPELINE_STAGES.
 *
 *	3.	The maximum number of stages is a controlled parameter - We
 *		don't start from the user defined maximum number of stages
 *		but from the lower IDETAPE_MIN_PIPELINE_STAGES (again, we
 *		will not even allocate this amount of stages if the user
 *		program can't handle the speed). We then implement a feedback
 *		loop which checks if the pipeline is empty, and if it is, we
 *		increase the maximum number of stages as necessary until we
 *		reach the optimum value which just manages to keep the tape
 *		busy with minimum allocated memory or until we reach
 *		IDETAPE_MAX_PIPELINE_STAGES.
 *
 * Concerning (2):
 *
 *	In pipelined write mode, ide-tape can not return accurate error codes
 *	to the user program since we usually just add the request to the
 *      pipeline without waiting for it to be serviced. In case an error
 *      occurs, I will report it on the next user request.
 *
 *	In the pipelined read mode, subsequent read requests or forward
 *	filemark spacing will perform correctly, as we preserve all blocks
 *	and filemarks which we encountered during our excess read-ahead.
 * 
 *	For accurate tape positioning and error reporting, disabling
 *	pipelined mode might be the best option.
 *
 * You can enable/disable/tune the pipelined operation mode by adjusting
 * the compile time parameters below.
 */

/*
 *	Possible improvements.
 *
 *	1.	Support for the ATAPI overlap protocol.
 *
 *		In order to maximize bus throughput, we currently use the DSC
 *		overlap method which enables ide.c to service requests from the
 *		other device while the tape is busy executing a command. The
 *		DSC overlap method involves polling the tape's status register
 *		for the DSC bit, and servicing the other device while the tape
 *		isn't ready.
 *
 *		In the current QIC development standard (December 1995),
 *		it is recommended that new tape drives will *in addition* 
 *		implement the ATAPI overlap protocol, which is used for the
 *		same purpose - efficient use of the IDE bus, but is interrupt
 *		driven and thus has much less CPU overhead.
 *
 *		ATAPI overlap is likely to be supported in most new ATAPI
 *		devices, including new ATAPI cdroms, and thus provides us
 *		a method by which we can achieve higher throughput when
 *		sharing a (fast) ATA-2 disk with any (slow) new ATAPI device.
 */

#define IDETAPE_VERSION "1.17a"

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/ide.h>
#include <linux/smp_lock.h>
#include <linux/completion.h>

#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/bitops.h>


#define NO_LONGER_REQUIRED	(1)

/*
 *	OnStream support
 */
#define ONSTREAM_DEBUG		(0)
#define OS_CONFIG_PARTITION	(0xff)
#define OS_DATA_PARTITION	(0)
#define OS_PARTITION_VERSION	(1)
#define OS_EW			300
#define OS_ADR_MINREV		2

#define OS_DATA_STARTFRAME1	20
#define OS_DATA_ENDFRAME1	2980
/*
 * partition
 */
typedef struct os_partition_s {
	__u8	partition_num;
	__u8	par_desc_ver;
	__u16	wrt_pass_cntr;
	__u32	first_frame_addr;
	__u32	last_frame_addr;
	__u32	eod_frame_addr;
} os_partition_t;

/*
 * DAT entry
 */
typedef struct os_dat_entry_s {
	__u32	blk_sz;
	__u16	blk_cnt;
	__u8	flags;
	__u8	reserved;
} os_dat_entry_t;

/*
 * DAT
 */
#define OS_DAT_FLAGS_DATA	(0xc)
#define OS_DAT_FLAGS_MARK	(0x1)

typedef struct os_dat_s {
	__u8		dat_sz;
	__u8		reserved1;
	__u8		entry_cnt;
	__u8		reserved3;
	os_dat_entry_t	dat_list[16];
} os_dat_t;

/*
 * Frame types
 */
#define OS_FRAME_TYPE_FILL	(0)
#define OS_FRAME_TYPE_EOD	(1 << 0)
#define OS_FRAME_TYPE_MARKER	(1 << 1)
#define OS_FRAME_TYPE_HEADER	(1 << 3)
#define OS_FRAME_TYPE_DATA	(1 << 7)

/*
 * AUX
 */
typedef struct os_aux_s {
	__u32		format_id;		/* hardware compability AUX is based on */
	char		application_sig[4];	/* driver used to write this media */
	__u32		hdwr;			/* reserved */
	__u32		update_frame_cntr;	/* for configuration frame */
	__u8		frame_type;
	__u8		frame_type_reserved;
	__u8		reserved_18_19[2];
	os_partition_t	partition;
	__u8		reserved_36_43[8];
	__u32		frame_seq_num;
	__u32		logical_blk_num_high;
	__u32		logical_blk_num;
	os_dat_t	dat;
	__u8		reserved188_191[4];
	__u32		filemark_cnt;
	__u32		phys_fm;
	__u32		last_mark_addr;
	__u8		reserved204_223[20];

	/*
	 * __u8		app_specific[32];
	 *
	 * Linux specific fields:
	 */
	 __u32		next_mark_addr;		/* when known, points to next marker */
	 __u8		linux_specific[28];

	__u8		reserved_256_511[256];
} os_aux_t;

typedef struct os_header_s {
	char		ident_str[8];
	__u8		major_rev;
	__u8		minor_rev;
	__u8		reserved10_15[6];
	__u8		par_num;
	__u8		reserved1_3[3];
	os_partition_t	partition;
} os_header_t;

/*
 *	OnStream Tape Parameters Page
 */
typedef struct {
	unsigned	page_code	:6;	/* Page code - Should be 0x2b */
	unsigned	reserved1_6	:1;
	unsigned	ps		:1;
	__u8		reserved2;
	__u8		density;		/* kbpi */
	__u8		reserved3,reserved4;
	__u16		segtrk;                 /* segment of per track */
	__u16		trks;                   /* tracks per tape */
	__u8		reserved5,reserved6,reserved7,reserved8,reserved9,reserved10;
} onstream_tape_paramtr_page_t;

/*
 * OnStream ADRL frame
 */
#define OS_FRAME_SIZE	(32 * 1024 + 512)
#define OS_DATA_SIZE	(32 * 1024)
#define OS_AUX_SIZE	(512)

/*
 * internal error codes for onstream
 */
#define OS_PART_ERROR    2
#define OS_WRITE_ERROR   1

#include <linux/mtio.h>

/**************************** Tunable parameters *****************************/


/*
 *	Pipelined mode parameters.
 *
 *	We try to use the minimum number of stages which is enough to
 *	keep the tape constantly streaming. To accomplish that, we implement
 *	a feedback loop around the maximum number of stages:
 *
 *	We start from MIN maximum stages (we will not even use MIN stages
 *      if we don't need them), increment it by RATE*(MAX-MIN)
 *	whenever we sense that the pipeline is empty, until we reach
 *	the optimum value or until we reach MAX.
 *
 *	Setting the following parameter to 0 will disable the pipelined mode.
 */
#define IDETAPE_MIN_PIPELINE_STAGES	200
#define IDETAPE_MAX_PIPELINE_STAGES	400
#define IDETAPE_INCREASE_STAGES_RATE	 20

/*
 *	The following are used to debug the driver:
 *
 *	Setting IDETAPE_DEBUG_INFO to 1 will report device capabilities.
 *	Setting IDETAPE_DEBUG_LOG to 1 will log driver flow control.
 *	Setting IDETAPE_DEBUG_BUGS to 1 will enable self-sanity checks in
 *	some places.
 *
 *	Setting them to 0 will restore normal operation mode:
 *
 *		1.	Disable logging normal successful operations.
 *		2.	Disable self-sanity checks.
 *		3.	Errors will still be logged, of course.
 *
 *	All the #if DEBUG code will be removed some day, when the driver
 *	is verified to be stable enough. This will make it much more
 *	esthetic.
 */
#define IDETAPE_DEBUG_INFO		1
#define IDETAPE_DEBUG_LOG		1
#define IDETAPE_DEBUG_LOG_VERBOSE	0
#define IDETAPE_DEBUG_BUGS		1

/*
 *	After each failed packet command we issue a request sense command
 *	and retry the packet command IDETAPE_MAX_PC_RETRIES times.
 *
 *	Setting IDETAPE_MAX_PC_RETRIES to 0 will disable retries.
 */
#define IDETAPE_MAX_PC_RETRIES		3

/*
 *	With each packet command, we allocate a buffer of
 *	IDETAPE_PC_BUFFER_SIZE bytes. This is used for several packet
 *	commands (Not for READ/WRITE commands).
 */
#define IDETAPE_PC_BUFFER_SIZE		256

/*
 *	In various places in the driver, we need to allocate storage
 *	for packet commands and requests, which will remain valid while
 *	we leave the driver to wait for an interrupt or a timeout event.
 */
#define IDETAPE_PC_STACK		(10 + IDETAPE_MAX_PC_RETRIES)

/*
 *	Some tape drives require a long irq timeout
 */
#define IDETAPE_WAIT_CMD		(60*HZ)

/*
 *	The following parameter is used to select the point in the internal
 *	tape fifo in which we will start to refill the buffer. Decreasing
 *	the following parameter will improve the system's latency and
 *	interactive response, while using a high value might improve sytem
 *	throughput.
 */
#define IDETAPE_FIFO_THRESHOLD 		2

/*
 *	DSC polling parameters.
 *
 *	Polling for DSC (a single bit in the status register) is a very
 *	important function in ide-tape. There are two cases in which we
 *	poll for DSC:
 *
 *	1.	Before a read/write packet command, to ensure that we
 *		can transfer data from/to the tape's data buffers, without
 *		causing an actual media access. In case the tape is not
 *		ready yet, we take out our request from the device
 *		request queue, so that ide.c will service requests from
 *		the other device on the same interface meanwhile.
 *
 *	2.	After the successful initialization of a "media access
 *		packet command", which is a command which can take a long
 *		time to complete (it can be several seconds or even an hour).
 *
 *		Again, we postpone our request in the middle to free the bus
 *		for the other device. The polling frequency here should be
 *		lower than the read/write frequency since those media access
 *		commands are slow. We start from a "fast" frequency -
 *		IDETAPE_DSC_MA_FAST (one second), and if we don't receive DSC
 *		after IDETAPE_DSC_MA_THRESHOLD (5 minutes), we switch it to a
 *		lower frequency - IDETAPE_DSC_MA_SLOW (1 minute).
 *
 *	We also set a timeout for the timer, in case something goes wrong.
 *	The timeout should be longer then the maximum execution time of a
 *	tape operation.
 */
 
/*
 *	DSC timings.
 */
#define IDETAPE_DSC_RW_MIN		5*HZ/100	/* 50 msec */
#define IDETAPE_DSC_RW_MAX		40*HZ/100	/* 400 msec */
#define IDETAPE_DSC_RW_TIMEOUT		2*60*HZ		/* 2 minutes */
#define IDETAPE_DSC_MA_FAST		2*HZ		/* 2 seconds */
#define IDETAPE_DSC_MA_THRESHOLD	5*60*HZ		/* 5 minutes */
#define IDETAPE_DSC_MA_SLOW		30*HZ		/* 30 seconds */
#define IDETAPE_DSC_MA_TIMEOUT		2*60*60*HZ	/* 2 hours */

/*************************** End of tunable parameters ***********************/

/*
 *	Debugging/Performance analysis
 *
 *	I/O trace support
 */
#define USE_IOTRACE	0
#if USE_IOTRACE
#include <linux/io_trace.h>
#define IO_IDETAPE_FIFO	500
#endif

/*
 *	Read/Write error simulation
 */
#define SIMULATE_ERRORS			0

/*
 *	For general magnetic tape device compatibility.
 */
typedef enum {
	idetape_direction_none,
	idetape_direction_read,
	idetape_direction_write
} idetape_chrdev_direction_t;

/*
 *	Our view of a packet command.
 */
typedef struct idetape_packet_command_s {
	u8 c[12];				/* Actual packet bytes */
	int retries;				/* On each retry, we increment retries */
	int error;				/* Error code */
	int request_transfer;			/* Bytes to transfer */
	int actually_transferred;		/* Bytes actually transferred */
	int buffer_size;			/* Size of our data buffer */
	struct buffer_head *bh;
	char *b_data;
	int b_count;
	byte *buffer;				/* Data buffer */
	byte *current_position;			/* Pointer into the above buffer */
	ide_startstop_t (*callback) (ide_drive_t *);	/* Called when this packet command is completed */
	byte pc_buffer[IDETAPE_PC_BUFFER_SIZE];	/* Temporary buffer */
	unsigned long flags;			/* Status/Action bit flags: long for set_bit */
} idetape_pc_t;

/*
 *	Packet command flag bits.
 */
#define	PC_ABORT			0	/* Set when an error is considered normal - We won't retry */
#define PC_WAIT_FOR_DSC			1	/* 1 When polling for DSC on a media access command */
#define PC_DMA_RECOMMENDED		2	/* 1 when we prefer to use DMA if possible */
#define	PC_DMA_IN_PROGRESS		3	/* 1 while DMA in progress */
#define	PC_DMA_ERROR			4	/* 1 when encountered problem during DMA */
#define	PC_WRITING			5	/* Data direction */

/*
 *	Capabilities and Mechanical Status Page
 */
typedef struct {
	unsigned	page_code	:6;	/* Page code - Should be 0x2a */
	__u8		reserved0_6	:1;
	__u8		ps		:1;	/* parameters saveable */
	__u8		page_length;		/* Page Length - Should be 0x12 */
	__u8		reserved2, reserved3;
	unsigned	ro		:1;	/* Read Only Mode */
	unsigned	reserved4_1234	:4;
	unsigned	sprev		:1;	/* Supports SPACE in the reverse direction */
	unsigned	reserved4_67	:2;
	unsigned	reserved5_012	:3;
	unsigned	efmt		:1;	/* Supports ERASE command initiated formatting */
	unsigned	reserved5_4	:1;
	unsigned	qfa		:1;	/* Supports the QFA two partition formats */
	unsigned	reserved5_67	:2;
	unsigned	lock		:1;	/* Supports locking the volume */
	unsigned	locked		:1;	/* The volume is locked */
	unsigned	prevent		:1;	/* The device defaults in the prevent state after power up */	
	unsigned	eject		:1;	/* The device can eject the volume */
	__u8		disconnect	:1;	/* The device can break request > ctl */	
	__u8		reserved6_5	:1;
	unsigned	ecc		:1;	/* Supports error correction */
	unsigned	cmprs		:1;	/* Supports data compression */
	unsigned	reserved7_0	:1;
	unsigned	blk512		:1;	/* Supports 512 bytes block size */
	unsigned	blk1024		:1;	/* Supports 1024 bytes block size */
	unsigned	reserved7_3_6	:4;
	unsigned	blk32768	:1;	/* slowb - the device restricts the byte count for PIO */
						/* transfers for slow buffer memory ??? */
						/* Also 32768 block size in some cases */
	__u16		max_speed;		/* Maximum speed supported in KBps */
	__u8		reserved10, reserved11;
	__u16		ctl;			/* Continuous Transfer Limit in blocks */
	__u16		speed;			/* Current Speed, in KBps */
	__u16		buffer_size;		/* Buffer Size, in 512 bytes */
	__u8		reserved18, reserved19;
} idetape_capabilities_page_t;

/*
 *	Block Size Page
 */
typedef struct {
	unsigned	page_code	:6;	/* Page code - Should be 0x30 */
	unsigned	reserved1_6	:1;
	unsigned	ps		:1;
	__u8		page_length;		/* Page Length - Should be 2 */
	__u8		reserved2;
	unsigned	play32		:1;
	unsigned	play32_5	:1;
	unsigned	reserved2_23	:2;
	unsigned	record32	:1;
	unsigned	record32_5	:1;
	unsigned	reserved2_6	:1;
	unsigned	one		:1;
} idetape_block_size_page_t;

/*
 *	A pipeline stage.
 */
typedef struct idetape_stage_s {
	struct request rq;			/* The corresponding request */
	struct buffer_head *bh;			/* The data buffers */
	struct idetape_stage_s *next;		/* Pointer to the next stage */
	os_aux_t *aux;				/* OnStream aux ptr */
} idetape_stage_t;

/*
 *	REQUEST SENSE packet command result - Data Format.
 */
typedef struct {
	unsigned	error_code	:7;	/* Current of deferred errors */
	unsigned	valid		:1;	/* The information field conforms to QIC-157C */
	__u8		reserved1	:8;	/* Segment Number - Reserved */
	unsigned	sense_key	:4;	/* Sense Key */
	unsigned	reserved2_4	:1;	/* Reserved */
	unsigned	ili		:1;	/* Incorrect Length Indicator */
	unsigned	eom		:1;	/* End Of Medium */
	unsigned	filemark 	:1;	/* Filemark */
	__u32		information __attribute__ ((packed));
	__u8		asl;			/* Additional sense length (n-7) */
	__u32		command_specific;	/* Additional command specific information */
	__u8		asc;			/* Additional Sense Code */
	__u8		ascq;			/* Additional Sense Code Qualifier */
	__u8		replaceable_unit_code;	/* Field Replaceable Unit Code */
	unsigned	sk_specific1 	:7;	/* Sense Key Specific */
	unsigned	sksv		:1;	/* Sense Key Specific information is valid */
	__u8		sk_specific2;		/* Sense Key Specific */
	__u8		sk_specific3;		/* Sense Key Specific */
	__u8		pad[2];			/* Padding to 20 bytes */
} idetape_request_sense_result_t;


/*
 *	Most of our global data which we need to save even as we leave the
 *	driver due to an interrupt or a timer event is stored in a variable
 *	of type idetape_tape_t, defined below.
 */
typedef struct {
	ide_drive_t *drive;
	devfs_handle_t de_r, de_n;

	/*
	 *	Since a typical character device operation requires more
	 *	than one packet command, we provide here enough memory
	 *	for the maximum of interconnected packet commands.
	 *	The packet commands are stored in the circular array pc_stack.
	 *	pc_stack_index points to the last used entry, and warps around
	 *	to the start when we get to the last array entry.
	 *
	 *	pc points to the current processed packet command.
	 *
	 *	failed_pc points to the last failed packet command, or contains
	 *	NULL if we do not need to retry any packet command. This is
	 *	required since an additional packet command is needed before the
	 *	retry, to get detailed information on what went wrong.
	 */
	idetape_pc_t *pc;			/* Current packet command */
	idetape_pc_t *failed_pc; 		/* Last failed packet command */
	idetape_pc_t pc_stack[IDETAPE_PC_STACK];/* Packet command stack */
	int pc_stack_index;			/* Next free packet command storage space */
	struct request rq_stack[IDETAPE_PC_STACK];
	int rq_stack_index;			/* We implement a circular array */

	/*
	 *	DSC polling variables.
	 *
	 *	While polling for DSC we use postponed_rq to postpone the
	 *	current request so that ide.c will be able to service
	 *	pending requests on the other device. Note that at most
	 *	we will have only one DSC (usually data transfer) request
	 *	in the device request queue. Additional requests can be
	 *	queued in our internal pipeline, but they will be visible
	 *	to ide.c only one at a time.
	 */
	struct request *postponed_rq;
	unsigned long dsc_polling_start;	/* The time in which we started polling for DSC */
	struct timer_list dsc_timer;		/* Timer used to poll for dsc */
	unsigned long best_dsc_rw_frequency;	/* Read/Write dsc polling frequency */
	unsigned long dsc_polling_frequency;	/* The current polling frequency */
	unsigned long dsc_timeout;		/* Maximum waiting time */

	/*
	 *	Read position information
	 */
	byte partition;
	unsigned int first_frame_position;		/* Current block */
	unsigned int last_frame_position;
	unsigned int blocks_in_buffer;

	/*
	 *	Last error information
	 */
	byte sense_key, asc, ascq;

	/*
	 *	Character device operation
	 */
	unsigned int minor;
	char name[4];					/* device name */
	idetape_chrdev_direction_t chrdev_direction;	/* Current character device data transfer direction */

	/*
	 *	Device information
	 */
	unsigned short tape_block_size;			/* Usually 512 or 1024 bytes */
	int user_bs_factor;
	idetape_capabilities_page_t capabilities;	/* Copy of the tape's Capabilities and Mechanical Page */

	/*
	 *	Active data transfer request parameters.
	 *
	 *	At most, there is only one ide-tape originated data transfer
	 *	request in the device request queue. This allows ide.c to
	 *	easily service requests from the other device when we
	 *	postpone our active request. In the pipelined operation
	 *	mode, we use our internal pipeline structure to hold
	 *	more data requests.
	 *
	 *	The data buffer size is chosen based on the tape's
	 *	recommendation.
	 */
	struct request *active_data_request;	/* Pointer to the request which is waiting in the device request queue */
	int stage_size;				/* Data buffer size (chosen based on the tape's recommendation */
	idetape_stage_t *merge_stage;
	int merge_stage_size;
	struct buffer_head *bh;
	char *b_data;
	int b_count;
	
	/*
	 *	Pipeline parameters.
	 *
	 *	To accomplish non-pipelined mode, we simply set the following
	 *	variables to zero (or NULL, where appropriate).
	 */
	int nr_stages;				/* Number of currently used stages */
	int nr_pending_stages;			/* Number of pending stages */
	int max_stages, min_pipeline, max_pipeline; /* We will not allocate more than this number of stages */
	idetape_stage_t *first_stage;		/* The first stage which will be removed from the pipeline */
	idetape_stage_t *active_stage;		/* The currently active stage */
	idetape_stage_t *next_stage;		/* Will be serviced after the currently active request */
	idetape_stage_t *last_stage;		/* New requests will be added to the pipeline here */
	idetape_stage_t *cache_stage;		/* Optional free stage which we can use */
	int pages_per_stage;
	int excess_bh_size;			/* Wasted space in each stage */

	unsigned long flags;			/* Status/Action flags: long for set_bit */
	spinlock_t spinlock;			/* protects the ide-tape queue */

	/*
	 * Measures average tape speed
	 */
	unsigned long avg_time;
	int avg_size;
	int avg_speed;

	idetape_request_sense_result_t sense;	/* last sense information */

	char vendor_id[10];
	char product_id[18];
	char firmware_revision[6];
	int firmware_revision_num;

	int door_locked;			/* the door is currently locked */

	/*
	 * OnStream flags
	 */
	int onstream;				/* the tape is an OnStream tape */
	int raw;				/* OnStream raw access (32.5KB block size) */
	int cur_frames;				/* current number of frames in internal buffer */
	int max_frames;				/* max number of frames in internal buffer */
	int logical_blk_num;			/* logical block number */
	__u16 wrt_pass_cntr;			/* write pass counter */
	__u32 update_frame_cntr;		/* update frame counter */
	struct completion *waiting;
	int onstream_write_error;		/* write error recovery active */
	int header_ok;				/* header frame verified ok */
	int linux_media;			/* reading linux-specifc media */
	int linux_media_version;
	char application_sig[5];		/* application signature */
	int filemark_cnt;
	int first_mark_addr;
	int last_mark_addr;
	int eod_frame_addr;
	unsigned long cmd_start_time;
	unsigned long max_cmd_time;
	unsigned capacity;

	/*
	 * Optimize the number of "buffer filling"
	 * mode sense commands.
	 */
	unsigned long last_buffer_fill;		/* last time in which we issued fill cmd */
	int req_buffer_fill;			/* buffer fill command requested */
	int writes_since_buffer_fill;
	int reads_since_buffer_fill;

	/*
	 * Limit the number of times a request can
	 * be postponed, to avoid an infinite postpone
	 * deadlock.
	 */
	int postpone_cnt;			/* request postpone count limit */

	/*
	 * Measures number of frames:
	 *
	 * 1. written/read to/from the driver pipeline (pipeline_head).
	 * 2. written/read to/from the tape buffers (buffer_head).
	 * 3. written/read by the tape to/from the media (tape_head).
	 */
	int pipeline_head;
	int buffer_head;
	int tape_head;
	int last_tape_head;

	/*
	 * Speed control at the tape buffers input/output
	 */
	unsigned long insert_time;
	int insert_size;
	int insert_speed;
	int max_insert_speed;
	int measure_insert_time;

	/*
	 * Measure tape still time, in milliseconds
	 */
	unsigned long tape_still_time_begin;
	int tape_still_time;

	/*
	 * Speed regulation negative feedback loop
	 */
	int speed_control;
	int pipeline_head_speed, controlled_pipeline_head_speed, uncontrolled_pipeline_head_speed;
	int controlled_last_pipeline_head, uncontrolled_last_pipeline_head;
	unsigned long uncontrolled_pipeline_head_time, controlled_pipeline_head_time;
	int controlled_previous_pipeline_head, uncontrolled_previous_pipeline_head;
	unsigned long controlled_previous_head_time, uncontrolled_previous_head_time;
	int restart_speed_control_req;

        /*
         * Debug_level determines amount of debugging output;
         * can be changed using /proc/ide/hdx/settings
         * 0 : almost no debugging output
         * 1 : 0+output errors only
         * 2 : 1+output all sensekey/asc
         * 3 : 2+follow all chrdev related procedures
         * 4 : 3+follow all procedures
         * 5 : 4+include pc_stack rq_stack info
         * 6 : 5+USE_COUNT updates
         */
         int debug_level; 
} idetape_tape_t;

/*
 *	Tape door status
 */
#define DOOR_UNLOCKED			0
#define DOOR_LOCKED			1
#define DOOR_EXPLICITLY_LOCKED		2

/*
 *	Tape flag bits values.
 */
#define IDETAPE_IGNORE_DSC		0
#define IDETAPE_ADDRESS_VALID		1	/* 0 When the tape position is unknown */
#define IDETAPE_BUSY			2	/* Device already opened */
#define IDETAPE_PIPELINE_ERROR		3	/* Error detected in a pipeline stage */
#define IDETAPE_DETECT_BS		4	/* Attempt to auto-detect the current user block size */
#define IDETAPE_FILEMARK		5	/* Currently on a filemark */
#define IDETAPE_DRQ_INTERRUPT		6	/* DRQ interrupt device */
#define IDETAPE_READ_ERROR		7
#define IDETAPE_PIPELINE_ACTIVE		8	/* pipeline active */

/*
 *	Supported ATAPI tape drives packet commands
 */
#define IDETAPE_TEST_UNIT_READY_CMD	0x00
#define IDETAPE_REWIND_CMD		0x01
#define IDETAPE_REQUEST_SENSE_CMD	0x03
#define IDETAPE_READ_CMD		0x08
#define IDETAPE_WRITE_CMD		0x0a
#define IDETAPE_WRITE_FILEMARK_CMD	0x10
#define IDETAPE_SPACE_CMD		0x11
#define IDETAPE_INQUIRY_CMD		0x12
#define IDETAPE_ERASE_CMD		0x19
#define IDETAPE_MODE_SENSE_CMD		0x1a
#define IDETAPE_MODE_SELECT_CMD		0x15
#define IDETAPE_LOAD_UNLOAD_CMD		0x1b
#define IDETAPE_PREVENT_CMD		0x1e
#define IDETAPE_LOCATE_CMD		0x2b
#define IDETAPE_READ_POSITION_CMD	0x34
#define IDETAPE_READ_BUFFER_CMD		0x3c
#define IDETAPE_SET_SPEED_CMD		0xbb

/*
 *	Some defines for the READ BUFFER command
 */
#define IDETAPE_RETRIEVE_FAULTY_BLOCK	6

/*
 *	Some defines for the SPACE command
 */
#define IDETAPE_SPACE_OVER_FILEMARK	1
#define IDETAPE_SPACE_TO_EOD		3

/*
 *	Some defines for the LOAD UNLOAD command
 */
#define IDETAPE_LU_LOAD_MASK		1
#define IDETAPE_LU_RETENSION_MASK	2
#define IDETAPE_LU_EOT_MASK		4

/*
 *	Special requests for our block device strategy routine.
 *
 *	In order to service a character device command, we add special
 *	requests to the tail of our block device request queue and wait
 *	for their completion.
 *
 */
#define IDETAPE_FIRST_RQ		90

/*
 * 	IDETAPE_PC_RQ is used to queue a packet command in the request queue.
 */
#define IDETAPE_PC_RQ1			90
#define IDETAPE_PC_RQ2			91

/*
 *	IDETAPE_READ_RQ and IDETAPE_WRITE_RQ are used by our
 *	character device interface to request read/write operations from
 *	our block device interface.
 */
#define IDETAPE_READ_RQ			92
#define IDETAPE_WRITE_RQ		93
#define IDETAPE_ABORTED_WRITE_RQ	94
#define IDETAPE_ABORTED_READ_RQ		95
#define IDETAPE_READ_BUFFER_RQ		96

#define IDETAPE_LAST_RQ			96

/*
 *	A macro which can be used to check if a we support a given
 *	request command.
 */
#define IDETAPE_RQ_CMD(cmd) 		((cmd >= IDETAPE_FIRST_RQ) && (cmd <= IDETAPE_LAST_RQ))

/*
 *	Error codes which are returned in rq->errors to the higher part
 *	of the driver.
 */
#define	IDETAPE_ERROR_GENERAL		101
#define	IDETAPE_ERROR_FILEMARK		102
#define	IDETAPE_ERROR_EOD		103

/*
 *	The ATAPI Status Register.
 */
typedef union {
	unsigned all			:8;
	struct {
		unsigned check		:1;	/* Error occurred */
		unsigned idx		:1;	/* Reserved */
		unsigned corr		:1;	/* Correctable error occurred */
		unsigned drq		:1;	/* Data is request by the device */
		unsigned dsc		:1;	/* Buffer availability / Media access command finished */
		unsigned reserved5	:1;	/* Reserved */
		unsigned drdy		:1;	/* Ignored for ATAPI commands (ready to accept ATA command) */
		unsigned bsy		:1;	/* The device has access to the command block */
	} b;
} idetape_status_reg_t;

/*
 *	The ATAPI error register.
 */
typedef union {
	unsigned all			:8;
	struct {
		unsigned ili		:1;	/* Illegal Length Indication */
		unsigned eom		:1;	/* End Of Media Detected */
		unsigned abrt		:1;	/* Aborted command - As defined by ATA */
		unsigned mcr		:1;	/* Media Change Requested - As defined by ATA */
		unsigned sense_key	:4;	/* Sense key of the last failed packet command */
	} b;
} idetape_error_reg_t;

/*
 *	ATAPI Feature Register
 */
typedef union {
	unsigned all			:8;
	struct {
		unsigned dma		:1;	/* Using DMA or PIO */
		unsigned reserved321	:3;	/* Reserved */
		unsigned reserved654	:3;	/* Reserved (Tag Type) */
		unsigned reserved7	:1;	/* Reserved */
	} b;
} idetape_feature_reg_t;

/*
 *	ATAPI Byte Count Register.
 */
typedef union {
	unsigned all			:16;
	struct {
		unsigned low		:8;	/* LSB */
		unsigned high		:8;	/* MSB */
	} b;
} idetape_bcount_reg_t;

/*
 *	ATAPI Interrupt Reason Register.
 */
typedef union {
	unsigned all			:8;
	struct {
		unsigned cod		:1;	/* Information transferred is command (1) or data (0) */
		unsigned io		:1;	/* The device requests us to read (1) or write (0) */
		unsigned reserved	:6;	/* Reserved */
	} b;
} idetape_ireason_reg_t;

/*
 *	ATAPI Drive Select Register
 */
typedef union {	
	unsigned all			:8;
	struct {
		unsigned sam_lun	:4;	/* Should be zero with ATAPI (not used) */
		unsigned drv		:1;	/* The responding drive will be drive 0 (0) or drive 1 (1) */
		unsigned one5		:1;	/* Should be set to 1 */
		unsigned reserved6	:1;	/* Reserved */
		unsigned one7		:1;	/* Should be set to 1 */
	} b;
} idetape_drivesel_reg_t;

/*
 *	ATAPI Device Control Register
 */
typedef union {			
	unsigned all			:8;
	struct {
		unsigned zero0		:1;	/* Should be set to zero */
		unsigned nien		:1;	/* Device interrupt is disabled (1) or enabled (0) */
		unsigned srst		:1;	/* ATA software reset. ATAPI devices should use the new ATAPI srst. */
		unsigned one3		:1;	/* Should be set to 1 */
		unsigned reserved4567	:4;	/* Reserved */
	} b;
} idetape_control_reg_t;

/*
 *	idetape_chrdev_t provides the link between out character device
 *	interface and our block device interface and the corresponding
 *	ide_drive_t structure.
 */
typedef struct {
	ide_drive_t *drive;
} idetape_chrdev_t;

/*
 *	The following is used to format the general configuration word of
 *	the ATAPI IDENTIFY DEVICE command.
 */
struct idetape_id_gcw {	
	unsigned packet_size		:2;	/* Packet Size */
	unsigned reserved234		:3;	/* Reserved */
	unsigned drq_type		:2;	/* Command packet DRQ type */
	unsigned removable		:1;	/* Removable media */
	unsigned device_type		:5;	/* Device type */
	unsigned reserved13		:1;	/* Reserved */
	unsigned protocol		:2;	/* Protocol type */
};

/*
 *	INQUIRY packet command - Data Format (From Table 6-8 of QIC-157C)
 */
typedef struct {
	unsigned	device_type	:5;	/* Peripheral Device Type */
	unsigned	reserved0_765	:3;	/* Peripheral Qualifier - Reserved */
	unsigned	reserved1_6t0	:7;	/* Reserved */
	unsigned	rmb		:1;	/* Removable Medium Bit */
	unsigned	ansi_version	:3;	/* ANSI Version */
	unsigned	ecma_version	:3;	/* ECMA Version */
	unsigned	iso_version	:2;	/* ISO Version */
	unsigned	response_format :4;	/* Response Data Format */
	unsigned	reserved3_45	:2;	/* Reserved */
	unsigned	reserved3_6	:1;	/* TrmIOP - Reserved */
	unsigned	reserved3_7	:1;	/* AENC - Reserved */
	__u8		additional_length;	/* Additional Length (total_length-4) */
	__u8		rsv5, rsv6, rsv7;	/* Reserved */
	__u8		vendor_id[8];		/* Vendor Identification */
	__u8		product_id[16];		/* Product Identification */
	__u8		revision_level[4];	/* Revision Level */
	__u8		vendor_specific[20];	/* Vendor Specific - Optional */
	__u8		reserved56t95[40];	/* Reserved - Optional */
						/* Additional information may be returned */
} idetape_inquiry_result_t;

/*
 *	READ POSITION packet command - Data Format (From Table 6-57)
 */
typedef struct {
	unsigned	reserved0_10	:2;	/* Reserved */
	unsigned	bpu		:1;	/* Block Position Unknown */	
	unsigned	reserved0_543	:3;	/* Reserved */
	unsigned	eop		:1;	/* End Of Partition */
	unsigned	bop		:1;	/* Beginning Of Partition */
	u8		partition;		/* Partition Number */
	u8		reserved2, reserved3;	/* Reserved */
	u32		first_block;		/* First Block Location */
	u32		last_block;		/* Last Block Location (Optional) */
	u8		reserved12;		/* Reserved */
	u8		blocks_in_buffer[3];	/* Blocks In Buffer - (Optional) */
	u32		bytes_in_buffer;	/* Bytes In Buffer (Optional) */
} idetape_read_position_result_t;

/*
 *	Follows structures which are related to the SELECT SENSE / MODE SENSE
 *	packet commands. Those packet commands are still not supported
 *	by ide-tape.
 */
#define IDETAPE_BLOCK_DESCRIPTOR	0
#define	IDETAPE_CAPABILITIES_PAGE	0x2a
#define IDETAPE_PARAMTR_PAGE		0x2b   /* Onstream DI-x0 only */
#define IDETAPE_BLOCK_SIZE_PAGE		0x30
#define IDETAPE_BUFFER_FILLING_PAGE	0x33

/*
 *	Mode Parameter Header for the MODE SENSE packet command
 */
typedef struct {
	__u8	mode_data_length;	/* Length of the following data transfer */
	__u8	medium_type;		/* Medium Type */
	__u8	dsp;			/* Device Specific Parameter */
	__u8	bdl;			/* Block Descriptor Length */
#if 0
	/* data transfer page */
	__u8	page_code	:6;
	__u8	reserved0_6	:1;
	__u8	ps		:1;	/* parameters saveable */
	__u8	page_length;		/* page Length == 0x02 */
	__u8	reserved2;
	__u8	read32k		:1;	/* 32k blk size (data only) */
	__u8	read32k5	:1;	/* 32.5k blk size (data&AUX) */
	__u8	reserved3_23	:2;
	__u8	write32k	:1;	/* 32k blk size (data only) */
	__u8	write32k5	:1;	/* 32.5k blk size (data&AUX) */
	__u8	reserved3_6	:1;
	__u8	streaming	:1;	/* streaming mode enable */
#endif
} idetape_mode_parameter_header_t;

/*
 *	Mode Parameter Block Descriptor the MODE SENSE packet command
 *
 *	Support for block descriptors is optional.
 */
typedef struct {
	__u8		density_code;		/* Medium density code */
	__u8		blocks[3];		/* Number of blocks */
	__u8		reserved4;		/* Reserved */
	__u8		length[3];		/* Block Length */
} idetape_parameter_block_descriptor_t;

/*
 *	The Data Compression Page, as returned by the MODE SENSE packet command.
 */
typedef struct {
	unsigned	page_code	:6;	/* Page Code - Should be 0xf */
	unsigned	reserved0	:1;	/* Reserved */
	unsigned	ps		:1;
	__u8		page_length;		/* Page Length - Should be 14 */
	unsigned	reserved2	:6;	/* Reserved */
	unsigned	dcc		:1;	/* Data Compression Capable */
	unsigned	dce		:1;	/* Data Compression Enable */
	unsigned	reserved3	:5;	/* Reserved */
	unsigned	red		:2;	/* Report Exception on Decompression */
	unsigned	dde		:1;	/* Data Decompression Enable */
	__u32		ca;			/* Compression Algorithm */
	__u32		da;			/* Decompression Algorithm */
	__u8		reserved[4];		/* Reserved */
} idetape_data_compression_page_t;

/*
 *	The Medium Partition Page, as returned by the MODE SENSE packet command.
 */
typedef struct {
	unsigned	page_code	:6;	/* Page Code - Should be 0x11 */
	unsigned	reserved1_6	:1;	/* Reserved */
	unsigned	ps		:1;
	__u8		page_length;		/* Page Length - Should be 6 */
	__u8		map;			/* Maximum Additional Partitions - Should be 0 */
	__u8		apd;			/* Additional Partitions Defined - Should be 0 */
	unsigned	reserved4_012	:3;	/* Reserved */
	unsigned	psum		:2;	/* Should be 0 */
	unsigned	idp		:1;	/* Should be 0 */
	unsigned	sdp		:1;	/* Should be 0 */
	unsigned	fdp		:1;	/* Fixed Data Partitions */
	__u8		mfr;			/* Medium Format Recognition */
	__u8		reserved[2];		/* Reserved */
} idetape_medium_partition_page_t;

/*
 *	Run time configurable parameters.
 */
typedef struct {
	int	dsc_rw_frequency;
	int	dsc_media_access_frequency;
	int	nr_stages;
} idetape_config_t;

/*
 *	The variables below are used for the character device interface.
 *	Additional state variables are defined in our ide_drive_t structure.
 */
static idetape_chrdev_t idetape_chrdevs[MAX_HWIFS * MAX_DRIVES];
static int idetape_chrdev_present = 0;

#if IDETAPE_DEBUG_LOG_VERBOSE

/*
 * DO NOT REMOVE, BUILDING A VERBOSE DEBUG SCHEME FOR ATAPI
 */

char *idetape_sense_key_verbose (byte idetape_sense_key)
{
	switch (idetape_sense_key) {
		default: {
			char buf[22];
			sprintf(buf, "IDETAPE_SENSE (0x%02x)", idetape_sense_key);
			return(buf);
		}

	}
}

char *idetape_command_key_verbose (byte idetape_command_key)
{
	switch (idetape_command_key) {
		case IDETAPE_TEST_UNIT_READY_CMD:	return("TEST_UNIT_READY_CMD");
		case IDETAPE_REWIND_CMD:		return("REWIND_CMD");
		case IDETAPE_REQUEST_SENSE_CMD:		return("REQUEST_SENSE_CMD");
		case IDETAPE_READ_CMD:			return("READ_CMD");
		case IDETAPE_WRITE_CMD:			return("WRITE_CMD");
		case IDETAPE_WRITE_FILEMARK_CMD:	return("WRITE_FILEMARK_CMD");
		case IDETAPE_SPACE_CMD:			return("SPACE_CMD");
		case IDETAPE_INQUIRY_CMD:		return("INQUIRY_CMD");
		case IDETAPE_ERASE_CMD:			return("ERASE_CMD");
		case IDETAPE_MODE_SENSE_CMD:		return("MODE_SENSE_CMD");
		case IDETAPE_MODE_SELECT_CMD:		return("MODE_SELECT_CMD");
		case IDETAPE_LOAD_UNLOAD_CMD:		return("LOAD_UNLOAD_CMD");
		case IDETAPE_PREVENT_CMD:		return("PREVENT_CMD");
		case IDETAPE_LOCATE_CMD:		return("LOCATE_CMD");
		case IDETAPE_READ_POSITION_CMD:		return("READ_POSITION_CMD");
		case IDETAPE_READ_BUFFER_CMD:		return("READ_BUFFER_CMD");
		case IDETAPE_SET_SPEED_CMD:		return("SET_SPEED_CMD");
		default: {
				char buf[20];
				sprintf(buf, "CMD (0x%02x)", idetape_command_key);
				return(buf);
			}
	}
}
#endif /* IDETAPE_DEBUG_LOG_VERBOSE */

/*
 *      Function declarations
 *
 */
static void idetape_onstream_mode_sense_tape_parameter_page(ide_drive_t *drive, int debug);
static int idetape_chrdev_release (struct inode *inode, struct file *filp);
static void idetape_write_release (struct inode *inode);

/*
 *	Too bad. The drive wants to send us data which we are not ready to accept.
 *	Just throw it away.
 */
static void idetape_discard_data (ide_drive_t *drive, unsigned int bcount)
{
	while (bcount--)
		IN_BYTE (IDE_DATA_REG);
}

static void idetape_input_buffers (ide_drive_t *drive, idetape_pc_t *pc, unsigned int bcount)
{
	struct buffer_head *bh = pc->bh;
	int count;

	while (bcount) {
#if IDETAPE_DEBUG_BUGS
		if (bh == NULL) {
			printk (KERN_ERR "ide-tape: bh == NULL in idetape_input_buffers\n");
			idetape_discard_data (drive, bcount);
			return;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		count = IDE_MIN (bh->b_size - atomic_read(&bh->b_count), bcount);
		atapi_input_bytes (drive, bh->b_data + atomic_read(&bh->b_count), count);
		bcount -= count;
		atomic_add(count, &bh->b_count);
		if (atomic_read(&bh->b_count) == bh->b_size) {
			bh = bh->b_reqnext;
			if (bh)
				atomic_set(&bh->b_count, 0);
		}
	}
	pc->bh = bh;
}

static void idetape_output_buffers (ide_drive_t *drive, idetape_pc_t *pc, unsigned int bcount)
{
	struct buffer_head *bh = pc->bh;
	int count;

	while (bcount) {
#if IDETAPE_DEBUG_BUGS
		if (bh == NULL) {
			printk (KERN_ERR "ide-tape: bh == NULL in idetape_output_buffers\n");
			return;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		count = IDE_MIN (pc->b_count, bcount);
		atapi_output_bytes (drive, pc->b_data, count);
		bcount -= count;
		pc->b_data += count;
		pc->b_count -= count;
		if (!pc->b_count) {
			pc->bh = bh = bh->b_reqnext;
			if (bh) {
				pc->b_data = bh->b_data;
				pc->b_count = atomic_read(&bh->b_count);
			}
		}
	}
}

#ifdef CONFIG_BLK_DEV_IDEDMA
static void idetape_update_buffers (idetape_pc_t *pc)
{
	struct buffer_head *bh = pc->bh;
	int count, bcount = pc->actually_transferred;

	if (test_bit (PC_WRITING, &pc->flags))
		return;
	while (bcount) {
#if IDETAPE_DEBUG_BUGS
		if (bh == NULL) {
			printk (KERN_ERR "ide-tape: bh == NULL in idetape_update_buffers\n");
			return;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		count = IDE_MIN (bh->b_size, bcount);
		atomic_set(&bh->b_count, count);
		if (atomic_read(&bh->b_count) == bh->b_size)
			bh = bh->b_reqnext;
		bcount -= count;
	}
	pc->bh = bh;
}
#endif /* CONFIG_BLK_DEV_IDEDMA */

/*
 *	idetape_next_pc_storage returns a pointer to a place in which we can
 *	safely store a packet command, even though we intend to leave the
 *	driver. A storage space for a maximum of IDETAPE_PC_STACK packet
 *	commands is allocated at initialization time.
 */
static idetape_pc_t *idetape_next_pc_storage (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 5)
		printk (KERN_INFO "ide-tape: pc_stack_index=%d\n",tape->pc_stack_index);
#endif /* IDETAPE_DEBUG_LOG */
	if (tape->pc_stack_index==IDETAPE_PC_STACK)
		tape->pc_stack_index=0;
	return (&tape->pc_stack[tape->pc_stack_index++]);
}

/*
 *	idetape_next_rq_storage is used along with idetape_next_pc_storage.
 *	Since we queue packet commands in the request queue, we need to
 *	allocate a request, along with the allocation of a packet command.
 */
 
/**************************************************************
 *                                                            *
 *  This should get fixed to use kmalloc(.., GFP_ATOMIC)      *
 *  followed later on by kfree().   -ml                       *
 *                                                            *
 **************************************************************/
 
static struct request *idetape_next_rq_storage (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 5)
		printk (KERN_INFO "ide-tape: rq_stack_index=%d\n",tape->rq_stack_index);
#endif /* IDETAPE_DEBUG_LOG */
	if (tape->rq_stack_index==IDETAPE_PC_STACK)
		tape->rq_stack_index=0;
	return (&tape->rq_stack[tape->rq_stack_index++]);
}

/*
 *	idetape_init_pc initializes a packet command.
 */
static void idetape_init_pc (idetape_pc_t *pc)
{
	memset (pc->c, 0, 12);
	pc->retries = 0;
	pc->flags = 0;
	pc->request_transfer = 0;
	pc->buffer = pc->pc_buffer;
	pc->buffer_size = IDETAPE_PC_BUFFER_SIZE;
	pc->bh = NULL;
	pc->b_data = NULL;
}

/*
 *	idetape_analyze_error is called on each failed packet command retry
 *	to analyze the request sense. We currently do not utilize this
 *	information.
 */
static void idetape_analyze_error (ide_drive_t *drive, idetape_request_sense_result_t *result)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t *pc = tape->failed_pc;

	tape->sense     = *result;
	tape->sense_key = result->sense_key;
	tape->asc       = result->asc;
	tape->ascq      = result->ascq;
#if IDETAPE_DEBUG_LOG
	/*
	 *	Without debugging, we only log an error if we decided to
	 *	give up retrying.
	 */
	if (tape->debug_level >= 1)
		printk (KERN_INFO "ide-tape: pc = %x, sense key = %x, asc = %x, ascq = %x\n",
				pc->c[0], result->sense_key, result->asc, result->ascq);
#if IDETAPE_DEBUG_LOG_VERBOSE
	if (tape->debug_level >= 1)
		printk (KERN_INFO "ide-tape: pc = %s, sense key = %x, asc = %x, ascq = %x\n",
			idetape_command_key_verbose((byte) pc->c[0]),
			result->sense_key,
			result->asc,
			result->ascq);
#endif /* IDETAPE_DEBUG_LOG_VERBOSE */
#endif /* IDETAPE_DEBUG_LOG */

	if (tape->onstream && result->sense_key == 2 && result->asc == 0x53 && result->ascq == 2) {
		clear_bit(PC_DMA_ERROR, &pc->flags);
		ide_stall_queue(drive, HZ / 2);
		return;
	}
#ifdef CONFIG_BLK_DEV_IDEDMA

	/*
	 *	Correct pc->actually_transferred by asking the tape.
	 */
	if (test_bit (PC_DMA_ERROR, &pc->flags)) {
		pc->actually_transferred = pc->request_transfer - tape->tape_block_size * ntohl (get_unaligned (&result->information));
		idetape_update_buffers (pc);
	}
#endif /* CONFIG_BLK_DEV_IDEDMA */
	if (pc->c[0] == IDETAPE_READ_CMD && result->filemark) {
		pc->error = IDETAPE_ERROR_FILEMARK;
		set_bit (PC_ABORT, &pc->flags);
	}
	if (pc->c[0] == IDETAPE_WRITE_CMD) {
		if (result->eom || (result->sense_key == 0xd && result->asc == 0x0 && result->ascq == 0x2)) {
			pc->error = IDETAPE_ERROR_EOD;
			set_bit (PC_ABORT, &pc->flags);
		}
	}
	if (pc->c[0] == IDETAPE_READ_CMD || pc->c[0] == IDETAPE_WRITE_CMD) {
		if (result->sense_key == 8) {
			pc->error = IDETAPE_ERROR_EOD;
			set_bit (PC_ABORT, &pc->flags);
		}
		if (!test_bit (PC_ABORT, &pc->flags) && (tape->onstream || pc->actually_transferred))
			pc->retries = IDETAPE_MAX_PC_RETRIES + 1;
	}
}

static void idetape_abort_pipeline (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage = tape->next_stage;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk(KERN_INFO "ide-tape: %s: idetape_abort_pipeline called\n", tape->name);
#endif
	while (stage) {
		if (stage->rq.cmd == IDETAPE_WRITE_RQ)
			stage->rq.cmd = IDETAPE_ABORTED_WRITE_RQ;
		else if (stage->rq.cmd == IDETAPE_READ_RQ)
			stage->rq.cmd = IDETAPE_ABORTED_READ_RQ;
		stage = stage->next;
	}
}

/*
 *	idetape_active_next_stage will declare the next stage as "active".
 */
static void idetape_active_next_stage (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage = tape->next_stage;
	struct request *rq = &stage->rq;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_active_next_stage\n");
#endif /* IDETAPE_DEBUG_LOG */
#if IDETAPE_DEBUG_BUGS
	if (stage == NULL) {
		printk (KERN_ERR "ide-tape: bug: Trying to activate a non existing stage\n");
		return;
	}
#endif /* IDETAPE_DEBUG_BUGS */	

	rq->buffer = NULL;
	rq->bh = stage->bh;
	tape->active_data_request = rq;
	tape->active_stage = stage;
	tape->next_stage = stage->next;
}

/*
 *	idetape_increase_max_pipeline_stages is a part of the feedback
 *	loop which tries to find the optimum number of stages. In the
 *	feedback loop, we are starting from a minimum maximum number of
 *	stages, and if we sense that the pipeline is empty, we try to
 *	increase it, until we reach the user compile time memory limit.
 */
static void idetape_increase_max_pipeline_stages (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int increase = (tape->max_pipeline - tape->min_pipeline) / 10;
	
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_increase_max_pipeline_stages\n");
#endif /* IDETAPE_DEBUG_LOG */

	tape->max_stages += increase;
	tape->max_stages = IDE_MAX(tape->max_stages, tape->min_pipeline);
	tape->max_stages = IDE_MIN(tape->max_stages, tape->max_pipeline);
}

/*
 *	idetape_kfree_stage calls kfree to completely free a stage, along with
 *	its related buffers.
 */
static void __idetape_kfree_stage (idetape_stage_t *stage)
{
	struct buffer_head *prev_bh, *bh = stage->bh;
	int size;

	while (bh != NULL) {
		if (bh->b_data != NULL) {
			size = (int) bh->b_size;
			while (size > 0) {
				free_page ((unsigned long) bh->b_data);
				size -= PAGE_SIZE;
				bh->b_data += PAGE_SIZE;
			}
		}
		prev_bh = bh;
		bh = bh->b_reqnext;
		kfree (prev_bh);
	}
	kfree (stage);
}

static void idetape_kfree_stage (idetape_tape_t *tape, idetape_stage_t *stage)
{
	__idetape_kfree_stage (stage);
}

/*
 *	idetape_remove_stage_head removes tape->first_stage from the pipeline.
 *	The caller should avoid race conditions.
 */
static void idetape_remove_stage_head (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_remove_stage_head\n");
#endif /* IDETAPE_DEBUG_LOG */
#if IDETAPE_DEBUG_BUGS
	if (tape->first_stage == NULL) {
		printk (KERN_ERR "ide-tape: bug: tape->first_stage is NULL\n");
		return;		
	}
	if (tape->active_stage == tape->first_stage) {
		printk (KERN_ERR "ide-tape: bug: Trying to free our active pipeline stage\n");
		return;
	}
#endif /* IDETAPE_DEBUG_BUGS */
	stage = tape->first_stage;
	tape->first_stage = stage->next;
	idetape_kfree_stage (tape, stage);
	tape->nr_stages--;
	if (tape->first_stage == NULL) {
		tape->last_stage = NULL;
#if IDETAPE_DEBUG_BUGS
		if (tape->next_stage != NULL)
			printk (KERN_ERR "ide-tape: bug: tape->next_stage != NULL\n");
		if (tape->nr_stages)
			printk (KERN_ERR "ide-tape: bug: nr_stages should be 0 now\n");
#endif /* IDETAPE_DEBUG_BUGS */
	}
}

/*
 *	idetape_end_request is used to finish servicing a request, and to
 *	insert a pending pipeline request into the main device queue.
 */
static void idetape_end_request (byte uptodate, ide_hwgroup_t *hwgroup)
{
	ide_drive_t *drive = hwgroup->drive;
	struct request *rq = hwgroup->rq;
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;
	int error;
	int remove_stage = 0;
#if ONSTREAM_DEBUG
	idetape_stage_t *stage;
	os_aux_t *aux;
	unsigned char *p;
#endif

#if IDETAPE_DEBUG_LOG
        if (tape->debug_level >= 4)
	printk (KERN_INFO "ide-tape: Reached idetape_end_request\n");
#endif /* IDETAPE_DEBUG_LOG */

	switch (uptodate) {
		case 0:	error = IDETAPE_ERROR_GENERAL; break;
		case 1: error = 0; break;
		default: error = uptodate;
	}
	rq->errors = error;
	if (error)
		tape->failed_pc = NULL;

	spin_lock_irqsave(&tape->spinlock, flags);
	if (tape->active_data_request == rq) {		/* The request was a pipelined data transfer request */
		tape->active_stage = NULL;
		tape->active_data_request = NULL;
		tape->nr_pending_stages--;
		if (rq->cmd == IDETAPE_WRITE_RQ) {
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 2) {
				if (tape->onstream) {
					stage = tape->first_stage;
					aux = stage->aux;
					p = stage->bh->b_data;
					if (ntohl(aux->logical_blk_num) < 11300 && ntohl(aux->logical_blk_num) > 11100)
						printk(KERN_INFO "ide-tape: finished writing logical blk %u (data %x %x %x %x)\n", ntohl(aux->logical_blk_num), *p++, *p++, *p++, *p++);
				}
			}
#endif
			if (tape->onstream && !tape->raw) {
				if (tape->first_frame_position == OS_DATA_ENDFRAME1) { 
#if ONSTREAM_DEBUG
					if (tape->debug_level >= 2)
						printk("ide-tape: %s: skipping over config parition..\n", tape->name);
#endif
					tape->onstream_write_error = OS_PART_ERROR;
					if (tape->waiting)
						complete(tape->waiting);
				}
			}
			remove_stage = 1;
			if (error) {
				set_bit (IDETAPE_PIPELINE_ERROR, &tape->flags);
				if (error == IDETAPE_ERROR_EOD)
					idetape_abort_pipeline (drive);
				if (tape->onstream && !tape->raw && error == IDETAPE_ERROR_GENERAL && tape->sense.sense_key == 3) {
					clear_bit (IDETAPE_PIPELINE_ERROR, &tape->flags);
					printk(KERN_ERR "ide-tape: %s: write error, enabling error recovery\n", tape->name);
					tape->onstream_write_error = OS_WRITE_ERROR;
					remove_stage = 0;
					tape->nr_pending_stages++;
					tape->next_stage = tape->first_stage;
					rq->current_nr_sectors = rq->nr_sectors;
					if (tape->waiting)
						complete(tape->waiting);
				}
			}
		} else if (rq->cmd == IDETAPE_READ_RQ) {
			if (error == IDETAPE_ERROR_EOD) {
				set_bit (IDETAPE_PIPELINE_ERROR, &tape->flags);
				idetape_abort_pipeline(drive);
			}
		}
		if (tape->next_stage != NULL && !tape->onstream_write_error) {
			idetape_active_next_stage (drive);

			/*
			 *	Insert the next request into the request queue.
			 */
			(void) ide_do_drive_cmd (drive, tape->active_data_request, ide_end);
		} else if (!error) {
			if (!tape->onstream)
				idetape_increase_max_pipeline_stages (drive);
		}
	}
	ide_end_drive_cmd (drive, 0, 0);
	if (remove_stage)
		idetape_remove_stage_head (drive);
	if (tape->active_data_request == NULL)
		clear_bit(IDETAPE_PIPELINE_ACTIVE, &tape->flags);
	spin_unlock_irqrestore(&tape->spinlock, flags);
}

static ide_startstop_t idetape_request_sense_callback (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_request_sense_callback\n");
#endif /* IDETAPE_DEBUG_LOG */
	if (!tape->pc->error) {
		idetape_analyze_error (drive, (idetape_request_sense_result_t *) tape->pc->buffer);
		idetape_end_request (1, HWGROUP (drive));
	} else {
		printk (KERN_ERR "ide-tape: Error in REQUEST SENSE itself - Aborting request!\n");
		idetape_end_request (0, HWGROUP (drive));
	}
	return ide_stopped;
}

static void idetape_create_request_sense_cmd (idetape_pc_t *pc)
{
	idetape_init_pc (pc);	
	pc->c[0] = IDETAPE_REQUEST_SENSE_CMD;
	pc->c[4] = 20;
	pc->request_transfer = 18;
	pc->callback = &idetape_request_sense_callback;
}

/*
 *	idetape_queue_pc_head generates a new packet command request in front
 *	of the request queue, before the current request, so that it will be
 *	processed immediately, on the next pass through the driver.
 *
 *	idetape_queue_pc_head is called from the request handling part of
 *	the driver (the "bottom" part). Safe storage for the request should
 *	be allocated with idetape_next_pc_storage and idetape_next_rq_storage
 *	before calling idetape_queue_pc_head.
 *
 *	Memory for those requests is pre-allocated at initialization time, and
 *	is limited to IDETAPE_PC_STACK requests. We assume that we have enough
 *	space for the maximum possible number of inter-dependent packet commands.
 *
 *	The higher level of the driver - The ioctl handler and the character
 *	device handling functions should queue request to the lower level part
 *	and wait for their completion using idetape_queue_pc_tail or
 *	idetape_queue_rw_tail.
 */
static void idetape_queue_pc_head (ide_drive_t *drive,idetape_pc_t *pc,struct request *rq)
{
	ide_init_drive_cmd (rq);
	rq->buffer = (char *) pc;
	rq->cmd = IDETAPE_PC_RQ1;
	(void) ide_do_drive_cmd (drive, rq, ide_preempt);
}

/*
 *	idetape_retry_pc is called when an error was detected during the
 *	last packet command. We queue a request sense packet command in
 *	the head of the request list.
 */
static ide_startstop_t idetape_retry_pc (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t *pc;
	struct request *rq;
	idetape_error_reg_t error;

	error.all = IN_BYTE (IDE_ERROR_REG);
	pc = idetape_next_pc_storage (drive);
	rq = idetape_next_rq_storage (drive);
	idetape_create_request_sense_cmd (pc);
	set_bit (IDETAPE_IGNORE_DSC, &tape->flags);
	idetape_queue_pc_head (drive, pc, rq);
	return ide_stopped;
}

/*
 *	idetape_postpone_request postpones the current request so that
 *	ide.c will be able to service requests from another device on
 *	the same hwgroup while we are polling for DSC.
 */
static void idetape_postpone_request (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk(KERN_INFO "ide-tape: idetape_postpone_request\n");
#endif
	tape->postponed_rq = HWGROUP(drive)->rq;
	ide_stall_queue(drive, tape->dsc_polling_frequency);
}

/*
 *	idetape_pc_intr is the usual interrupt handler which will be called
 *	during a packet command. We will transfer some of the data (as
 *	requested by the drive) and will re-point interrupt handler to us.
 *	When data transfer is finished, we will act according to the
 *	algorithm described before idetape_issue_packet_command.
 *
 */
static ide_startstop_t idetape_pc_intr (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_status_reg_t status;
	idetape_bcount_reg_t bcount;
	idetape_ireason_reg_t ireason;
	idetape_pc_t *pc = tape->pc;

	unsigned int temp;
	unsigned long cmd_time;
#if SIMULATE_ERRORS
	static int error_sim_count = 0;
#endif

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_pc_intr interrupt handler\n");
#endif /* IDETAPE_DEBUG_LOG */	

	status.all = GET_STAT();					/* Clear the interrupt */

#ifdef CONFIG_BLK_DEV_IDEDMA
	if (test_bit (PC_DMA_IN_PROGRESS, &pc->flags)) {
		if (HWIF(drive)->dmaproc(ide_dma_end, drive)) {
			/*
			 * A DMA error is sometimes expected. For example,
			 * if the tape is crossing a filemark during a
			 * READ command, it will issue an irq and position
			 * itself before the filemark, so that only a partial
			 * data transfer will occur (which causes the DMA
			 * error). In that case, we will later ask the tape
			 * how much bytes of the original request were
			 * actually transferred (we can't receive that
			 * information from the DMA engine on most chipsets).
			 */
			set_bit (PC_DMA_ERROR, &pc->flags);
		} else if (!status.b.check) {
			pc->actually_transferred = pc->request_transfer;
			idetape_update_buffers (pc);
		}
#if IDETAPE_DEBUG_LOG
		if (tape->debug_level >= 4)
			printk (KERN_INFO "ide-tape: DMA finished\n");
#endif /* IDETAPE_DEBUG_LOG */
	}
#endif /* CONFIG_BLK_DEV_IDEDMA */

	if (!status.b.drq) {						/* No more interrupts */
		cmd_time = (jiffies - tape->cmd_start_time) * 1000 / HZ;
		tape->max_cmd_time = IDE_MAX(cmd_time, tape->max_cmd_time);
#if IDETAPE_DEBUG_LOG
		if (tape->debug_level >= 2)
			printk (KERN_INFO "ide-tape: Packet command completed, %d bytes transferred\n", pc->actually_transferred);
#endif /* IDETAPE_DEBUG_LOG */
		clear_bit (PC_DMA_IN_PROGRESS, &pc->flags);

		ide__sti();	/* local CPU only */

#if SIMULATE_ERRORS
		if ((pc->c[0] == IDETAPE_WRITE_CMD || pc->c[0] == IDETAPE_READ_CMD) && (++error_sim_count % 100) == 0) {
			printk(KERN_INFO "ide-tape: %s: simulating error\n", tape->name);
			status.b.check = 1;
		}
#endif
		if (status.b.check && pc->c[0] == IDETAPE_REQUEST_SENSE_CMD)
			status.b.check = 0;
		if (status.b.check || test_bit (PC_DMA_ERROR, &pc->flags)) {	/* Error detected */
#if IDETAPE_DEBUG_LOG
			if (tape->debug_level >= 1)
				printk (KERN_INFO "ide-tape: %s: I/O error, ",tape->name);
#endif /* IDETAPE_DEBUG_LOG */
			if (pc->c[0] == IDETAPE_REQUEST_SENSE_CMD) {
				printk (KERN_ERR "ide-tape: I/O error in request sense command\n");
				return ide_do_reset (drive);
			}
#if IDETAPE_DEBUG_LOG
			if (tape->debug_level >= 1)
				printk(KERN_INFO "ide-tape: [cmd %x]: check condition\n", pc->c[0]);
#endif
			return idetape_retry_pc (drive);				/* Retry operation */
		}
		pc->error = 0;
		if (!tape->onstream && test_bit (PC_WAIT_FOR_DSC, &pc->flags) && !status.b.dsc) {	/* Media access command */
			tape->dsc_polling_start = jiffies;
			tape->dsc_polling_frequency = IDETAPE_DSC_MA_FAST;
			tape->dsc_timeout = jiffies + IDETAPE_DSC_MA_TIMEOUT;
			idetape_postpone_request (drive);		/* Allow ide.c to handle other requests */
			return ide_stopped;
		}
		if (tape->failed_pc == pc)
			tape->failed_pc = NULL;
		return pc->callback(drive);			/* Command finished - Call the callback function */
	}
#ifdef CONFIG_BLK_DEV_IDEDMA
	if (test_and_clear_bit (PC_DMA_IN_PROGRESS, &pc->flags)) {
		printk (KERN_ERR "ide-tape: The tape wants to issue more interrupts in DMA mode\n");
		printk (KERN_ERR "ide-tape: DMA disabled, reverting to PIO\n");
		(void) HWIF(drive)->dmaproc(ide_dma_off, drive);
		return ide_do_reset (drive);
	}
#endif /* CONFIG_BLK_DEV_IDEDMA */
	bcount.b.high = IN_BYTE (IDE_BCOUNTH_REG);			/* Get the number of bytes to transfer */
	bcount.b.low  = IN_BYTE (IDE_BCOUNTL_REG);			/* on this interrupt */
	ireason.all   = IN_BYTE (IDE_IREASON_REG);

	if (ireason.b.cod) {
		printk (KERN_ERR "ide-tape: CoD != 0 in idetape_pc_intr\n");
		return ide_do_reset (drive);
	}
	if (ireason.b.io == test_bit (PC_WRITING, &pc->flags)) {	/* Hopefully, we will never get here */
		printk (KERN_ERR "ide-tape: We wanted to %s, ", ireason.b.io ? "Write":"Read");
		printk (KERN_ERR "ide-tape: but the tape wants us to %s !\n",ireason.b.io ? "Read":"Write");
		return ide_do_reset (drive);
	}
	if (!test_bit (PC_WRITING, &pc->flags)) {			/* Reading - Check that we have enough space */
		temp = pc->actually_transferred + bcount.all;
		if ( temp > pc->request_transfer) {
			if (temp > pc->buffer_size) {
				printk (KERN_ERR "ide-tape: The tape wants to send us more data than expected - discarding data\n");
				idetape_discard_data (drive, bcount.all);
				ide_set_handler (drive, &idetape_pc_intr, IDETAPE_WAIT_CMD, NULL);
				return ide_started;
			}
#if IDETAPE_DEBUG_LOG
			if (tape->debug_level >= 2)
				printk (KERN_NOTICE "ide-tape: The tape wants to send us more data than expected - allowing transfer\n");
#endif /* IDETAPE_DEBUG_LOG */
		}
	}
	if (test_bit (PC_WRITING, &pc->flags)) {
		if (pc->bh != NULL)
			idetape_output_buffers (drive, pc, bcount.all);
		else
			atapi_output_bytes (drive,pc->current_position,bcount.all);	/* Write the current buffer */
	} else {
		if (pc->bh != NULL)
			idetape_input_buffers (drive, pc, bcount.all);
		else
			atapi_input_bytes (drive,pc->current_position,bcount.all);	/* Read the current buffer */
	}
	pc->actually_transferred += bcount.all;					/* Update the current position */
	pc->current_position+=bcount.all;
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: [cmd %x] transferred %d bytes on that interrupt\n", pc->c[0], bcount.all);
#endif
	ide_set_handler (drive, &idetape_pc_intr, IDETAPE_WAIT_CMD, NULL);	/* And set the interrupt handler again */
	return ide_started;
}

/*
 *	Packet Command Interface
 *
 *	The current Packet Command is available in tape->pc, and will not
 *	change until we finish handling it. Each packet command is associated
 *	with a callback function that will be called when the command is
 *	finished.
 *
 *	The handling will be done in three stages:
 *
 *	1.	idetape_issue_packet_command will send the packet command to the
 *		drive, and will set the interrupt handler to idetape_pc_intr.
 *
 *	2.	On each interrupt, idetape_pc_intr will be called. This step
 *		will be repeated until the device signals us that no more
 *		interrupts will be issued.
 *
 *	3.	ATAPI Tape media access commands have immediate status with a
 *		delayed process. In case of a successful initiation of a
 *		media access packet command, the DSC bit will be set when the
 *		actual execution of the command is finished. 
 *		Since the tape drive will not issue an interrupt, we have to
 *		poll for this event. In this case, we define the request as
 *		"low priority request" by setting rq_status to
 *		IDETAPE_RQ_POSTPONED, 	set a timer to poll for DSC and exit
 *		the driver.
 *
 *		ide.c will then give higher priority to requests which
 *		originate from the other device, until will change rq_status
 *		to RQ_ACTIVE.
 *
 *	4.	When the packet command is finished, it will be checked for errors.
 *
 *	5.	In case an error was found, we queue a request sense packet command
 *		in front of the request queue and retry the operation up to
 *		IDETAPE_MAX_PC_RETRIES times.
 *
 *	6.	In case no error was found, or we decided to give up and not
 *		to retry again, the callback function will be called and then
 *		we will handle the next request.
 *
 */
static ide_startstop_t idetape_transfer_pc(ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t *pc = tape->pc;
	idetape_ireason_reg_t ireason;
	int retries = 100;
	ide_startstop_t startstop;

	if (ide_wait_stat (&startstop,drive,DRQ_STAT,BUSY_STAT,WAIT_READY)) {
		printk (KERN_ERR "ide-tape: Strange, packet command initiated yet DRQ isn't asserted\n");
		return startstop;
	}
	ireason.all = IN_BYTE (IDE_IREASON_REG);
	while (retries-- && (!ireason.b.cod || ireason.b.io)) {
		printk(KERN_ERR "ide-tape: (IO,CoD != (0,1) while issuing a packet command, retrying\n");
		udelay(100);
		ireason.all = IN_BYTE(IDE_IREASON_REG);
		if (retries == 0) {
			printk(KERN_ERR "ide-tape: (IO,CoD != (0,1) while issuing a packet command, ignoring\n");
			ireason.b.cod = 1;
			ireason.b.io = 0;
		}
	}
	if (!ireason.b.cod || ireason.b.io) {
		printk (KERN_ERR "ide-tape: (IO,CoD) != (0,1) while issuing a packet command\n");
		return ide_do_reset (drive);
	}
	tape->cmd_start_time = jiffies;
	ide_set_handler(drive, &idetape_pc_intr, IDETAPE_WAIT_CMD, NULL);	/* Set the interrupt routine */
	atapi_output_bytes (drive,pc->c,12);			/* Send the actual packet */
	return ide_started;
}

static ide_startstop_t idetape_issue_packet_command (ide_drive_t *drive, idetape_pc_t *pc)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_bcount_reg_t bcount;
	int dma_ok = 0;

#if IDETAPE_DEBUG_BUGS
	if (tape->pc->c[0] == IDETAPE_REQUEST_SENSE_CMD && pc->c[0] == IDETAPE_REQUEST_SENSE_CMD) {
		printk (KERN_ERR "ide-tape: possible ide-tape.c bug - Two request sense in serial were issued\n");
	}
#endif /* IDETAPE_DEBUG_BUGS */

	if (tape->failed_pc == NULL && pc->c[0] != IDETAPE_REQUEST_SENSE_CMD)
		tape->failed_pc = pc;
	tape->pc = pc;							/* Set the current packet command */

	if (pc->retries > IDETAPE_MAX_PC_RETRIES || test_bit (PC_ABORT, &pc->flags)) {
		/*
		 *	We will "abort" retrying a packet command in case
		 *	a legitimate error code was received (crossing a
		 *	filemark, or DMA error in the end of media, for
		 *	example).
		 */
		if (!test_bit (PC_ABORT, &pc->flags)) {
			if (!(pc->c[0] == IDETAPE_TEST_UNIT_READY_CMD && tape->sense_key == 2 &&
			      tape->asc == 4 && (tape->ascq == 1 || tape->ascq == 8))) {
				printk (KERN_ERR "ide-tape: %s: I/O error, pc = %2x, key = %2x, asc = %2x, ascq = %2x\n",
					tape->name, pc->c[0], tape->sense_key, tape->asc, tape->ascq);
				if (tape->onstream && pc->c[0] == IDETAPE_READ_CMD && tape->sense_key == 3 && tape->asc == 0x11)  /* AJN-1: 11 should be 0x11 */
					printk(KERN_ERR "ide-tape: %s: enabling read error recovery\n", tape->name);
			}
			pc->error = IDETAPE_ERROR_GENERAL;		/* Giving up */
		}
		tape->failed_pc = NULL;
		return pc->callback(drive);
	}
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 2)
		printk (KERN_INFO "ide-tape: Retry number - %d\n", pc->retries);
#endif /* IDETAPE_DEBUG_LOG */

	pc->retries++;
	pc->actually_transferred = 0;					/* We haven't transferred any data yet */
	pc->current_position=pc->buffer;
	bcount.all=pc->request_transfer;				/* Request to transfer the entire buffer at once */

#ifdef CONFIG_BLK_DEV_IDEDMA
	if (test_and_clear_bit (PC_DMA_ERROR, &pc->flags)) {
		printk (KERN_WARNING "ide-tape: DMA disabled, reverting to PIO\n");
		(void) HWIF(drive)->dmaproc(ide_dma_off, drive);
	}
	if (test_bit (PC_DMA_RECOMMENDED, &pc->flags) && drive->using_dma)
		dma_ok = !HWIF(drive)->dmaproc(test_bit (PC_WRITING, &pc->flags) ? ide_dma_write : ide_dma_read, drive);
#endif /* CONFIG_BLK_DEV_IDEDMA */

	if (IDE_CONTROL_REG)
		OUT_BYTE (drive->ctl, IDE_CONTROL_REG);
	OUT_BYTE (dma_ok ? 1 : 0,    IDE_FEATURE_REG);			/* Use PIO/DMA */
	OUT_BYTE (bcount.b.high,     IDE_BCOUNTH_REG);
	OUT_BYTE (bcount.b.low,      IDE_BCOUNTL_REG);
	OUT_BYTE (drive->select.all, IDE_SELECT_REG);
#ifdef CONFIG_BLK_DEV_IDEDMA
	if (dma_ok) {						/* Begin DMA, if necessary */
		set_bit (PC_DMA_IN_PROGRESS, &pc->flags);
		(void) (HWIF(drive)->dmaproc(ide_dma_begin, drive));
	}
#endif /* CONFIG_BLK_DEV_IDEDMA */
	if (test_bit(IDETAPE_DRQ_INTERRUPT, &tape->flags)) {
		ide_set_handler(drive, &idetape_transfer_pc, IDETAPE_WAIT_CMD, NULL);
		OUT_BYTE(WIN_PACKETCMD, IDE_COMMAND_REG);
		return ide_started;
	} else {
		OUT_BYTE(WIN_PACKETCMD, IDE_COMMAND_REG);
		return idetape_transfer_pc(drive);
	}
}

/*
 *	General packet command callback function.
 */
static ide_startstop_t idetape_pc_callback (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_pc_callback\n");
#endif /* IDETAPE_DEBUG_LOG */

	idetape_end_request (tape->pc->error ? 0 : 1, HWGROUP(drive));
	return ide_stopped;
}

/*
 *	A mode sense command is used to "sense" tape parameters.
 */
static void idetape_create_mode_sense_cmd (idetape_pc_t *pc, byte page_code)
{
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_MODE_SENSE_CMD;
	if (page_code != IDETAPE_BLOCK_DESCRIPTOR)
		pc->c[1] = 8;			/* DBD = 1 - Don't return block descriptors */
	pc->c[2] = page_code;
	pc->c[3] = 255;				/* Don't limit the returned information */
	pc->c[4] = 255;				/* (We will just discard data in that case) */
	if (page_code == IDETAPE_BLOCK_DESCRIPTOR)
		pc->request_transfer = 12;
	else if (page_code == IDETAPE_CAPABILITIES_PAGE)
		pc->request_transfer = 24;
	else
		pc->request_transfer = 50;
	pc->callback = &idetape_pc_callback;
}

static ide_startstop_t idetape_onstream_buffer_fill_callback (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

	tape->max_frames = tape->pc->buffer[4 + 2];
	tape->cur_frames = tape->pc->buffer[4 + 3];
	if (tape->chrdev_direction == idetape_direction_write)
		tape->tape_head = tape->buffer_head - tape->cur_frames;
	else
		tape->tape_head = tape->buffer_head + tape->cur_frames;
	if (tape->tape_head != tape->last_tape_head) {
		tape->last_tape_head = tape->tape_head;
		tape->tape_still_time_begin = jiffies;
		if (tape->tape_still_time > 200)
			tape->measure_insert_time = 1;
	}
	tape->tape_still_time = (jiffies - tape->tape_still_time_begin) * 1000 / HZ;
#if USE_IOTRACE
	IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 1)
		printk(KERN_INFO "ide-tape: buffer fill callback, %d/%d\n", tape->cur_frames, tape->max_frames);
#endif
	idetape_end_request (tape->pc->error ? 0 : 1, HWGROUP(drive));
	return ide_stopped;
}

static void idetape_queue_onstream_buffer_fill (ide_drive_t *drive)
{
	idetape_pc_t *pc;
	struct request *rq;

	pc = idetape_next_pc_storage (drive);
	rq = idetape_next_rq_storage (drive);
	idetape_create_mode_sense_cmd (pc, IDETAPE_BUFFER_FILLING_PAGE);
	pc->callback = idetape_onstream_buffer_fill_callback;
	idetape_queue_pc_head (drive, pc, rq);
}

static void calculate_speeds(ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int full = 125, empty = 75;

	if (jiffies > tape->controlled_pipeline_head_time + 120 * HZ) {
		tape->controlled_previous_pipeline_head = tape->controlled_last_pipeline_head;
		tape->controlled_previous_head_time = tape->controlled_pipeline_head_time;
		tape->controlled_last_pipeline_head = tape->pipeline_head;
		tape->controlled_pipeline_head_time = jiffies;
	}
	if (jiffies > tape->controlled_pipeline_head_time + 60 * HZ)
		tape->controlled_pipeline_head_speed = (tape->pipeline_head - tape->controlled_last_pipeline_head) * 32 * HZ / (jiffies - tape->controlled_pipeline_head_time);
	else if (jiffies > tape->controlled_previous_head_time)
		tape->controlled_pipeline_head_speed = (tape->pipeline_head - tape->controlled_previous_pipeline_head) * 32 * HZ / (jiffies - tape->controlled_previous_head_time);

	if (tape->nr_pending_stages < tape->max_stages /*- 1 */) { /* -1 for read mode error recovery */
		if (jiffies > tape->uncontrolled_previous_head_time + 10 * HZ) {
			tape->uncontrolled_pipeline_head_time = jiffies;
			tape->uncontrolled_pipeline_head_speed = (tape->pipeline_head - tape->uncontrolled_previous_pipeline_head) * 32 * HZ / (jiffies - tape->uncontrolled_previous_head_time);
		}
	} else {
		tape->uncontrolled_previous_head_time = jiffies;
		tape->uncontrolled_previous_pipeline_head = tape->pipeline_head;
		if (jiffies > tape->uncontrolled_pipeline_head_time + 30 * HZ) {
			tape->uncontrolled_pipeline_head_time = jiffies;
		}
	}
	tape->pipeline_head_speed = IDE_MAX(tape->uncontrolled_pipeline_head_speed, tape->controlled_pipeline_head_speed);
	if (tape->speed_control == 0) {
		tape->max_insert_speed = 5000;
	} else if (tape->speed_control == 1) {
		if (tape->nr_pending_stages >= tape->max_stages / 2)
			tape->max_insert_speed = tape->pipeline_head_speed +
				(1100 - tape->pipeline_head_speed) * 2 * (tape->nr_pending_stages - tape->max_stages / 2) / tape->max_stages;
		else
			tape->max_insert_speed = 500 +
				(tape->pipeline_head_speed - 500) * 2 * tape->nr_pending_stages / tape->max_stages;
		if (tape->nr_pending_stages >= tape->max_stages * 99 / 100)
			tape->max_insert_speed = 5000;
	} else if (tape->speed_control == 2) {
		tape->max_insert_speed = tape->pipeline_head_speed * empty / 100 +
			(tape->pipeline_head_speed * full / 100 - tape->pipeline_head_speed * empty / 100) * tape->nr_pending_stages / tape->max_stages;
	} else
		tape->max_insert_speed = tape->speed_control;
	tape->max_insert_speed = IDE_MAX(tape->max_insert_speed, 500);
}

static ide_startstop_t idetape_media_access_finished (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t *pc = tape->pc;
	idetape_status_reg_t status;

	if (tape->onstream)
		printk(KERN_INFO "ide-tape: bug: onstream, media_access_finished\n");
	status.all = GET_STAT();
	if (status.b.dsc) {
		if (status.b.check) {					/* Error detected */
			printk (KERN_ERR "ide-tape: %s: I/O error, ",tape->name);
			return idetape_retry_pc (drive);			/* Retry operation */
		}
		pc->error = 0;
		if (tape->failed_pc == pc)
			tape->failed_pc = NULL;
	} else {
		pc->error = IDETAPE_ERROR_GENERAL;
		tape->failed_pc = NULL;
	}
	return pc->callback (drive);
}

static ide_startstop_t idetape_rw_callback (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	struct request *rq = HWGROUP(drive)->rq;
	int blocks = tape->pc->actually_transferred / tape->tape_block_size;

	tape->avg_size += blocks * tape->tape_block_size;
	tape->insert_size += blocks * tape->tape_block_size;
	if (tape->insert_size > 1024 * 1024)
		tape->measure_insert_time = 1;
	if (tape->measure_insert_time) {
		tape->measure_insert_time = 0;
		tape->insert_time = jiffies;
		tape->insert_size = 0;
	}
	if (jiffies > tape->insert_time)
		tape->insert_speed = tape->insert_size / 1024 * HZ / (jiffies - tape->insert_time);
	if (jiffies - tape->avg_time >= HZ) {
		tape->avg_speed = tape->avg_size * HZ / (jiffies - tape->avg_time) / 1024;
		tape->avg_size = 0;
		tape->avg_time = jiffies;
	}

#if IDETAPE_DEBUG_LOG	
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_rw_callback\n");
#endif /* IDETAPE_DEBUG_LOG */

	tape->first_frame_position += blocks;
	rq->current_nr_sectors -= blocks;

	if (!tape->pc->error)
		idetape_end_request (1, HWGROUP (drive));
	else
		idetape_end_request (tape->pc->error, HWGROUP (drive));
	return ide_stopped;
}

static void idetape_create_read_cmd (idetape_tape_t *tape, idetape_pc_t *pc, unsigned int length, struct buffer_head *bh)
{
	struct buffer_head *p = bh;
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_READ_CMD;
	put_unaligned (htonl (length), (unsigned int *) &pc->c[1]);
	pc->c[1] = 1;
	pc->callback = &idetape_rw_callback;
	pc->bh = bh;
	atomic_set(&bh->b_count, 0);
	pc->buffer = NULL;
	if (tape->onstream) {
		while (p) {
			atomic_set(&p->b_count, 0);
			p = p->b_reqnext;
		}
	}
	if (!tape->onstream) {
		pc->request_transfer = pc->buffer_size = length * tape->tape_block_size;
		if (pc->request_transfer == tape->stage_size)
			set_bit (PC_DMA_RECOMMENDED, &pc->flags);
	} else  {
		if (length) {
			pc->request_transfer = pc->buffer_size = 32768 + 512;
			set_bit (PC_DMA_RECOMMENDED, &pc->flags);
		} else
			pc->request_transfer = 0;
	}
}

static void idetape_create_read_buffer_cmd(idetape_tape_t *tape, idetape_pc_t *pc, unsigned int length, struct buffer_head *bh)
{
	int size = 32768;

	struct buffer_head *p = bh;
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_READ_BUFFER_CMD;
	pc->c[1] = IDETAPE_RETRIEVE_FAULTY_BLOCK;
	pc->c[7] = size >> 8;
	pc->c[8] = size & 0xff;
	pc->callback = &idetape_pc_callback;
	pc->bh = bh;
	atomic_set(&bh->b_count, 0);
	pc->buffer = NULL;
	while (p) {
		atomic_set(&p->b_count, 0);
		p = p->b_reqnext;
	}
	pc->request_transfer = pc->buffer_size = size;
}

static void idetape_create_write_cmd (idetape_tape_t *tape, idetape_pc_t *pc, unsigned int length, struct buffer_head *bh)
{
	struct buffer_head *p = bh;
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_WRITE_CMD;
	put_unaligned (htonl (length), (unsigned int *) &pc->c[1]);
	pc->c[1] = 1;
	pc->callback = &idetape_rw_callback;
	set_bit (PC_WRITING, &pc->flags);
	if (tape->onstream) {
		while (p) {
			atomic_set(&p->b_count, p->b_size);
			p = p->b_reqnext;
		}
	}
	pc->bh = bh;
	pc->b_data = bh->b_data;
	pc->b_count = atomic_read(&bh->b_count);
	pc->buffer = NULL;
	if (!tape->onstream) {
		pc->request_transfer = pc->buffer_size = length * tape->tape_block_size;
		if (pc->request_transfer == tape->stage_size)
			set_bit (PC_DMA_RECOMMENDED, &pc->flags);
	} else  {
		if (length) {
			pc->request_transfer = pc->buffer_size = 32768 + 512;
			set_bit (PC_DMA_RECOMMENDED, &pc->flags);
		} else
			pc->request_transfer = 0;
	}
}

/*
 *	idetape_do_request is our request handling function.	
 */
static ide_startstop_t idetape_do_request (ide_drive_t *drive, struct request *rq, unsigned long block)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t *pc;
	struct request *postponed_rq = tape->postponed_rq;
	idetape_status_reg_t status;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 5)
		printk (KERN_INFO "ide-tape: rq_status: %d, rq_dev: %u, cmd: %d, errors: %d\n",rq->rq_status,(unsigned int) rq->rq_dev,rq->cmd,rq->errors);
	if (tape->debug_level >= 2)
		printk (KERN_INFO "ide-tape: sector: %ld, nr_sectors: %ld, current_nr_sectors: %ld\n",rq->sector,rq->nr_sectors,rq->current_nr_sectors);
#endif /* IDETAPE_DEBUG_LOG */

	if (!IDETAPE_RQ_CMD (rq->cmd)) {
		/*
		 *	We do not support buffer cache originated requests.
		 */
		printk (KERN_NOTICE "ide-tape: %s: Unsupported command in request queue (%d)\n", drive->name, rq->cmd);
		ide_end_request (0, HWGROUP (drive));			/* Let the common code handle it */
		return ide_stopped;
	}

	/*
	 *	Retry a failed packet command
	 */
	if (tape->failed_pc != NULL && tape->pc->c[0] == IDETAPE_REQUEST_SENSE_CMD) {
		return idetape_issue_packet_command (drive, tape->failed_pc);
	}
#if IDETAPE_DEBUG_BUGS
	if (postponed_rq != NULL)
		if (rq != postponed_rq) {
			printk (KERN_ERR "ide-tape: ide-tape.c bug - Two DSC requests were queued\n");
			idetape_end_request (0, HWGROUP (drive));
			return ide_stopped;
		}
#endif /* IDETAPE_DEBUG_BUGS */

	tape->postponed_rq = NULL;

	/*
	 *	If the tape is still busy, postpone our request and service
	 *	the other device meanwhile.
	 */
	status.all = GET_STAT();

	/*
	 * The OnStream tape drive doesn't support DSC. Assume
	 * that DSC is always set.
	 */
	if (tape->onstream)
		status.b.dsc = 1;
	if (!drive->dsc_overlap && rq->cmd != IDETAPE_PC_RQ2)
		set_bit (IDETAPE_IGNORE_DSC, &tape->flags);

	/*
	 * For the OnStream tape, check the current status of the tape
	 * internal buffer using data gathered from the buffer fill
	 * mode page, and postpone our request, effectively "disconnecting"
	 * from the IDE bus, in case the buffer is full (writing) or
	 * empty (reading), and there is a danger that our request will
	 * hold the IDE bus during actual media access.
	 */
	if (tape->tape_still_time > 100 && tape->tape_still_time < 200)
		tape->measure_insert_time = 1;
	if (tape->req_buffer_fill && (rq->cmd == IDETAPE_WRITE_RQ || rq->cmd == IDETAPE_READ_RQ)) {
		tape->req_buffer_fill = 0;
		tape->writes_since_buffer_fill = 0;
		tape->reads_since_buffer_fill = 0;
		tape->last_buffer_fill = jiffies;
		idetape_queue_onstream_buffer_fill(drive);
		if (jiffies > tape->insert_time)
			tape->insert_speed = tape->insert_size / 1024 * HZ / (jiffies - tape->insert_time);
		return ide_stopped;
	}
	if (jiffies > tape->insert_time)
		tape->insert_speed = tape->insert_size / 1024 * HZ / (jiffies - tape->insert_time);
	calculate_speeds(drive);
	if (tape->onstream && tape->max_frames &&
	    ((rq->cmd == IDETAPE_WRITE_RQ &&
              ( tape->cur_frames == tape->max_frames ||
                ( tape->speed_control && tape->cur_frames > 5 &&
                       (tape->insert_speed > tape->max_insert_speed ||
                        (0 /* tape->cur_frames > 30 && tape->tape_still_time > 200 */) ) ) ) ) ||
	     (rq->cmd == IDETAPE_READ_RQ &&
	      ( tape->cur_frames == 0 ||
		( tape->speed_control && (tape->cur_frames < tape->max_frames - 5) &&
			tape->insert_speed > tape->max_insert_speed ) ) && rq->nr_sectors) ) ) {
#if IDETAPE_DEBUG_LOG
		if (tape->debug_level >= 4)
			printk(KERN_INFO "ide-tape: postponing request, cmd %d, cur %d, max %d\n",
				rq->cmd, tape->cur_frames, tape->max_frames);
#endif
		if (tape->postpone_cnt++ < 500) {
			status.b.dsc = 0;
			tape->req_buffer_fill = 1;
		}
#if ONSTREAM_DEBUG
		else if (tape->debug_level >= 4) 
			printk(KERN_INFO "ide-tape: %s: postpone_cnt %d\n", tape->name, tape->postpone_cnt);
#endif
	}
	if (!test_and_clear_bit (IDETAPE_IGNORE_DSC, &tape->flags) && !status.b.dsc) {
		if (postponed_rq == NULL) {
			tape->dsc_polling_start = jiffies;
			tape->dsc_polling_frequency = tape->best_dsc_rw_frequency;
			tape->dsc_timeout = jiffies + IDETAPE_DSC_RW_TIMEOUT;
		} else if ((signed long) (jiffies - tape->dsc_timeout) > 0) {
			printk (KERN_ERR "ide-tape: %s: DSC timeout\n", tape->name);
			if (rq->cmd == IDETAPE_PC_RQ2) {
				idetape_media_access_finished (drive);
				return ide_stopped;
			} else {
				return ide_do_reset (drive);
			}
		} else if (jiffies - tape->dsc_polling_start > IDETAPE_DSC_MA_THRESHOLD)
			tape->dsc_polling_frequency = IDETAPE_DSC_MA_SLOW;
		idetape_postpone_request (drive);
		return ide_stopped;
	}
	switch (rq->cmd) {
		case IDETAPE_READ_RQ:
			tape->buffer_head++;
#if USE_IOTRACE
			IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
			tape->postpone_cnt = 0;
			tape->reads_since_buffer_fill++;
			if (tape->onstream) {
				if (tape->cur_frames - tape->reads_since_buffer_fill <= 0)
					tape->req_buffer_fill = 1;
				if (jiffies > tape->last_buffer_fill + 5 * HZ / 100)
					tape->req_buffer_fill = 1;
			}
			pc = idetape_next_pc_storage (drive);
			idetape_create_read_cmd (tape, pc, rq->current_nr_sectors, rq->bh);
			break;
		case IDETAPE_WRITE_RQ:
			tape->buffer_head++;
#if USE_IOTRACE
			IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
			tape->postpone_cnt = 0;
			tape->writes_since_buffer_fill++;
			if (tape->onstream) {
				if (tape->cur_frames + tape->writes_since_buffer_fill >= tape->max_frames)
					tape->req_buffer_fill = 1;
				if (jiffies > tape->last_buffer_fill + 5 * HZ / 100)
					tape->req_buffer_fill = 1;
				calculate_speeds(drive);
			}
			pc = idetape_next_pc_storage (drive);
			idetape_create_write_cmd (tape, pc, rq->current_nr_sectors, rq->bh);
			break;
		case IDETAPE_READ_BUFFER_RQ:
			tape->postpone_cnt = 0;
			pc = idetape_next_pc_storage (drive);
			idetape_create_read_buffer_cmd (tape, pc, rq->current_nr_sectors, rq->bh);
			break;
		case IDETAPE_ABORTED_WRITE_RQ:
			rq->cmd = IDETAPE_WRITE_RQ;
			idetape_end_request (IDETAPE_ERROR_EOD, HWGROUP(drive));
			return ide_stopped;
		case IDETAPE_ABORTED_READ_RQ:
#if IDETAPE_DEBUG_LOG
			if (tape->debug_level >= 2)
				printk(KERN_INFO "ide-tape: %s: detected aborted read rq\n", tape->name);
#endif
			rq->cmd = IDETAPE_READ_RQ;
			idetape_end_request (IDETAPE_ERROR_EOD, HWGROUP(drive));
			return ide_stopped;
		case IDETAPE_PC_RQ1:
			pc = (idetape_pc_t *) rq->buffer;
			rq->cmd = IDETAPE_PC_RQ2;
			break;
		case IDETAPE_PC_RQ2:
			idetape_media_access_finished (drive);
			return ide_stopped;
		default:
			printk (KERN_ERR "ide-tape: bug in IDETAPE_RQ_CMD macro\n");
			idetape_end_request (0, HWGROUP (drive));
			return ide_stopped;
	}
	return idetape_issue_packet_command (drive, pc);
}

/*
 *	Pipeline related functions
 */
static inline int idetape_pipeline_active (idetape_tape_t *tape)
{
	int rc1, rc2;

	rc1 = test_bit(IDETAPE_PIPELINE_ACTIVE, &tape->flags);
	rc2 = (tape->active_data_request != NULL);
	return rc1;
}

/*
 *	idetape_kmalloc_stage uses __get_free_page to allocate a pipeline
 *	stage, along with all the necessary small buffers which together make
 *	a buffer of size tape->stage_size (or a bit more). We attempt to
 *	combine sequential pages as much as possible.
 *
 *	Returns a pointer to the new allocated stage, or NULL if we
 *	can't (or don't want to) allocate a stage.
 *
 *	Pipeline stages are optional and are used to increase performance.
 *	If we can't allocate them, we'll manage without them.
 */
static idetape_stage_t *__idetape_kmalloc_stage (idetape_tape_t *tape, int full, int clear)
{
	idetape_stage_t *stage;
	struct buffer_head *prev_bh, *bh;
	int pages = tape->pages_per_stage;
	char *b_data;

	if ((stage = (idetape_stage_t *) kmalloc (sizeof (idetape_stage_t),GFP_KERNEL)) == NULL)
		return NULL;
	stage->next = NULL;

	bh = stage->bh = (struct buffer_head *) kmalloc (sizeof (struct buffer_head), GFP_KERNEL);
	if (bh == NULL)
		goto abort;
	bh->b_reqnext = NULL;
	if ((bh->b_data = (char *) __get_free_page (GFP_KERNEL)) == NULL)
		goto abort;
	if (clear)
		memset(bh->b_data, 0, PAGE_SIZE);
	bh->b_size = PAGE_SIZE;
	atomic_set(&bh->b_count, full ? bh->b_size : 0);
	set_bit (BH_Lock, &bh->b_state);

	while (--pages) {
		if ((b_data = (char *) __get_free_page (GFP_KERNEL)) == NULL)
			goto abort;
		if (clear)
			memset(b_data, 0, PAGE_SIZE);
		if (bh->b_data == b_data + PAGE_SIZE) {
			bh->b_size += PAGE_SIZE;
			bh->b_data -= PAGE_SIZE;
			if (full)
				atomic_add(PAGE_SIZE, &bh->b_count);
			continue;
		}
		if (b_data == bh->b_data + bh->b_size) {
			bh->b_size += PAGE_SIZE;
			if (full)
				atomic_add(PAGE_SIZE, &bh->b_count);
			continue;
		}
		prev_bh = bh;
		if ((bh = (struct buffer_head *) kmalloc (sizeof (struct buffer_head), GFP_KERNEL)) == NULL) {
			free_page ((unsigned long) b_data);
			goto abort;
		}
		bh->b_reqnext = NULL;
		bh->b_data = b_data;
		bh->b_size = PAGE_SIZE;
		atomic_set(&bh->b_count, full ? bh->b_size : 0);
		set_bit (BH_Lock, &bh->b_state);
		prev_bh->b_reqnext = bh;
	}
	bh->b_size -= tape->excess_bh_size;
	if (full)
		atomic_sub(tape->excess_bh_size, &bh->b_count);
	if (tape->onstream)
		stage->aux = (os_aux_t *) (bh->b_data + bh->b_size - OS_AUX_SIZE);
	return stage;
abort:
	__idetape_kfree_stage (stage);
	return NULL;
}

static idetape_stage_t *idetape_kmalloc_stage (idetape_tape_t *tape)
{
	idetape_stage_t *cache_stage = tape->cache_stage;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_kmalloc_stage\n");
#endif /* IDETAPE_DEBUG_LOG */

	if (tape->nr_stages >= tape->max_stages)
		return NULL;
	if (cache_stage != NULL) {
		tape->cache_stage = NULL;
		return cache_stage;
	}
	return __idetape_kmalloc_stage (tape, 0, 0);
}

static void idetape_copy_stage_from_user (idetape_tape_t *tape, idetape_stage_t *stage, const char *buf, int n)
{
	struct buffer_head *bh = tape->bh;
	int count;

	while (n) {
#if IDETAPE_DEBUG_BUGS
		if (bh == NULL) {
			printk (KERN_ERR "ide-tape: bh == NULL in idetape_copy_stage_from_user\n");
			return;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		count = IDE_MIN (bh->b_size - atomic_read(&bh->b_count), n);
		copy_from_user (bh->b_data + atomic_read(&bh->b_count), buf, count);
		n -= count;
		atomic_add(count, &bh->b_count);
		buf += count;
		if (atomic_read(&bh->b_count) == bh->b_size) {
			bh = bh->b_reqnext;
			if (bh)
				atomic_set(&bh->b_count, 0);
		}
	}
	tape->bh = bh;
}

static void idetape_copy_stage_to_user (idetape_tape_t *tape, char *buf, idetape_stage_t *stage, int n)
{
	struct buffer_head *bh = tape->bh;
	int count;

	while (n) {
#if IDETAPE_DEBUG_BUGS
		if (bh == NULL) {
			printk (KERN_ERR "ide-tape: bh == NULL in idetape_copy_stage_to_user\n");
			return;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		count = IDE_MIN (tape->b_count, n);
		copy_to_user (buf, tape->b_data, count);
		n -= count;
		tape->b_data += count;
		tape->b_count -= count;
		buf += count;
		if (!tape->b_count) {
			tape->bh = bh = bh->b_reqnext;
			if (bh) {
				tape->b_data = bh->b_data;
				tape->b_count = atomic_read(&bh->b_count);
			}
		}
	}
}

static void idetape_init_merge_stage (idetape_tape_t *tape)
{
	struct buffer_head *bh = tape->merge_stage->bh;
	
	tape->bh = bh;
	if (tape->chrdev_direction == idetape_direction_write)
		atomic_set(&bh->b_count, 0);
	else {
		tape->b_data = bh->b_data;
		tape->b_count = atomic_read(&bh->b_count);
	}
}

static void idetape_switch_buffers (idetape_tape_t *tape, idetape_stage_t *stage)
{
	struct buffer_head *tmp;
	os_aux_t *tmp_aux;

	tmp = stage->bh; tmp_aux = stage->aux;
	stage->bh = tape->merge_stage->bh; stage->aux = tape->merge_stage->aux;
	tape->merge_stage->bh = tmp; tape->merge_stage->aux = tmp_aux;
	idetape_init_merge_stage (tape);
}

/*
 *	idetape_add_stage_tail adds a new stage at the end of the pipeline.
 */
static void idetape_add_stage_tail (ide_drive_t *drive,idetape_stage_t *stage)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;
	
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_add_stage_tail\n");
#endif /* IDETAPE_DEBUG_LOG */
	spin_lock_irqsave(&tape->spinlock, flags);
	stage->next=NULL;
	if (tape->last_stage != NULL)
		tape->last_stage->next=stage;
	else
		tape->first_stage = tape->next_stage=stage;
	tape->last_stage = stage;
	if (tape->next_stage == NULL)
		tape->next_stage = tape->last_stage;
	tape->nr_stages++;
	tape->nr_pending_stages++;
	spin_unlock_irqrestore(&tape->spinlock, flags);
}

/*
 * Initialize the OnStream AUX
 */
static void idetape_init_stage (ide_drive_t *drive, idetape_stage_t *stage, int frame_type, int logical_blk_num)
{
	idetape_tape_t *tape = drive->driver_data;
	os_aux_t *aux = stage->aux;
	os_partition_t *par = &aux->partition;
	os_dat_t *dat = &aux->dat;

	if (!tape->onstream || tape->raw)
		return;
	memset(aux, 0, sizeof(*aux));
	aux->format_id = htonl(0);
	memcpy(aux->application_sig, "LIN3", 4);
	aux->hdwr = htonl(0);
	aux->frame_type = frame_type;

	if (frame_type == OS_FRAME_TYPE_HEADER) {
		aux->update_frame_cntr = htonl(tape->update_frame_cntr);
		par->partition_num = OS_CONFIG_PARTITION;
		par->par_desc_ver = OS_PARTITION_VERSION;
		par->wrt_pass_cntr = htons(0xffff);
		par->first_frame_addr = htonl(0);
		par->last_frame_addr = htonl(0xbb7); /* 2999 */
		aux->frame_seq_num = htonl(0);
		aux->logical_blk_num_high = htonl(0);
		aux->logical_blk_num = htonl(0);
		aux->next_mark_addr = htonl(tape->first_mark_addr);
	} else {
		aux->update_frame_cntr = htonl(0);
		par->partition_num = OS_DATA_PARTITION;
		par->par_desc_ver = OS_PARTITION_VERSION;
		par->wrt_pass_cntr = htons(tape->wrt_pass_cntr);
		par->first_frame_addr = htonl(OS_DATA_STARTFRAME1);
		par->last_frame_addr = htonl(tape->capacity);
		aux->frame_seq_num = htonl(logical_blk_num);
		aux->logical_blk_num_high = htonl(0);
		aux->logical_blk_num = htonl(logical_blk_num);
		dat->dat_sz = 8;
		dat->reserved1 = 0;
		dat->entry_cnt = 1;
		dat->reserved3 = 0;
		if (frame_type == OS_FRAME_TYPE_DATA)
			dat->dat_list[0].blk_sz = htonl(32 * 1024);
		else
			dat->dat_list[0].blk_sz = 0;
		dat->dat_list[0].blk_cnt = htons(1);
		if (frame_type == OS_FRAME_TYPE_MARKER)
			dat->dat_list[0].flags = OS_DAT_FLAGS_MARK;
		else
			dat->dat_list[0].flags = OS_DAT_FLAGS_DATA;
		dat->dat_list[0].reserved = 0;
	} 
	aux->filemark_cnt = ntohl(tape->filemark_cnt);		/* shouldn't this be htonl ?? */
	aux->phys_fm = ntohl(0xffffffff);			/* shouldn't this be htonl ?? */
	aux->last_mark_addr = ntohl(tape->last_mark_addr);	/* shouldn't this be htonl ?? */
}

/*
 *	idetape_wait_for_request installs a completion in a pending request
 *	and sleeps until it is serviced.
 *
 *	The caller should ensure that the request will not be serviced
 *	before we install the completion (usually by disabling interrupts).
 */
static void idetape_wait_for_request (ide_drive_t *drive, struct request *rq)
{
	DECLARE_COMPLETION(wait);
	idetape_tape_t *tape = drive->driver_data;

#if IDETAPE_DEBUG_BUGS
	if (rq == NULL || !IDETAPE_RQ_CMD (rq->cmd)) {
		printk (KERN_ERR "ide-tape: bug: Trying to sleep on non-valid request\n");
		return;
	}
#endif /* IDETAPE_DEBUG_BUGS */
	rq->waiting = &wait;
	tape->waiting = &wait;
	spin_unlock(&tape->spinlock);
	wait_for_completion(&wait);
	rq->waiting = NULL;
	tape->waiting = NULL;
	spin_lock_irq(&tape->spinlock);
}

static ide_startstop_t idetape_read_position_callback (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_read_position_result_t *result;
	
//#if IDETAPE_DEBUG_LOG
//	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_read_position_callback\n");
//#endif /* IDETAPE_DEBUG_LOG */

	if (!tape->pc->error) {
		result = (idetape_read_position_result_t *) tape->pc->buffer;
#if IDETAPE_DEBUG_LOG
		if (tape->debug_level >= 2)
			printk (KERN_INFO "ide-tape: BOP - %s\n",result->bop ? "Yes":"No");
		if (tape->debug_level >= 2)
			printk (KERN_INFO "ide-tape: EOP - %s\n",result->eop ? "Yes":"No");
#endif /* IDETAPE_DEBUG_LOG */
		if (result->bpu) {
			printk (KERN_INFO "ide-tape: Block location is unknown to the tape\n");
			clear_bit (IDETAPE_ADDRESS_VALID, &tape->flags);
			idetape_end_request (0, HWGROUP (drive));
		} else {
#if IDETAPE_DEBUG_LOG
			if (tape->debug_level >= 2)
				printk (KERN_INFO "ide-tape: Block Location - %u\n", ntohl (result->first_block));
#endif /* IDETAPE_DEBUG_LOG */
			tape->partition = result->partition;
			tape->first_frame_position = ntohl (result->first_block);
			tape->last_frame_position = ntohl (result->last_block);
			tape->blocks_in_buffer = result->blocks_in_buffer[2];
			set_bit (IDETAPE_ADDRESS_VALID, &tape->flags);
			idetape_end_request (1, HWGROUP (drive));
		}
	} else {
		idetape_end_request (0, HWGROUP (drive));
	}
	return ide_stopped;
}

/*
 *	idetape_create_write_filemark_cmd will:
 *
 *		1.	Write a filemark if write_filemark=1.
 *		2.	Flush the device buffers without writing a filemark
 *			if write_filemark=0.
 *
 */
static void idetape_create_write_filemark_cmd (ide_drive_t *drive, idetape_pc_t *pc,int write_filemark)
{
	idetape_tape_t *tape = drive->driver_data;

	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_WRITE_FILEMARK_CMD;
	if (tape->onstream)
		pc->c[1] = 1; /* Immed bit */
	pc->c[4] = write_filemark;  /* not used for OnStream ?? */
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

static void idetape_create_test_unit_ready_cmd(idetape_pc_t *pc)
{
	idetape_init_pc(pc);
	pc->c[0] = IDETAPE_TEST_UNIT_READY_CMD;
	pc->callback = &idetape_pc_callback;
}

/*
 *	idetape_queue_pc_tail is based on the following functions:
 *
 *	ide_do_drive_cmd from ide.c
 *	cdrom_queue_request and cdrom_queue_packet_command from ide-cd.c
 *
 *	We add a special packet command request to the tail of the request queue,
 *	and wait for it to be serviced.
 *
 *	This is not to be called from within the request handling part
 *	of the driver ! We allocate here data in the stack, and it is valid
 *	until the request is finished. This is not the case for the bottom
 *	part of the driver, where we are always leaving the functions to wait
 *	for an interrupt or a timer event.
 *
 *	From the bottom part of the driver, we should allocate safe memory
 *	using idetape_next_pc_storage and idetape_next_rq_storage, and add
 *	the request to the request list without waiting for it to be serviced !
 *	In that case, we usually use idetape_queue_pc_head.
 */
static int __idetape_queue_pc_tail (ide_drive_t *drive, idetape_pc_t *pc)
{
	struct request rq;

	ide_init_drive_cmd (&rq);
	rq.buffer = (char *) pc;
	rq.cmd = IDETAPE_PC_RQ1;
	return ide_do_drive_cmd (drive, &rq, ide_wait);
}

static void idetape_create_load_unload_cmd (ide_drive_t *drive, idetape_pc_t *pc,int cmd)
{
	idetape_tape_t *tape = drive->driver_data;

	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_LOAD_UNLOAD_CMD;
	pc->c[4] = cmd;
	if (tape->onstream) {
		pc->c[1] = 1;
		if (cmd == !IDETAPE_LU_LOAD_MASK)
			pc->c[4] = 4;
	}
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

static int idetape_wait_ready (ide_drive_t *drive, unsigned long long timeout)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;

	/*
	 * Wait for the tape to become ready
	 */
	timeout += jiffies;
	while (jiffies < timeout) {
		idetape_create_test_unit_ready_cmd(&pc);
		if (!__idetape_queue_pc_tail(drive, &pc))
			return 0;
		if (tape->sense_key == 2 && tape->asc == 4 && tape->ascq == 2) {
			idetape_create_load_unload_cmd (drive, &pc, IDETAPE_LU_LOAD_MASK);
			__idetape_queue_pc_tail(drive, &pc);
			idetape_create_test_unit_ready_cmd(&pc);
			if (!__idetape_queue_pc_tail(drive, &pc))
				return 0;
		}
		if (!(tape->sense_key == 2 && tape->asc == 4 && (tape->ascq == 1 || tape->ascq == 8)))
			break;
		current->state = TASK_INTERRUPTIBLE;
  		schedule_timeout(HZ / 10);
	}
	return -EIO;
}

static int idetape_queue_pc_tail (ide_drive_t *drive,idetape_pc_t *pc)
{
	idetape_tape_t *tape = drive->driver_data;
	int rc;

	rc = __idetape_queue_pc_tail(drive, pc);
	if (rc)
		return rc;
	if (tape->onstream && test_bit(PC_WAIT_FOR_DSC, &pc->flags))
		rc = idetape_wait_ready(drive, 60 * 10 * HZ);   /* AJN-4: Changed from 5 to 10 minutes;
                          because retension takes approx. 8:20 with Onstream 30GB tape */
	return rc;
}

static int idetape_flush_tape_buffers (ide_drive_t *drive)
{
	idetape_pc_t pc;
	int rc;

	idetape_create_write_filemark_cmd(drive, &pc, 0);
	if ((rc = idetape_queue_pc_tail (drive, &pc)))
		return rc;
	idetape_wait_ready(drive, 60 * 5 * HZ);
	return 0;
}

static void idetape_create_read_position_cmd (idetape_pc_t *pc)
{
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_READ_POSITION_CMD;
	pc->request_transfer = 20;
	pc->callback = &idetape_read_position_callback;
}

static int idetape_read_position (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	int position;

//#if IDETAPE_DEBUG_LOG
//        if (tape->debug_level >= 4)
	printk (KERN_INFO "ide-tape: Reached idetape_read_position\n");
//#endif /* IDETAPE_DEBUG_LOG */

#ifdef NO_LONGER_REQUIRED
	idetape_flush_tape_buffers(drive);
#endif
	idetape_create_read_position_cmd(&pc);
	if (idetape_queue_pc_tail (drive, &pc))
		return -1;
	position = tape->first_frame_position;
#ifdef NO_LONGER_REQUIRED
	if (tape->onstream) {
		if ((position != tape->last_frame_position - tape->blocks_in_buffer) &&
		    (position != tape->last_frame_position + tape->blocks_in_buffer)) {
			if (tape->blocks_in_buffer == 0) {
				printk("ide-tape: %s: correcting read position %d, %d, %d\n", tape->name, position, tape->last_frame_position, tape->blocks_in_buffer);
				position = tape->last_frame_position;
				tape->first_frame_position = position;
			}
		}
	}
#endif
	return position;
}

static void idetape_create_locate_cmd (ide_drive_t *drive, idetape_pc_t *pc, unsigned int block, byte partition, int skip)
{
	idetape_tape_t *tape = drive->driver_data;

	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_LOCATE_CMD;
	if (tape->onstream)
		pc->c[1] = 1; /* Immediate bit */
	else
		pc->c[1] = 2;
	put_unaligned (htonl (block), (unsigned int *) &pc->c[3]);
	pc->c[8] = partition;
	if (tape->onstream)
                /*
                 * Set SKIP bit.
                 * In case of write error this will write buffered
                 * data in the drive to this new position!
                 */
		pc->c[9] = skip << 7;
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

static int idetape_create_prevent_cmd (ide_drive_t *drive, idetape_pc_t *pc, int prevent)
{
	idetape_tape_t *tape = drive->driver_data;

	if (!tape->capabilities.lock)
		return 0;

	idetape_init_pc(pc);
	pc->c[0] = IDETAPE_PREVENT_CMD;
	pc->c[4] = prevent;
	pc->callback = &idetape_pc_callback;
	return 1;
}

static int __idetape_discard_read_pipeline (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;
	int cnt;

	if (tape->chrdev_direction != idetape_direction_read)
		return 0;
	tape->merge_stage_size = 0;
	if (tape->merge_stage != NULL) {
		__idetape_kfree_stage (tape->merge_stage);
		tape->merge_stage = NULL;
	}
	tape->chrdev_direction = idetape_direction_none;
	
	if (tape->first_stage == NULL)
		return 0;

	spin_lock_irqsave(&tape->spinlock, flags);
	tape->next_stage = NULL;
	if (idetape_pipeline_active (tape))
		idetape_wait_for_request(drive, tape->active_data_request);
	spin_unlock_irqrestore(&tape->spinlock, flags);

	cnt = tape->nr_stages - tape->nr_pending_stages;
	while (tape->first_stage != NULL)
		idetape_remove_stage_head (drive);
	tape->nr_pending_stages = 0;
	tape->max_stages = tape->min_pipeline;
	return cnt;
}

/*
 *	idetape_position_tape positions the tape to the requested block
 *	using the LOCATE packet command. A READ POSITION command is then
 *	issued to check where we are positioned.
 *
 *	Like all higher level operations, we queue the commands at the tail
 *	of the request queue and wait for their completion.
 *	
 */
static int idetape_position_tape (ide_drive_t *drive, unsigned int block, byte partition, int skip)
{
	idetape_tape_t *tape = drive->driver_data;
	int retval;
	idetape_pc_t pc;

	if (tape->chrdev_direction == idetape_direction_read)
		__idetape_discard_read_pipeline(drive);
	idetape_wait_ready(drive, 60 * 5 * HZ);
	idetape_create_locate_cmd (drive, &pc, block, partition, skip);
	retval = idetape_queue_pc_tail (drive, &pc);
	if (retval)
		return (retval);

	idetape_create_read_position_cmd (&pc);
	return (idetape_queue_pc_tail (drive, &pc));
}

static void idetape_discard_read_pipeline (ide_drive_t *drive, int restore_position)
{
	idetape_tape_t *tape = drive->driver_data;
	int cnt;
	int seek, position;

	cnt = __idetape_discard_read_pipeline(drive);
	if (restore_position) {
		position = idetape_read_position(drive);
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: address %u, nr_stages %d\n", position, cnt);
#endif
		seek = position > cnt ? position - cnt : 0;
		if (idetape_position_tape(drive, seek, 0, 0)) {
			printk(KERN_INFO "ide-tape: %s: position_tape failed in discard_pipeline()\n", tape->name);
			return;
		}
	}
}

static void idetape_update_stats (ide_drive_t *drive)
{
	idetape_pc_t pc;

	idetape_create_mode_sense_cmd (&pc, IDETAPE_BUFFER_FILLING_PAGE);
	pc.callback = idetape_onstream_buffer_fill_callback;
	(void) idetape_queue_pc_tail(drive, &pc);
}

/*
 *	idetape_queue_rw_tail generates a read/write request for the block
 *	device interface and wait for it to be serviced.
 */
static int idetape_queue_rw_tail (ide_drive_t *drive, int cmd, int blocks, struct buffer_head *bh)
{
	idetape_tape_t *tape = drive->driver_data;
	struct request rq;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 2)
		printk (KERN_INFO "ide-tape: idetape_queue_rw_tail: cmd=%d\n",cmd);
#endif /* IDETAPE_DEBUG_LOG */
#if IDETAPE_DEBUG_BUGS
	if (idetape_pipeline_active (tape)) {
		printk (KERN_ERR "ide-tape: bug: the pipeline is active in idetape_queue_rw_tail\n");
		return (0);
	}
#endif /* IDETAPE_DEBUG_BUGS */	

	ide_init_drive_cmd (&rq);
	rq.bh = bh;
	rq.cmd = cmd;
	rq.sector = tape->first_frame_position;
	rq.nr_sectors = rq.current_nr_sectors = blocks;
	if (tape->onstream)
		tape->postpone_cnt = 600;
	(void) ide_do_drive_cmd (drive, &rq, ide_wait);

	if (cmd != IDETAPE_READ_RQ && cmd != IDETAPE_WRITE_RQ)
		return 0;

	if (tape->merge_stage)
		idetape_init_merge_stage (tape);
	if (rq.errors == IDETAPE_ERROR_GENERAL)
		return -EIO;
	return (tape->tape_block_size * (blocks-rq.current_nr_sectors));
}

/*
 * Read back the drive's internal buffer contents, as a part
 * of the write error recovery mechanism for old OnStream
 * firmware revisions.
 */
static void idetape_onstream_read_back_buffer (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int frames, i, logical_blk_num;
	idetape_stage_t *stage, *first = NULL, *last = NULL;
	os_aux_t *aux;
	struct request *rq;
	unsigned char *p;
	unsigned long flags;

	idetape_update_stats(drive);
	frames = tape->cur_frames;
	logical_blk_num = ntohl(tape->first_stage->aux->logical_blk_num) - frames;
	printk(KERN_INFO "ide-tape: %s: reading back %d frames from the drive's internal buffer\n", tape->name, frames);
	for (i = 0; i < frames; i++) {
		stage = __idetape_kmalloc_stage(tape, 0, 0);
		if (!first)
			first = stage;
		aux = stage->aux;
		p = stage->bh->b_data;
		idetape_queue_rw_tail(drive, IDETAPE_READ_BUFFER_RQ, tape->capabilities.ctl, stage->bh);
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: %s: read back logical block %d, data %x %x %x %x\n", tape->name, logical_blk_num, *p++, *p++, *p++, *p++);
#endif
		rq = &stage->rq;
		ide_init_drive_cmd (rq);
		rq->cmd = IDETAPE_WRITE_RQ;
		rq->sector = tape->first_frame_position;
		rq->nr_sectors = rq->current_nr_sectors = tape->capabilities.ctl;
		idetape_init_stage(drive, stage, OS_FRAME_TYPE_DATA, logical_blk_num++);
		stage->next = NULL;
		if (last)
			last->next = stage;
		last = stage;
	}
	if (frames) {
		spin_lock_irqsave(&tape->spinlock, flags);
		last->next = tape->first_stage;
		tape->next_stage = tape->first_stage = first;
		tape->nr_stages += frames;
		tape->nr_pending_stages += frames;
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	idetape_update_stats(drive);
#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: %s: frames left in buffer: %d\n", tape->name, tape->cur_frames);
#endif
}

/*
 * Error recovery algorithm for the OnStream tape.
 */
static void idetape_onstream_write_error_recovery (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned int block;

	if (tape->onstream_write_error == OS_WRITE_ERROR) {
		printk(KERN_ERR "ide-tape: %s: onstream_write_error_recovery: detected physical bad block at %u, logical %u first frame %u last_frame %u bufblocks %u stages %u skipping %u frames\n",
			tape->name, ntohl(tape->sense.information), tape->logical_blk_num,
			tape->first_frame_position, tape->last_frame_position,
			tape->blocks_in_buffer, tape->nr_stages,
 			(ntohl(tape->sense.command_specific) >> 16) & 0xff );
		block = ntohl(tape->sense.information) + ((ntohl(tape->sense.command_specific) >> 16) & 0xff);
		idetape_update_stats(drive);
		printk(KERN_ERR "ide-tape: %s: relocating %d buffered logical blocks to physical block %u\n", tape->name, tape->cur_frames, block);
#if 0  /* isn't once enough ??? MM */
		idetape_update_stats(drive);
#endif
		if (tape->firmware_revision_num >= 106)
			idetape_position_tape(drive, block, 0, 1);
		else {
			idetape_onstream_read_back_buffer(drive);
			idetape_position_tape(drive, block, 0, 0);
		}
#if 0     /* already done in idetape_position_tape MM */
		idetape_read_position(drive);
#endif
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 1)
			printk(KERN_ERR "ide-tape: %s: positioning complete, cur_frames %d, pos %d, tape pos %d\n", tape->name, tape->cur_frames, tape->first_frame_position, tape->last_frame_position);
#endif
	} else if (tape->onstream_write_error == OS_PART_ERROR) {
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 1)
			printk(KERN_INFO "ide-tape: %s: skipping over config partition\n", tape->name);
#endif
		idetape_flush_tape_buffers(drive);
		block = idetape_read_position(drive);
		if (block != OS_DATA_ENDFRAME1)  
			printk(KERN_ERR "ide-tape: warning, current position %d, expected %d\n", block, OS_DATA_ENDFRAME1);
		idetape_position_tape(drive, 0xbb8, 0, 0); /* 3000 */
	}
	tape->onstream_write_error = 0;
}

/*
 *	idetape_insert_pipeline_into_queue is used to start servicing the
 *	pipeline stages, starting from tape->next_stage.
 */
static void idetape_insert_pipeline_into_queue (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

	if (tape->next_stage == NULL)
		return;
	if (!idetape_pipeline_active (tape)) {
		if (tape->onstream_write_error)
			idetape_onstream_write_error_recovery(drive);
		set_bit(IDETAPE_PIPELINE_ACTIVE, &tape->flags);
		idetape_active_next_stage (drive);
		(void) ide_do_drive_cmd (drive, tape->active_data_request, ide_end);
	}
}

static void idetape_create_inquiry_cmd (idetape_pc_t *pc)
{
	idetape_init_pc(pc);
	pc->c[0] = IDETAPE_INQUIRY_CMD;
	pc->c[4] = pc->request_transfer = 254;
	pc->callback = &idetape_pc_callback;
}

static void idetape_create_rewind_cmd (ide_drive_t *drive, idetape_pc_t *pc)
{
	idetape_tape_t *tape = drive->driver_data;

	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_REWIND_CMD;
	if (tape->onstream)
		pc->c[1] = 1;
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

static void idetape_create_mode_select_cmd (idetape_pc_t *pc, int length)
{
	idetape_init_pc (pc);
	set_bit (PC_WRITING, &pc->flags);
	pc->c[0] = IDETAPE_MODE_SELECT_CMD;
	pc->c[1] = 0x10;
	put_unaligned (htons(length), (unsigned short *) &pc->c[3]);
	pc->request_transfer = 255;
	pc->callback = &idetape_pc_callback;
}

static void idetape_create_erase_cmd (idetape_pc_t *pc)
{
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_ERASE_CMD;
	pc->c[1] = 1;
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

static void idetape_create_space_cmd (idetape_pc_t *pc,int count,byte cmd)
{
	idetape_init_pc (pc);
	pc->c[0] = IDETAPE_SPACE_CMD;
	put_unaligned (htonl (count), (unsigned int *) &pc->c[1]);
	pc->c[1] = cmd;
	set_bit (PC_WAIT_FOR_DSC, &pc->flags);
	pc->callback = &idetape_pc_callback;
}

/*
 * Verify that we have the correct tape frame
 */
static int idetape_verify_stage (ide_drive_t *drive, idetape_stage_t *stage, int logical_blk_num, int quiet)
{
	idetape_tape_t *tape = drive->driver_data;
	os_aux_t *aux = stage->aux;
	os_partition_t *par = &aux->partition;
	struct request *rq = &stage->rq;
	struct buffer_head *bh;

	if (!tape->onstream)
		return 1;
	if (tape->raw) {
		if (rq->errors) {
			bh = stage->bh;
			while (bh) {
				memset(bh->b_data, 0, bh->b_size);
				bh = bh->b_reqnext;
			}
			strcpy(stage->bh->b_data, "READ ERROR ON FRAME");
		}
		return 1;
	}
	if (rq->errors == IDETAPE_ERROR_GENERAL) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, read error\n", tape->name, tape->first_frame_position);
		return 0;
	}
	if (rq->errors == IDETAPE_ERROR_EOD) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, eod\n", tape->name, tape->first_frame_position);
		return 0;
	}
	if (ntohl(aux->format_id) != 0) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, format_id %u\n", tape->name, tape->first_frame_position, ntohl(aux->format_id));
		return 0;
	}
	if (memcmp(aux->application_sig, tape->application_sig, 4) != 0) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, incorrect application signature\n", tape->name, tape->first_frame_position);
		return 0;
	}
	if (aux->frame_type != OS_FRAME_TYPE_DATA &&
	    aux->frame_type != OS_FRAME_TYPE_EOD &&
	    aux->frame_type != OS_FRAME_TYPE_MARKER) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, frame type %x\n", tape->name, tape->first_frame_position, aux->frame_type);
		return 0;
	}
	if (par->partition_num != OS_DATA_PARTITION) {
		if (!tape->linux_media || tape->linux_media_version != 2) {
			printk(KERN_INFO "ide-tape: %s: skipping frame %d, partition num %d\n", tape->name, tape->first_frame_position, par->partition_num);
			return 0;
		}
	}
	if (par->par_desc_ver != OS_PARTITION_VERSION) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, partition version %d\n", tape->name, tape->first_frame_position, par->par_desc_ver);
		return 0;
	}
	if (ntohs(par->wrt_pass_cntr) != tape->wrt_pass_cntr) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, wrt_pass_cntr %d (expected %d)(logical_blk_num %u)\n", tape->name, tape->first_frame_position, ntohs(par->wrt_pass_cntr), tape->wrt_pass_cntr, ntohl(aux->logical_blk_num));
		return 0;
	}
	if (aux->frame_seq_num != aux->logical_blk_num) {
		printk(KERN_INFO "ide-tape: %s: skipping frame %d, seq != logical\n", tape->name, tape->first_frame_position);
		return 0;
	}
	if (logical_blk_num != -1 && ntohl(aux->logical_blk_num) != logical_blk_num) {
		if (!quiet)
			printk(KERN_INFO "ide-tape: %s: skipping frame %d, logical_blk_num %u (expected %d)\n", tape->name, tape->first_frame_position, ntohl(aux->logical_blk_num), logical_blk_num);
		return 0;
	}
	if (aux->frame_type == OS_FRAME_TYPE_MARKER) {
		rq->errors = IDETAPE_ERROR_FILEMARK;
		rq->current_nr_sectors = rq->nr_sectors;
	}
	return 1;
}

static void idetape_wait_first_stage (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;

	if (tape->first_stage == NULL)
		return;
	spin_lock_irqsave(&tape->spinlock, flags);
	if (tape->active_stage == tape->first_stage)
		idetape_wait_for_request(drive, tape->active_data_request);
	spin_unlock_irqrestore(&tape->spinlock, flags);
}

/*
 *	idetape_add_chrdev_write_request tries to add a character device
 *	originated write request to our pipeline. In case we don't succeed,
 *	we revert to non-pipelined operation mode for this request.
 *
 *	1.	Try to allocate a new pipeline stage.
 *	2.	If we can't, wait for more and more requests to be serviced
 *		and try again each time.
 *	3.	If we still can't allocate a stage, fallback to
 *		non-pipelined operation mode for this request.
 */
static int idetape_add_chrdev_write_request (ide_drive_t *drive, int blocks)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *new_stage;
	unsigned long flags;
	struct request *rq;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 3)
		printk (KERN_INFO "ide-tape: Reached idetape_add_chrdev_write_request\n");
#endif /* IDETAPE_DEBUG_LOG */

     	/*
     	 *	Attempt to allocate a new stage.
	 *	Pay special attention to possible race conditions.
	 */
	while ((new_stage = idetape_kmalloc_stage (tape)) == NULL) {
		spin_lock_irqsave(&tape->spinlock, flags);
		if (idetape_pipeline_active (tape)) {
			idetape_wait_for_request(drive, tape->active_data_request);
			spin_unlock_irqrestore(&tape->spinlock, flags);
		} else {
			spin_unlock_irqrestore(&tape->spinlock, flags);
			idetape_insert_pipeline_into_queue (drive);
			if (idetape_pipeline_active (tape))
				continue;
			/*
			 *	Linux is short on memory. Fallback to
			 *	non-pipelined operation mode for this request.
			 */
			return idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, blocks, tape->merge_stage->bh);
		}
	}
	rq = &new_stage->rq;
	ide_init_drive_cmd (rq);
	rq->cmd = IDETAPE_WRITE_RQ;
	rq->sector = tape->first_frame_position;	/* Doesn't actually matter - We always assume sequential access */
	rq->nr_sectors = rq->current_nr_sectors = blocks;

	idetape_switch_buffers (tape, new_stage);
	idetape_init_stage(drive, new_stage, OS_FRAME_TYPE_DATA, tape->logical_blk_num);
	tape->logical_blk_num++;
	idetape_add_stage_tail (drive, new_stage);
	tape->pipeline_head++;
#if USE_IOTRACE
	IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
	calculate_speeds(drive);

	/*
	 *	Estimate whether the tape has stopped writing by checking
	 *	if our write pipeline is currently empty. If we are not
	 *	writing anymore, wait for the pipeline to be full enough
	 *	(90%) before starting to service requests, so that we will
	 *	be able to keep up with the higher speeds of the tape.
	 *
	 *	For the OnStream drive, we can query the number of pending
	 *	frames in the drive's internal buffer. As long as the tape
	 *	is still writing, it is better to write frames immediately
	 *	rather than gather them in the pipeline. This will give the
	 *	tape's firmware the ability to sense the current incoming
	 *	data rate more accurately, and since the OnStream tape
	 *	supports variable speeds, it can try to adjust itself to the
	 *	incoming data rate.
	 */
	if (!idetape_pipeline_active(tape)) {
		if (tape->nr_stages >= tape->max_stages * 9 / 10 ||
		    tape->nr_stages >= tape->max_stages - tape->uncontrolled_pipeline_head_speed * 3 * 1024 / tape->tape_block_size) {
			tape->measure_insert_time = 1;
			tape->insert_time = jiffies;
			tape->insert_size = 0;
			tape->insert_speed = 0;
			idetape_insert_pipeline_into_queue (drive);
		} else if (tape->onstream) {
			idetape_update_stats(drive);
			if (tape->cur_frames > 5)
				idetape_insert_pipeline_into_queue (drive);
		}
	}
	if (test_and_clear_bit (IDETAPE_PIPELINE_ERROR, &tape->flags))		/* Return a deferred error */
		return -EIO;
	return blocks;
}

/*
 *	idetape_wait_for_pipeline will wait until all pending pipeline
 *	requests are serviced. Typically called on device close.
 */
static void idetape_wait_for_pipeline (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;

	while (tape->next_stage || idetape_pipeline_active(tape)) {
		idetape_insert_pipeline_into_queue (drive);
		spin_lock_irqsave(&tape->spinlock, flags);
		if (idetape_pipeline_active(tape))
			idetape_wait_for_request(drive, tape->active_data_request);
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
}

static void idetape_empty_write_pipeline (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int blocks, i, min;
	struct buffer_head *bh;
	
#if IDETAPE_DEBUG_BUGS
	if (tape->chrdev_direction != idetape_direction_write) {
		printk (KERN_ERR "ide-tape: bug: Trying to empty write pipeline, but we are not writing.\n");
		return;
	}
	if (tape->merge_stage_size > tape->stage_size) {
		printk (KERN_ERR "ide-tape: bug: merge_buffer too big\n");
		tape->merge_stage_size = tape->stage_size;
	}
#endif /* IDETAPE_DEBUG_BUGS */
	if (tape->merge_stage_size) {
		blocks = tape->merge_stage_size / tape->tape_block_size;
		if (tape->merge_stage_size % tape->tape_block_size) {
			blocks++;
			i = tape->tape_block_size - tape->merge_stage_size % tape->tape_block_size;
			bh = tape->bh->b_reqnext;
			while (bh) {
				atomic_set(&bh->b_count, 0);
				bh = bh->b_reqnext;
			}
			bh = tape->bh;
			while (i) {
				if (bh == NULL) {
					printk(KERN_INFO "ide-tape: bug, bh NULL\n");
					break;
				}
				min = IDE_MIN(i, bh->b_size - atomic_read(&bh->b_count));
				memset(bh->b_data + atomic_read(&bh->b_count), 0, min);
				atomic_add(min, &bh->b_count);
				i -= min;
				bh = bh->b_reqnext;
			}
		}
		(void) idetape_add_chrdev_write_request (drive, blocks);
		tape->merge_stage_size = 0;
	}
	idetape_wait_for_pipeline (drive);
	if (tape->merge_stage != NULL) {
		__idetape_kfree_stage (tape->merge_stage);
		tape->merge_stage = NULL;
	}
	clear_bit (IDETAPE_PIPELINE_ERROR, &tape->flags);
	tape->chrdev_direction = idetape_direction_none;

	/*
	 *	On the next backup, perform the feedback loop again.
	 *	(I don't want to keep sense information between backups,
	 *	 as some systems are constantly on, and the system load
	 *	 can be totally different on the next backup).
	 */
	tape->max_stages = tape->min_pipeline;
#if IDETAPE_DEBUG_BUGS
	if (tape->first_stage != NULL || tape->next_stage != NULL || tape->last_stage != NULL || tape->nr_stages != 0) {
		printk (KERN_ERR "ide-tape: ide-tape pipeline bug, "
		"first_stage %p, next_stage %p, last_stage %p, nr_stages %d\n",
		tape->first_stage, tape->next_stage, tape->last_stage, tape->nr_stages);
	}
#endif /* IDETAPE_DEBUG_BUGS */
}

static void idetape_restart_speed_control (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

	tape->restart_speed_control_req = 0;
	tape->pipeline_head = 0;
	tape->buffer_head = tape->tape_head = tape->cur_frames;
	tape->controlled_last_pipeline_head = tape->uncontrolled_last_pipeline_head = 0;
	tape->controlled_previous_pipeline_head = tape->uncontrolled_previous_pipeline_head = 0;
	tape->pipeline_head_speed = tape->controlled_pipeline_head_speed = 5000;
	tape->uncontrolled_pipeline_head_speed = 0;
	tape->controlled_pipeline_head_time = tape->uncontrolled_pipeline_head_time = jiffies;
	tape->controlled_previous_head_time = tape->uncontrolled_previous_head_time = jiffies;
}

static int idetape_initiate_read (ide_drive_t *drive, int max_stages)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *new_stage;
	struct request rq;
	int bytes_read;
	int blocks = tape->capabilities.ctl;

	if (tape->chrdev_direction != idetape_direction_read) {		/* Initialize read operation */
		if (tape->chrdev_direction == idetape_direction_write) {
			idetape_empty_write_pipeline (drive);
			idetape_flush_tape_buffers (drive);
		}
#if IDETAPE_DEBUG_BUGS
		if (tape->merge_stage || tape->merge_stage_size) {
			printk (KERN_ERR "ide-tape: merge_stage_size should be 0 now\n");
			tape->merge_stage_size = 0;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		if ((tape->merge_stage = __idetape_kmalloc_stage (tape, 0, 0)) == NULL)
			return -ENOMEM;
		tape->chrdev_direction = idetape_direction_read;
		tape->logical_blk_num = 0;

		/*
		 *	Issue a read 0 command to ensure that DSC handshake
		 *	is switched from completion mode to buffer available
		 *	mode.
		 */
		bytes_read = idetape_queue_rw_tail (drive, IDETAPE_READ_RQ, 0, tape->merge_stage->bh);
		if (bytes_read < 0) {
			kfree (tape->merge_stage);
			tape->merge_stage = NULL;
			tape->chrdev_direction = idetape_direction_none;
			return bytes_read;
		}
	}
	if (tape->restart_speed_control_req)
		idetape_restart_speed_control(drive);
	ide_init_drive_cmd (&rq);
	rq.cmd = IDETAPE_READ_RQ;
	rq.sector = tape->first_frame_position;
	rq.nr_sectors = rq.current_nr_sectors = blocks;
	if (!test_bit(IDETAPE_PIPELINE_ERROR, &tape->flags) && tape->nr_stages <= max_stages) {
		new_stage = idetape_kmalloc_stage (tape);
		while (new_stage != NULL) {
			new_stage->rq = rq;
			idetape_add_stage_tail (drive, new_stage);
			if (tape->nr_stages >= max_stages)
				break;
			new_stage = idetape_kmalloc_stage (tape);
		}
	}
	if (!idetape_pipeline_active(tape)) {
		if (tape->nr_pending_stages >= 3 * max_stages / 4) {
			tape->measure_insert_time = 1;
			tape->insert_time = jiffies;
			tape->insert_size = 0;
			tape->insert_speed = 0;
			idetape_insert_pipeline_into_queue (drive);
		} else if (tape->onstream) {
			idetape_update_stats(drive);
			if (tape->cur_frames < tape->max_frames - 5)
				idetape_insert_pipeline_into_queue (drive);
		}
	}
	return 0;
}

static int idetape_get_logical_blk (ide_drive_t *drive, int logical_blk_num, int max_stages, int quiet)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;
	int cnt = 0, x, position;

	/*
	 * Search and wait for the next logical tape block
	 */
	while (1) {
		if (cnt++ > 1000) {   /* AJN: was 100 */
			printk(KERN_INFO "ide-tape: %s: couldn't find logical block %d, aborting\n", tape->name, logical_blk_num);
			return 0;
		}
		idetape_initiate_read(drive, max_stages);
		if (tape->first_stage == NULL) {
			if (tape->onstream) {
#if ONSTREAM_DEBUG
				if (tape->debug_level >= 1)
					printk(KERN_INFO "ide-tape: %s: first_stage == NULL, pipeline error %ld\n", tape->name, (long)test_bit(IDETAPE_PIPELINE_ERROR, &tape->flags));
#endif
				clear_bit(IDETAPE_PIPELINE_ERROR, &tape->flags);
				position = idetape_read_position(drive);
				printk(KERN_INFO "ide-tape: %s: blank block detected at %d\n", tape->name, position);
				if (position >= 3000 && position < 3080)
					position += 32;  /* Why is this check and number ??? MM */
				if (position >= OS_DATA_ENDFRAME1 && position < 3000)
					position = 3000;
				else
					/*
					 * compensate for write errors that generally skip 80 frames,
					 * expect around 20 read errors in a row...
					 */
					position += 60;
				if (position >= OS_DATA_ENDFRAME1 && position < 3000)
					position = 3000;
				printk(KERN_INFO "ide-tape: %s: positioning tape to block %d\n", tape->name, position);
				if (position == 3000)  /* seems to be needed to correctly position at block 3000 MM */
					idetape_position_tape(drive, 0, 0, 0);
				idetape_position_tape(drive, position, 0, 0);
				cnt += 40;
				continue;
			} else
				return 0;
		}
		idetape_wait_first_stage(drive);
		if (idetape_verify_stage(drive, tape->first_stage, logical_blk_num, quiet))
			break;
		if (tape->first_stage->rq.errors == IDETAPE_ERROR_EOD)
			cnt--;
		if (idetape_verify_stage(drive, tape->first_stage, -1, quiet)) {
			x = ntohl(tape->first_stage->aux->logical_blk_num);
			if (x > logical_blk_num) {
				printk(KERN_ERR "ide-tape: %s: couldn't find logical block %d, aborting (block %d found)\n", tape->name, logical_blk_num, x);
				return 0;
			}
		}
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head(drive);
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	if (tape->onstream)
		tape->logical_blk_num = ntohl(tape->first_stage->aux->logical_blk_num);
	return 1;
}

/*
 *	idetape_add_chrdev_read_request is called from idetape_chrdev_read
 *	to service a character device read request and add read-ahead
 *	requests to our pipeline.
 */
static int idetape_add_chrdev_read_request (ide_drive_t *drive,int blocks)
{
	idetape_tape_t *tape = drive->driver_data;
	unsigned long flags;
	struct request *rq_ptr;
	int bytes_read;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_add_chrdev_read_request, %d blocks\n", blocks);
#endif /* IDETAPE_DEBUG_LOG */

	/*
	 * Wait for the next logical block to be available at the head
	 * of the pipeline
	 */
	if (!idetape_get_logical_blk(drive, tape->logical_blk_num, tape->max_stages, 0)) {
		if (tape->onstream) {
			set_bit(IDETAPE_READ_ERROR, &tape->flags);
			return 0;
		}
		if (test_bit(IDETAPE_PIPELINE_ERROR, &tape->flags))
		 	return 0;
		return idetape_queue_rw_tail (drive, IDETAPE_READ_RQ, blocks, tape->merge_stage->bh);
	}
	rq_ptr = &tape->first_stage->rq;
	bytes_read = tape->tape_block_size * (rq_ptr->nr_sectors - rq_ptr->current_nr_sectors);
	rq_ptr->nr_sectors = rq_ptr->current_nr_sectors = 0;


	if (tape->onstream && !tape->raw && tape->first_stage->aux->frame_type == OS_FRAME_TYPE_EOD) {
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: %s: EOD reached\n", tape->name);
#endif
		return 0;
	}
	if (rq_ptr->errors == IDETAPE_ERROR_EOD)
		return 0;
	if (rq_ptr->errors == IDETAPE_ERROR_FILEMARK) {
		idetape_switch_buffers (tape, tape->first_stage);
		set_bit (IDETAPE_FILEMARK, &tape->flags);
#if USE_IOTRACE
		IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
		calculate_speeds(drive);
	} else {
		idetape_switch_buffers (tape, tape->first_stage);
		if (rq_ptr->errors == IDETAPE_ERROR_GENERAL) {
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 1)
				printk(KERN_INFO "ide-tape: error detected, bytes_read %d\n", bytes_read);
#endif
		}
		clear_bit (IDETAPE_FILEMARK, &tape->flags);
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head (drive);
		spin_unlock_irqrestore(&tape->spinlock, flags);
		tape->logical_blk_num++;
		tape->pipeline_head++;
#if USE_IOTRACE
		IO_trace(IO_IDETAPE_FIFO, tape->pipeline_head, tape->buffer_head, tape->tape_head, tape->minor);
#endif
		calculate_speeds(drive);
	}
#if IDETAPE_DEBUG_BUGS
	if (bytes_read > blocks*tape->tape_block_size) {
		printk (KERN_ERR "ide-tape: bug: trying to return more bytes than requested\n");
		bytes_read=blocks*tape->tape_block_size;
	}
#endif /* IDETAPE_DEBUG_BUGS */
	return (bytes_read);
}

static void idetape_pad_zeros (ide_drive_t *drive, int bcount)
{
	idetape_tape_t *tape = drive->driver_data;
	struct buffer_head *bh;
	int count, blocks;
	
	while (bcount) {
		bh = tape->merge_stage->bh;
		count = IDE_MIN (tape->stage_size, bcount);
		bcount -= count;
		blocks = count / tape->tape_block_size;
		while (count) {
			atomic_set(&bh->b_count, IDE_MIN (count, bh->b_size));
			memset (bh->b_data, 0, atomic_read(&bh->b_count));
			count -= atomic_read(&bh->b_count);
			bh = bh->b_reqnext;
		}
		idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, blocks, tape->merge_stage->bh);
	}
}

static int idetape_pipeline_size (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	struct request *rq;
	int size = 0;

	idetape_wait_for_pipeline (drive);
	stage = tape->first_stage;
	while (stage != NULL) {
		rq = &stage->rq;
		size += tape->tape_block_size * (rq->nr_sectors-rq->current_nr_sectors);
		if (rq->errors == IDETAPE_ERROR_FILEMARK)
			size += tape->tape_block_size;
		stage = stage->next;
	}
	size += tape->merge_stage_size;
	return size;
}

/*
 *	Rewinds the tape to the Beginning Of the current Partition (BOP).
 *
 *	We currently support only one partition.
 */ 
static int idetape_rewind_tape (ide_drive_t *drive)
{
	int retval;
	idetape_pc_t pc;
	idetape_tape_t *tape = drive->driver_data;
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 2)
		printk (KERN_INFO "ide-tape: Reached idetape_rewind_tape\n");
#endif /* IDETAPE_DEBUG_LOG */	
	
	idetape_create_rewind_cmd (drive, &pc);
	retval = idetape_queue_pc_tail (drive, &pc);
	if (retval)
		return retval;

	idetape_create_read_position_cmd (&pc);
	retval = idetape_queue_pc_tail (drive, &pc);
	if (retval)
		return retval;
	tape->logical_blk_num = 0;
	return 0;
}

/*
 *	Our special ide-tape ioctl's.
 *
 *	Currently there aren't any ioctl's.
 *	mtio.h compatible commands should be issued to the character device
 *	interface.
 */
static int idetape_blkdev_ioctl (ide_drive_t *drive, struct inode *inode, struct file *file,
				 unsigned int cmd, unsigned long arg)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_config_t config;

#if IDETAPE_DEBUG_LOG	
	if (tape->debug_level >= 4)
		printk (KERN_INFO "ide-tape: Reached idetape_blkdev_ioctl\n");
#endif /* IDETAPE_DEBUG_LOG */
	switch (cmd) {
		case 0x0340:
			if (copy_from_user ((char *) &config, (char *) arg, sizeof (idetape_config_t)))
				return -EFAULT;
			tape->best_dsc_rw_frequency = config.dsc_rw_frequency;
			tape->max_stages = config.nr_stages;
			break;
		case 0x0350:
			config.dsc_rw_frequency = (int) tape->best_dsc_rw_frequency;
			config.nr_stages = tape->max_stages; 
			if (copy_to_user ((char *) arg, (char *) &config, sizeof (idetape_config_t)))
				return -EFAULT;
			break;
		default:
			return -EIO;
	}
	return 0;
}

/*
 *	The block device interface should not be used for data transfers.
 *	However, we still allow opening it so that we can issue general
 *	ide driver configuration ioctl's, such as the interrupt unmask feature.
 */
static int idetape_blkdev_open (struct inode *inode, struct file *filp, ide_drive_t *drive)
{
	MOD_INC_USE_COUNT;
#if ONSTREAM_DEBUG
        printk(KERN_INFO "ide-tape: MOD_INC_USE_COUNT in idetape_blkdev_open\n");
#endif
	return 0;
}

static void idetape_blkdev_release (struct inode *inode, struct file *filp, ide_drive_t *drive)
{
	MOD_DEC_USE_COUNT;
#if ONSTREAM_DEBUG
        printk(KERN_INFO "ide-tape: MOD_DEC_USE_COUNT in idetape_blkdev_release\n");
#endif
}

/*
 *	idetape_pre_reset is called before an ATAPI/ATA software reset.
 */
static void idetape_pre_reset (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	if (tape != NULL)
		set_bit (IDETAPE_IGNORE_DSC, &tape->flags);
}

/*
 *	Character device interface functions
 */
static ide_drive_t *get_drive_ptr (kdev_t i_rdev)
{
	unsigned int i = MINOR(i_rdev) & ~0xc0;

	if (i >= MAX_HWIFS * MAX_DRIVES)
		return NULL;
	return (idetape_chrdevs[i].drive);
}

static int idetape_onstream_space_over_filemarks_backward (ide_drive_t *drive,short mt_op,int mt_count)
{
	idetape_tape_t *tape = drive->driver_data;
	int cnt = 0;
	int last_mark_addr;
	unsigned long flags;

	if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
		printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks_bwd\n", tape->name);
		return -EIO;
	}
	while (cnt != mt_count) {
		last_mark_addr = ntohl(tape->first_stage->aux->last_mark_addr);
		if (last_mark_addr == -1)
			return -EIO;
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: positioning to last mark at %d\n", last_mark_addr);
#endif
		idetape_position_tape(drive, last_mark_addr, 0, 0);
		cnt++;
		if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
			printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks\n", tape->name);
			return -EIO;
		}
		if (tape->first_stage->aux->frame_type != OS_FRAME_TYPE_MARKER) {
			printk(KERN_INFO "ide-tape: %s: expected to find marker at block %d, not found\n", tape->name, last_mark_addr);
			return -EIO;
		}
	}
	if (mt_op == MTBSFM) {
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head (drive);
		tape->logical_blk_num++;
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	return 0;
}

/*
 * ADRL 1.1 compatible "slow" space filemarks fwd version
 *
 * Just scans for the filemark sequentially.
 */
static int idetape_onstream_space_over_filemarks_forward_slow (ide_drive_t *drive,short mt_op,int mt_count)
{
	idetape_tape_t *tape = drive->driver_data;
	int cnt = 0;
	unsigned long flags;

	if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
		printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks_fwd\n", tape->name);
		return -EIO;
	}
	while (1) {
		if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
			printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks\n", tape->name);
			return -EIO;
		}
		if (tape->first_stage->aux->frame_type == OS_FRAME_TYPE_MARKER)
			cnt++;
		if (tape->first_stage->aux->frame_type == OS_FRAME_TYPE_EOD) {
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 2)
				printk(KERN_INFO "ide-tape: %s: space_fwd: EOD reached\n", tape->name);
#endif
			return -EIO;
		}
		if (cnt == mt_count)
			break;
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head (drive);
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	if (mt_op == MTFSF) {
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head (drive);
		tape->logical_blk_num++;
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	return 0;
}


/*
 * Fast linux specific version of OnStream FSF
 */
static int idetape_onstream_space_over_filemarks_forward_fast (ide_drive_t *drive,short mt_op,int mt_count)
{
	idetape_tape_t *tape = drive->driver_data;
	int cnt = 0, next_mark_addr;
	unsigned long flags;

	if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
		printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks_fwd\n", tape->name);
		return -EIO;
	}

	/*
	 * Find nearest (usually previous) marker
	 */
	while (1) {
		if (tape->first_stage->aux->frame_type == OS_FRAME_TYPE_MARKER)
			break;
		if (tape->first_stage->aux->frame_type == OS_FRAME_TYPE_EOD) {
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 2)
				printk(KERN_INFO "ide-tape: %s: space_fwd: EOD reached\n", tape->name);
#endif
			return -EIO;
		}
		if (ntohl(tape->first_stage->aux->filemark_cnt) == 0) {
			if (tape->first_mark_addr == -1) {
				printk(KERN_INFO "ide-tape: %s: reverting to slow filemark space\n", tape->name);
				return idetape_onstream_space_over_filemarks_forward_slow(drive, mt_op, mt_count);
			}
			idetape_position_tape(drive, tape->first_mark_addr, 0, 0);
			if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
				printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks_fwd_fast\n", tape->name);
				return -EIO;
			}
			if (tape->first_stage->aux->frame_type != OS_FRAME_TYPE_MARKER) {
				printk(KERN_INFO "ide-tape: %s: expected to find filemark at %d\n", tape->name, tape->first_mark_addr);
				return -EIO;
			}
		} else {
			if (idetape_onstream_space_over_filemarks_backward(drive, MTBSF, 1) < 0)
				return -EIO;
			mt_count++;
		}
	}
	cnt++;
	while (cnt != mt_count) {
		next_mark_addr = ntohl(tape->first_stage->aux->next_mark_addr);
		if (!next_mark_addr || next_mark_addr > tape->eod_frame_addr) {
			printk(KERN_INFO "ide-tape: %s: reverting to slow filemark space\n", tape->name);
			return idetape_onstream_space_over_filemarks_forward_slow(drive, mt_op, mt_count - cnt);
#if ONSTREAM_DEBUG
		} else if (tape->debug_level >= 2) {
		     printk(KERN_INFO "ide-tape: positioning to next mark at %d\n", next_mark_addr);
#endif
		}
		idetape_position_tape(drive, next_mark_addr, 0, 0);
		cnt++;
		if (!idetape_get_logical_blk(drive, -1, 10, 0)) {
			printk(KERN_INFO "ide-tape: %s: couldn't get logical blk num in space_filemarks\n", tape->name);
			return -EIO;
		}
		if (tape->first_stage->aux->frame_type != OS_FRAME_TYPE_MARKER) {
			printk(KERN_INFO "ide-tape: %s: expected to find marker at block %d, not found\n", tape->name, next_mark_addr);
			return -EIO;
		}
	}
	if (mt_op == MTFSF) {
		spin_lock_irqsave(&tape->spinlock, flags);
		idetape_remove_stage_head (drive);
		tape->logical_blk_num++;
		spin_unlock_irqrestore(&tape->spinlock, flags);
	}
	return 0;
}

/*
 *	idetape_space_over_filemarks is now a bit more complicated than just
 *	passing the command to the tape since we may have crossed some
 *	filemarks during our pipelined read-ahead mode.
 *
 *	As a minor side effect, the pipeline enables us to support MTFSFM when
 *	the filemark is in our internal pipeline even if the tape doesn't
 *	support spacing over filemarks in the reverse direction.
 */
static int idetape_space_over_filemarks (ide_drive_t *drive,short mt_op,int mt_count)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	unsigned long flags;
	int retval,count=0;
	int speed_control;

	if (tape->onstream) {
		if (tape->raw)
			return -EIO;
		speed_control = tape->speed_control;
		tape->speed_control = 0;
		if (mt_op == MTFSF || mt_op == MTFSFM) {
			if (tape->linux_media)
				retval = idetape_onstream_space_over_filemarks_forward_fast(drive, mt_op, mt_count);
			else
				retval = idetape_onstream_space_over_filemarks_forward_slow(drive, mt_op, mt_count);
		} else
			retval = idetape_onstream_space_over_filemarks_backward(drive, mt_op, mt_count);
		tape->speed_control = speed_control;
		tape->restart_speed_control_req = 1;
		return retval;
	}

	if (tape->chrdev_direction == idetape_direction_read) {
		/*
		 *	We have a read-ahead buffer. Scan it for crossed
		 *	filemarks.
		 */
		tape->merge_stage_size = 0;
		clear_bit (IDETAPE_FILEMARK, &tape->flags);
		while (tape->first_stage != NULL) {
			idetape_wait_first_stage(drive);
			if (tape->first_stage->rq.errors == IDETAPE_ERROR_FILEMARK)
				count++;
			if (count == mt_count) {
				switch (mt_op) {
					case MTFSF:
						spin_lock_irqsave(&tape->spinlock, flags);
						idetape_remove_stage_head (drive);
						spin_unlock_irqrestore(&tape->spinlock, flags);
					case MTFSFM:
						return (0);
					default:
						break;
				}
			}
			spin_lock_irqsave(&tape->spinlock, flags);
			idetape_remove_stage_head (drive);
			spin_unlock_irqrestore(&tape->spinlock, flags);
		}
		idetape_discard_read_pipeline (drive, 1);
	}

	/*
	 *	The filemark was not found in our internal pipeline.
	 *	Now we can issue the space command.
	 */
	switch (mt_op) {
		case MTFSF:
			idetape_create_space_cmd (&pc,mt_count-count,IDETAPE_SPACE_OVER_FILEMARK);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTFSFM:
			if (!tape->capabilities.sprev)
				return (-EIO);
			retval = idetape_space_over_filemarks (drive, MTFSF, mt_count-count);
			if (retval) return (retval);
			return (idetape_space_over_filemarks (drive, MTBSF, 1));
		case MTBSF:
			if (!tape->capabilities.sprev)
				return (-EIO);
			idetape_create_space_cmd (&pc,-(mt_count+count),IDETAPE_SPACE_OVER_FILEMARK);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTBSFM:
			if (!tape->capabilities.sprev)
				return (-EIO);
			retval = idetape_space_over_filemarks (drive, MTBSF, mt_count+count);
			if (retval) return (retval);
			return (idetape_space_over_filemarks (drive, MTFSF, 1));
		default:
			printk (KERN_ERR "ide-tape: MTIO operation %d not supported\n",mt_op);
			return (-EIO);
	}
}


/*
 *	Our character device read / write functions.
 *
 *	The tape is optimized to maximize throughput when it is transferring
 *	an integral number of the "continuous transfer limit", which is
 *	a parameter of the specific tape (26 KB on my particular tape).
 *      (32 kB for Onstream)
 *
 *	As of version 1.3 of the driver, the character device provides an
 *	abstract continuous view of the media - any mix of block sizes (even 1
 *	byte) on the same backup/restore procedure is supported. The driver
 *	will internally convert the requests to the recommended transfer unit,
 *	so that an unmatch between the user's block size to the recommended
 *	size will only result in a (slightly) increased driver overhead, but
 *	will no longer hit performance.
 *      This is not applicable to Onstream.
 */
static ssize_t idetape_chrdev_read (struct file *file, char *buf,
				    size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_dentry->d_inode;
	ide_drive_t *drive = get_drive_ptr (inode->i_rdev);
	idetape_tape_t *tape = drive->driver_data;
	ssize_t bytes_read,temp, actually_read = 0, rc;

	if (ppos != &file->f_pos) {
		/* "A request was outside the capabilities of the device." */
		return -ENXIO;
	}
	if (tape->onstream && (count != tape->tape_block_size)) {
		printk(KERN_ERR "ide-tape: %s: use %d bytes as block size (%Zd used)\n", tape->name, tape->tape_block_size, count);
		return -EINVAL;
	}
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 3)
		printk (KERN_INFO "ide-tape: Reached idetape_chrdev_read, count %Zd\n", count);
#endif /* IDETAPE_DEBUG_LOG */

	if (tape->chrdev_direction != idetape_direction_read) {
		if (test_bit (IDETAPE_DETECT_BS, &tape->flags))
			if (count > tape->tape_block_size && (count % tape->tape_block_size) == 0)
				tape->user_bs_factor = count / tape->tape_block_size;
	}
	if ((rc = idetape_initiate_read(drive, tape->max_stages)) < 0)
		return rc;
	if (count == 0)
		return (0);
	if (tape->merge_stage_size) {
		actually_read = IDE_MIN (tape->merge_stage_size, count);
		idetape_copy_stage_to_user (tape, buf, tape->merge_stage, actually_read);
		buf += actually_read;
		tape->merge_stage_size -= actually_read;
		count -= actually_read;
	}
	while (count >= tape->stage_size) {
		bytes_read = idetape_add_chrdev_read_request (drive, tape->capabilities.ctl);
		if (bytes_read <= 0)
			goto finish;
		idetape_copy_stage_to_user (tape, buf, tape->merge_stage, bytes_read);
		buf += bytes_read;
		count -= bytes_read;
		actually_read += bytes_read;
	}
	if (count) {
		bytes_read=idetape_add_chrdev_read_request (drive, tape->capabilities.ctl);
		if (bytes_read <= 0)
			goto finish;
		temp = IDE_MIN (count, bytes_read);
		idetape_copy_stage_to_user (tape, buf, tape->merge_stage, temp);
		actually_read += temp;
		tape->merge_stage_size = bytes_read-temp;
	}
finish:
	if (!actually_read && test_bit (IDETAPE_FILEMARK, &tape->flags)) {
#if IDETAPE_DEBUG_LOG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: %s: spacing over filemark\n", tape->name);
#endif
		idetape_space_over_filemarks (drive, MTFSF, 1);
		return 0;
	}
	if (tape->onstream && !actually_read && test_and_clear_bit(IDETAPE_READ_ERROR, &tape->flags)) {
		printk(KERN_ERR "ide-tape: %s: unrecovered read error on logical block number %d, skipping\n",
					tape->name, tape->logical_blk_num);
		tape->logical_blk_num++;
		return -EIO;
	}
	return actually_read;
}

static void idetape_update_last_marker (ide_drive_t *drive, int last_mark_addr, int next_mark_addr)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	os_aux_t *aux;
	int position;

	if (!tape->onstream || tape->raw)
		return;
	if (last_mark_addr == -1)
		return;
	stage = __idetape_kmalloc_stage(tape, 0, 0);
	if (stage == NULL)
		return;
	idetape_flush_tape_buffers(drive);
	position = idetape_read_position(drive);
#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: current position (2) %d, lblk %d\n", position, tape->logical_blk_num);
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: current position (2) tape block %d\n", tape->last_frame_position);
#endif
	idetape_position_tape(drive, last_mark_addr, 0, 0);
	if (!idetape_queue_rw_tail (drive, IDETAPE_READ_RQ, 1, stage->bh)) {
		printk(KERN_INFO "ide-tape: %s: couldn't read last marker\n", tape->name);
		__idetape_kfree_stage (stage);
		idetape_position_tape(drive, position, 0, 0);
		return;
	}
	aux = stage->aux;
	if (aux->frame_type != OS_FRAME_TYPE_MARKER) {
		printk(KERN_INFO "ide-tape: %s: expected to find marker at addr %d\n", tape->name, last_mark_addr);
		__idetape_kfree_stage (stage);
		idetape_position_tape(drive, position, 0, 0);
		return;
	}
#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: writing back marker\n");
#endif
	aux->next_mark_addr = htonl(next_mark_addr);
	idetape_position_tape(drive, last_mark_addr, 0, 0);
	if (!idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, 1, stage->bh)) {
		printk(KERN_INFO "ide-tape: %s: couldn't write back marker frame at %d\n", tape->name, last_mark_addr);
		__idetape_kfree_stage (stage);
		idetape_position_tape(drive, position, 0, 0);
		return;
	}
	__idetape_kfree_stage (stage);
	idetape_flush_tape_buffers (drive);
	idetape_position_tape(drive, position, 0, 0);
	return;
}

static void idetape_write_filler (ide_drive_t *drive, int block, int cnt)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	int rc;

	if (!tape->onstream || tape->raw)
		return;
	stage = __idetape_kmalloc_stage(tape, 1, 1);
	if (stage == NULL)
		return;
	idetape_init_stage(drive, stage, OS_FRAME_TYPE_FILL, 0);
	idetape_wait_ready(drive, 60 * 5 * HZ);
	rc = idetape_position_tape(drive, block, 0, 0);
#if ONSTREAM_DEBUG
	printk(KERN_INFO "write_filler: positioning failed it returned %d\n", rc);
#endif
	if (rc != 0) 
		return;	/* don't write fillers if we cannot position the tape. */

	strcpy(stage->bh->b_data, "Filler");
	while (cnt--) {
		if (!idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, 1, stage->bh)) {
			printk(KERN_INFO "ide-tape: %s: write_filler: couldn't write header frame\n", tape->name);
			__idetape_kfree_stage (stage);
			return;
		}
	}
	__idetape_kfree_stage (stage);
}

static void __idetape_write_header (ide_drive_t *drive, int block, int cnt)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	os_header_t header;

	stage = __idetape_kmalloc_stage(tape, 1, 1);
	if (stage == NULL)
		return;
	idetape_init_stage(drive, stage, OS_FRAME_TYPE_HEADER, tape->logical_blk_num);
	idetape_wait_ready(drive, 60 * 5 * HZ);
	idetape_position_tape(drive, block, 0, 0);
	memset(&header, 0, sizeof(header));
	strcpy(header.ident_str, "ADR_SEQ");
	header.major_rev = 1;
	header.minor_rev = OS_ADR_MINREV;
	header.par_num = 1;
	header.partition.partition_num = OS_DATA_PARTITION;
	header.partition.par_desc_ver = OS_PARTITION_VERSION;
	header.partition.first_frame_addr = htonl(OS_DATA_STARTFRAME1);
	header.partition.last_frame_addr = htonl(tape->capacity);
	header.partition.wrt_pass_cntr = htons(tape->wrt_pass_cntr);
	header.partition.eod_frame_addr = htonl(tape->eod_frame_addr);
	memcpy(stage->bh->b_data, &header, sizeof(header));
	while (cnt--) {
		if (!idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, 1, stage->bh)) {
			printk(KERN_INFO "ide-tape: %s: couldn't write header frame\n", tape->name);
			__idetape_kfree_stage (stage);
			return;
		}
	}
	__idetape_kfree_stage (stage);
	idetape_flush_tape_buffers (drive);
}

static void idetape_write_header (ide_drive_t *drive, int locate_eod)
{
	idetape_tape_t *tape = drive->driver_data;

#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: %s: writing tape header\n", tape->name);
#endif
	if (!tape->onstream || tape->raw)
		return;
	tape->update_frame_cntr++;
	__idetape_write_header(drive, 5, 5);
	__idetape_write_header(drive, 0xbae, 5); /* 2990 */
	if (locate_eod) {
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk(KERN_INFO "ide-tape: %s: locating back to eod frame addr %d\n", tape->name, tape->eod_frame_addr);
#endif
		idetape_position_tape(drive, tape->eod_frame_addr, 0, 0);
	}
}

static ssize_t idetape_chrdev_write (struct file *file, const char *buf,
				     size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_dentry->d_inode;
	ide_drive_t *drive = get_drive_ptr (inode->i_rdev);
	idetape_tape_t *tape = drive->driver_data;
	ssize_t retval, actually_written = 0;
	int position;

	if (ppos != &file->f_pos) {
		/* "A request was outside the capabilities of the device." */
		return -ENXIO;
	}

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 3)
		printk (KERN_INFO "ide-tape: Reached idetape_chrdev_write, count %Zd\n", count);
#endif /* IDETAPE_DEBUG_LOG */

	if (tape->onstream) {
		if (count != tape->tape_block_size) {
			printk(KERN_ERR "ide-tape: %s: chrdev_write: use %d bytes as block size (%Zd used)\n",
					tape->name, tape->tape_block_size, count);
			return -EINVAL;
		}
		/*
		 * Check if we reach the end of the tape. Just assume the whole pipeline
		 * is filled with write requests!
		 */
		if (tape->first_frame_position + tape->nr_stages >= tape->capacity - OS_EW)  {
#if ONSTREAM_DEBUG
			printk(KERN_INFO, "chrdev_write: Write truncated at EOM early warning");
#endif
			if (tape->chrdev_direction == idetape_direction_write)
				idetape_write_release(inode);
			return -ENOSPC;
		}
	}

	if (tape->chrdev_direction != idetape_direction_write) {	/* Initialize write operation */
		if (tape->chrdev_direction == idetape_direction_read)
			idetape_discard_read_pipeline (drive, 1);
#if IDETAPE_DEBUG_BUGS
		if (tape->merge_stage || tape->merge_stage_size) {
			printk (KERN_ERR "ide-tape: merge_stage_size should be 0 now\n");
			tape->merge_stage_size = 0;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		if ((tape->merge_stage = __idetape_kmalloc_stage (tape, 0, 0)) == NULL)
			return -ENOMEM;
		tape->chrdev_direction = idetape_direction_write;
		idetape_init_merge_stage (tape);

		if (tape->onstream) {
			position = idetape_read_position(drive);
			if (position <= OS_DATA_STARTFRAME1) {
				tape->logical_blk_num = 0;
				tape->wrt_pass_cntr++;
#if ONSTREAM_DEBUG
				if (tape->debug_level >= 2)
					printk(KERN_INFO "ide-tape: %s: logical block num 0, setting eod to %d\n", tape->name, OS_DATA_STARTFRAME1);
				if (tape->debug_level >= 2)
					printk(KERN_INFO "ide-tape: %s: allocating new write pass counter %d\n", tape->name, tape->wrt_pass_cntr);
#endif
				tape->filemark_cnt = 0;
				tape->eod_frame_addr = OS_DATA_STARTFRAME1;
				tape->first_mark_addr = tape->last_mark_addr = -1;
				idetape_write_header(drive, 1);
			}
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 2)
				printk(KERN_INFO "ide-tape: %s: positioning tape to eod at %d\n", tape->name, tape->eod_frame_addr);
#endif
			position = idetape_read_position(drive);
			if (position != tape->eod_frame_addr)
				idetape_position_tape(drive, tape->eod_frame_addr, 0, 0);
#if ONSTREAM_DEBUG
			if (tape->debug_level >= 2)
				printk(KERN_INFO "ide-tape: %s: first_frame_position %d\n", tape->name, tape->first_frame_position);
#endif
		}

		/*
		 *	Issue a write 0 command to ensure that DSC handshake
		 *	is switched from completion mode to buffer available
		 *	mode.
		 */
		retval = idetape_queue_rw_tail (drive, IDETAPE_WRITE_RQ, 0, tape->merge_stage->bh);
		if (retval < 0) {
			kfree (tape->merge_stage);
			tape->merge_stage = NULL;
			tape->chrdev_direction = idetape_direction_none;
			return retval;
		}
#if ONSTREAM_DEBUG
		if (tape->debug_level >= 2)
			printk("ide-tape: first_frame_position %d\n", tape->first_frame_position);
#endif
	}
	if (count == 0)
		return (0);
	if (tape->restart_speed_control_req)
		idetape_restart_speed_control(drive);
	if (tape->merge_stage_size) {
#if IDETAPE_DEBUG_BUGS
		if (tape->merge_stage_size >= tape->stage_size) {
			printk (KERN_ERR "ide-tape: bug: merge buffer too big\n");
			tape->merge_stage_size = 0;
		}
#endif /* IDETAPE_DEBUG_BUGS */
		actually_written = IDE_MIN (tape->stage_size - tape->merge_stage_size, count);
		idetape_copy_stage_from_user (tape, tape->merge_stage, buf, actually_written);
		buf += actually_written;
		tape->merge_stage_size += actually_written;
		count -= actually_written;

		if (tape->merge_stage_size == tape->stage_size) {
			tape->merge_stage_size = 0;
			retval = idetape_add_chrdev_write_request (drive, tape->capabilities.ctl);
			if (retval <= 0)
				return (retval);
		}
	}
	while (count >= tape->stage_size) {
		idetape_copy_stage_from_user (tape, tape->merge_stage, buf, tape->stage_size);
		buf += tape->stage_size;
		count -= tape->stage_size;
		retval = idetape_add_chrdev_write_request (drive, tape->capabilities.ctl);
		actually_written += tape->stage_size;
		if (retval <= 0)
			return (retval);
	}
	if (count) {
		actually_written+=count;
		idetape_copy_stage_from_user (tape, tape->merge_stage, buf, count);
		tape->merge_stage_size += count;
	}
	return (actually_written);
}

static int idetape_write_filemark (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int last_mark_addr;
	idetape_pc_t pc;

	if (!tape->onstream) {
		idetape_create_write_filemark_cmd(drive, &pc, 1);	/* Write a filemark */
		if (idetape_queue_pc_tail (drive, &pc)) {
			printk (KERN_ERR "ide-tape: Couldn't write a filemark\n");
			return -EIO;
		}
	} else if (!tape->raw) {
		last_mark_addr = idetape_read_position(drive);
		tape->merge_stage = __idetape_kmalloc_stage (tape, 1, 0);
		if (tape->merge_stage != NULL) {
			idetape_init_stage(drive, tape->merge_stage, OS_FRAME_TYPE_MARKER, tape->logical_blk_num);
			idetape_pad_zeros (drive, tape->stage_size);
			tape->logical_blk_num++;
			__idetape_kfree_stage (tape->merge_stage);
			tape->merge_stage = NULL;
		}
		if (tape->filemark_cnt)
			idetape_update_last_marker(drive, tape->last_mark_addr, last_mark_addr);
		tape->last_mark_addr = last_mark_addr;
		if (tape->filemark_cnt++ == 0)
			tape->first_mark_addr = last_mark_addr;
	}
	return 0;
}

static void idetape_write_eod (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

	if (!tape->onstream || tape->raw)
		return;
	tape->merge_stage = __idetape_kmalloc_stage (tape, 1, 0);
	if (tape->merge_stage != NULL) {
		tape->eod_frame_addr = idetape_read_position(drive);
		idetape_init_stage(drive, tape->merge_stage, OS_FRAME_TYPE_EOD, tape->logical_blk_num);
		idetape_pad_zeros (drive, tape->stage_size);
		__idetape_kfree_stage (tape->merge_stage);
		tape->merge_stage = NULL;
	}
	return;
}

int idetape_seek_logical_blk (ide_drive_t *drive, int logical_blk_num)
{
	idetape_tape_t *tape = drive->driver_data;
	int estimated_address = logical_blk_num + 20;
	int retries = 0;
	int speed_control;

	speed_control = tape->speed_control;
	tape->speed_control = 0;
	if (logical_blk_num < 0)
		logical_blk_num = 0;
	if (idetape_get_logical_blk(drive, logical_blk_num, 10, 1))
		goto ok;
	while (++retries < 10) {
		idetape_discard_read_pipeline(drive, 0);
		idetape_position_tape(drive, estimated_address, 0, 0);
		if (idetape_get_logical_blk(drive, logical_blk_num, 10, 1))
			goto ok;
		if (!idetape_get_logical_blk(drive, -1, 10, 1))
			goto error;
		if (tape->logical_blk_num < logical_blk_num)
			estimated_address += logical_blk_num - tape->logical_blk_num;
		else
			break;
	}
error:
	tape->speed_control = speed_control;
	tape->restart_speed_control_req = 1;
	printk(KERN_INFO "ide-tape: %s: couldn't seek to logical block %d (at %d), %d retries\n", tape->name, logical_blk_num, tape->logical_blk_num, retries);
	return -EIO;
ok:
	tape->speed_control = speed_control;
	tape->restart_speed_control_req = 1;
	return 0;
}

/*
 *	idetape_mtioctop is called from idetape_chrdev_ioctl when
 *	the general mtio MTIOCTOP ioctl is requested.
 *
 *	We currently support the following mtio.h operations:
 *
 *	MTFSF	-	Space over mt_count filemarks in the positive direction.
 *			The tape is positioned after the last spaced filemark.
 *
 *	MTFSFM	-	Same as MTFSF, but the tape is positioned before the
 *			last filemark.
 *
 *	MTBSF	-	Steps background over mt_count filemarks, tape is
 *			positioned before the last filemark.
 *
 *	MTBSFM	-	Like MTBSF, only tape is positioned after the last filemark.
 *
 *	Note:
 *
 *		MTBSF and MTBSFM are not supported when the tape doesn't
 *		supports spacing over filemarks in the reverse direction.
 *		In this case, MTFSFM is also usually not supported (it is
 *		supported in the rare case in which we crossed the filemark
 *		during our read-ahead pipelined operation mode).
 *		
 *	MTWEOF	-	Writes mt_count filemarks. Tape is positioned after
 *			the last written filemark.
 *
 *	MTREW	-	Rewinds tape.
 *
 *	MTLOAD	-	Loads the tape.
 *
 *	MTOFFL	-	Puts the tape drive "Offline": Rewinds the tape and
 *	MTUNLOAD	prevents further access until the media is replaced.
 *
 *	MTNOP	-	Flushes tape buffers.
 *
 *	MTRETEN	-	Retension media. This typically consists of one end
 *			to end pass on the media.
 *
 *	MTEOM	-	Moves to the end of recorded data.
 *
 *	MTERASE	-	Erases tape.
 *
 *	MTSETBLK - 	Sets the user block size to mt_count bytes. If
 *			mt_count is 0, we will attempt to autodetect
 *			the block size.
 *
 *	MTSEEK	-	Positions the tape in a specific block number, where
 *			each block is assumed to contain which user_block_size
 *			bytes.
 *
 *	MTSETPART - 	Switches to another tape partition.
 *
 *	MTLOCK - 	Locks the tape door.
 *
 *	MTUNLOCK - 	Unlocks the tape door.
 *
 *	The following commands are currently not supported:
 *
 *	MTFSS, MTBSS, MTWSM, MTSETDENSITY,
 *	MTSETDRVBUFFER, MT_ST_BOOLEANS, MT_ST_WRITE_THRESHOLD.
 */
static int idetape_mtioctop (ide_drive_t *drive,short mt_op,int mt_count)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	int i,retval;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 1)
		printk (KERN_INFO "ide-tape: Handling MTIOCTOP ioctl: mt_op=%d, mt_count=%d\n",mt_op,mt_count);
#endif /* IDETAPE_DEBUG_LOG */
	/*
	 *	Commands which need our pipelined read-ahead stages.
	 */
	switch (mt_op) {
		case MTFSF:
		case MTFSFM:
		case MTBSF:
		case MTBSFM:
			if (!mt_count)
				return (0);
			return (idetape_space_over_filemarks (drive,mt_op,mt_count));
		default:
			break;
	}
	switch (mt_op) {
		case MTWEOF:
			idetape_discard_read_pipeline (drive, 1);
			for (i = 0; i < mt_count; i++) {
				retval = idetape_write_filemark(drive);
				if (retval) return retval;
			}
			return (0);
		case MTREW:
			idetape_discard_read_pipeline (drive, 0);
			if (idetape_rewind_tape(drive))
				return -EIO;
			if (tape->onstream && !tape->raw)
				return idetape_position_tape(drive, OS_DATA_STARTFRAME1, 0, 0);
			return 0;
		case MTLOAD:
			idetape_discard_read_pipeline (drive, 0);
			idetape_create_load_unload_cmd (drive, &pc, IDETAPE_LU_LOAD_MASK);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTUNLOAD:
		case MTOFFL:
			idetape_discard_read_pipeline (drive, 0);
			idetape_create_load_unload_cmd (drive, &pc,!IDETAPE_LU_LOAD_MASK);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTNOP:
			idetape_discard_read_pipeline (drive, 0);
			return (idetape_flush_tape_buffers (drive));
		case MTRETEN:
			idetape_discard_read_pipeline (drive, 0);
			idetape_create_load_unload_cmd (drive, &pc,IDETAPE_LU_RETENSION_MASK | IDETAPE_LU_LOAD_MASK);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTEOM:
			if (tape->onstream) {
#if ONSTREAM_DEBUG
				if (tape->debug_level >= 2)
					printk(KERN_INFO "ide-tape: %s: positioning tape to eod at %d\n", tape->name, tape->eod_frame_addr);
#endif
				idetape_position_tape(drive, tape->eod_frame_addr, 0, 0);
				if (!idetape_get_logical_blk(drive, -1, 10, 0))
					return -EIO;
				if (tape->first_stage->aux->frame_type != OS_FRAME_TYPE_EOD)
					return -EIO;
				return 0;
			}
			idetape_create_space_cmd (&pc, 0, IDETAPE_SPACE_TO_EOD);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTERASE:
			if (tape->onstream) {
				tape->eod_frame_addr = OS_DATA_STARTFRAME1;
				tape->logical_blk_num = 0;
				tape->first_mark_addr = tape->last_mark_addr = -1;
				idetape_position_tape(drive, tape->eod_frame_addr, 0, 0);
				idetape_write_eod(drive);
				idetape_flush_tape_buffers (drive);
				idetape_write_header(drive, 0);
				/*
				 * write filler frames to the unused frames...
				 * REMOVE WHEN going to LIN4 application type...
				 */
				idetape_write_filler(drive, OS_DATA_STARTFRAME1 - 10, 10);
				idetape_write_filler(drive, OS_DATA_ENDFRAME1, 10);
				idetape_flush_tape_buffers (drive);
				(void) idetape_rewind_tape (drive);
				return 0;
			}
			(void) idetape_rewind_tape (drive);
			idetape_create_erase_cmd (&pc);
			return (idetape_queue_pc_tail (drive, &pc));
		case MTSETBLK:
			if (tape->onstream) {
				if (mt_count != tape->tape_block_size) {
					printk(KERN_INFO "ide-tape: %s: MTSETBLK %d -- only %d bytes block size supported\n", tape->name, mt_count, tape->tape_block_size);
					return -EINVAL;
				}
				return 0;
			}
			if (mt_count) {
				if (mt_count < tape->tape_block_size || mt_count % tape->tape_block_size)
					return -EIO;
				tape->user_bs_factor = mt_count / tape->tape_block_size;
				clear_bit (IDETAPE_DETECT_BS, &tape->flags);
			} else
				set_bit (IDETAPE_DETECT_BS, &tape->flags);
			return 0;
		case MTSEEK:
			if (!tape->onstream || tape->raw) {
				idetape_discard_read_pipeline (drive, 0);
				return idetape_position_tape (drive, mt_count * tape->user_bs_factor, tape->partition, 0);
			}
			return idetape_seek_logical_blk(drive, mt_count);
		case MTSETPART:
			idetape_discard_read_pipeline (drive, 0);
			if (tape->onstream)
				return -EIO;
			return (idetape_position_tape (drive, 0, mt_count, 0));
		case MTFSR:
		case MTBSR:
			if (tape->onstream) {
				if (!idetape_get_logical_blk(drive, -1, 10, 0))
					return -EIO;
				if (mt_op == MTFSR)
					return idetape_seek_logical_blk(drive, tape->logical_blk_num + mt_count);
				else {
					idetape_discard_read_pipeline (drive, 0);
					return idetape_seek_logical_blk(drive, tape->logical_blk_num - mt_count);
				}
			}
		case MTLOCK:
			if (!idetape_create_prevent_cmd(drive, &pc, 1))
				return 0;
			retval = idetape_queue_pc_tail (drive, &pc);
			if (retval) return retval;
			tape->door_locked = DOOR_EXPLICITLY_LOCKED;
			return 0;
		case MTUNLOCK:
			if (!idetape_create_prevent_cmd(drive, &pc, 0))
				return 0;
			retval = idetape_queue_pc_tail (drive, &pc);
			if (retval) return retval;
			tape->door_locked = DOOR_UNLOCKED;
			return 0;
		default:
			printk (KERN_ERR "ide-tape: MTIO operation %d not supported\n",mt_op);
			return (-EIO);
	}
}

/*
 *	Our character device ioctls.
 *
 *	General mtio.h magnetic io commands are supported here, and not in
 *	the corresponding block interface.
 *
 *	The following ioctls are supported:
 *
 *	MTIOCTOP -	Refer to idetape_mtioctop for detailed description.
 *
 *	MTIOCGET - 	The mt_dsreg field in the returned mtget structure
 *			will be set to (user block size in bytes <<
 *			MT_ST_BLKSIZE_SHIFT) & MT_ST_BLKSIZE_MASK.
 *
 *			The mt_blkno is set to the current user block number.
 *			The other mtget fields are not supported.
 *
 *	MTIOCPOS -	The current tape "block position" is returned. We
 *			assume that each block contains user_block_size
 *			bytes.
 *
 *	Our own ide-tape ioctls are supported on both interfaces.
 */
static int idetape_chrdev_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	ide_drive_t *drive = get_drive_ptr (inode->i_rdev);
	idetape_tape_t *tape = drive->driver_data;
	struct mtop mtop;
	struct mtget mtget;
	struct mtpos mtpos;
	int block_offset = 0, position = tape->first_frame_position;

#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 3)
		printk (KERN_INFO "ide-tape: Reached idetape_chrdev_ioctl, cmd=%u\n",cmd);
#endif /* IDETAPE_DEBUG_LOG */

	tape->restart_speed_control_req = 1;
	if (tape->chrdev_direction == idetape_direction_write) {
		idetape_empty_write_pipeline (drive);
		idetape_flush_tape_buffers (drive);
	}
	if (cmd == MTIOCGET || cmd == MTIOCPOS) {
		block_offset = idetape_pipeline_size (drive) / (tape->tape_block_size * tape->user_bs_factor);
		if ((position = idetape_read_position(drive)) < 0)
			return -EIO;
	}
	switch (cmd) {
		case MTIOCTOP:
			if (copy_from_user ((char *) &mtop, (char *) arg, sizeof (struct mtop)))
				return -EFAULT;
			return (idetape_mtioctop (drive,mtop.mt_op,mtop.mt_count));
		case MTIOCGET:
			memset (&mtget, 0, sizeof (struct mtget));
			mtget.mt_type = MT_ISSCSI2;
			if (!tape->onstream || tape->raw)
				mtget.mt_blkno = position / tape->user_bs_factor - block_offset;
			else {
				if (!idetape_get_logical_blk(drive, -1, 10, 0))
					mtget.mt_blkno = -1;
				else
					mtget.mt_blkno = tape->logical_blk_num;
			}
			mtget.mt_dsreg = ((tape->tape_block_size * tape->user_bs_factor) << MT_ST_BLKSIZE_SHIFT) & MT_ST_BLKSIZE_MASK;
			if (tape->onstream) {
				mtget.mt_gstat |= GMT_ONLINE(0xffffffff);
				if (tape->first_stage && tape->first_stage->aux->frame_type == OS_FRAME_TYPE_EOD)
					mtget.mt_gstat |= GMT_EOD(0xffffffff);
				if (position <= OS_DATA_STARTFRAME1)
					mtget.mt_gstat |= GMT_BOT(0xffffffff);
			}
			if (copy_to_user ((char *) arg,(char *) &mtget, sizeof (struct mtget)))
				return -EFAULT;
			return 0;
		case MTIOCPOS:
			if (tape->onstream && !tape->raw) {
				if (!idetape_get_logical_blk(drive, -1, 10, 0))
					return -EIO;
				mtpos.mt_blkno = tape->logical_blk_num;
			} else
				mtpos.mt_blkno = position / tape->user_bs_factor - block_offset;
			if (copy_to_user ((char *) arg,(char *) &mtpos, sizeof (struct mtpos)))
				return -EFAULT;
			return 0;
		default:
			if (tape->chrdev_direction == idetape_direction_read)
				idetape_discard_read_pipeline (drive, 1);
			return (idetape_blkdev_ioctl (drive,inode,file,cmd,arg));
	}
}

static int __idetape_analyze_headers (ide_drive_t *drive, int block)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_stage_t *stage;
	os_header_t *header;
	os_aux_t *aux;

	if (!tape->onstream || tape->raw) {
		tape->header_ok = tape->linux_media = 1;
		return 1;
	}
	tape->header_ok = tape->linux_media = 0;
	tape->update_frame_cntr = 0;
	tape->wrt_pass_cntr = 0;
	tape->eod_frame_addr = OS_DATA_STARTFRAME1;
	tape->first_mark_addr = tape->last_mark_addr = -1;
	stage = __idetape_kmalloc_stage (tape, 0, 0);
	if (stage == NULL)
		return 0;
#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: %s: reading header\n", tape->name);
#endif
	idetape_position_tape(drive, block, 0, 0);
	if (!idetape_queue_rw_tail (drive, IDETAPE_READ_RQ, 1, stage->bh)) {
		printk(KERN_INFO "ide-tape: %s: couldn't read header frame\n", tape->name);
		__idetape_kfree_stage (stage);
		return 0;
	}
	header = (os_header_t *) stage->bh->b_data;
	aux = stage->aux;
	if (strncmp(header->ident_str, "ADR_SEQ", 7) != 0) {
		printk(KERN_INFO "ide-tape: %s: invalid header identification string\n", tape->name);
		__idetape_kfree_stage (stage);
		return 0;
	}
	if (header->major_rev != 1 || (header->minor_rev > OS_ADR_MINREV))
		printk(KERN_INFO "ide-tape: warning: revision %d.%d detected (up to 1.%d supported)\n", header->major_rev, header->minor_rev, OS_ADR_MINREV);
	if (header->par_num != 1)
		printk(KERN_INFO "ide-tape: warning: %d partitions defined, only one supported\n", header->par_num);
	tape->wrt_pass_cntr = ntohs(header->partition.wrt_pass_cntr);
	tape->eod_frame_addr = ntohl(header->partition.eod_frame_addr);
	tape->filemark_cnt = ntohl(aux->filemark_cnt);
	tape->first_mark_addr = ntohl(aux->next_mark_addr);
	tape->last_mark_addr = ntohl(aux->last_mark_addr);
	tape->update_frame_cntr = ntohl(aux->update_frame_cntr);
	memcpy(tape->application_sig, aux->application_sig, 4);
	tape->application_sig[4] = 0;
	if (memcmp(tape->application_sig, "LIN", 3) == 0) {
		tape->linux_media = 1;
		tape->linux_media_version = tape->application_sig[3] - '0';
		if (tape->linux_media_version != 3)
			printk(KERN_INFO "ide-tape: %s: Linux media version %d detected (current 3)\n",
					 tape->name, tape->linux_media_version);
	} else {
		printk(KERN_INFO "ide-tape: %s: non Linux media detected (%s)\n", tape->name, tape->application_sig);
		tape->linux_media = 0;
	}
#if ONSTREAM_DEBUG
	if (tape->debug_level >= 2)
		printk(KERN_INFO "ide-tape: %s: detected write pass counter %d, eod frame addr %d\n", tape->name, tape->wrt_pass_cntr, tape->eod_frame_addr);
#endif
	__idetape_kfree_stage (stage);
	return 1;
}

static int idetape_analyze_headers (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int position, block;

	if (!tape->onstream || tape->raw) {
		tape->header_ok = tape->linux_media = 1;
		return 1;
	}
	tape->header_ok = tape->linux_media = 0;
	position = idetape_read_position(drive);
	for (block = 5; block < 10; block++)
		if (__idetape_analyze_headers(drive, block))
			goto ok;
	for (block = 0xbae; block < 0xbb3; block++) /* 2990 - 2994 */
		if (__idetape_analyze_headers(drive, block))
			goto ok;
	printk(KERN_ERR "ide-tape: %s: failed to find valid ADRL header\n", tape->name);
	return 0;
ok:
	if (position < OS_DATA_STARTFRAME1)
		position = OS_DATA_STARTFRAME1;
	idetape_position_tape(drive, position, 0, 0);
	tape->header_ok = 1;
	return 1;
}

/*
 *	Our character device open function.
 */
static int idetape_chrdev_open (struct inode *inode, struct file *filp)
{
	ide_drive_t *drive;
	idetape_tape_t *tape;
	idetape_pc_t pc;
	unsigned int minor=MINOR (inode->i_rdev);
			
#if IDETAPE_DEBUG_LOG
	printk (KERN_INFO "ide-tape: Reached idetape_chrdev_open\n");
#endif /* IDETAPE_DEBUG_LOG */
	
	if ((drive = get_drive_ptr (inode->i_rdev)) == NULL)
		return -ENXIO;
	tape = drive->driver_data;

	if (test_and_set_bit (IDETAPE_BUSY, &tape->flags))
		return -EBUSY;
	MOD_INC_USE_COUNT;
	if (!tape->onstream) {	
		idetape_read_position(drive);
		if (!test_bit (IDETAPE_ADDRESS_VALID, &tape->flags))
			(void) idetape_rewind_tape (drive);
	} else {
		if (minor & 64) {
			tape->tape_block_size = tape->stage_size = 32768 + 512;
			tape->raw = 1;
		} else {
			tape->tape_block_size = tape->stage_size = 32768;
			tape->raw = 0;
		}
                idetape_onstream_mode_sense_tape_parameter_page(drive, tape->debug_level);
	}
	if (idetape_wait_ready(drive, 60 * HZ)) {
		clear_bit(IDETAPE_BUSY, &tape->flags);
		printk(KERN_ERR "ide-tape: %s: drive not ready\n", tape->name);
		MOD_DEC_USE_COUNT;
		return -EBUSY;
	}
	idetape_read_position(drive);
	MOD_DEC_USE_COUNT;
	clear_bit (IDETAPE_PIPELINE_ERROR, &tape->flags);

	if (tape->chrdev_direction == idetape_direction_none) {
		MOD_INC_USE_COUNT;
		if (idetape_create_prevent_cmd(drive, &pc, 1)) {
			if (!idetape_queue_pc_tail (drive, &pc)) {
				if (tape->door_locked != DOOR_EXPLICITLY_LOCKED)
					tape->door_locked = DOOR_LOCKED;
			}
		}
		idetape_analyze_headers(drive);
	}
	tape->max_frames = tape->cur_frames = tape->req_buffer_fill = 0;
	idetape_restart_speed_control(drive);
	tape->restart_speed_control_req = 0;
	return 0;
}

static void idetape_write_release (struct inode *inode)
{
	ide_drive_t *drive = get_drive_ptr (inode->i_rdev);
	idetape_tape_t *tape = drive->driver_data;
	unsigned int minor=MINOR (inode->i_rdev);

	idetape_empty_write_pipeline (drive);
	tape->merge_stage = __idetape_kmalloc_stage (tape, 1, 0);
	if (tape->merge_stage != NULL) {
		idetape_pad_zeros (drive, tape->tape_block_size * (tape->user_bs_factor - 1));
		__idetape_kfree_stage (tape->merge_stage);
		tape->merge_stage = NULL;
	}
	idetape_write_filemark(drive);
	idetape_write_eod(drive);
	idetape_flush_tape_buffers (drive);
	idetape_write_header(drive, minor >= 128);
	idetape_flush_tape_buffers (drive);

	return;
}

/*
 *	Our character device release function.
 */
static int idetape_chrdev_release (struct inode *inode, struct file *filp)
{
	ide_drive_t *drive = get_drive_ptr (inode->i_rdev);
	idetape_tape_t *tape;
	idetape_pc_t pc;
	unsigned int minor=MINOR (inode->i_rdev);

	lock_kernel();
	tape = drive->driver_data;
#if IDETAPE_DEBUG_LOG
	if (tape->debug_level >= 3)
		printk (KERN_INFO "ide-tape: Reached idetape_chrdev_release\n");
#endif /* IDETAPE_DEBUG_LOG */

	if (tape->chrdev_direction == idetape_direction_write) {
		idetape_write_release(inode);
	}
	if (tape->chrdev_direction == idetape_direction_read) {
		if (minor < 128)
			idetape_discard_read_pipeline (drive, 1);
		else
			idetape_wait_for_pipeline (drive);
	}
	if (tape->cache_stage != NULL) {
		__idetape_kfree_stage (tape->cache_stage);
		tape->cache_stage = NULL;
	}
	if (minor < 128)
		(void) idetape_rewind_tape (drive);
	if (tape->chrdev_direction == idetape_direction_none) {
		if (tape->door_locked != DOOR_EXPLICITLY_LOCKED) {
			if (idetape_create_prevent_cmd(drive, &pc, 0))
				if (!idetape_queue_pc_tail (drive, &pc))
					tape->door_locked = DOOR_UNLOCKED;
		}
		MOD_DEC_USE_COUNT;
	}
	clear_bit (IDETAPE_BUSY, &tape->flags);
	unlock_kernel();
	return 0;
}

/*
 *	idetape_identify_device is called to check the contents of the
 *	ATAPI IDENTIFY command results. We return:
 *
 *	1	If the tape can be supported by us, based on the information
 *		we have so far.
 *
 *	0 	If this tape driver is not currently supported by us.
 */
static int idetape_identify_device (ide_drive_t *drive,struct hd_driveid *id)
{
	struct idetape_id_gcw gcw;
#if IDETAPE_DEBUG_INFO
	unsigned short mask,i;
#endif /* IDETAPE_DEBUG_INFO */

	if (!id)
		return 0;

	*((unsigned short *) &gcw) = id->config;

#if IDETAPE_DEBUG_INFO
	printk (KERN_INFO "ide-tape: Dumping ATAPI Identify Device tape parameters\n");
	printk (KERN_INFO "ide-tape: Protocol Type: ");
	switch (gcw.protocol) {
		case 0: case 1: printk (KERN_INFO "ATA\n");break;
		case 2:	printk (KERN_INFO "ATAPI\n");break;
		case 3: printk (KERN_INFO "Reserved (Unknown to ide-tape)\n");break;
	}
	printk (KERN_INFO "ide-tape: Device Type: %x - ",gcw.device_type);	
	switch (gcw.device_type) {
		case 0: printk (KERN_INFO "Direct-access Device\n");break;
		case 1: printk (KERN_INFO "Streaming Tape Device\n");break;
		case 2: case 3: case 4: printk (KERN_INFO "Reserved\n");break;
		case 5: printk (KERN_INFO "CD-ROM Device\n");break;
		case 6: printk (KERN_INFO "Reserved\n");
		case 7: printk (KERN_INFO "Optical memory Device\n");break;
		case 0x1f: printk (KERN_INFO "Unknown or no Device type\n");break;
		default: printk (KERN_INFO "Reserved\n");
	}
	printk (KERN_INFO "ide-tape: Removable: %s",gcw.removable ? "Yes\n":"No\n");	
	printk (KERN_INFO "ide-tape: Command Packet DRQ Type: ");
	switch (gcw.drq_type) {
		case 0: printk (KERN_INFO "Microprocessor DRQ\n");break;
		case 1: printk (KERN_INFO "Interrupt DRQ\n");break;
		case 2: printk (KERN_INFO "Accelerated DRQ\n");break;
		case 3: printk (KERN_INFO "Reserved\n");break;
	}
	printk (KERN_INFO "ide-tape: Command Packet Size: ");
	switch (gcw.packet_size) {
		case 0: printk (KERN_INFO "12 bytes\n");break;
		case 1: printk (KERN_INFO "16 bytes\n");break;
		default: printk (KERN_INFO "Reserved\n");break;
	}
	printk (KERN_INFO "ide-tape: Model: %.40s\n",id->model);
	printk (KERN_INFO "ide-tape: Firmware Revision: %.8s\n",id->fw_rev);
	printk (KERN_INFO "ide-tape: Serial Number: %.20s\n",id->serial_no);
	printk (KERN_INFO "ide-tape: Write buffer size: %d bytes\n",id->buf_size*512);
	printk (KERN_INFO "ide-tape: DMA: %s",id->capability & 0x01 ? "Yes\n":"No\n");
	printk (KERN_INFO "ide-tape: LBA: %s",id->capability & 0x02 ? "Yes\n":"No\n");
	printk (KERN_INFO "ide-tape: IORDY can be disabled: %s",id->capability & 0x04 ? "Yes\n":"No\n");
	printk (KERN_INFO "ide-tape: IORDY supported: %s",id->capability & 0x08 ? "Yes\n":"Unknown\n");
	printk (KERN_INFO "ide-tape: ATAPI overlap supported: %s",id->capability & 0x20 ? "Yes\n":"No\n");
	printk (KERN_INFO "ide-tape: PIO Cycle Timing Category: %d\n",id->tPIO);
	printk (KERN_INFO "ide-tape: DMA Cycle Timing Category: %d\n",id->tDMA);
	printk (KERN_INFO "ide-tape: Single Word DMA supported modes: ");
	for (i=0,mask=1;i<8;i++,mask=mask << 1) {
		if (id->dma_1word & mask)
			printk (KERN_INFO "%d ",i);
		if (id->dma_1word & (mask << 8))
			printk (KERN_INFO "(active) ");
	}
	printk (KERN_INFO "\n");
	printk (KERN_INFO "ide-tape: Multi Word DMA supported modes: ");
	for (i=0,mask=1;i<8;i++,mask=mask << 1) {
		if (id->dma_mword & mask)
			printk (KERN_INFO "%d ",i);
		if (id->dma_mword & (mask << 8))
			printk (KERN_INFO "(active) ");
	}
	printk (KERN_INFO "\n");
	if (id->field_valid & 0x0002) {
		printk (KERN_INFO "ide-tape: Enhanced PIO Modes: %s\n",id->eide_pio_modes & 1 ? "Mode 3":"None");
		printk (KERN_INFO "ide-tape: Minimum Multi-word DMA cycle per word: ");
		if (id->eide_dma_min == 0)
			printk (KERN_INFO "Not supported\n");
		else
			printk (KERN_INFO "%d ns\n",id->eide_dma_min);

		printk (KERN_INFO "ide-tape: Manufacturer\'s Recommended Multi-word cycle: ");
		if (id->eide_dma_time == 0)
			printk (KERN_INFO "Not supported\n");
		else
			printk (KERN_INFO "%d ns\n",id->eide_dma_time);

		printk (KERN_INFO "ide-tape: Minimum PIO cycle without IORDY: ");
		if (id->eide_pio == 0)
			printk (KERN_INFO "Not supported\n");
		else
			printk (KERN_INFO "%d ns\n",id->eide_pio);

		printk (KERN_INFO "ide-tape: Minimum PIO cycle with IORDY: ");
		if (id->eide_pio_iordy == 0)
			printk (KERN_INFO "Not supported\n");
		else
			printk (KERN_INFO "%d ns\n",id->eide_pio_iordy);
		
	} else
		printk (KERN_INFO "ide-tape: According to the device, fields 64-70 are not valid.\n");
#endif /* IDETAPE_DEBUG_INFO */

	/* Check that we can support this device */

	if (gcw.protocol !=2 )
		printk (KERN_ERR "ide-tape: Protocol is not ATAPI\n");
	else if (gcw.device_type != 1)
		printk (KERN_ERR "ide-tape: Device type is not set to tape\n");
	else if (!gcw.removable)
		printk (KERN_ERR "ide-tape: The removable flag is not set\n");
	else if (gcw.packet_size != 0) {
		printk (KERN_ERR "ide-tape: Packet size is not 12 bytes long\n");
		if (gcw.packet_size == 1)
			printk (KERN_ERR "ide-tape: Sorry, padding to 16 bytes is still not supported\n");
	} else
		return 1;
	return 0;
}

/*
 * Notify vendor ID to the OnStream tape drive
 */
static void idetape_onstream_set_vendor (ide_drive_t *drive, char *vendor)
{
	idetape_pc_t pc;
	idetape_mode_parameter_header_t *header;

	idetape_create_mode_select_cmd(&pc, sizeof(*header) + 8);
	pc.buffer[0] = 3 + 8;	/* Mode Data Length */
	pc.buffer[1] = 0;	/* Medium Type - ignoring */
	pc.buffer[2] = 0;	/* Reserved */
	pc.buffer[3] = 0;	/* Block Descriptor Length */
	pc.buffer[4 + 0] = 0x36 | (1 << 7);
	pc.buffer[4 + 1] = 6;
	pc.buffer[4 + 2] = vendor[0];
	pc.buffer[4 + 3] = vendor[1];
	pc.buffer[4 + 4] = vendor[2];
	pc.buffer[4 + 5] = vendor[3];
	pc.buffer[4 + 6] = 0;
	pc.buffer[4 + 7] = 0;
	if (idetape_queue_pc_tail (drive, &pc))
		printk (KERN_ERR "ide-tape: Couldn't set vendor name to %s\n", vendor);

}

/*
 * Various unused OnStream commands
 */
#if ONSTREAM_DEBUG
static void idetape_onstream_set_retries (ide_drive_t *drive, int retries)
{
	idetape_pc_t pc;

	idetape_create_mode_select_cmd(&pc, sizeof(idetape_mode_parameter_header_t) + 4);
	pc.buffer[0] = 3 + 4;
	pc.buffer[1] = 0;	/* Medium Type - ignoring */
	pc.buffer[2] = 0;	/* Reserved */
	pc.buffer[3] = 0;	/* Block Descriptor Length */
	pc.buffer[4 + 0] = 0x2f | (1 << 7);
	pc.buffer[4 + 1] = 2;
	pc.buffer[4 + 2] = 4;
	pc.buffer[4 + 3] = retries;
	if (idetape_queue_pc_tail (drive, &pc))
		printk (KERN_ERR "ide-tape: Couldn't set retries to %d\n", retries);
}
#endif

/*
 * Configure 32.5KB block size.
 */
static void idetape_onstream_configure_block_size (ide_drive_t *drive)
{
	idetape_pc_t pc;
	idetape_mode_parameter_header_t *header;
	idetape_block_size_page_t *bs;

	/*
	 * Get the current block size from the block size mode page
	 */
	idetape_create_mode_sense_cmd (&pc, IDETAPE_BLOCK_SIZE_PAGE);
	if (idetape_queue_pc_tail (drive, &pc))
		printk (KERN_ERR "ide-tape: can't get tape block size mode page\n");
	header = (idetape_mode_parameter_header_t *) pc.buffer;
	bs = (idetape_block_size_page_t *) (pc.buffer + sizeof(idetape_mode_parameter_header_t) + header->bdl);

#if IDETAPE_DEBUG_INFO
	printk(KERN_INFO "ide-tape: 32KB play back: %s\n", bs->play32 ? "Yes" : "No");
	printk(KERN_INFO "ide-tape: 32.5KB play back: %s\n", bs->play32_5 ? "Yes" : "No");
	printk(KERN_INFO "ide-tape: 32KB record: %s\n", bs->record32 ? "Yes" : "No");
	printk(KERN_INFO "ide-tape: 32.5KB record: %s\n", bs->record32_5 ? "Yes" : "No");
#endif /* IDETAPE_DEBUG_INFO */

	/*
	 * Configure default auto columns mode, 32.5KB block size
	 */ 
	bs->one = 1;
	bs->play32 = 0;
	bs->play32_5 = 1;
	bs->record32 = 0;
	bs->record32_5 = 1;
	idetape_create_mode_select_cmd(&pc, sizeof(*header) + sizeof(*bs));
	if (idetape_queue_pc_tail (drive, &pc))
		printk (KERN_ERR "ide-tape: Couldn't set tape block size mode page\n");

#if ONSTREAM_DEBUG
	/*
	 * In debug mode, we want to see as many errors as possible
	 * to test the error recovery mechanism.
	 */
	idetape_onstream_set_retries(drive, 0);
#endif
}

/*
 * Use INQUIRY to get the firmware revision
 */
static void idetape_get_inquiry_results (ide_drive_t *drive)
{
	char *r;
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	idetape_inquiry_result_t *inquiry;
	
	idetape_create_inquiry_cmd(&pc);
	if (idetape_queue_pc_tail (drive, &pc)) {
		printk (KERN_ERR "ide-tape: %s: can't get INQUIRY results\n", tape->name);
		return;
	}
	inquiry = (idetape_inquiry_result_t *) pc.buffer;
	memcpy(tape->vendor_id, inquiry->vendor_id, 8);
	memcpy(tape->product_id, inquiry->product_id, 16);
	memcpy(tape->firmware_revision, inquiry->revision_level, 4);
	ide_fixstring(tape->vendor_id, 10, 0);
	ide_fixstring(tape->product_id, 18, 0);
	ide_fixstring(tape->firmware_revision, 6, 0);
	r = tape->firmware_revision;
	if (*(r + 1) == '.')
		tape->firmware_revision_num = (*r - '0') * 100 + (*(r + 2) - '0') * 10 + *(r + 3) - '0';
	else if (tape->onstream)
		tape->firmware_revision_num = (*r - '0') * 100 + (*(r + 1) - '0') * 10 + *(r + 2) - '0';
	printk(KERN_INFO "ide-tape: %s <-> %s: %s %s rev %s\n", drive->name, tape->name, tape->vendor_id, tape->product_id, tape->firmware_revision);
}

/*
 * Configure the OnStream ATAPI tape drive for default operation
 */
static void idetape_configure_onstream (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

	if (tape->firmware_revision_num < 105) {
		printk(KERN_INFO "ide-tape: %s: Old OnStream firmware revision detected (%s)\n", tape->name, tape->firmware_revision);
		printk(KERN_INFO "ide-tape: %s: An upgrade to version 1.05 or above is recommended\n", tape->name);
	}

	/*
	 * Configure 32.5KB (data+aux) block size.
	 */
	idetape_onstream_configure_block_size(drive);

	/*
	 * Set vendor name to 'LIN3' for "Linux support version 3".
	 */
	idetape_onstream_set_vendor(drive, "LIN3");
}

/*
 *	idetape_get_mode_sense_parameters asks the tape about its various
 *	parameters. This may work for other drives to???
 */
static void idetape_onstream_mode_sense_tape_parameter_page(ide_drive_t *drive, int debug)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	idetape_mode_parameter_header_t *header;
	onstream_tape_paramtr_page_t *prm;
	
	idetape_create_mode_sense_cmd (&pc, IDETAPE_PARAMTR_PAGE);
	if (idetape_queue_pc_tail (drive, &pc)) {
		printk (KERN_ERR "ide-tape: Can't get tape parameters page - probably no tape inserted in onstream drive\n");
		return;
	}
	header = (idetape_mode_parameter_header_t *) pc.buffer;
	prm = (onstream_tape_paramtr_page_t *) (pc.buffer + sizeof(idetape_mode_parameter_header_t) + header->bdl);

        tape->capacity = ntohs(prm->segtrk) * ntohs(prm->trks);
        if (debug) {
	    printk (KERN_INFO "ide-tape: %s <-> %s: Tape length %dMB (%d frames/track, %d tracks = %d blocks, density: %dKbpi)\n",
               drive->name, tape->name, tape->capacity/32, ntohs(prm->segtrk), ntohs(prm->trks), tape->capacity, prm->density);
        }

        return;
}

/*
 *	idetape_get_mode_sense_results asks the tape about its various
 *	parameters. In particular, we will adjust our data transfer buffer
 *	size to the recommended value as returned by the tape.
 */
static void idetape_get_mode_sense_results (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	idetape_mode_parameter_header_t *header;
	idetape_capabilities_page_t *capabilities;
	
	idetape_create_mode_sense_cmd (&pc, IDETAPE_CAPABILITIES_PAGE);
	if (idetape_queue_pc_tail (drive, &pc)) {
		printk (KERN_ERR "ide-tape: Can't get tape parameters - assuming some default values\n");
		tape->tape_block_size = 512; tape->capabilities.ctl = 52;
		tape->capabilities.speed = 450; tape->capabilities.buffer_size = 6 * 52;
		return;
	}
	header = (idetape_mode_parameter_header_t *) pc.buffer;
	capabilities = (idetape_capabilities_page_t *) (pc.buffer + sizeof(idetape_mode_parameter_header_t) + header->bdl);

	capabilities->max_speed = ntohs (capabilities->max_speed);
	capabilities->ctl = ntohs (capabilities->ctl);
	capabilities->speed = ntohs (capabilities->speed);
	capabilities->buffer_size = ntohs (capabilities->buffer_size);

	if (!capabilities->speed) {
		printk(KERN_INFO "ide-tape: %s: overriding capabilities->speed (assuming 650KB/sec)\n", drive->name);
		capabilities->speed = 650;
	}
	if (!capabilities->max_speed) {
		printk(KERN_INFO "ide-tape: %s: overriding capabilities->max_speed (assuming 650KB/sec)\n", drive->name);
		capabilities->max_speed = 650;
	}

	tape->capabilities = *capabilities;		/* Save us a copy */
	if (capabilities->blk512)
		tape->tape_block_size = 512;
	else if (capabilities->blk1024)
		tape->tape_block_size = 1024;
	else if (tape->onstream && capabilities->blk32768)
		tape->tape_block_size = 32768;

#if IDETAPE_DEBUG_INFO
	printk (KERN_INFO "ide-tape: Dumping the results of the MODE SENSE packet command\n");
	printk (KERN_INFO "ide-tape: Mode Parameter Header:\n");
	printk (KERN_INFO "ide-tape: Mode Data Length - %d\n",header->mode_data_length);
	printk (KERN_INFO "ide-tape: Medium Type - %d\n",header->medium_type);
	printk (KERN_INFO "ide-tape: Device Specific Parameter - %d\n",header->dsp);
	printk (KERN_INFO "ide-tape: Block Descriptor Length - %d\n",header->bdl);
	
	printk (KERN_INFO "ide-tape: Capabilities and Mechanical Status Page:\n");
	printk (KERN_INFO "ide-tape: Page code - %d\n",capabilities->page_code);
	printk (KERN_INFO "ide-tape: Page length - %d\n",capabilities->page_length);
	printk (KERN_INFO "ide-tape: Read only - %s\n",capabilities->ro ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports reverse space - %s\n",capabilities->sprev ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports erase initiated formatting - %s\n",capabilities->efmt ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports QFA two Partition format - %s\n",capabilities->qfa ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports locking the medium - %s\n",capabilities->lock ? "Yes":"No");
	printk (KERN_INFO "ide-tape: The volume is currently locked - %s\n",capabilities->locked ? "Yes":"No");
	printk (KERN_INFO "ide-tape: The device defaults in the prevent state - %s\n",capabilities->prevent ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports ejecting the medium - %s\n",capabilities->eject ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports error correction - %s\n",capabilities->ecc ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports data compression - %s\n",capabilities->cmprs ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports 512 bytes block size - %s\n",capabilities->blk512 ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports 1024 bytes block size - %s\n",capabilities->blk1024 ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Supports 32768 bytes block size / Restricted byte count for PIO transfers - %s\n",capabilities->blk32768 ? "Yes":"No");
	printk (KERN_INFO "ide-tape: Maximum supported speed in KBps - %d\n",capabilities->max_speed);
	printk (KERN_INFO "ide-tape: Continuous transfer limits in blocks - %d\n",capabilities->ctl);
	printk (KERN_INFO "ide-tape: Current speed in KBps - %d\n",capabilities->speed);	
	printk (KERN_INFO "ide-tape: Buffer size - %d\n",capabilities->buffer_size*512);
#endif /* IDETAPE_DEBUG_INFO */
}

/*
 *	ide_get_blocksize_from_block_descriptor does a mode sense page 0 with block descriptor
 *	and if it succeeds sets the tape block size with the reported value
 */
static void idetape_get_blocksize_from_block_descriptor(ide_drive_t *drive)
{

	idetape_tape_t *tape = drive->driver_data;
	idetape_pc_t pc;
	idetape_mode_parameter_header_t *header;
	idetape_parameter_block_descriptor_t *block_descrp;
	
	idetape_create_mode_sense_cmd (&pc, IDETAPE_BLOCK_DESCRIPTOR);
	if (idetape_queue_pc_tail (drive, &pc)) {
		printk (KERN_ERR "ide-tape: Can't get block descriptor\n");
		if (tape->tape_block_size == 0) {
			printk(KERN_WARNING "ide-tape: Cannot deal with zero block size, assume 32k\n");
			tape->tape_block_size =  32768;
		}
		return;
	}
	header = (idetape_mode_parameter_header_t *) pc.buffer;
	block_descrp = (idetape_parameter_block_descriptor_t *) (pc.buffer + sizeof(idetape_mode_parameter_header_t));
	tape->tape_block_size =( block_descrp->length[0]<<16) + (block_descrp->length[1]<<8) + block_descrp->length[2];
#if IDETAPE_DEBUG_INFO
	printk (KERN_INFO "ide-tape: Adjusted block size - %d\n", tape->tape_block_size);
#endif /* IDETAPE_DEBUG_INFO */
}
static void idetape_add_settings (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;

/*
 *			drive	setting name	read/write	ioctl	ioctl		data type	min			max			mul_factor			div_factor			data pointer				set function
 */
	ide_add_setting(drive,	"buffer",	SETTING_READ,	-1,	-1,		TYPE_SHORT,	0,			0xffff,			1,				2,				&tape->capabilities.buffer_size,	NULL);
	ide_add_setting(drive,	"pipeline_min",	SETTING_RW,	-1,	-1,		TYPE_INT,	2,			0xffff,			tape->stage_size / 1024,	1,				&tape->min_pipeline,			NULL);
	ide_add_setting(drive,	"pipeline",	SETTING_RW,	-1,	-1,		TYPE_INT,	2,			0xffff,			tape->stage_size / 1024,	1,				&tape->max_stages,			NULL);
	ide_add_setting(drive,	"pipeline_max",	SETTING_RW,	-1,	-1,		TYPE_INT,	2,			0xffff,			tape->stage_size / 1024,	1,				&tape->max_pipeline,			NULL);
	ide_add_setting(drive,	"pipeline_used",SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			tape->stage_size / 1024,	1,				&tape->nr_stages,			NULL);
	ide_add_setting(drive,	"pipeline_pending",SETTING_READ,-1,	-1,		TYPE_INT,	0,			0xffff,			tape->stage_size / 1024,	1,				&tape->nr_pending_stages,		NULL);
	ide_add_setting(drive,	"speed",	SETTING_READ,	-1,	-1,		TYPE_SHORT,	0,			0xffff,			1,				1,				&tape->capabilities.speed,		NULL);
	ide_add_setting(drive,	"stage",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1024,				&tape->stage_size,			NULL);
	ide_add_setting(drive,	"tdsc",		SETTING_RW,	-1,	-1,		TYPE_INT,	IDETAPE_DSC_RW_MIN,	IDETAPE_DSC_RW_MAX,	1000,				HZ,				&tape->best_dsc_rw_frequency,		NULL);
	ide_add_setting(drive,	"dsc_overlap",	SETTING_RW,	-1,	-1,		TYPE_BYTE,	0,			1,			1,				1,				&drive->dsc_overlap,			NULL);
	ide_add_setting(drive,	"pipeline_head_speed_c",SETTING_READ,	-1,	-1,	TYPE_INT,	0,			0xffff,			1,				1,				&tape->controlled_pipeline_head_speed,	NULL);
	ide_add_setting(drive,	"pipeline_head_speed_u",SETTING_READ,	-1,	-1,	TYPE_INT,	0,			0xffff,			1,				1,				&tape->uncontrolled_pipeline_head_speed,	NULL);
	ide_add_setting(drive,	"avg_speed",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->avg_speed,		NULL);
	ide_add_setting(drive,	"debug_level",SETTING_RW,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->debug_level,		NULL);
	if (tape->onstream) {
		ide_add_setting(drive,	"cur_frames",	SETTING_READ,	-1,	-1,		TYPE_SHORT,	0,			0xffff,			1,				1,				&tape->cur_frames,		NULL);
		ide_add_setting(drive,	"max_frames",	SETTING_READ,	-1,	-1,		TYPE_SHORT,	0,			0xffff,			1,				1,				&tape->max_frames,		NULL);
		ide_add_setting(drive,	"insert_speed",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->insert_speed,		NULL);
		ide_add_setting(drive,	"speed_control",SETTING_RW,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->speed_control,		NULL);
		ide_add_setting(drive,	"tape_still_time",SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->tape_still_time,		NULL);
		ide_add_setting(drive,	"max_insert_speed",SETTING_RW,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->max_insert_speed,	NULL);
		ide_add_setting(drive,	"insert_size",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->insert_size,		NULL);
		ide_add_setting(drive,	"capacity",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->capacity,		NULL);
		ide_add_setting(drive,	"first_frame",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->first_frame_position,		NULL);
		ide_add_setting(drive,	"logical_blk",	SETTING_READ,	-1,	-1,		TYPE_INT,	0,			0xffff,			1,				1,				&tape->logical_blk_num,		NULL);
	}
}

/*
 *	ide_setup is called to:
 *
 *		1.	Initialize our various state variables.
 *		2.	Ask the tape for its capabilities.
 *		3.	Allocate a buffer which will be used for data
 *			transfer. The buffer size is chosen based on
 *			the recommendation which we received in step (2).
 *
 *	Note that at this point ide.c already assigned us an irq, so that
 *	we can queue requests here and wait for their completion.
 */
static void idetape_setup (ide_drive_t *drive, idetape_tape_t *tape, int minor)
{
	unsigned long t1, tmid, tn, t;
	int speed;
	struct idetape_id_gcw gcw;
	int stage_size;
	struct sysinfo si;

	memset (tape, 0, sizeof (idetape_tape_t));
	spin_lock_init(&tape->spinlock);
	drive->driver_data = tape;
	drive->ready_stat = 0;			/* An ATAPI device ignores DRDY */
	if (strstr(drive->id->model, "OnStream DI-"))
		tape->onstream = 1;
	drive->dsc_overlap = 1;
#ifdef CONFIG_BLK_DEV_IDEPCI
	if (!tape->onstream && HWIF(drive)->pci_dev != NULL) {
		/*
		 * These two ide-pci host adapters appear to need DSC overlap disabled.
		 * This probably needs further analysis.
		 */
		if ((HWIF(drive)->pci_dev->device == PCI_DEVICE_ID_ARTOP_ATP850UF) ||
		    (HWIF(drive)->pci_dev->device == PCI_DEVICE_ID_TTI_HPT343)) {
			printk(KERN_INFO "ide-tape: %s: disabling DSC overlap\n", tape->name);
		    	drive->dsc_overlap = 0;
		}
	}
#endif /* CONFIG_BLK_DEV_IDEPCI */
	tape->drive = drive;
	tape->minor = minor;
	tape->name[0] = 'h'; tape->name[1] = 't'; tape->name[2] = '0' + minor;
	tape->chrdev_direction = idetape_direction_none;
	tape->pc = tape->pc_stack;
	tape->max_insert_speed = 10000;
	tape->speed_control = 1;
	*((unsigned short *) &gcw) = drive->id->config;
	if (gcw.drq_type == 1)
		set_bit(IDETAPE_DRQ_INTERRUPT, &tape->flags);

	tape->min_pipeline = tape->max_pipeline = tape->max_stages = 10;
	
	idetape_get_inquiry_results(drive);
	idetape_get_mode_sense_results(drive);
	idetape_get_blocksize_from_block_descriptor(drive);
	if (tape->onstream) {
		idetape_onstream_mode_sense_tape_parameter_page(drive, 1);
		idetape_configure_onstream(drive);
	}
	tape->user_bs_factor = 1;
	tape->stage_size = tape->capabilities.ctl * tape->tape_block_size;
	while (tape->stage_size > 0xffff) {
		printk (KERN_NOTICE "ide-tape: decreasing stage size\n");
		tape->capabilities.ctl /= 2;
		tape->stage_size = tape->capabilities.ctl * tape->tape_block_size;
	}
	stage_size = tape->stage_size;
	if (tape->onstream)
		stage_size = 32768 + 512;
	tape->pages_per_stage = stage_size / PAGE_SIZE;
	if (stage_size % PAGE_SIZE) {
		tape->pages_per_stage++;
		tape->excess_bh_size = PAGE_SIZE - stage_size % PAGE_SIZE;
	}

	/*
	 *	Select the "best" DSC read/write polling frequency
	 *	and pipeline size.
	 */
	speed = IDE_MAX (tape->capabilities.speed, tape->capabilities.max_speed);

	tape->max_stages = speed * 1000 * 10 / tape->stage_size;

	/*
	 * 	Limit memory use for pipeline to 10% of physical memory
	 */
	si_meminfo(&si);
	if ( tape->max_stages * tape->stage_size > si.totalram * si.mem_unit / 10)
		tape->max_stages = si.totalram * si.mem_unit / (10 * tape->stage_size);
	tape->min_pipeline = tape->max_stages;
	tape->max_pipeline = tape->max_stages * 2;

	t1 = (tape->stage_size * HZ) / (speed * 1000);
	tmid = (tape->capabilities.buffer_size * 32 * HZ) / (speed * 125);
	tn = (IDETAPE_FIFO_THRESHOLD * tape->stage_size * HZ) / (speed * 1000);

	if (tape->max_stages)
		t = tn;
	else
		t = t1;

	/*
	 *	Ensure that the number we got makes sense; limit
	 *	it within IDETAPE_DSC_RW_MIN and IDETAPE_DSC_RW_MAX.
	 */
	tape->best_dsc_rw_frequency = IDE_MAX (IDE_MIN (t, IDETAPE_DSC_RW_MAX), IDETAPE_DSC_RW_MIN);
	printk (KERN_INFO "ide-tape: %s <-> %s: %dKBps, %d*%dkB buffer, %dkB pipeline, %lums tDSC%s\n",
		drive->name, tape->name, tape->capabilities.speed, (tape->capabilities.buffer_size * 512) / tape->stage_size,
		tape->stage_size / 1024, tape->max_stages * tape->stage_size / 1024,
		tape->best_dsc_rw_frequency * 1000 / HZ, drive->using_dma ? ", DMA":"");

	idetape_add_settings(drive);
}

static int idetape_cleanup (ide_drive_t *drive)
{
	idetape_tape_t *tape = drive->driver_data;
	int minor = tape->minor;
	unsigned long flags;

	save_flags (flags);	/* all CPUs (overkill?) */
	cli();			/* all CPUs (overkill?) */
	if (test_bit (IDETAPE_BUSY, &tape->flags) || tape->first_stage != NULL || tape->merge_stage_size || drive->usage) {
		restore_flags(flags);	/* all CPUs (overkill?) */
		return 1;
	}
	idetape_chrdevs[minor].drive = NULL;
	restore_flags (flags);	/* all CPUs (overkill?) */
	DRIVER(drive)->busy = 0;
	(void) ide_unregister_subdriver (drive);
	drive->driver_data = NULL;
	devfs_unregister (tape->de_r);
	devfs_unregister (tape->de_n);
	kfree (tape);
	for (minor = 0; minor < MAX_HWIFS * MAX_DRIVES; minor++)
		if (idetape_chrdevs[minor].drive != NULL)
			return 0;
	devfs_unregister_chrdev (IDETAPE_MAJOR, "ht");
	idetape_chrdev_present = 0;
	return 0;
}

#ifdef CONFIG_PROC_FS

static int proc_idetape_read_name
	(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	ide_drive_t	*drive = (ide_drive_t *) data;
	idetape_tape_t	*tape = drive->driver_data;
	char		*out = page;
	int		len;

	len = sprintf(out, "%s\n", tape->name);
	PROC_IDE_READ_RETURN(page, start, off, count, eof, len);
}

static ide_proc_entry_t idetape_proc[] = {
	{ "name",	S_IFREG|S_IRUGO,	proc_idetape_read_name,	NULL },
	{ NULL, 0, NULL, NULL }
};

#else

#define	idetape_proc	NULL

#endif

static int idetape_reinit (ide_drive_t *drive)
{
	return 0;
}

/*
 *	IDE subdriver functions, registered with ide.c
 */
static ide_driver_t idetape_driver = {
	name:			"ide-tape",
	version:		IDETAPE_VERSION,
	media:			ide_tape,
	busy:			1,
	supports_dma:		1,
	supports_dsc_overlap: 	1,
	cleanup:		idetape_cleanup,
	do_request:		idetape_do_request,
	end_request:		idetape_end_request,
	ioctl:			idetape_blkdev_ioctl,
	open:			idetape_blkdev_open,
	release:		idetape_blkdev_release,
	media_change:		NULL,
	revalidate:		NULL,
	pre_reset:		idetape_pre_reset,
	capacity:		NULL,
	proc:			idetape_proc,
	driver_reinit:		idetape_reinit,
};

int idetape_init (void);
static ide_module_t idetape_module = {
	IDE_DRIVER_MODULE,
	idetape_init,
	&idetape_driver,
	NULL
};

/*
 *	Our character device supporting functions, passed to register_chrdev.
 */
static struct file_operations idetape_fops = {
	owner:		THIS_MODULE,
	read:		idetape_chrdev_read,
	write:		idetape_chrdev_write,
	ioctl:		idetape_chrdev_ioctl,
	open:		idetape_chrdev_open,
	release:	idetape_chrdev_release,
};

MODULE_DESCRIPTION("ATAPI Streaming TAPE Driver");
MODULE_LICENSE("GPL");

static void __exit idetape_exit (void)
{
	ide_drive_t *drive;
	int minor;

	for (minor = 0; minor < MAX_HWIFS * MAX_DRIVES; minor++) {
		drive = idetape_chrdevs[minor].drive;
		if (drive != NULL && idetape_cleanup (drive))
		printk (KERN_ERR "ide-tape: %s: cleanup_module() called while still busy\n", drive->name);
	}
	ide_unregister_module(&idetape_module);
}

/*
 *	idetape_init will register the driver for each tape.
 */
int idetape_init (void)
{
	ide_drive_t *drive;
	idetape_tape_t *tape;
	int minor, failed = 0, supported = 0;

	MOD_INC_USE_COUNT;
#if ONSTREAM_DEBUG
        printk(KERN_INFO "ide-tape: MOD_INC_USE_COUNT in idetape_init\n");
#endif
	if (!idetape_chrdev_present)
		for (minor = 0; minor < MAX_HWIFS * MAX_DRIVES; minor++ )
			idetape_chrdevs[minor].drive = NULL;

	if ((drive = ide_scan_devices (ide_tape, idetape_driver.name, NULL, failed++)) == NULL) {
		ide_register_module (&idetape_module);
		MOD_DEC_USE_COUNT;
#if ONSTREAM_DEBUG
		printk(KERN_INFO "ide-tape: MOD_DEC_USE_COUNT in idetape_init\n");
#endif
		return 0;
	}
	if (!idetape_chrdev_present &&
	    devfs_register_chrdev (IDETAPE_MAJOR, "ht", &idetape_fops)) {
		printk (KERN_ERR "ide-tape: Failed to register character device interface\n");
		MOD_DEC_USE_COUNT;
#if ONSTREAM_DEBUG
		printk(KERN_INFO "ide-tape: MOD_DEC_USE_COUNT in idetape_init\n");
#endif
		return -EBUSY;
	}
	do {
		if (!idetape_identify_device (drive, drive->id)) {
			printk (KERN_ERR "ide-tape: %s: not supported by this version of ide-tape\n", drive->name);
			continue;
		}
		if (drive->scsi) {
			if (strstr(drive->id->model, "OnStream DI-")) {
				printk("ide-tape: ide-scsi emulation is not supported for %s.\n", drive->id->model);
			} else {
				printk("ide-tape: passing drive %s to ide-scsi emulation.\n", drive->name);
				continue;
			}
		}
		tape = (idetape_tape_t *) kmalloc (sizeof (idetape_tape_t), GFP_KERNEL);
		if (tape == NULL) {
			printk (KERN_ERR "ide-tape: %s: Can't allocate a tape structure\n", drive->name);
			continue;
		}
		if (ide_register_subdriver (drive, &idetape_driver, IDE_SUBDRIVER_VERSION)) {
			printk (KERN_ERR "ide-tape: %s: Failed to register the driver with ide.c\n", drive->name);
			kfree (tape);
			continue;
		}
		for (minor = 0; idetape_chrdevs[minor].drive != NULL; minor++);
		idetape_setup (drive, tape, minor);
		idetape_chrdevs[minor].drive = drive;
		tape->de_r =
		    devfs_register (drive->de, "mt", DEVFS_FL_DEFAULT,
				    HWIF(drive)->major, minor,
				    S_IFCHR | S_IRUGO | S_IWUGO,
				    &idetape_fops, NULL);
		tape->de_n =
		    devfs_register (drive->de, "mtn", DEVFS_FL_DEFAULT,
				    HWIF(drive)->major, minor + 128,
				    S_IFCHR | S_IRUGO | S_IWUGO,
				    &idetape_fops, NULL);
		devfs_register_tape (tape->de_r);
		supported++; failed--;
	} while ((drive = ide_scan_devices (ide_tape, idetape_driver.name, NULL, failed++)) != NULL);
	if (!idetape_chrdev_present && !supported) {
		devfs_unregister_chrdev (IDETAPE_MAJOR, "ht");
	} else
		idetape_chrdev_present = 1;
	ide_register_module (&idetape_module);
	MOD_DEC_USE_COUNT;
#if ONSTREAM_DEBUG
	printk(KERN_INFO "ide-tape: MOD_DEC_USE_COUNT in idetape_init\n");
#endif
	return 0;
}

module_init(idetape_init);
module_exit(idetape_exit);
