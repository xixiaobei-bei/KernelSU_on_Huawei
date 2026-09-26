# da_combine_v2 dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_audio/
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/fpga_test/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/om/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/config/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/download/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_DA_COMBINE_HIFI_DEBUG
endif
endif

ifeq ($(CONFIG_HIFI_IPC_3660),y)
	EXTRA_CFLAGS += -DPLATFORM_HI3660
endif

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DAUDIO_FACTORY_MODE
endif

obj-$(CONFIG_CODEC_DSP_MISC) += da_combine_dsp_misc.o
obj-$(CONFIG_CODEC_DSP_MISC) += om/
obj-$(CONFIG_CODEC_DSP_MISC) += comm/
obj-$(CONFIG_CODEC_DSP_MISC) += config/
obj-$(CONFIG_CODEC_DSP_MISC) += download/
