subdir-ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -Idrivers/devkit/audiokit
ifeq ($(CONFIG_HIFI_MAILBOX), y)
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/ipcm
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_dsp
endif



EXTRA_CFLAGS +=	-Idrivers/hisi/slimbus/
EXTRA_CFLAGS +=	-Iinclude/linux/hisi/da_combine/
EXTRA_CFLAGS +=	-I$(srctree)/drivers/hisi/hifi_dsp

obj-$(CONFIG_SND_SOC_SLIMBUS_DAI) += slimbus_dai.o
snd-soc-pcm-dsp-objs := audio_pcm_hifi.o
snd-soc-asp-cfg-objs := asp_cfg.o
obj-$(CONFIG_HIFI_DSP_ONE_TRACK) += snd-soc-pcm-dsp.o

# DP Support
ifeq ($(CONFIG_HISI_FB_V600),y)
EXTRA_CFLAGS += -DDP_AUDIO_ASP_HDMI_I2S
endif
snd-soc-dp-objs := audio_dp_machine.o audio_pcm_dp.o audio_dp_i2s.o asp_hdmi_dma.o

snd-soc-da-separate-machine-objs += da_separate_machine.o
snd-soc-da-combine-machine-objs := da_combine_machine.o
snd-soc-analog-less-machine-objs := analog_less_machine.o

obj-$(CONFIG_SND_DA_SEPARATE_MACHINE) +=snd-soc-da-separate-machine.o
obj-$(CONFIG_SND_DA_COMBINE_MACHINE) += snd-soc-da-combine-machine.o
obj-$(CONFIG_SND_ANALOG_LESS_MACHINE) += snd-soc-analog-less-machine.o

obj-$(CONFIG_SND_SOC_DP_AUDIO) += snd-soc-dp.o
obj-$(CONFIG_SND_AUTO_TEST) += auto_test/
obj-$(CONFIG_SND_SOC_PCM_CODECS) += pcm_codec/
obj-y += snd-soc-asp-cfg.o
