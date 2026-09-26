# hwevext codec module

ifdef CONFIG_HWEVEXT_CODEC_MODULE
	ccflags-y += -Idrivers/huawei_platform/audio/hwevext_codec
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DHWEVEXT_CODEC_INFO_PERMISSION_ENABLE
endif

obj-$(CONFIG_HWEVEXT_CODEC_INTERFACE) += hwevext_codec_interface.o
obj-$(CONFIG_HWEVEXT_MBHC) += hwevext_mbhc.o
obj-$(CONFIG_HWEVEXT_I2C_OPS) += hwevext_i2c_ops.o
obj-$(CONFIG_HWEVEXT_CODEC_V1) += hwevext_codec_v1.o
ifneq ($(TARGET_BUILD_VARIANT),user)
	obj-$(CONFIG_HWEVEXT_CODEC_MODULE) += hwevext_codec_info.o
endif