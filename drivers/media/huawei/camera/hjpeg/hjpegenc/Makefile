ifeq ($(es_low_freq),true)
EXTRA_CFLAGS += -DCONFIG_ES_LOW_FREQ
endif

ccflags-y += -Idrivers/media/huawei/camera/hjpeg
ccflags-y += -Idrivers/media/huawei/camera
ccflags-y += -Idrivers/media/huwei/camera/hjpeg/hjpegenc
obj-y += hjpeg_process.o smmu_cfg.o jpegenc_cfg.o cvdr_cfg.o
ifneq ($(CONFIG_HISI_CAMERA_HISTAR_ISPV300), y)
obj-y += hjpeg_debug.o
endif
