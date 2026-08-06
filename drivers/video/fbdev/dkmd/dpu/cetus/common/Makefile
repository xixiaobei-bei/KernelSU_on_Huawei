obj-$(CONFIG_DKMD_DPU_COMMON) += dpu_composer_common.o

dpu_composer_common-objs := \
	cmdlist/dpu_cmdlist.o \
	config/dpu_config.o \
	dacc/dpu_dacc.o \
	isr/dpu_isr.o \
	utils/dpu_chrdev.o \
	utils/dpu_sysfs.o

EXTRA_CFLAGS += \
	-Iinclude/ \
	-Iarch/arm64/include/ \
	-Idrivers/hisi/ap/platform/$(TARGET_PRODUCT)_es \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/utils \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/isr \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/dacc \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/config

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f
