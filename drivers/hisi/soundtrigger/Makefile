# soundtrigger driver
subdir-ccflags-y += -Wall -Werror

EXTRA_CFLAGS += -Idrivers/hisi/slimbus
EXTRA_CFLAGS += -Idrivers/hisi/hifi_dsp
EXTRA_CFLAGS += -Iinclude/linux/hisi/da_combine
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/da_combine_dsp
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/da_combine_dsp/comm
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/hifi_dsp
EXTRA_CFLAGS += -I$(srctree)/sound/soc/hisilicon

obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_ring_buffer.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_dma_drv.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_dma_drv_comm.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_dma_drv_da_combine.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_dma_drv_third_codec.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_third_codec_data_proc.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_event.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_socdsp_mailbox.o
obj-$(CONFIG_SOUND_TRIGGER) += soundtrigger_socdsp_pcm.o

