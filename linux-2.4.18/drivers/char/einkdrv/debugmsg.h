// Copyright (c) Sigma Designs, Inc. 2003
// debug message handling for linux kernel modules

#ifndef _DEBUGMSG_H
#define _DEBUGMSG_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>

#ifdef DEBUG
static inline void DebugBreak(){};
#define DEBUGMSG(cond,printf_exp)	((void)((cond)?(printk printf_exp),1:0))
#define ASSERT(exp)					((void)((exp)?1:(printk ("ASSERT failed in file %s at line %d\n", __FILE__,__LINE__))))

#else
#define DEBUGMSG(cond,printf_exp)
#define ASSERT(exp)
#endif

#define RETAILMSG(cond,printf_exp)	((void)((cond)?(printk printf_exp),1:0))
#define RETAILASSERT(exp)			((void)((exp)?1:(printk ("ASSERT failed in file %s at line %d\n", __FILE__,__LINE__))))

#endif

