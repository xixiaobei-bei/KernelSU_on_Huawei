#
# Makefile for the Omnivision TCM touchscreen driver.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_OMNIVISION_TCM_TS_KIT) += omnivision_tcm_core_module.o
omnivision_tcm_core_module-objs += omnivision_tcm_spi.o omnivision_tcm_core.o omnivision_tcm_touch.o omnivision_tcm_zeroflash.o omnivision_tcm_testing.o omnivision_tcm_device.o
#obj-y += omnivision_tcm_zeroflash.o
##obj-$(CONFIG_TOUCHSCREEN_OMNIVISION_TCM_REFLASH) += omnivision_tcm_reflash.o
#obj-$(CONFIG_TOUCHSCREEN_OMNIVISION_TCM_DEVICE) += omnivision_tcm_device.o
#obj-$(CONFIG_TOUCHSCREEN_OMNIVISION_TCM_TESTING) += omnivision_tcm_testing.o
#obj-$(CONFIG_TOUCHSCREEN_OMNIVISION_TCM_RECOVERY) += omnivision_tcm_recovery.o
#obj-$(CONFIG_TOUCHSCREEN_OMNIVISION_TCM_DIAGNOSTICS) += omnivision_tcm_diagnostics.o
#obj-$(CONFIG_OMNIVISION_TCM_TS_KIT) += omnivision_tcm_core.o \



EXTRA_CFLAGS += -Iarch/arm/mach-hisi
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/core/common
EXTRA_CFLAGS += -Idrivers/devkit/tpkit
