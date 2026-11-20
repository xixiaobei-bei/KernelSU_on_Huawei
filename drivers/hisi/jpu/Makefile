ccflags-$(CONFIG_HISI_FB_970) += -Idrivers/hisi/jpu/jpgdecv400
ccflags-$(CONFIG_HISI_FB_V501) += -Idrivers/hisi/jpu/jpgdecv501
ccflags-$(CONFIG_HISI_FB_V510) += -Idrivers/hisi/jpu/jpgdecv511
ccflags-$(CONFIG_HISI_FB_V600) += -Idrivers/hisi/jpu/jpgdecv600
ccflags-$(CONFIG_HISI_FB_V700) += -Idrivers/hisi/jpu/jpgdecv700

obj-$(CONFIG_JPU_SUPPORT) := hisijpu.o

ifeq ($(es_low_freq),true)
	EXTRA_CFLAGS += -DCONFIG_ES_LOW_FREQ
endif

EXTRA_CFLAGS += -Idrivers/video/hisi/dss \
	-Idrivers/video/fbdev/hisi/dss \
	-I$(srctree)/lib/libc_sec/securec_v2/include/ \

ifeq ($(CONFIG_HISI_FB_V510), y)
subdir-ccflags-y += -DJPGDEC_USE_MERGED_IRQ
endif

ifeq ($(CONFIG_HISI_FB_V600), y)
subdir-ccflags-y += -DJPGDEC_USE_MERGED_IRQ
endif

ifeq ($(CONFIG_HISI_FB_V700), y)
subdir-ccflags-y += -DJPGDEC_USE_MERGED_IRQ
endif

hisijpu-objs := \
	hisi_jpu.o \
	hisi_jpu_iommu.o \
	hisi_jpu_utils.o \
	hisi_jpu_isr.o \
	hisi_jpu_exec.o

clean:
	rm *.o .*cmd
