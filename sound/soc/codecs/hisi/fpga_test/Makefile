EXTRA_CFLAGS += -Isound/soc/codecs/hisi/
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/
EXTRA_CFLAGS += -Idrivers/hisi/slimbus/
EXTRA_CFLAGS += -Idrivers/hisi/da_combine_dsp/
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine/
EXTRA_CFLAGS += -Iinclude/linux/hisi/

snd-soc-da_separate-fpga-test-objs :=da_separate_fpga_test.o
#snd-soc-da_separate-fpga-i2c-slave-objs :=da_separate_fpga_i2c_slave.o

obj-$(CONFIG_SND_SOC_CODEC_STUB) += snd-soc-da_separate-fpga-test.o
#obj-$(CONFIG_SND_SOC_CODEC_STUB) += snd-soc-da_separate-fpga-i2c-slave.o
#obj-$(CONFIG_SND_SOC_CODEC_STUB) += hi_audio_test/
#obj-$(CONFIG_SND_SOC_CODEC_STUB) += soundwire_test/
