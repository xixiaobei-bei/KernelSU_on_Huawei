subdir-ccflags-y += -Wall -Werror

# hifi dsp device driver
ifeq ($(CONFIG_HIFI_IPC_3650),y)
EXTRA_CFLAGS += -Iinclude/modem/include/config/hi365x
EXTRA_CFLAGS += -Iinclude/modem/include/drv/hi365x
endif

EXTRA_CFLAGS += -Idrivers/hisi/hifi_mailbox/mailbox
EXTRA_CFLAGS += -Idrivers/hisi/hifi_mailbox/ipcm
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Isound/usb/hifi/

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HIFI_DEBUG
endif
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(CONFIG_SND_SOC_CODEC_STUB),y)
EXTRA_CFLAGS += -DDYNAMIC_LOAD_HIFIIMG
endif
endif

obj-y += dsp_om.o
obj-y += om_debug.o
obj-y += om_bigdata.o

ccflags-$(CONFIG_HIFI_DSP_ONE_TRACK) += -Idrivers/devkit/audiokit/