ifdef CONFIG_HWEVEXT_EXTERN_ADC
	ccflags-y += -Idrivers/huawei_platform/audio/hwevext_adc
	ccflags-y += -Idrivers/devkit/audiokit/
	ccflags-y += -I$(srctree)/drivers/devkit/audiokit/
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
	obj-$(CONFIG_HWEVEXT_EXTERN_ADC)	+= hwevext_adc_info.o
endif
obj-$(CONFIG_HWEVEXT_EXTERN_ADC)	+= hwevext_adc.o
obj-$(CONFIG_HWEVEXT_EXTERN_ADC)	+= hwevext_adc_i2c_ops.o
