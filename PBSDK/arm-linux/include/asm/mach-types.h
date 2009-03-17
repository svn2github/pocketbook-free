/*
 * This was automagically generated from mach-types!
 * Do NOT edit
 */

#ifndef __ASM_ARM_MACH_TYPE_H
#define __ASM_ARM_MACH_TYPE_H

#include <linux/config.h>

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

/* see arch/arm/kernel/arch.c for a description of these */
#define MACH_TYPE_EBSA110              0
#define MACH_TYPE_RISCPC               1
#define MACH_TYPE_NEXUSPCI             3
#define MACH_TYPE_EBSA285              4
#define MACH_TYPE_NETWINDER            5
#define MACH_TYPE_CATS                 6
#define MACH_TYPE_TBOX                 7
#define MACH_TYPE_CO285                8
#define MACH_TYPE_CLPS7110             9
#define MACH_TYPE_ARCHIMEDES           10
#define MACH_TYPE_A5K                  11
#define MACH_TYPE_ETOILE               12
#define MACH_TYPE_LACIE_NAS            13
#define MACH_TYPE_CLPS7500             14
#define MACH_TYPE_SHARK                15
#define MACH_TYPE_BRUTUS               16
#define MACH_TYPE_PERSONAL_SERVER      17
#define MACH_TYPE_ITSY                 18
#define MACH_TYPE_L7200                19
#define MACH_TYPE_PLEB                 20
#define MACH_TYPE_INTEGRATOR           21
#define MACH_TYPE_BITSY                22
#define MACH_TYPE_IXP1200              23
#define MACH_TYPE_P720T                24
#define MACH_TYPE_ASSABET              25
#define MACH_TYPE_VICTOR               26
#define MACH_TYPE_LART                 27
#define MACH_TYPE_RANGER               28
#define MACH_TYPE_GRAPHICSCLIENT       29
#define MACH_TYPE_XP860                30
#define MACH_TYPE_CERF                 31
#define MACH_TYPE_NANOENGINE           32
#define MACH_TYPE_FPIC                 33
#define MACH_TYPE_EXTENEX1             34
#define MACH_TYPE_SHERMAN              35
#define MACH_TYPE_ACCELENT_SA          36
#define MACH_TYPE_ACCELENT_L7200       37
#define MACH_TYPE_NETPORT              38
#define MACH_TYPE_PANGOLIN             39
#define MACH_TYPE_YOPY                 40
#define MACH_TYPE_COOLIDGE             41
#define MACH_TYPE_HUW_WEBPANEL         42
#define MACH_TYPE_SPOTME               43
#define MACH_TYPE_FREEBIRD             44
#define MACH_TYPE_TI925                45
#define MACH_TYPE_RISCSTATION          46
#define MACH_TYPE_CAVY                 47
#define MACH_TYPE_JORNADA720           48
#define MACH_TYPE_OMNIMETER            49
#define MACH_TYPE_EDB7211              50
#define MACH_TYPE_CITYGO               51
#define MACH_TYPE_PFS168               52
#define MACH_TYPE_SPOT                 53
#define MACH_TYPE_FLEXANET             54
#define MACH_TYPE_WEBPAL               55
#define MACH_TYPE_LINPDA               56
#define MACH_TYPE_ANAKIN               57
#define MACH_TYPE_MVI                  58
#define MACH_TYPE_JUPITER              59
#define MACH_TYPE_PSIONW               60
#define MACH_TYPE_ALN                  61
#define MACH_TYPE_CAMELOT              62
#define MACH_TYPE_GDS2200              63
#define MACH_TYPE_PSION_SERIES7        64
#define MACH_TYPE_XFILE                65
#define MACH_TYPE_ACCELENT_EP9312      66
#define MACH_TYPE_IC200                67
#define MACH_TYPE_CREDITLART           68
#define MACH_TYPE_HTM                  69
#define MACH_TYPE_IQ80310              70
#define MACH_TYPE_FREEBOT              71
#define MACH_TYPE_ENTEL                72
#define MACH_TYPE_ENP3510              73
#define MACH_TYPE_TRIZEPS              74
#define MACH_TYPE_NESA                 75
#define MACH_TYPE_VENUS                76
#define MACH_TYPE_TARDIS               77
#define MACH_TYPE_MERCURY              78
#define MACH_TYPE_EMPEG                79
#define MACH_TYPE_I80200FCC            80
#define MACH_TYPE_ITT_CPB              81
#define MACH_TYPE_SVC                  82
#define MACH_TYPE_ALPHA2               84
#define MACH_TYPE_ALPHA1               85
#define MACH_TYPE_NETARM               86
#define MACH_TYPE_SIMPAD               87
#define MACH_TYPE_PDA1                 88
#define MACH_TYPE_LUBBOCK              89
#define MACH_TYPE_ANIKO                90
#define MACH_TYPE_CLEP7212             91
#define MACH_TYPE_CS89712              92
#define MACH_TYPE_WEARARM              93
#define MACH_TYPE_POSSIO_PX            94
#define MACH_TYPE_SIDEARM              95
#define MACH_TYPE_STORK                96
#define MACH_TYPE_SHANNON              97
#define MACH_TYPE_ACE                  98
#define MACH_TYPE_BALLYARM             99
#define MACH_TYPE_SIMPUTER             100

