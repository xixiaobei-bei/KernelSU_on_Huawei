EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_separate/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/



#  da_separate_v2 codec
snd-soc-da-separate-v2-objs :=da_separate_v2.o
snd-soc-da-separate-v2-utility-objs :=da_separate_v2_utility.o
snd-soc-da-separate-v2-i2s-objs :=da_separate_v2_i2s.o

obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V2) += snd-soc-da-separate-v2.o
obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V2) += snd-soc-da-separate-v2-utility.o
obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V2) += snd-soc-da-separate-v2-i2s.o
