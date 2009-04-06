#
# Makefile for the Intel PXA250/210 PCMCIA driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now inherited from the
# parent makes..

O_TARGET := pxa_cs.o

export-objs :=

list-multi := pxa_cs.o

obj-$(CONFIG_PCMCIA_PXA)	+= pxa_cs.o

pxa_cs-objs-y				:= pxa.o
pxa_cs-objs-$(CONFIG_ARCH_LUBBOCK)		+= lubbock.o
pxa_cs-objs-$(CONFIG_ARCH_PXA_IDP)	+= pxa_idp.o
pxa_cs-objs-$(CONFIG_ARCH_PXA_CERF)		+= ../sa1100_cerf.o

include $(TOPDIR)/Rules.make

pxa_cs.o: $(pxa_cs-objs-y)
	$(LD) -r -o $@ $(pxa_cs-objs-y)