#ifdef CONFIG_ARCH_EBSA110
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_EBSA110
# endif
# define machine_is_ebsa110()	(machine_arch_type == MACH_TYPE_EBSA110)
#else
# define machine_is_ebsa110()	(0)
#endif

#ifdef CONFIG_ARCH_RPC
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_RISCPC
# endif
# define machine_is_riscpc()	(machine_arch_type == MACH_TYPE_RISCPC)
#else
# define machine_is_riscpc()	(0)
#endif

#ifdef CONFIG_ARCH_NEXUSPCI
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NEXUSPCI
# endif
# define machine_is_nexuspci()	(machine_arch_type == MACH_TYPE_NEXUSPCI)
#else
# define machine_is_nexuspci()	(0)
#endif

#ifdef CONFIG_ARCH_EBSA285
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_EBSA285
# endif
# define machine_is_ebsa285()	(machine_arch_type == MACH_TYPE_EBSA285)
#else
# define machine_is_ebsa285()	(0)
#endif

#ifdef CONFIG_ARCH_NETWINDER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NETWINDER
# endif
# define machine_is_netwinder()	(machine_arch_type == MACH_TYPE_NETWINDER)
#else
# define machine_is_netwinder()	(0)
#endif

#ifdef CONFIG_ARCH_CATS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CATS
# endif
# define machine_is_cats()	(machine_arch_type == MACH_TYPE_CATS)
#else
# define machine_is_cats()	(0)
#endif

#ifdef CONFIG_ARCH_TBOX
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_TBOX
# endif
# define machine_is_tbox()	(machine_arch_type == MACH_TYPE_TBOX)
#else
# define machine_is_tbox()	(0)
#endif

#ifdef CONFIG_ARCH_CO285
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CO285
# endif
# define machine_is_co285()	(machine_arch_type == MACH_TYPE_CO285)
#else
# define machine_is_co285()	(0)
#endif

#ifdef CONFIG_ARCH_CLPS7110
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CLPS7110
# endif
# define machine_is_clps7110()	(machine_arch_type == MACH_TYPE_CLPS7110)
#else
# define machine_is_clps7110()	(0)
#endif

#ifdef CONFIG_ARCH_ARC
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ARCHIMEDES
# endif
# define machine_is_archimedes()	(machine_arch_type == MACH_TYPE_ARCHIMEDES)
#else
# define machine_is_archimedes()	(0)
#endif

#ifdef CONFIG_ARCH_A5K
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_A5K
# endif
# define machine_is_a5k()	(machine_arch_type == MACH_TYPE_A5K)
#else
# define machine_is_a5k()	(0)
#endif

#ifdef CONFIG_ARCH_ETOILE
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ETOILE
# endif
# define machine_is_etoile()	(machine_arch_type == MACH_TYPE_ETOILE)
#else
# define machine_is_etoile()	(0)
#endif

#ifdef CONFIG_ARCH_LACIE_NAS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_LACIE_NAS
# endif
# define machine_is_lacie_nas()	(machine_arch_type == MACH_TYPE_LACIE_NAS)
#else
# define machine_is_lacie_nas()	(0)
#endif

