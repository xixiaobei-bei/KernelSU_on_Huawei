subdir-ccflags-y += -Idrivers/hisi/tzdriver
ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ifeq ($(TARGET_BOARD_PLATFORM), miamicw)
obj-y += kirin710/
else
obj-y += default/
endif
else
obj-y += $(TARGET_BOARD_PLATFORM)/
endif
