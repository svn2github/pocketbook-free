/*
 * Automatically generated by make menuconfig: don't edit
 */
#define AUTOCONF_INCLUDED
#define CONFIG_ARM 1
#undef  CONFIG_EISA
#undef  CONFIG_SBUS
#undef  CONFIG_MCA
#define CONFIG_UID16 1
#define CONFIG_RWSEM_GENERIC_SPINLOCK 1
#undef  CONFIG_RWSEM_XCHGADD_ALGORITHM
#undef  CONFIG_GENERIC_BUST_SPINLOCK
#undef  CONFIG_GENERIC_ISA_DMA

/*
 * PVI eBook Reader options
 */
#define CONFIG_PVI_PM1 1
#define CONFIG_ADC 1

/*
 * Code maturity level options
 */
#define CONFIG_EXPERIMENTAL 1
#undef  CONFIG_OBSOLETE

/*
 * Loadable module support
 */
#define CONFIG_MODULES 1
#undef  CONFIG_MODVERSIONS
#define CONFIG_KMOD 1

/*
 * System Type
 */
#undef  CONFIG_ARCH_ANAKIN
#undef  CONFIG_ARCH_ARCA5K
#undef  CONFIG_ARCH_CLPS7500
#undef  CONFIG_ARCH_CLPS711X
#undef  CONFIG_ARCH_CO285
#undef  CONFIG_ARCH_PXA
#undef  CONFIG_ARCH_EBSA110
#undef  CONFIG_ARCH_CAMELOT
#undef  CONFIG_ARCH_FOOTBRIDGE
#undef  CONFIG_ARCH_INTEGRATOR
#undef  CONFIG_ARCH_L7200
#undef  CONFIG_ARCH_MX1ADS
#undef  CONFIG_ARCH_RPC
#undef  CONFIG_ARCH_SA1100
#undef  CONFIG_ARCH_S3C2400
#define CONFIG_ARCH_S3C2410 1
#undef  CONFIG_ARCH_SHARK
#undef  CONFIG_ARCH_LUBBOCK
#undef  CONFIG_ARCH_PXA_IDP
#undef  CONFIG_ARCH_PXA_CERF
#undef  CONFIG_ARCH_PREMIUM
#undef  CONFIG_ARCH_NIPC2
#undef  CONFIG_ARCH_AUTCPU12
#undef  CONFIG_ARCH_CDB89712
#undef  CONFIG_ARCH_CLEP7312
#undef  CONFIG_ARCH_EDB7211
#undef  CONFIG_ARCH_P720T
#undef  CONFIG_ARCH_FORTUNET
#define CONFIG_S3C2410_SMDK 1
#undef  CONFIG_SMDK_AIJI
#define CONFIG_S3C2410_USB 1
#define CONFIG_S3C2410_USB_CHAR 1
#undef  CONFIG_ARCH_ACORN
#undef  CONFIG_FOOTBRIDGE
#undef  CONFIG_FOOTBRIDGE_HOST
#undef  CONFIG_FOOTBRIDGE_ADDIN
#define CONFIG_CPU_32 1
#undef  CONFIG_CPU_26
#undef  CONFIG_CPU_32v3
#define CONFIG_CPU_32v4 1
#undef  CONFIG_CPU_ARM610
#undef  CONFIG_CPU_ARM710
#undef  CONFIG_CPU_ARM720T
#define CONFIG_CPU_ARM920T 1
#define CONFIG_CPU_ARM920_CPU_IDLE 1
#define CONFIG_CPU_ARM920_I_CACHE_ON 1
#define CONFIG_CPU_ARM920_D_CACHE_ON 1
#undef  CONFIG_CPU_ARM920_WRITETHROUGH
#undef  CONFIG_CPU_ARM922T
#undef  CONFIG_CPU_ARM926T
#undef  CONFIG_CPU_ARM1020
#undef  CONFIG_CPU_SA110
#undef  CONFIG_CPU_SA1100
#undef  CONFIG_ARM_THUMB
#undef  CONFIG_DISCONTIGMEM

/*
 * General setup
 */