#ifdef CONFIG_ARCH_CLPS7500
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CLPS7500
# endif
# define machine_is_clps7500()	(machine_arch_type == MACH_TYPE_CLPS7500)
#else
# define machine_is_clps7500()	(0)
#endif

#ifdef CONFIG_ARCH_SHARK
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SHARK
# endif
# define machine_is_shark()	(machine_arch_type == MACH_TYPE_SHARK)
#else
# define machine_is_shark()	(0)
#endif

#ifdef CONFIG_SA1100_BRUTUS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_BRUTUS
# endif
# define machine_is_brutus()	(machine_arch_type == MACH_TYPE_BRUTUS)
#else
# define machine_is_brutus()	(0)
#endif

#ifdef CONFIG_ARCH_PERSONAL_SERVER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PERSONAL_SERVER
# endif
# define machine_is_personal_server()	(machine_arch_type == MACH_TYPE_PERSONAL_SERVER)
#else
# define machine_is_personal_server()	(0)
#endif

#ifdef CONFIG_SA1100_ITSY
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ITSY
# endif
# define machine_is_itsy()	(machine_arch_type == MACH_TYPE_ITSY)
#else
# define machine_is_itsy()	(0)
#endif

#ifdef CONFIG_ARCH_L7200
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_L7200
# endif
# define machine_is_l7200()	(machine_arch_type == MACH_TYPE_L7200)
#else
# define machine_is_l7200()	(0)
#endif

#ifdef CONFIG_SA1100_PLEB
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PLEB
# endif
# define machine_is_pleb()	(machine_arch_type == MACH_TYPE_PLEB)
#else
# define machine_is_pleb()	(0)
#endif

#ifdef CONFIG_ARCH_INTEGRATOR
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_INTEGRATOR
# endif
# define machine_is_integrator()	(machine_arch_type == MACH_TYPE_INTEGRATOR)
#else
# define machine_is_integrator()	(0)
#endif

#ifdef CONFIG_SA1100_BITSY
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_BITSY
# endif
# define machine_is_bitsy()	(machine_arch_type == MACH_TYPE_BITSY)
#else
# define machine_is_bitsy()	(0)
#endif

#ifdef CONFIG_ARCH_IXP1200
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_IXP1200
# endif
# define machine_is_ixp1200()	(machine_arch_type == MACH_TYPE_IXP1200)
#else
# define machine_is_ixp1200()	(0)
#endif

#ifdef CONFIG_ARCH_P720T
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_P720T
# endif
# define machine_is_p720t()	(machine_arch_type == MACH_TYPE_P720T)
#else
# define machine_is_p720t()	(0)
#endif

#ifdef CONFIG_SA1100_ASSABET
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ASSABET
# endif
# define machine_is_assabet()	(machine_arch_type == MACH_TYPE_ASSABET)
#else
# define machine_is_assabet()	(0)
#endif

#ifdef CONFIG_SA1100_VICTOR
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_VICTOR
# endif
# define machine_is_victor()	(machine_arch_type == MACH_TYPE_VICTOR)
#else
# define machine_is_victor()	(0)
#endif

#ifdef CONFIG_SA1100_LART
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_LART
# endif
# define machine_is_lart()	(machine_arch_type == MACH_TYPE_LART)
#else
# define machine_is_lart()	(0)
#endif

#ifdef CONFIG_SA1100_RANGER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_RANGER
# endif
# define machine_is_ranger()	(machine_arch_type == MACH_TYPE_RANGER)
#else
# define machine_is_ranger()	(0)
#endif

#ifdef CONFIG_SA1100_GRAPHICSCLIENT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_GRAPHICSCLIENT
# endif
# define machine_is_graphicsclient()	(machine_arch_type == MACH_TYPE_GRAPHICSCLIENT)
#else
# define machine_is_graphicsclient()	(0)
#endif

#ifdef CONFIG_SA1100_XP860
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_XP860
# endif
# define machine_is_xp860()	(machine_arch_type == MACH_TYPE_XP860)
#else
# define machine_is_xp860()	(0)
#endif

