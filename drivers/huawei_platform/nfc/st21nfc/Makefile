#
# Makefile for nfc devices
#
#EXTRA_CFLAGS += -Idrivers/huawei/device
obj-$(CONFIG_NFC_ST21NFC)		+= st21nfc.o
#obj-y		+= st21nfc.o

ifdef CONFIG_EXT_INPUTHUB
ifeq ($(wildcard $(srctree)/drivers/huawei_platform/sensor/ext_inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/$(TARGET_BOARD_PLATFORM)/
endif
endif