#define CONFIG_MIZI 1
#undef  CONFIG_PCI
#define CONFIG_ISA 1
#undef  CONFIG_ISA_DMA
#undef  CONFIG_ZBOOT_ROM
#define CONFIG_ZBOOT_ROM_TEXT 0x0
#define CONFIG_ZBOOT_ROM_BSS 0x0
#define CONFIG_HOTPLUG 1

/*
 * PCMCIA/CardBus support
 */
#undef  CONFIG_PCMCIA
#define CONFIG_PCMCIA_MODULE 1
#define CONFIG_PCMCIA_PROBE 1
#undef  CONFIG_I82092
#define CONFIG_I82365 1
#undef  CONFIG_TCIC
#undef  CONFIG_PCMCIA_CLPS6700
#undef  CONFIG_PCMCIA_SA1100
#undef  CONFIG_PCMCIA_PXA
#undef  CONFIG_NET
#define CONFIG_SYSVIPC 1
#undef  CONFIG_BSD_PROCESS_ACCT
#define CONFIG_SYSCTL 1
#define CONFIG_FPE_NWFPE 1
#undef  CONFIG_FPE_FASTFPE
#define CONFIG_KCORE_ELF 1
#undef  CONFIG_KCORE_AOUT
#undef  CONFIG_BINFMT_AOUT
#define CONFIG_BINFMT_ELF 1
#undef  CONFIG_BINFMT_MISC
#define CONFIG_PM 1
#undef  CONFIG_APM
#define CONFIG_CONSOLE_PM 1
#undef  CONFIG_NO_OOM_KILLER
#undef  CONFIG_REDUCE_TTY_MODULAR
#undef  CONFIG_ARTHUR
#undef  CONFIG_NO_TAG_CMDLINE
#define CONFIG_CMDLINE "root=1f04 mem=32M"
#define CONFIG_LEDS 1
#define CONFIG_LEDS_TIMER 1
#define CONFIG_LEDS_CPU 1
#define CONFIG_ALIGNMENT_TRAP 1

/*
 * Parallel port support
 */
#undef  CONFIG_PARPORT

/*
 * Memory Technology Devices (MTD)
 */
#define CONFIG_MTD 1
#undef  CONFIG_MTD_DEBUG
#define CONFIG_MTD_PARTITIONS 1
#define CONFIG_MTD_CONCAT 1
#undef  CONFIG_MTD_REDBOOT_PARTS
#undef  CONFIG_MTD_CMDLINE_PARTS
#undef  CONFIG_MTD_AFS_PARTS
#define CONFIG_MTD_CHAR 1
#define CONFIG_MTD_BLOCK 1
#undef  CONFIG_FTL
#undef  CONFIG_NFTL

/*
 * RAM/ROM/Flash chip drivers
 */
#define CONFIG_MTD_CFI 1
#undef  CONFIG_MTD_JEDECPROBE
#define CONFIG_MTD_GEN_PROBE 1
#undef  CONFIG_MTD_CFI_ADV_OPTIONS
#undef  CONFIG_MTD_CFI_INTELEXT
#define CONFIG_MTD_CFI_AMDSTD 1
#undef  CONFIG_MTD_RAM
#undef  CONFIG_MTD_ROM
#undef  CONFIG_MTD_ABSENT
#undef  CONFIG_MTD_OBSOLETE_CHIPS
#undef  CONFIG_MTD_AMDSTD
#undef  CONFIG_MTD_SHARP
#undef  CONFIG_MTD_JEDEC

/*
 * Mapping drivers for chip access
 */
#undef  CONFIG_MTD_PHYSMAP
#undef  CONFIG_MTD_ARM_INTEGRATOR
#define CONFIG_MTD_S3C2410 1
#undef  CONFIG_MTD_CDB89712
#undef  CONFIG_MTD_SA1100
#undef  CONFIG_MTD_DC21285
#undef  CONFIG_MTD_LUBBOCK
#undef  CONFIG_MTD_FORTUNET
#undef  CONFIG_MTD_EPXA10DB
#undef  CONFIG_MTD_PXA_CERF
#undef  CONFIG_MTD_AUTCPU12
#undef  CONFIG_MTD_PCI

/*
 * Self-contained MTD device drivers
 */
