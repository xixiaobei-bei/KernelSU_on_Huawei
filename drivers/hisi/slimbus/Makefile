# slimbus driver
subdir-ccflags-y += -Wall -Werror
ccflags-y += -Idrivers/devkit/audiokit/

ccflags-y += -Idrivers/hisi/slimbus/vendor/candance/include
ccflags-y += -Idrivers/hisi/slimbus/vendor/candance
ccflags-y += -Isound/soc/hisilicon
ccflags-y += -Idrivers/hisi/mntn/blackbox/platform_hifi/

hisilicon-slimbus-platform-objs := vendor/candance/cps_bm.o vendor/candance/src/csmi.o vendor/candance/slimbus_drv.o
hisilicon-slimbus-platform-objs += slimbus.o slimbus_utils.o slimbus_64xx.o slimbus_6403.o

ifeq ($(CONFIG_SND_SOC_HI6405),y)
hisilicon-slimbus-platform-objs += slimbus_6405.o
endif

ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifneq ($(kcov),true)
hisilicon-slimbus-platform-objs += slimbus_debug.o
endif
endif

obj-$(CONFIG_HISILICON_PLATFORM_SLIMBUS) += hisilicon-slimbus-platform.o
