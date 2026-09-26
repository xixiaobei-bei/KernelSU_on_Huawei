subdir-ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/om/
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/fpga_test/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/hs_auto_calib/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_audio/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/

ifneq ($(strip $(CONFIG_SWITCH)),)
ccflags-y += -DCONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
EXTRA_CFLAGS += -DCONFIG_HUAWEI_HEADSET_DEBUG
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/headset_debug/
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DKCOV_AUDIO
else
EXTRA_CFLAGS += -DENABLE_DA_COMBINE_CODEC_DEBUG
obj-y += da_combine_mad.o
endif
#ifeq ($(CONFIG_SND_SOC_CODEC_STUB),y)
#EXTRA_CFLAGS += -DDA_SEPARATE_FPGA_CODEC_STUB
#endif
endif

ccflags-y += -Idrivers/devkit/audiokit/

obj-y += hi_cdc_ctrl.o
obj-y += hi_cdc_ssi.o
obj-y += da_combine_resmgr.o
obj-y += da_combine_mbhc.o
obj-y += da_combine_mbhc_rear_jack.o
obj-y += da_combine_irq.o
obj-y += da_combine_compat.o
obj-y += da_combine_utils.o
obj-y += da_combine_vad.o
obj-y += asp_dma.o
