ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)/
endif
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

obj-$(CONFIG_CONTEXTHUB_IGS) += igs_dev.o
obj-$(CONFIG_CONTEXTHUB_IGS) += igs_ion.o
obj-$(CONFIG_CONTEXTHUB_IGS_DBG) += igs_dbg_dev.o
obj-$(CONFIG_CONTEXTHUB_IGS_TOF) += igs_tof_dev.o
