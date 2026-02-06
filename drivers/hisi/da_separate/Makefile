subdir-ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -Isound/soc/codecs/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/da_separate/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/hs_auto_calib/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/da_separate/da_separate_v3/
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/

ifneq ($(strip $(CONFIG_SWITCH)),)
ccflags-y += -DCONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
EXTRA_CFLAGS += -DCONFIG_HUAWEI_HEADSET_DEBUG
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/headset_debug/
endif
snd-soc-da_separate_mbhc-objs := da_separate_mbhc_custom.o
snd-soc-da_separate_mbhc-objs += da_separate_mbhc.o

obj-$(CONFIG_SND_SOC_DA_SEPARATE_MBHC) += snd-soc-da_separate_mbhc.o
