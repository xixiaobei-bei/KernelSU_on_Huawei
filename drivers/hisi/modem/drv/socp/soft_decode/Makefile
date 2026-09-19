#
# Makefile for the memory technology device drivers.
#
# Core functionality.

ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

obj-y				+= hdlc.o 
obj-y				+= ring_buffer.o 
obj-y				+= soft_decode.o 
obj-y				+= soft_cfg.o 

subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/tools




