# da_combine dsp device driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_audio/
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/om/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_DA_COMBINE_HIFI_DEBUG
endif
endif

obj-y += audio_file.o
obj-y += dsp_utils.o
