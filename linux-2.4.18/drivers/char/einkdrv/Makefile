# Makefile for PIO driver Kernel Module
# Copyright(c) 2002 Sigma Designs, Inc.

O_TARGET		:= eink.o


# Objects that export symbols.

export-objs:= einkdrv.o EinkController.o 

include $(TOPDIR)/Rules.make


all: eink.o

eink.o: einkdrv.o EinkController.o 
	$(LD) -r -o $@ $(export-objs)

einkdrv.o: einkdrv.c 
	$(CC) $(CFLAGS) -o einkdrv.o -c einkdrv.c 


EinkController.o: EinkController.c 
	$(CC) $(CFLAGS) -o EinkController.o -c EinkController.c 