#undef  CONFIG_MTD_PMC551
#undef  CONFIG_MTD_SLRAM
#undef  CONFIG_MTD_MTDRAM
#undef  CONFIG_MTD_BLKMTD
#undef  CONFIG_MTD_DOC1000
#undef  CONFIG_MTD_DOC2000
#undef  CONFIG_MTD_DOC2001
#undef  CONFIG_MTD_DOCPROBE

/*
 * NAND Flash Device Drivers
 */
#undef  CONFIG_MTD_NAND
#undef  CONFIG_MTD_SMC
#undef  CONFIG_MTD_NANDY

/*
 * Plug and Play configuration
 */
#undef  CONFIG_PNP
#undef  CONFIG_ISAPNP

/*
 * Block devices
 */
#undef  CONFIG_BLK_DEV_FD
#undef  CONFIG_BLK_DEV_XD
#undef  CONFIG_PARIDE
#undef  CONFIG_BLK_CPQ_DA
#undef  CONFIG_BLK_CPQ_CISS_DA
#undef  CONFIG_BLK_DEV_DAC960
#define CONFIG_BLK_DEV_LOOP 1
#undef  CONFIG_BLK_DEV_NBD
#undef  CONFIG_BLK_DEV_RAM
#undef  CONFIG_BLK_DEV_INITRD

/*
 * Multi-device support (RAID and LVM)
 */
#undef  CONFIG_MD
#undef  CONFIG_BLK_DEV_MD
#undef  CONFIG_MD_LINEAR
#undef  CONFIG_MD_RAID0
#undef  CONFIG_MD_RAID1
#undef  CONFIG_MD_RAID5
#undef  CONFIG_MD_MULTIPATH
#undef  CONFIG_BLK_DEV_LVM

/*
 * ATA/IDE/MFM/RLL support
 */
#undef  CONFIG_IDE
#undef  CONFIG_BLK_DEV_IDE_MODES
#undef  CONFIG_BLK_DEV_HD

/*
 * SCSI support
 */
#define CONFIG_SCSI 1
#define CONFIG_BLK_DEV_SD 1
#define CONFIG_SD_EXTRA_DEVS (40)
#undef  CONFIG_CHR_DEV_ST
#undef  CONFIG_CHR_DEV_OSST
#undef  CONFIG_BLK_DEV_SR
#undef  CONFIG_CHR_DEV_SG
#undef  CONFIG_SCSI_DEBUG_QUEUES
#define CONFIG_SCSI_MULTI_LUN 1
#undef  CONFIG_SCSI_CONSTANTS
#undef  CONFIG_SCSI_LOGGING

/*
 * SCSI low-level drivers
 */
#undef  CONFIG_SCSI_7000FASST
#undef  CONFIG_SCSI_ACARD
#undef  CONFIG_SCSI_AHA152X
#undef  CONFIG_SCSI_AHA1542
#undef  CONFIG_SCSI_AHA1740
#undef  CONFIG_SCSI_AACRAID
#undef  CONFIG_SCSI_AIC7XXX
#undef  CONFIG_SCSI_AIC7XXX_OLD
#undef  CONFIG_SCSI_DPT_I2O
#undef  CONFIG_SCSI_ADVANSYS
#undef  CONFIG_SCSI_IN2000
#undef  CONFIG_SCSI_AM53C974
#undef  CONFIG_SCSI_MEGARAID
#undef  CONFIG_SCSI_BUSLOGIC
#undef  CONFIG_SCSI_DMX3191D
#undef  CONFIG_SCSI_DTC3280
#undef  CONFIG_SCSI_EATA
#undef  CONFIG_SCSI_EATA_DMA
#undef  CONFIG_SCSI_EATA_PIO
#undef  CONFIG_SCSI_FUTURE_DOMAIN
#undef  CONFIG_SCSI_GDTH
#undef  CONFIG_SCSI_GENERIC_NCR5380
#undef  CONFIG_SCSI_INITIO
#undef  CONFIG_SCSI_INIA100
#undef  CONFIG_SCSI_NCR53C406A
#undef  CONFIG_SCSI_NCR53C7xx
#undef  CONFIG_SCSI_PAS16
#undef  CONFIG_SCSI_PCI2000
#undef  CONFIG_SCSI_PCI2220I
#undef  CONFIG_SCSI_PSI240I
#undef  CONFIG_SCSI_QLOGIC_FAS
#undef  CONFIG_SCSI_SIM710
#undef  CONFIG_SCSI_SYM53C416
#undef  CONFIG_SCSI_T128
#undef  CONFIG_SCSI_U14_34F
#undef  CONFIG_SCSI_DEBUG

