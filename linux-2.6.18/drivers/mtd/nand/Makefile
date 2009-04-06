#
# linux/drivers/nand/Makefile
#
# $Id: Makefile,v 1.1.1.1 2004/02/04 12:56:25 laputa Exp $

O_TARGET	:= nandlink.o

export-objs	:= nand.o nand_ecc.o

nandobjs-y			:= nand.o
nandobjs-$(CONFIG_MTD_NAND_ECC) += nand_ecc.o

obj-$(CONFIG_MTD_NAND)		+= $(nandobjs-y)
obj-$(CONFIG_MTD_NAND_SPIA)	+= spia.o
obj-$(CONFIG_MTD_NAND_AUTCPU12)	+= autcpu12.o

export-objs	+= smc.o
obj-$(CONFIG_MTD_SMC)		+= smc.o nand_ecc.o
obj-$(CONFIG_MTD_SMC_GAMEPARK)	+= smc_s3c2400.o
obj-$(CONFIG_MTD_SMC_S3C2410_SMDK)	+= smc_s3c2410.o
obj-$(CONFIG_MTD_NAND_PREMIUM)	+= premium.o
obj-$(CONFIG_MTD_NAND_BONFS)	+= bon.o

include $(TOPDIR)/Rules.make
