
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/ipcm
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_dsp

obj-$(CONFIG_CLASSD_VIBRATOR) += classd_vibrator.o
obj-$(CONFIG_CLASSD_VIBRATOR) += dsp_vibrator.o

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DCLASSD_VIBRATOR_DEBUG
endif
