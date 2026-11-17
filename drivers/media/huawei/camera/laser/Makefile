#
# Makefile for hisilicon camera laser driver
#

EXTRA_CFLAGS += -Idrivers/media/huawei/camera/
EXTRA_CFLAGS += -fno-pic

obj-y += laser_common.o
obj-$(CONFIG_HISI_CAMERA_VL53L0_970) += hw_laser_subdev.o
obj-$(CONFIG_HISI_CAMERA_VL53L0) += vl53l0.o
obj-$(CONFIG_HISI_CAMERA_VL53L0_970) += vl53Lx/
obj-$(CONFIG_HISI_CAMERA_VL53L0_970) += vi53xx/