/*
 * PCMCIA SCSI adapter support
 */
#undef  CONFIG_SCSI_PCMCIA

/*
 * I2O device support
 */
#undef  CONFIG_I2O
#undef  CONFIG_I2O_BLOCK
#undef  CONFIG_I2O_SCSI
#undef  CONFIG_I2O_PROC

/*
 * ISDN subsystem
 */
#undef  CONFIG_ISDN

/*
 * Input core support
 */
#undef  CONFIG_INPUT
#undef  CONFIG_INPUT_KEYBDEV
#undef  CONFIG_INPUT_MOUSEDEV
#undef  CONFIG_INPUT_JOYDEV
#undef  CONFIG_INPUT_EVDEV

/*
 * Character devices
 */
#define CONFIG_VT 1
#undef  CONFIG_VT_CONSOLE
#undef  CONFIG_SERIAL
#undef  CONFIG_SERIAL_EXTENDED
#undef  CONFIG_SERIAL_NONSTANDARD

/*
 * Serial drivers
 */
#undef  CONFIG_SERIAL_ANAKIN
#undef  CONFIG_SERIAL_ANAKIN_CONSOLE
#undef  CONFIG_SERIAL_AMBA
#undef  CONFIG_SERIAL_AMBA_CONSOLE
#undef  CONFIG_SERIAL_CLPS711X
#undef  CONFIG_SERIAL_CLPS711X_CONSOLE
#undef  CONFIG_SERIAL_21285
#undef  CONFIG_SERIAL_21285_OLD
#undef  CONFIG_SERIAL_21285_CONSOLE
#undef  CONFIG_SERIAL_UART00
#undef  CONFIG_SERIAL_UART00_CONSOLE
#undef  CONFIG_SERIAL_SA1100
#undef  CONFIG_SERIAL_SA1100_CONSOLE
#undef  CONFIG_SERIAL_S3C2400
#undef  CONFIG_SERIAL_S3C2400_CONSOLE
#define CONFIG_SERIAL_S3C2410 1
#define CONFIG_SERIAL_S3C2410_CONSOLE 1
#undef  CONFIG_SERIAL_8250
#undef  CONFIG_SERIAL_8250_CONSOLE
#undef  CONFIG_SERIAL_8250_EXTENDED
#undef  CONFIG_SERIAL_8250_MANY_PORTS
#undef  CONFIG_SERIAL_8250_SHARE_IRQ
#undef  CONFIG_SERIAL_8250_DETECT_IRQ
#undef  CONFIG_SERIAL_8250_MULTIPORT
#undef  CONFIG_SERIAL_8250_HUB6
#define CONFIG_SERIAL_CORE 1
#define CONFIG_SERIAL_CORE_CONSOLE 1
#undef  CONFIG_S3C2410_TOUCHSCREEN
#undef  CONFIG_S3C2410_GPIO_BUTTONS
#define CONFIG_UNIX98_PTYS 1
#define CONFIG_UNIX98_PTY_COUNT (256)

/*
 * I2C support
 */
#undef  CONFIG_I2C

/*
 * L3 serial bus support
 */
#undef  CONFIG_L3
#undef  CONFIG_L3_ALGOBIT
#undef  CONFIG_L3_BIT_SA1100_GPIO
#undef  CONFIG_L3_SA1111
#undef  CONFIG_BIT_SA1100_GPIO

/*
 * Mice
 */
#undef  CONFIG_BUSMOUSE
#undef  CONFIG_MOUSE

/*
 * Joysticks
 */
#undef  CONFIG_INPUT_GAMEPORT
#undef  CONFIG_QIC02_TAPE

/*
 * Watchdog Cards
 */
