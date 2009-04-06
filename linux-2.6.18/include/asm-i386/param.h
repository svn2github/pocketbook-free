#ifndef _ASMi386_PARAM_H
#define _ASMi386_PARAM_H

#ifndef HZ
#define HZ 100
#ifdef __KERNEL__
#if HZ == 100
/* X86 is defined to provide userspace with a world where HZ=100
   We have to do this, (x*const)/const2 isnt optimised out because its not
   a null operation as it might overflow.. */
#define hz_to_std(a) (a)
#else
#define hz_to_std(a) ((a)*(100/HZ)+((a)*(100%HZ))/HZ)
#endif
#endif
#endif

#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#ifdef __KERNEL__
# define CLOCKS_PER_SEC	100	/* frequency at which times() counts */
#endif

#endif
