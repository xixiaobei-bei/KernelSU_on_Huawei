subdir-ccflags-y += -Wall -Werror

# hifi dsp device driver
ifeq ($(CONFIG_HIFI_IPC_3650),y)
EXTRA_CFLAGS += -Iinclude/modem/include/config/hi365x
EXTRA_CFLAGS += -Iinclude/modem/include/drv/hi365x
endif

ifeq ($(CONFIG_HISI_FB_970),y)
EXTRA_CFLAGS += -DCONFIG_DP_MULTI_CHANNELS
endif

ifeq ($(CONFIG_HISI_FB_V501),y)
EXTRA_CFLAGS += -DCONFIG_DP_MULTI_CHANNELS
endif

ifeq ($(CONFIG_HISI_FB_V510),y)
EXTRA_CFLAGS += -DCONFIG_DP_MULTI_CHANNELS
endif

ifeq ($(CONFIG_HISI_FB_V600),y)
EXTRA_CFLAGS += -DCONFIG_DP_MULTI_CHANNELS
endif

ifeq ($(CONFIG_HIFI_SUPPORT_VIBRATOR),y)
EXTRA_CFLAGS += -DCONFIG_HIFI_SUPPORT_VIBRATOR
endif

ifeq ($(CONFIG_HIFI_IPC_3660),y)
EXTRA_CFLAGS += -Iinclude/modem/include/config/hi365x
EXTRA_CFLAGS += -Iinclude/modem/include/drv/hi365x
endif
ifeq ($(CONFIG_HIFI_IPC_6250),y)
EXTRA_CFLAGS += -DPLATFORM_HI6250
EXTRA_CFLAGS += -DEXPAND_HIFI_RUNNING_REGION
EXTRA_CFLAGS += -Iinclude/modem/include/config/hi62x0
EXTRA_CFLAGS += -Iinclude/modem/include/drv/hi62x0
endif

EXTRA_CFLAGS += -Idrivers/hisi/hifi_mailbox/mailbox
EXTRA_CFLAGS += -Idrivers/hisi/hifi_mailbox/ipcm
EXTRA_CFLAGS += -Idrivers/hisi/soundtrigger
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/om
EXTRA_CFLAGS += -Isound/usb/hifi/
ifeq ($(CONFIG_AUDIO_DEBUG),y)
ifeq ($(kcov),true)
EXTRA_CFLAGS += -DENABLE_AUDIO_KCOV
else
EXTRA_CFLAGS += -DENABLE_HIFI_DEBUG
endif
endif

obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= om/
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= dsp_misc.o
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= dsp_msg.o
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= dsp_lpp.o
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= algorithm_dft.o

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= audio_pcie_proc.o
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= b5000_strategy_acore.o
EXTRA_CFLAGS += -DHISI_EXTERNAL_MODEM
endif
ifeq ($(CONFIG_AUDIO_DEBUG),y)
obj-$(CONFIG_SND_SOC_CODEC_STUB)	+= memcpy_opt.o
ifeq ($(CONFIG_SND_SOC_CODEC_STUB),y)
EXTRA_CFLAGS += -DDYNAMIC_LOAD_HIFIIMG
endif
endif

obj-y	+= voice_proxy/

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
obj-y	+= ap_clk_correct/
endif

ccflags-$(CONFIG_HIFI_DSP_ONE_TRACK) += -Idrivers/devkit/audiokit/
