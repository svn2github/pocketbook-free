/*
 * linux/include/asm-arm/arch-s3c2410/time.h
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * History
 *
 * 2002-05-20: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 * 2003-10-02 : kwanghyun la <laputa : nala.la@samsung.com>
 *    - dynamic clock calcurate & recall function
 */

#include <linux/time.h>	/* for mktime() */
#include <linux/rtc.h>  /* struct rtc_time */
#include "cpu_s3c2410.h"

/* copy from linux/arch/arm/kernel/time.c */
#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val)	((val)=((val)&15) + ((val)>>4)*10)
#endif

#ifndef BIN_TO_BCD
#define BIN_TO_BCD(val)	((val)=(((val)/10)<<4) + (val)%10)
#endif

#ifndef RTC_LEAP_YEAR
#define RTC_LEAP_YEAR        2000
#endif

extern spinlock_t rtc_lock;

unsigned long s3c2410_get_rtc_time(void)
{
	unsigned int year, mon, day, hour, min, sec;

	spin_lock_irq(&rtc_lock);
read_rtc_bcd_time:
	year = BCDYEAR & Msk_RTCYEAR;
	mon  = BCDMON  & Msk_RTCMON;
	day  = BCDDAY  & Msk_RTCDAY;
	hour = BCDHOUR & Msk_RTCHOUR;
	min  = BCDMIN  & Msk_RTCMIN;
	sec  = BCDSEC  & Msk_RTCSEC;
	if (sec == 0) {
		/* If BCDSEC is zero, reread all bcd registers.
		   See Section 17.2 READ/WRITE REGISTERS for more info. */
		goto read_rtc_bcd_time;
	}
	spin_unlock_irq(&rtc_lock);

	BCD_TO_BIN(year);
	BCD_TO_BIN(mon);
	BCD_TO_BIN(day);
	BCD_TO_BIN(hour);
	BCD_TO_BIN(min);
	BCD_TO_BIN(sec);

	year += RTC_LEAP_YEAR;

	return (mktime(year, mon, day, hour, min, sec));
}

/* 
 * Copyed from drivers/char/sa1100-rtc.c.
 */
#define epoch			1970

static const unsigned char days_in_mo[] =
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#ifndef is_leap
#define is_leap(year) \
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#endif

/*
 * Converts seconds since 1970-01-01 00:00:00 to Gregorian date.
 */
static void decodetime (unsigned long t, struct rtc_time *tval)
{
	unsigned long days, month, year, rem;

	days = t / 86400;
	rem = t % 86400;
	tval->tm_hour = rem / 3600;
	rem %= 3600;
	tval->tm_min = rem / 60;
	tval->tm_sec = rem % 60;
	tval->tm_wday = (4 + days) % 7;

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

	year = epoch;
	while (days >= (365 + is_leap(year))) {
		unsigned long yg = year + days / 365;
		days -= ((yg - year) * 365
				+ LEAPS_THRU_END_OF (yg - 1)
				- LEAPS_THRU_END_OF (year - 1));
		year = yg;
	}
	tval->tm_year = year - 1900;
	tval->tm_yday = days + 1;

	month = 0;
	if (days >= 31) {
		days -= 31;
		month++;
		if (days >= (28 + is_leap(year))) {
			days -= (28 + is_leap(year));
			month++;
			while (days >= days_in_mo[month]) {
				days -= days_in_mo[month];
				month++;
			}
		}
	}
	tval->tm_mon = month;
	tval->tm_mday = days + 1;
}

int s3c2410_set_rtc(void)
{
	unsigned long current_time = xtime.tv_sec;
	unsigned char year, mon, day, hour, min, sec;
	signed int yeardiff;
	struct rtc_time rtc_tm;

	decodetime(current_time, &rtc_tm);

	yeardiff = (rtc_tm.tm_year + 1900) - RTC_LEAP_YEAR;
	if (yeardiff < 0) {
		/* S3C2410 RTC forces that the year must be higher or 
		   equal than 2000, so initailize it. */
		yeardiff = 0;
	}

	year = (unsigned char) yeardiff;
	mon = rtc_tm.tm_mon + 1; /* tm_mon starts at zero */
	day = rtc_tm.tm_mday;
	hour = rtc_tm.tm_hour;
	min = rtc_tm.tm_min;
	sec = rtc_tm.tm_sec;

	BIN_TO_BCD(sec);
	BIN_TO_BCD(min);
	BIN_TO_BCD(hour);
	BIN_TO_BCD(day);
	BIN_TO_BCD(mon);
	BIN_TO_BCD(year);

	spin_lock_irq(&rtc_lock);
	RTCCON |= RTCCON_EN; 
	BCDSEC  = sec  & Msk_RTCSEC;
	BCDMIN  = min  & Msk_RTCMIN;
	BCDHOUR = hour & Msk_RTCHOUR;
	BCDDAY  = day  & Msk_RTCDAY;
	BCDMON  = mon  & Msk_RTCMON;
	BCDYEAR = year & Msk_RTCYEAR;
	RTCCON &= ~RTCCON_EN;
	spin_unlock_irq(&rtc_lock);

	return 0;
}

