# deviceboxID driver

ifneq ($(TARGET_BUILD_VARIANT), user)
	ccflags-y += -DDEVICEBOXID_DEBUG
endif
obj-$(CONFIG_HUAWEI_DEVICEBOX_AUDIO) += boxid.o