#ifdef CONFIG_SA1100_CERF
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CERF
# endif
# define machine_is_cerf()	(machine_arch_type == MACH_TYPE_CERF)
#else
# define machine_is_cerf()	(0)
#endif

#ifdef CONFIG_SA1100_NANOENGINE
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NANOENGINE
# endif
# define machine_is_nanoengine()	(machine_arch_type == MACH_TYPE_NANOENGINE)
#else
# define machine_is_nanoengine()	(0)
#endif

#ifdef CONFIG_SA1100_FPIC
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_FPIC
# endif
# define machine_is_fpic()	(machine_arch_type == MACH_TYPE_FPIC)
#else
# define machine_is_fpic()	(0)
#endif

#ifdef CONFIG_SA1100_EXTENEX1
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_EXTENEX1
# endif
# define machine_is_extenex1()	(machine_arch_type == MACH_TYPE_EXTENEX1)
#else
# define machine_is_extenex1()	(0)
#endif

#ifdef CONFIG_SA1100_SHERMAN
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SHERMAN
# endif
# define machine_is_sherman()	(machine_arch_type == MACH_TYPE_SHERMAN)
#else
# define machine_is_sherman()	(0)
#endif

#ifdef CONFIG_SA1100_ACCELENT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ACCELENT_SA
# endif
# define machine_is_accelent_sa()	(machine_arch_type == MACH_TYPE_ACCELENT_SA)
#else
# define machine_is_accelent_sa()	(0)
#endif

#ifdef CONFIG_ARCH_L7200_ACCELENT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ACCELENT_L7200
# endif
# define machine_is_accelent_l7200()	(machine_arch_type == MACH_TYPE_ACCELENT_L7200)
#else
# define machine_is_accelent_l7200()	(0)
#endif

#ifdef CONFIG_SA1100_NETPORT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NETPORT
# endif
# define machine_is_netport()	(machine_arch_type == MACH_TYPE_NETPORT)
#else
# define machine_is_netport()	(0)
#endif

#ifdef CONFIG_SA1100_PANGOLIN
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PANGOLIN
# endif
# define machine_is_pangolin()	(machine_arch_type == MACH_TYPE_PANGOLIN)
#else
# define machine_is_pangolin()	(0)
#endif

#ifdef CONFIG_SA1100_YOPY
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_YOPY
# endif
# define machine_is_yopy()	(machine_arch_type == MACH_TYPE_YOPY)
#else
# define machine_is_yopy()	(0)
#endif

#ifdef CONFIG_SA1100_COOLIDGE
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_COOLIDGE
# endif
# define machine_is_coolidge()	(machine_arch_type == MACH_TYPE_COOLIDGE)
#else
# define machine_is_coolidge()	(0)
#endif

#ifdef CONFIG_SA1100_HUW_WEBPANEL
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_HUW_WEBPANEL
# endif
# define machine_is_huw_webpanel()	(machine_arch_type == MACH_TYPE_HUW_WEBPANEL)
#else
# define machine_is_huw_webpanel()	(0)
#endif

#ifdef CONFIG_ARCH_SPOTME
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SPOTME
# endif
# define machine_is_spotme()	(machine_arch_type == MACH_TYPE_SPOTME)
#else
# define machine_is_spotme()	(0)
#endif

#ifdef CONFIG_ARCH_FREEBIRD
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_FREEBIRD
# endif
# define machine_is_freebird()	(machine_arch_type == MACH_TYPE_FREEBIRD)
#else
# define machine_is_freebird()	(0)
#endif

#ifdef CONFIG_ARCH_TI925
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_TI925
# endif
# define machine_is_ti925()	(machine_arch_type == MACH_TYPE_TI925)
#else
# define machine_is_ti925()	(0)
#endif

#ifdef CONFIG_ARCH_RISCSTATION
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_RISCSTATION
# endif
# define machine_is_riscstation()	(machine_arch_type == MACH_TYPE_RISCSTATION)
#else
# define machine_is_riscstation()	(0)
#endif

#ifdef CONFIG_SA1100_CAVY
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CAVY
# endif
# define machine_is_cavy()	(machine_arch_type == MACH_TYPE_CAVY)
#else
# define machine_is_cavy()	(0)
#endif

