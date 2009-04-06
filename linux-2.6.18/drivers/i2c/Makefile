#
# Makefile for the kernel i2c bus driver.
#

O_TARGET := i2c.o

export-objs	:= i2c-core.o i2c-algo-bit.o i2c-algo-pcf.o \
		   i2c-algo-ite.o i2c-algo-pxa.o i2c-proc.o \
		   i2c-algo-s3c2410.o i2c-s3c2410.o

# Init order: core, chardev, bit adapters, pcf adapters

obj-$(CONFIG_I2C)		+= i2c-core.o
obj-$(CONFIG_I2C_CHARDEV)	+= i2c-dev.o

# Bit adapters
obj-$(CONFIG_I2C_ALGOBIT)	+= i2c-algo-bit.o
obj-$(CONFIG_I2C_PHILIPSPAR)	+= i2c-philips-par.o
obj-$(CONFIG_I2C_ELV)		+= i2c-elv.o
obj-$(CONFIG_I2C_VELLEMAN)	+= i2c-velleman.o

# PCF adapters
obj-$(CONFIG_I2C_ALGOPCF)	+= i2c-algo-pcf.o
obj-$(CONFIG_I2C_ELEKTOR)	+= i2c-elektor.o
obj-$(CONFIG_ITE_I2C_ALGO)	+= i2c-algo-ite.o
obj-$(CONFIG_ITE_I2C_ADAP)	+= i2c-adap-ite.o
obj-$(CONFIG_I2C_PROC)		+= i2c-proc.o
obj-$(CONFIG_I2C_KEYWEST)	+= i2c-keywest.o

# PXA adapters
obj-$(CONFIG_I2C_PXA_ALGO)	+= i2c-algo-pxa.o
obj-$(CONFIG_I2C_PXA_ADAP)	+= i2c-adap-pxa.o

#laputa append obj for to support I2C 
obj-$(CONFIG_I2C_S3C2410_ALGO) +=i2c-algo-s3c2410.o
obj-$(CONFIG_I2C_S3C2410_ADAP) +=i2c-s3c2410.o
#laputa end 030821

# This is needed for automatic patch generation: sensors code starts here
# This is needed for automatic patch generation: sensors code ends here

include $(TOPDIR)/Rules.make