#undef  CONFIG_WATCHDOG
#undef  CONFIG_INTEL_RNG
#undef  CONFIG_NVRAM
#undef  CONFIG_RTC
#define CONFIG_S3C2410_RTC 1
#undef  CONFIG_DTLK
#undef  CONFIG_R3964
#undef  CONFIG_APPLICOM

/*
 * Ftape, the floppy tape device driver
 */
#undef  CONFIG_FTAPE
#undef  CONFIG_AGP
#undef  CONFIG_DRM

/*
 * PCMCIA character devices
 */

/*
 * Multimedia devices
 */
#undef  CONFIG_VIDEO_DEV

/*
 * File systems
 */
#undef  CONFIG_QUOTA
#undef  CONFIG_AUTOFS_FS
#undef  CONFIG_AUTOFS4_FS
#undef  CONFIG_REISERFS_FS
#undef  CONFIG_REISERFS_CHECK
#undef  CONFIG_REISERFS_PROC_INFO
#undef  CONFIG_ADFS_FS
#undef  CONFIG_ADFS_FS_RW
#undef  CONFIG_AFFS_FS
#undef  CONFIG_HFS_FS
#undef  CONFIG_BFS_FS
#define CONFIG_EXT3_FS 1
#define CONFIG_JBD 1
#undef  CONFIG_JBD_DEBUG
#define CONFIG_FAT_FS 1
#define CONFIG_MSDOS_FS 1
#define CONFIG_UMSDOS_FS 1
#define CONFIG_VFAT_FS 1
#undef  CONFIG_EFS_FS
#undef  CONFIG_JFFS_FS
#undef  CONFIG_JFFS2_FS
#define CONFIG_CRAMFS 1
#define CONFIG_TMPFS 1
#define CONFIG_RAMFS 1
#undef  CONFIG_ISO9660_FS
#undef  CONFIG_JOLIET
#undef  CONFIG_ZISOFS
#undef  CONFIG_MINIX_FS
#undef  CONFIG_VXFS_FS
#undef  CONFIG_NTFS_FS
#undef  CONFIG_NTFS_RW
#undef  CONFIG_HPFS_FS
#define CONFIG_PROC_FS 1
#define CONFIG_DEVFS_FS 1
#define CONFIG_DEVFS_MOUNT 1
#undef  CONFIG_DEVFS_DEBUG
#define CONFIG_DEVPTS_FS 1
#undef  CONFIG_QNX4FS_FS
#undef  CONFIG_QNX4FS_RW
#undef  CONFIG_ROMFS_FS
#undef  CONFIG_EXT2_FS
#undef  CONFIG_SYSV_FS
#undef  CONFIG_UDF_FS
#undef  CONFIG_UDF_RW
#undef  CONFIG_UFS_FS
#undef  CONFIG_UFS_FS_WRITE
#undef  CONFIG_NCPFS_NLS
#undef  CONFIG_SMB_FS
#undef  CONFIG_ZISOFS_FS
#define CONFIG_ZLIB_FS_INFLATE 1

/*
 * Partition Types
 */
#define CONFIG_PARTITION_ADVANCED 1
#undef  CONFIG_ACORN_PARTITION
#undef  CONFIG_OSF_PARTITION
#undef  CONFIG_AMIGA_PARTITION
#undef  CONFIG_ATARI_PARTITION
#undef  CONFIG_MAC_PARTITION
#define CONFIG_MSDOS_PARTITION 1
#undef  CONFIG_BSD_DISKLABEL
#undef  CONFIG_MINIX_SUBPARTITION
#undef  CONFIG_SOLARIS_X86_PARTITION
#undef  CONFIG_UNIXWARE_DISKLABEL
#undef  CONFIG_LDM_PARTITION
#undef  CONFIG_SGI_PARTITION
#undef  CONFIG_ULTRIX_PARTITION
#undef  CONFIG_SUN_PARTITION
#undef  CONFIG_SMB_NLS
#define CONFIG_NLS 1

/*
 * Native Language Support
 */
