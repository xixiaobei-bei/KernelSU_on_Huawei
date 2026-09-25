# hi6402 dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/hi64xx_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_hifi/
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/hi6xxx_fpga_test/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/om/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/config/
EXTRA_CFLAGS += -Idrivers/hisi/hi64xx_dsp/download/

ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HI64XX_HIFI_DEBUG
endif
endif

ifeq ($(CONFIG_HIFI_IPC_3660),y)
	EXTRA_CFLAGS += -DPLATFORM_HI3660
endif

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DAUDIO_FACTORY_MODE
endif

obj-$(CONFIG_HI6402_HIFI_MISC) += hi64xx_dsp_misc.o
obj-$(CONFIG_HI6402_HIFI_MISC) += om/
obj-$(CONFIG_HI6402_HIFI_MISC) += comm/
obj-$(CONFIG_HI6402_HIFI_MISC) += config/
obj-$(CONFIG_HI6402_HIFI_MISC) += download/
