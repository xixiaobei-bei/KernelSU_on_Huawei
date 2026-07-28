subdir-ccflags-y += -Wall -Werror

ifeq ($(CLT_IV), true)
EXTRA_CFLAGS += -DCLT_AUDIO
endif

EXTRA_CFLAGS += -Isound/soc/codecs/
ifneq ($(TARGET_BUILD_VARIANT),user)
	EXTRA_CFLAGS += -DCONFIG_HUAWEI_HEADSET_DEBUG
	EXTRA_CFLAGS += -Idrivers/devkit/audiokit/headset_debug/
endif
snd-soc-hi6xxx_mbhc-objs := hi6xxx_mbhc.o

obj-$(CONFIG_SND_SOC_CODEC_HI6555V2) += snd-soc-hi6xxx_mbhc.o


