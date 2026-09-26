obj-$(CONFIG_CHIPONE_TS_KIT_3_0) += chipone.o

chipone-objs := cts_core.o cts_sfctrlv2.o cts_spi_flash.o cts_firmware.o cts_tool.o cts_sysfs.o cts_test.o cts_strerror.o
chipone-objs += cts_plat_huawei_tskit.o chipone_tskit_ops.o

EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/devkit/tpkit/3_0