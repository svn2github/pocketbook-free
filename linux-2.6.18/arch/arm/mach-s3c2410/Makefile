#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).

USE_STANDARD_AS_RULE := true

O_TARGET		:= s3c2410.o

obj-y :=
obj-m := 
obj-n :=
obj-  :=

export-objs := smdk.o irq.o generic.o cpu.o dma.o pcibuf.o pcipool.o \
		usb_ctl.o usb_ep0.o usb-char.o usb_recv.o usb_send.o \
		pm_drv.o pm.o

# Common support (must be linked before board specific support)
obj-y += generic.o irq.o cpu.o dma.o

# Specific board support
obj-$(CONFIG_S3C2410_SMDK) += smdk.o
obj-$(CONFIG_USB_OHCI_S3C2410) += pcibuf.o pcipool.o

# Power Management
obj-$(CONFIG_PM) += pm.o sleep.o pm_drv.o

# LEDs support
leds-y := leds.o
leds-$(CONFIG_S3C2410_SMDK) += leds-smdk.o
obj-$(CONFIG_LEDS) += $(leds-y)

# S3C2410 USB client support
list-multi += s3c2410usb_core.o
s3c2410usb_core-objs := usb_ctl.o usb_ep0.o usb_send.o usb_recv.o 
obj-$(CONFIG_S3C2410_USB) += s3c2410usb_core.o
obj-$(CONFIG_S3C2410_USB_CHAR) += usb-char.o

include $(TOPDIR)/Rules.make

s3c2410usb_core.o: $(s3c2410usb_core-objs)
	$(LD) -r -o $@ $(s3c2410usb_core-objs)
