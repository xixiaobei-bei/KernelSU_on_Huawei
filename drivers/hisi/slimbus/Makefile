# slimbus driver
subdir-ccflags-y += -Wall -Werror
ccflags-y += -Idrivers/devkit/audiokit/

ccflags-y += -Idrivers/hisi/slimbus/vendor/candance/include
ccflags-y += -Idrivers/hisi/slimbus/vendor/candance
ccflags-y += -Idrivers/hisi/slimbus/
ccflags-y += -Isound/soc/hisilicon
ccflags-y += -Idrivers/hisi/mntn/blackbox/platform_audio/

slimbus-platform-objs := vendor/candance/cps_bm.o vendor/candance/src/csmi.o vendor/candance/slimbus_drv.o
slimbus-platform-objs += slimbus.o slimbus_utils.o slimbus_pm.o slimbus_da_combine.o slimbus_da_combine_v3.o

ifeq ($(CONFIG_SND_SOC_DA_COMBINE_V5),y)
slimbus-platform-objs += slimbus_da_combine_v5.o
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifneq ($(kcov),true)
slimbus-platform-objs += slimbus_debug.o
endif
endif

obj-$(CONFIG_PLATFORM_SLIMBUS) += slimbus-platform.o
