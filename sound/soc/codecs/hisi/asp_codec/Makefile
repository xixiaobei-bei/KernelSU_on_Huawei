EXTRA_CFLAGS += -Isound/soc/codecs/hisi/asp_codec/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Iinclude/linux/hisi/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/asp_codec/multi_kcontrol/
EXTRA_CFLAGS += -Isound/soc/codecs/hisi/asp_codec/single_kcontrol/

snd-soc-aspcodec-objs :=asp_codec.o
snd-soc-aspcodec-utils-objs :=asp_codec_utils.o
snd-soc-aspcodec-kcontrol-objs := asp_codec_kcontrol.o
snd-soc-aspcodec-multi-objs :=/multi_kcontrol/asp_codec_resource_widget.o /multi_kcontrol/asp_codec_path_widget.o /multi_kcontrol/asp_codec_switch_widget.o /multi_kcontrol/asp_codec_route.o
snd-soc-aspcodec-single-objs :=/single_kcontrol/asp_codec_single_switch_widget.o /single_kcontrol/asp_codec_single_kcontrol.o  /single_kcontrol/asp_codec_single_route.o

obj-$(CONFIG_SND_SOC_ASP_CODEC) += snd-soc-aspcodec.o
obj-$(CONFIG_SND_SOC_ASP_CODEC) += snd-soc-aspcodec-utils.o
obj-$(CONFIG_SND_SOC_ASP_CODEC) += snd-soc-aspcodec-kcontrol.o
obj-$(CONFIG_SND_SOC_ASP_CODEC) += snd-soc-aspcodec-multi.o
obj-$(CONFIG_SND_SOC_ASP_CODEC) += snd-soc-aspcodec-single.o