#ifdef CONFIG_SA1100_JORNADA720
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_JORNADA720
# endif
# define machine_is_jornada720()	(machine_arch_type == MACH_TYPE_JORNADA720)
#else
# define machine_is_jornada720()	(0)
#endif

#ifdef CONFIG_SA1100_OMNIMETER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_OMNIMETER
# endif
# define machine_is_omnimeter()	(machine_arch_type == MACH_TYPE_OMNIMETER)
#else
# define machine_is_omnimeter()	(0)
#endif

#ifdef CONFIG_ARCH_EDB7211
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_EDB7211
# endif
# define machine_is_edb7211()	(machine_arch_type == MACH_TYPE_EDB7211)
#else
# define machine_is_edb7211()	(0)
#endif

#ifdef CONFIG_SA1100_CITYGO
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CITYGO
# endif
# define machine_is_citygo()	(machine_arch_type == MACH_TYPE_CITYGO)
#else
# define machine_is_citygo()	(0)
#endif

#ifdef CONFIG_SA1100_PFS168
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PFS168
# endif
# define machine_is_pfs168()	(machine_arch_type == MACH_TYPE_PFS168)
#else
# define machine_is_pfs168()	(0)
#endif

#ifdef CONFIG_SA1100_SPOT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SPOT
# endif
# define machine_is_spot()	(machine_arch_type == MACH_TYPE_SPOT)
#else
# define machine_is_spot()	(0)
#endif

#ifdef CONFIG_SA1100_FLEXANET
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_FLEXANET
# endif
# define machine_is_flexanet()	(machine_arch_type == MACH_TYPE_FLEXANET)
#else
# define machine_is_flexanet()	(0)
#endif

#ifdef CONFIG_ARCH_WEBPAL
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_WEBPAL
# endif
# define machine_is_webpal()	(machine_arch_type == MACH_TYPE_WEBPAL)
#else
# define machine_is_webpal()	(0)
#endif

#ifdef CONFIG_SA1100_LINPDA
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_LINPDA
# endif
# define machine_is_linpda()	(machine_arch_type == MACH_TYPE_LINPDA)
#else
# define machine_is_linpda()	(0)
#endif

#ifdef CONFIG_ARCH_ANAKIN
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ANAKIN
# endif
# define machine_is_anakin()	(machine_arch_type == MACH_TYPE_ANAKIN)
#else
# define machine_is_anakin()	(0)
#endif

#ifdef CONFIG_SA1100_MVI
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_MVI
# endif
# define machine_is_mvi()	(machine_arch_type == MACH_TYPE_MVI)
#else
# define machine_is_mvi()	(0)
#endif

#ifdef CONFIG_SA1100_JUPITER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_JUPITER
# endif
# define machine_is_jupiter()	(machine_arch_type == MACH_TYPE_JUPITER)
#else
# define machine_is_jupiter()	(0)
#endif

#ifdef CONFIG_ARCH_PSIONW
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PSIONW
# endif
# define machine_is_psionw()	(machine_arch_type == MACH_TYPE_PSIONW)
#else
# define machine_is_psionw()	(0)
#endif

#ifdef CONFIG_SA1100_ALN
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ALN
# endif
# define machine_is_aln()	(machine_arch_type == MACH_TYPE_ALN)
#else
# define machine_is_aln()	(0)
#endif

#ifdef CONFIG_ARCH_CAMELOT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CAMELOT
# endif
# define machine_is_camelot()	(machine_arch_type == MACH_TYPE_CAMELOT)
#else
# define machine_is_camelot()	(0)
#endif

#ifdef CONFIG_SA1100_GDS2200
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_GDS2200
# endif
# define machine_is_gds2200()	(machine_arch_type == MACH_TYPE_GDS2200)
#else
# define machine_is_gds2200()	(0)
#endif

#ifdef CONFIG_SA1100_PSION_SERIES7
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PSION_SERIES7
# endif
# define machine_is_psion_series7()	(machine_arch_type == MACH_TYPE_PSION_SERIES7)
#else
# define machine_is_psion_series7()	(0)
#endif

#ifdef CONFIG_SA1100_XFILE
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_XFILE
# endif
# define machine_is_xfile()	(machine_arch_type == MACH_TYPE_XFILE)
#else
# define machine_is_xfile()	(0)
#endif

