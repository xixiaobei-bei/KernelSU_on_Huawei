EXTRA_CFLAGS += -Idrivers/hisi/da_separate/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/da_separate/da_separate_v3/
EXTRA_CFLAGS += -Idrivers/hisi/soc_mad/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/

snd-soc-da_separate-store-objs :=da_separate_store.o
snd-soc-da_separate-core-path-objs :=da_separate_core_path.o
snd-soc-da_separate-audio-path-objs :=da_separate_audio_path.o
snd-soc-da_separate-ultr-path-objs :=da_separate_ultr_path.o
snd-soc-da_separate-voice-path-objs :=da_separate_voice_path.o
snd-soc-da_separate-smpa-path-objs :=da_separate_smpa_path.o
snd-soc-da_separate-third-codec-path-objs :=da_separate_third_codec_path.o
snd-soc-da_separate-btfm-path-objs :=da_separate_btfm_path.o
snd-soc-da_separate-mad-path-objs :=da_separate_mad_path.o
snd-soc-da_separate-au_pa-path-objs :=da_separate_au_pa_path.o

snd-soc-da_separate-i2s-objs :=da_separate_i2s.o
snd-soc-da_separate-codec-objs :=da_separate_codec.o
snd-soc-da_separate-utils-objs :=da_separate_utils.o
snd-soc-da_separate-fpga-objs :=da_separate_fpga.o
snd-soc-da_separate_v5-objs :=da_separate_v5.o

obj-$(CONFIG_SND_SOC_DA_SEPARATE_STORE) += snd-soc-da_separate-store.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_CORE_PATH) += snd-soc-da_separate-core-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_AUDIO_PATH) += snd-soc-da_separate-audio-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_ULTR_PATH) += snd-soc-da_separate-ultr-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_VOICE_PATH) += snd-soc-da_separate-voice-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_SMPA_PATH) += snd-soc-da_separate-smpa-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_THIRD_CODEC_PATH) += snd-soc-da_separate-third-codec-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_BTFM_PATH) += snd-soc-da_separate-btfm-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_MAD_PATH) += snd-soc-da_separate-mad-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_AU_PA_PATH) += snd-soc-da_separate-au_pa-path.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_I2S) += snd-soc-da_separate-i2s.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_CODEC) += snd-soc-da_separate-codec.o
obj-$(CONFIG_SND_SOC_DA_SEPARATE_UTILITY) += snd-soc-da_separate-utils.o
obj-y += snd-soc-da_separate-fpga.o
obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V5) += snd-soc-da_separate_v5.o

obj-$(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V3) += da_separate_v3/
