obj-$(CONFIG_DKMD_DPU_COMPOSER) += dpu_composer.o

dpu_composer-objs := \
	comp_drv.o \
	comp_dev.o \
	comp_drv_mgr.o \
	dpu_comp_vactive.o \
	vsync/dpu_vsync.o

EXTRA_CFLAGS += \
	-Iinclude/ \
	-Iarch/arm64/include/ \
	-Iinclude/media/dkmd/ \
	-Idrivers/hisi/ap/platform/$(TARGET_PRODUCT)_es \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Idrivers/video/fbdev/dkmd/dksm/timeline \
	-Idrivers/video/fbdev/dkmd/dksm/buf_sync \
	-Idrivers/video/fbdev/dkmd/dksm/fence \
	-Idrivers/video/fbdev/dkmd/dksm/smmu \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/device \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/composer \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/composer/vsync \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/cmdlist \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/dacc \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/isr \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/utils

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f