#ifdef CONFIG_ARCH_ACCELENT_EP9312
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ACCELENT_EP9312
# endif
# define machine_is_accelent_ep9312()	(machine_arch_type == MACH_TYPE_ACCELENT_EP9312)
#else
# define machine_is_accelent_ep9312()	(0)
#endif

#ifdef CONFIG_ARCH_IC200
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_IC200
# endif
# define machine_is_ic200()	(machine_arch_type == MACH_TYPE_IC200)
#else
# define machine_is_ic200()	(0)
#endif

#ifdef CONFIG_SA1100_CREDITLART
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CREDITLART
# endif
# define machine_is_creditlart()	(machine_arch_type == MACH_TYPE_CREDITLART)
#else
# define machine_is_creditlart()	(0)
#endif

#ifdef CONFIG_SA1100_HTM
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_HTM
# endif
# define machine_is_htm()	(machine_arch_type == MACH_TYPE_HTM)
#else
# define machine_is_htm()	(0)
#endif

#ifdef CONFIG_ARCH_IQ80310
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_IQ80310
# endif
# define machine_is_iq80310()	(machine_arch_type == MACH_TYPE_IQ80310)
#else
# define machine_is_iq80310()	(0)
#endif

#ifdef CONFIG_SA1100_FREEBOT
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_FREEBOT
# endif
# define machine_is_freebot()	(machine_arch_type == MACH_TYPE_FREEBOT)
#else
# define machine_is_freebot()	(0)
#endif

#ifdef CONFIG_ARCH_ENTEL
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ENTEL
# endif
# define machine_is_entel()	(machine_arch_type == MACH_TYPE_ENTEL)
#else
# define machine_is_entel()	(0)
#endif

#ifdef CONFIG_ARCH_ENP3510
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ENP3510
# endif
# define machine_is_enp3510()	(machine_arch_type == MACH_TYPE_ENP3510)
#else
# define machine_is_enp3510()	(0)
#endif

#ifdef CONFIG_SA1100_TRIZEPS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_TRIZEPS
# endif
# define machine_is_trizeps()	(machine_arch_type == MACH_TYPE_TRIZEPS)
#else
# define machine_is_trizeps()	(0)
#endif

#ifdef CONFIG_SA1100_NESA
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NESA
# endif
# define machine_is_nesa()	(machine_arch_type == MACH_TYPE_NESA)
#else
# define machine_is_nesa()	(0)
#endif

#ifdef CONFIG_ARCH_VENUS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_VENUS
# endif
# define machine_is_venus()	(machine_arch_type == MACH_TYPE_VENUS)
#else
# define machine_is_venus()	(0)
#endif

#ifdef CONFIG_ARCH_TARDIS
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_TARDIS
# endif
# define machine_is_tardis()	(machine_arch_type == MACH_TYPE_TARDIS)
#else
# define machine_is_tardis()	(0)
#endif

#ifdef CONFIG_ARCH_MERCURY
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_MERCURY
# endif
# define machine_is_mercury()	(machine_arch_type == MACH_TYPE_MERCURY)
#else
# define machine_is_mercury()	(0)
#endif

#ifdef CONFIG_SA1100_EMPEG
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_EMPEG
# endif
# define machine_is_empeg()	(machine_arch_type == MACH_TYPE_EMPEG)
#else
# define machine_is_empeg()	(0)
#endif

#ifdef CONFIG_ARCH_I80200FCC
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_I80200FCC
# endif
# define machine_is_adi_eb()	(machine_arch_type == MACH_TYPE_I80200FCC)
#else
# define machine_is_adi_eb()	(0)
#endif

#ifdef CONFIG_SA1100_ITT_CPB
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ITT_CPB
# endif
# define machine_is_itt_cpb()	(machine_arch_type == MACH_TYPE_ITT_CPB)
#else
# define machine_is_itt_cpb()	(0)
#endif

#ifdef CONFIG_SA1100_SVC
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SVC
# endif
# define machine_is_svc()	(machine_arch_type == MACH_TYPE_SVC)
#else
# define machine_is_svc()	(0)
#endif

