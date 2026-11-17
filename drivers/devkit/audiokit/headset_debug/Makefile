# Makefile for the Headset Debug Audio drivers.
#
# Each configuration option enables a list of files.

headset-debug-objs :=headset_debug.o
ifneq ($(strip $(CONFIG_SWITCH)),)
ccflags-y += -DCONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
endif
ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DCONFIG_HUAWEI_HEADSET_DEBUG
obj-y += headset-debug.o
endif