static unsigned long s3c2410_gettimeoffset(void)
{
	unsigned long elapsed, usec;
	unsigned long latch;

	/* Use TCNTB4 as LATCH */
	latch = TCNTB4;

	elapsed = latch - TCNTO4;
	usec = (elapsed * tick) / latch;

	return usec;
}

static void s3c2410_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	long flags;

	do_leds();
	do_set_rtc();
	save_flags_cli(flags);
	do_timer(regs);
	restore_flags(flags);
}

/* laputa for auto calcurate tick count register set 031002
 * Unit of 'freq' is khz 
struct timer_counts {		// --
	unsigned int freq;		// --
	unsigned int count;		// --
};						// --
*laputa end 
*/							

/*
 * priod = (prescaler value + 1) * (divider value) * buffer count / PCLK = 10 ms
 *
 * e.g.; PCLK = 50 Mhz
 * 10 ms = (15 + 1) * 2 * 15625 / (50000 * 1000)
 * 15626 = 10 ms * (50000 * 1000) / 2 / (15 + 1)
 *
 * Other values
 *  5156 = 10 ms * ( 16.5 * 1,000,000) / 2 / (15+1)
 *  6250 = 10 ms * ( 20 * 1,000,000) / 2 / (15+1)
 * 10312 = 10 ms * ( 33 * 1,000,000) / 2 / (15+1)
 * 20625 = 10 ms * ( 66 * 1,000,000) / 2 / (15+1)
 * 21875 = 10 ms * ( 70 * 1,000,000) / 2 / (15+1)
 * 23437 = 10 ms * ( 75 * 1,000,000) / 2 / (15+1)
 * 25000 = 10 ms * ( 80 * 1,000,000) / 2 / (15+1)
 * 28125 = 10 ms * ( 90 * 1,000,000) / 2 / (15+1)
 * 31250 = 10 ms * ( 100 * 1,000,000) / 2 / (15+1)
 */

/* laputa for auto calcurate tick count register set 031002
struct timer_counts count_values[] = {
	{  16500,	 5156 },
	{  20000,	 6250 },
	{  33000,	10312 },
	{  50000,	15626 },
	{  66000,	20625 },
	{  70000,	21875 },
	{  75000,	23437 },
	{  80000,	25000 },
	{  90000,	28125 },
	{ 100000,	31250 },
	{      0,	    0 }	
};
*/

//laputa append 031002 for tick count value set calcurate
#define PRESCALE	(15+1)
#define DIVIDER		2
#define TICK_PERIOD	10	 // 10ms -> 1/10ms = 100 clock freq
#define mS			1000	 // 10ms -> 1/10ms = 100 clock freq
// laputa end 


static inline void setup_timer(void)
{
/* laputa for auto calcurate tick count register set 031002
	struct timer_counts *timer_count = count_values; //--
*  laputa end 
*/	
	unsigned long pclk;

	gettimeoffset = s3c2410_gettimeoffset;
	set_rtc = s3c2410_set_rtc;
	xtime.tv_sec = s3c2410_get_rtc_time();

#if 0	
	/* set timer interrupt */
	TCFG0 = (TCFG0_DZONE(0) | TCFG0_PRE1(15) | TCFG0_PRE0(0));
 
	pclk = s3c2410_get_bus_clk(GET_PCLK)/1000;	
	while (timer_count != 0) {
		if (pclk == timer_count->freq) {
			printk("DEBUG: timer count %d\n", timer_count->count);
			TCNTB4 = timer_count->count;
			break;
		}
		timer_count++;
	}

	if (timer_count == 0) {
	    	/* Error, assume that PCLK is 50 Mhz */
		TCNTB4 = 15626;	/* down-counter, maximum value is 65535 (2^16) */
	}
#else
	TCFG0 = (TCFG0_DZONE(0) | TCFG0_PRE1(PRESCALE) | TCFG0_PRE0(0));
	pclk = s3c2410_get_bus_clk(GET_PCLK);	
	TCNTB4 = (unsigned int)(TICK_PERIOD * pclk / (PRESCALE+1) / DIVIDER ) /   mS;
	//laputa test only
	//printk("---> tick count [%d] \n",TCNTB4);
	
#endif

	TCON = (TCON_4_AUTO | TCON_4_UPDATE | COUNT_4_OFF);	
	timer_irq.handler = s3c2410_timer_interrupt;
	setup_arm_irq(IRQ_TIMER4, &timer_irq);
	TCON = (TCON_4_AUTO | COUNT_4_ON);
}

EXPORT_SYMBOL(s3c2410_get_rtc_time);
EXPORT_SYMBOL(s3c2410_set_rtc);
