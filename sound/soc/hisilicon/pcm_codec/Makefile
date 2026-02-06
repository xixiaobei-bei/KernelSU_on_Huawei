EXTRA_CFLAGS += -Idrivers/devkit/audiokit
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine/

ifeq ($(CONFIG_PLATFORM_SLIMBUS),y)
EXTRA_CFLAGS += -Idrivers/hisi/slimbus
endif

snd-soc-format-objs := format.o
snd-soc-platform-io-objs := platform_io.o
snd-soc-pcm-codec-objs := pcm_codec.o

ifeq ($(CONFIG_HISI_DEBUG_FS),y)
EXTRA_CFLAGS += -DAUDIO_PCM_CODEC_DEBUG
snd-soc-om-objs := om.o
obj-$(CONFIG_SND_SOC_PCM_CODECS) += snd-soc-om.o
endif

obj-$(CONFIG_SND_SOC_PCM_CODECS) += snd-soc-format.o
obj-$(CONFIG_SND_SOC_PCM_CODECS) += snd-soc-platform-io.o
obj-$(CONFIG_SND_SOC_PCM_CODECS) += snd-soc-pcm-codec.o
