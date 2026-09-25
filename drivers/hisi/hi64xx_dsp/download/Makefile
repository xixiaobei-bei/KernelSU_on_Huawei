# hi64xx dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Iinclude/linux/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_hifi/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/om/

ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HI64XX_HIFI_DEBUG
endif
endif

obj-$(CONFIG_HI6402_HIFI_MISC) += download_image.o
