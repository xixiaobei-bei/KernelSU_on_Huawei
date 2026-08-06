ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)/
endif

ccflags-y  += -I$(srctree)/include/linux/hisi/contexthub


ifneq ($(CONFIG_INPUTHUB_AS),y)
obj-$(CONFIG_CONTEXTHUB) += inputhub_api.o
endif

ifeq ($(TARGET_BOARD_PLATFORM), kirin970)
obj-$(CONFIG_CONTEXTHUB) += $(TARGET_BOARD_PLATFORM)/common.o
else
obj-$(CONFIG_CONTEXTHUB) += common.o
endif

ifneq ($(CONFIG_INPUTHUB_30),y)
obj-$(CONFIG_CONTEXTHUB_SHMEM) += shmem.o
endif
obj-$(CONFIG_CONTEXTHUB_SHELL) += shell_dbg.o
obj-$(CONFIG_CONTEXTHUB_LOADMONITOR) += loadmonitor.o
obj-$(CONFIG_CONTEXTHUB_PLAYBACK) += dbg/playback.o
obj-$(CONFIG_CONTEXTHUB_CHRE) += chre.o
obj-$(CONFIG_CONTEXTHUB)   += flp/
obj-$(CONFIG_CONTEXTHUB_IGS)   += igs/
obj-$(CONFIG_INPUTHUB_MOCK_SENSOR)  += mock_sensor_channel.o
obj-$(CONFIG_CONTEXTHUB)   += bsp/
obj-$(CONFIG_INPUTHUB_AS)   += inputhub_as/
obj-$(CONFIG_CONTEXTHUB)   += inputhub_wrapper.o
obj-$(CONFIG_CONTEXTHUB_UDI) += dump_inject.o
obj-$(CONFIG_CONTEXTHUB_BLE) += ble_channel.o
