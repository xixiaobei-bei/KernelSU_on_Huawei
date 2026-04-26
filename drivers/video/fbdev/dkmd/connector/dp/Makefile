obj-$(CONFIG_DKMD_DPU_DP) += dpu_dp.o

dpu_dp-objs := \
	dp_drv.o \
	dp_dev.o

EXTRA_CFLAGS += \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/connector/dp \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Iarch/arm64/include/ \
	-Iinclude/

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d