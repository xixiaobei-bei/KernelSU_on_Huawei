obj-$(CONFIG_DKMD_DPU_DEVICE) += dpu_device.o

dpu_device-objs := \
	gfxdev_mgr.o \
	fb/dpu_fb.o \
	drm/dpu_drm.o \
	chr/dpu_chr.o

EXTRA_CFLAGS += \
	-Iinclude/media/dkmd/ \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/device \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/device/fb \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/device/drm \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/device/chr \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/composer \
	-Iarch/arm64/include/ \
	-Iinclude/ \

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f

