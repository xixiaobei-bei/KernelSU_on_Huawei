obj-$(CONFIG_DKMD_DPU_RES_MGR) += dpu_res_mgr.o

dpu_res_mgr-objs := \
	dvfs.o \
	opr_mgr.o \
	res_mgr.o

EXTRA_CFLAGS += \
	-Iinclude/media/dkmd/ \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/utils \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/common/config \
	-Idrivers/video/fbdev/dkmd/dpu/cetus/res_mgr \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Iarch/arm64/include/ \
	-Iinclude/

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
