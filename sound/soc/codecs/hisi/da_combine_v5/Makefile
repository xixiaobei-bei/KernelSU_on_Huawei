EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/devkit/audiokit/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/adapter/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/config/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/da_combine_v5/multi_kcontrol/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/da_combine_v5/single_kcontrol/
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_audio/

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DAUDIO_FACTORY_MODE
endif

snd-soc-da-combine-v5-objs :=da_combine_v5.o
snd-soc-da-combine-v5-kcontrol-objs :=kcontrol.o
snd-soc-da-combine-v5-codec-probe-objs :=codec_probe.o
snd-soc-da-combine-v5-utils-objs :=utils.o
snd-soc-da-combine-v5-codec-pm-objs := codec_pm.o
snd-soc-da-combine-v5-multi-objs :=/multi_kcontrol/resource_widget.o /multi_kcontrol/path_widget.o /multi_kcontrol/switch_widget.o /multi_kcontrol/switch_widget_utils.o /multi_kcontrol/route.o /multi_kcontrol/pga_widget.o
snd-soc-da-combine-v5-single-objs :=/single_kcontrol/single_drv_widget.o /single_kcontrol/single_pga_widget.o /single_kcontrol/single_switch_route.o /single_kcontrol/single_switch_widget.o

obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-kcontrol.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-single.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-multi.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-codec-probe.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-utils.o
obj-$(CONFIG_SND_SOC_DA_COMBINE_V5) += snd-soc-da-combine-v5-codec-pm.o
