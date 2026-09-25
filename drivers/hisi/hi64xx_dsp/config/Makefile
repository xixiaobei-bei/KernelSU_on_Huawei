# hi64xx dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_hifi/
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/om/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/download/

ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HI64XX_HIFI_DEBUG
endif
endif

obj-$(CONFIG_HI6402_HIFI_MISC) += hi6403_dsp_config.o

ifeq ($(CONFIG_SND_SOC_HI6405),y)
obj-$(CONFIG_SND_SOC_HI6405) += hi6405_dsp_config.o
endif