#ifdef CONFIG_SA1100_ALPHA2
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ALPHA2
# endif
# define machine_is_alpha2()	(machine_arch_type == MACH_TYPE_ALPHA2)
#else
# define machine_is_alpha2()	(0)
#endif

#ifdef CONFIG_SA1100_ALPHA1
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ALPHA1
# endif
# define machine_is_alpha1()	(machine_arch_type == MACH_TYPE_ALPHA1)
#else
# define machine_is_alpha1()	(0)
#endif

#ifdef CONFIG_ARCH_NETARM
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_NETARM
# endif
# define machine_is_netarm()	(machine_arch_type == MACH_TYPE_NETARM)
#else
# define machine_is_netarm()	(0)
#endif

#ifdef CONFIG_SA1100_SIMPAD
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SIMPAD
# endif
# define machine_is_simpad()	(machine_arch_type == MACH_TYPE_SIMPAD)
#else
# define machine_is_simpad()	(0)
#endif

#ifdef CONFIG_ARCH_PDA1
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_PDA1
# endif
# define machine_is_pda1()	(machine_arch_type == MACH_TYPE_PDA1)
#else
# define machine_is_pda1()	(0)
#endif

#ifdef CONFIG_ARCH_LUBBOCK
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_LUBBOCK
# endif
# define machine_is_lubbock()	(machine_arch_type == MACH_TYPE_LUBBOCK)
#else
# define machine_is_lubbock()	(0)
#endif

#ifdef CONFIG_ARCH_ANIKO
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ANIKO
# endif
# define machine_is_aniko()	(machine_arch_type == MACH_TYPE_ANIKO)
#else
# define machine_is_aniko()	(0)
#endif

#ifdef CONFIG_ARCH_CLEP7212
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CLEP7212
# endif
# define machine_is_clep7212()	(machine_arch_type == MACH_TYPE_CLEP7212)
#else
# define machine_is_clep7212()	(0)
#endif

#ifdef CONFIG_ARCH_CS89712
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_CS89712
# endif
# define machine_is_cs89712()	(machine_arch_type == MACH_TYPE_CS89712)
#else
# define machine_is_cs89712()	(0)
#endif

#ifdef CONFIG_SA1100_WEARARM
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_WEARARM
# endif
# define machine_is_weararm()	(machine_arch_type == MACH_TYPE_WEARARM)
#else
# define machine_is_weararm()	(0)
#endif

#ifdef CONFIG_SA1100_POSSIO_PX
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_POSSIO_PX
# endif
# define machine_is_possio_px()	(machine_arch_type == MACH_TYPE_POSSIO_PX)
#else
# define machine_is_possio_px()	(0)
#endif

#ifdef CONFIG_SA1100_SIDEARM
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SIDEARM
# endif
# define machine_is_sidearm()	(machine_arch_type == MACH_TYPE_SIDEARM)
#else
# define machine_is_sidearm()	(0)
#endif

#ifdef CONFIG_SA1100_STORK
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_STORK
# endif
# define machine_is_stork()	(machine_arch_type == MACH_TYPE_STORK)
#else
# define machine_is_stork()	(0)
#endif

#ifdef CONFIG_ARCH_SHANNON
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SHANNON
# endif
# define machine_is_shannon()	(machine_arch_type == MACH_TYPE_SHANNON)
#else
# define machine_is_shannon()	(0)
#endif

#ifdef CONFIG_ARCH_ACE
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ACE
# endif
# define machine_is_ace()	(machine_arch_type == MACH_TYPE_ACE)
#else
# define machine_is_ace()	(0)
#endif

#ifdef CONFIG_SA1100_BALLYARM
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_BALLYARM
# endif
# define machine_is_ballyarm()	(machine_arch_type == MACH_TYPE_BALLYARM)
#else
# define machine_is_ballyarm()	(0)
#endif

#ifdef CONFIG_SA1100_SIMPUTER
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_SIMPUTER
# endif
# define machine_is_simputer()	(machine_arch_type == MACH_TYPE_SIMPUTER)
#else
# define machine_is_simputer()	(0)
#endif

/*
 * These have not yet been registered
 */

#ifndef machine_arch_type
#define machine_arch_type	__machine_arch_type
#endif

#endif
