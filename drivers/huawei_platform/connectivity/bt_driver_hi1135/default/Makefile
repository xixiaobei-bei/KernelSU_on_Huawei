ifeq ($(wildcard $(srctree)/drivers/huawei_platform/sensor/ext_inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/$(TARGET_BOARD_PLATFORM)/
endif

obj-$(CONFIG_BT_HCI_DRIVER_HI1135) += bt_dev.o
obj-$(CONFIG_BT_HCI_DRIVER_HI1135) += bt_data_parse.o