#define CONFIG_NLS_DEFAULT "ISO 8859-1"
#define CONFIG_NLS_CODEPAGE_437 1
#undef  CONFIG_NLS_CODEPAGE_737
#undef  CONFIG_NLS_CODEPAGE_775
#undef  CONFIG_NLS_CODEPAGE_850
#undef  CONFIG_NLS_CODEPAGE_852
#undef  CONFIG_NLS_CODEPAGE_855
#undef  CONFIG_NLS_CODEPAGE_857
#undef  CONFIG_NLS_CODEPAGE_860
#undef  CONFIG_NLS_CODEPAGE_861
#undef  CONFIG_NLS_CODEPAGE_862
#undef  CONFIG_NLS_CODEPAGE_863
#undef  CONFIG_NLS_CODEPAGE_864
#undef  CONFIG_NLS_CODEPAGE_865
#undef  CONFIG_NLS_CODEPAGE_866
#undef  CONFIG_NLS_CODEPAGE_869
#define CONFIG_NLS_CODEPAGE_936 1
#define CONFIG_NLS_CODEPAGE_950 1
#undef  CONFIG_NLS_CODEPAGE_932
#undef  CONFIG_NLS_CODEPAGE_949
#undef  CONFIG_NLS_CODEPAGE_874
#undef  CONFIG_NLS_ISO8859_8
#undef  CONFIG_NLS_CODEPAGE_1250
#undef  CONFIG_NLS_CODEPAGE_1251
#define CONFIG_NLS_ISO8859_1 1
#undef  CONFIG_NLS_ISO8859_2
#undef  CONFIG_NLS_ISO8859_3
#undef  CONFIG_NLS_ISO8859_4
#undef  CONFIG_NLS_ISO8859_5
#undef  CONFIG_NLS_ISO8859_6
#undef  CONFIG_NLS_ISO8859_7
#undef  CONFIG_NLS_ISO8859_9
#undef  CONFIG_NLS_ISO8859_13
#undef  CONFIG_NLS_ISO8859_14
#undef  CONFIG_NLS_ISO8859_15
#undef  CONFIG_NLS_KOI8_R
#undef  CONFIG_NLS_KOI8_U
#define CONFIG_NLS_UTF8 1

/*
 * Console drivers
 */
#define CONFIG_PC_KEYMAP 1
#undef  CONFIG_VGA_CONSOLE

/*
 * Frame-buffer support
 */
#undef  CONFIG_FB

/*
 * Sound
 */
#define CONFIG_SOUND 1
#undef  CONFIG_SOUND_MK4002
#undef  CONFIG_SOUND_BT878
#undef  CONFIG_SOUND_CMPCI
#undef  CONFIG_SOUND_EMU10K1
#undef  CONFIG_MIDI_EMU10K1
#undef  CONFIG_SOUND_FUSION
#undef  CONFIG_SOUND_CS4281
#undef  CONFIG_SOUND_ES1370
#undef  CONFIG_SOUND_ES1371
#undef  CONFIG_SOUND_ESSSOLO1
#undef  CONFIG_SOUND_MAESTRO
#undef  CONFIG_SOUND_MAESTRO3
#undef  CONFIG_SOUND_ICH
#undef  CONFIG_SOUND_RME96XX
#undef  CONFIG_SOUND_SONICVIBES
#undef  CONFIG_SOUND_TRIDENT
#undef  CONFIG_SOUND_MSNDCLAS
#undef  CONFIG_SOUND_MSNDPIN
#undef  CONFIG_SOUND_VIA82CXXX
#undef  CONFIG_MIDI_VIA82CXXX
#define CONFIG_SOUND_SMDK2410_UDA1341 1
#undef  CONFIG_SOUND_OSS
#undef  CONFIG_SOUND_WAVEARTIST
#undef  CONFIG_SOUND_PXA_AC97
#undef  CONFIG_SOUND_TVMIXER

/*
 * Multimedia Capabilities Port drivers
 */
#undef  CONFIG_MCP
#undef  CONFIG_MCP_SA1100
#undef  CONFIG_MCP_UCB1200
#undef  CONFIG_MCP_UCB1200_AUDIO
#undef  CONFIG_MCP_UCB1200_TS
#undef  CONFIG_MCP_UCB1400_TS

/*
 * USB support
 */
