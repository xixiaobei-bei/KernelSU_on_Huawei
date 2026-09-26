#Makefile for the touch driver of mstar
obj-$(CONFIG_MSTAR_TS_KIT_3_0) += mstar_ts.o
mstar_ts-objs += mstar_main.o mstar_apknode.o mstar_update.o mstar_dts.o mstar_mp.o

EXTRA_CFLAGS += -Iarch/arm/mach-hisi
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
EXTRA_CFLAGS += -Idrivers/devkit/tpkit/3_0



