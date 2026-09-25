# hi64xx dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_hifi/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/download/

ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HI64XX_HIFI_DEBUG
obj-$(CONFIG_HI6402_HIFI_MISC) += om_debug.o
endif
endif

ifeq ($(CONFIG_HIFI_IPC_3660),y)
	EXTRA_CFLAGS += -DPLATFORM_HI3660
endif

obj-$(CONFIG_HI6402_HIFI_MISC) += om_hook.o
obj-$(CONFIG_HI6402_HIFI_MISC) += om_beta.o
obj-$(CONFIG_HI6402_HIFI_MISC) += om.o
