# da_combine dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_audio/
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/om/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/comm/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/download/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_DA_COMBINE_HIFI_DEBUG
endif
endif

obj-$(CONFIG_CODEC_DSP_MISC) += da_combine_v3_dsp_config.o

ifeq ($(CONFIG_SND_SOC_DA_COMBINE_V5),y)
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += da_combine_v5_dsp_config.o
endif


