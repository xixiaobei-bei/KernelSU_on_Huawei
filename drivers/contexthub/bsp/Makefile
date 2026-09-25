ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)/
endif
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

obj-$(CONFIG_INPUTHUB_30) += iomcu_boot.o
obj-$(CONFIG_INPUTHUB_30) += iomcu_dump.o
obj-$(CONFIG_INPUTHUB_30) += iomcu_ipc.o
obj-$(CONFIG_INPUTHUB_30) += iomcu_shmem.o
