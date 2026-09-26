EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/ipcm
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/mailbox

EXTRA_CFLAGS += -DBSP_CORE_APP
ifdef CONFIG_HIFI_RESET
EXTRA_CFLAGS += -D_HIFI_WD_DEBUG
endif

ifeq ($(CONFIG_AUDIO_DEBUG),y)
EXTRA_CFLAGS += -DENABLE_MAILBOX_DEBUG
endif

EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp/om

obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_gut.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_msg.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_debug.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox_port_linux.o
obj-$(CONFIG_HIFI_MAILBOX)	+= drv_mailbox.o
