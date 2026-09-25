# virtual voice proxy driver
obj-$(CONFIG_VIRTUAL_VOICE_PROXY)		+= virtual_voice_proxy.o
obj-$(CONFIG_VIRTUAL_VOICE_PROXY_EXTDEV)		+= virtual_voice_proxy_extdev.o

ifeq ($(CONFIG_HIFI_EXTRA_MODEM_MODE),y)
EXTRA_CFLAGS += -DHISI_EXTERNAL_MODEM
endif

EXTRA_CFLAGS += -Idrivers/hisi/hifi_mailbox/mailbox \
		-Idrivers/hisi/hifi_mailbox/ipcm \
		-Idrivers/hisi/hifi_dsp