#undef  CONFIG_USB
#define CONFIG_USB_MODULE 1
#undef  CONFIG_USB_DEBUG
#undef  CONFIG_USB_DEVICEFS
#undef  CONFIG_USB_BANDWIDTH
#undef  CONFIG_USB_LONG_TIMEOUT
#undef  CONFIG_USB_UHCI
#undef  CONFIG_USB_UHCI_ALT
#undef  CONFIG_USB_UHCI124
#undef  CONFIG_USB_OHCI
#undef  CONFIG_USB_OHCI_SA1111
#undef  CONFIG_USB_OHCI_S3C2410
#define CONFIG_USB_OHCI_S3C2410_MODULE 1
#define CONFIG_MAX_ROOT_PORTS (1)
#undef  CONFIG_USB_AUDIO
#undef  CONFIG_USB_BLUETOOTH
#undef  CONFIG_USB_STORAGE
#define CONFIG_USB_STORAGE_MODULE 1
#undef  CONFIG_USB_STORAGE_DEBUG
#undef  CONFIG_USB_STORAGE_DATAFAB
#undef  CONFIG_USB_STORAGE_FREECOM
#undef  CONFIG_USB_STORAGE_ISD200
#undef  CONFIG_USB_STORAGE_DPCM
#undef  CONFIG_USB_STORAGE_HP8200e
#undef  CONFIG_USB_STORAGE_SDDR09
#undef  CONFIG_USB_STORAGE_JUMPSHOT
#undef  CONFIG_USB_ACM
#undef  CONFIG_USB_PRINTER
#undef  CONFIG_USB_DC2XX
#undef  CONFIG_USB_MDC800
#undef  CONFIG_USB_SCANNER
#undef  CONFIG_USB_MICROTEK
#undef  CONFIG_USB_HPUSBSCSI
#undef  CONFIG_USB_USS720

/*
 * USB Serial Converter support
 */
#undef  CONFIG_USB_SERIAL
#undef  CONFIG_USB_SERIAL_GENERIC
#undef  CONFIG_USB_SERIAL_BELKIN
#undef  CONFIG_USB_SERIAL_WHITEHEAT
#undef  CONFIG_USB_SERIAL_DIGI_ACCELEPORT
#undef  CONFIG_USB_SERIAL_EMPEG
#undef  CONFIG_USB_SERIAL_FTDI_SIO
#undef  CONFIG_USB_SERIAL_VISOR
#undef  CONFIG_USB_SERIAL_IPAQ
#undef  CONFIG_USB_SERIAL_IR
#undef  CONFIG_USB_SERIAL_EDGEPORT
#undef  CONFIG_USB_SERIAL_KEYSPAN_PDA
#undef  CONFIG_USB_SERIAL_KEYSPAN
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA28
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA28X
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA28XA
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA28XB
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA19
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA18X
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA19W
#undef  CONFIG_USB_SERIAL_KEYSPAN_USA49W
#undef  CONFIG_USB_SERIAL_MCT_U232
#undef  CONFIG_USB_SERIAL_KLSI
#undef  CONFIG_USB_SERIAL_PL2303
#undef  CONFIG_USB_SERIAL_CYBERJACK
#undef  CONFIG_USB_SERIAL_XIRCOM
#undef  CONFIG_USB_SERIAL_OMNINET
#undef  CONFIG_USB_RIO500

/*
 * Kernel hacking
 */
#define CONFIG_FRAME_POINTER 1
#define CONFIG_DEBUG_USER 1
#undef  CONFIG_DEBUG_INFO
#undef  CONFIG_NO_PGT_CACHE
#define CONFIG_DEBUG_KERNEL 1
#undef  CONFIG_DEBUG_SLAB
#define CONFIG_MAGIC_SYSRQ 1
#undef  CONFIG_DEBUG_SPINLOCK
#undef  CONFIG_DEBUG_WAITQ
#define CONFIG_DEBUG_BUGVERBOSE 1
#define CONFIG_DEBUG_ERRORS 1
#define CONFIG_DEBUG_LL 1
#undef  CONFIG_DEBUG_DC21285_PORT
#undef  CONFIG_DEBUG_CLPS711X_UART2