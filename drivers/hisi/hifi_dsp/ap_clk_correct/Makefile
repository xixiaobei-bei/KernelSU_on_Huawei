# hifi dsp device driver
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp
EXTRA_CFLAGS += -Iinclude/linux/mfd
EXTRA_CFLAGS += -Idrivers/mmc/host

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)	+= ap_clk_correct.o
EXTRA_CFLAGS += -DHISI_EXTERNAL_MODEM
endif
