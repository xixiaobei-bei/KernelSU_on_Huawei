# Makefile for the Himax touchscreen drivers. //himax_dsm.o
# drivers/input/touchscreen
obj-$(CONFIG_HIMAX_TS_KIT_3_0) += himax_ts.o
himax_ts-objs += himax_ic.o himax_platform.o himax_fw_update.o himax_factory_test.o  himax_debug.o
EXTRA_CFLAGS += -Iarch/arm/mach-hisi
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
EXTRA_CFLAGS += -Idrivers/devkit/tpkit/3_0

