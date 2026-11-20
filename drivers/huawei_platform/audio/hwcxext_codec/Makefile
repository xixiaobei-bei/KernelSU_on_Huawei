# hwcxext codec module

ifdef CONFIG_HWCXEXT_CODEC_MODULE
	ccflags-y += -Idrivers/huawei_platform/audio/hwcxext_codec
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
	ccflags-y += -Idrivers/hisi/tzdriver
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DHWCXEXT_CODEC_INFO_PERMISSION_ENABLE
endif

obj-$(CONFIG_HWCXEXT_CODEC_INTERFACE) += hwcxext_codec_interface.o
obj-$(CONFIG_HWCXEXT_MBHC) += hwcxext_mbhc.o
obj-$(CONFIG_HWCXEXT_I2C_OPS) += hwcxext_i2c_ops.o
obj-$(CONFIG_HWCXEXT_CODEC_V1) += hwcxext_codec_v1.o
ifneq ($(TARGET_BUILD_VARIANT),user)
	obj-$(CONFIG_HWCXEXT_CODEC_MODULE) += hwcxext_codec_info.o
endif