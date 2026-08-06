obj-$(CONFIG_DKMD_DPU_PANEL) += dpu_panel.o

dpu_panel-objs := \
	panel_drv.o \
	panel_dev.o \
	panel_lg_td4322.o

EXTRA_CFLAGS += \
	-Iinclude/ \
	-Iarch/arm64/include/ \
	-Idrivers/hisi/ap/platform/$(TARGET_PRODUCT)_es \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/connector/mipi \
	-Idrivers/video/fbdev/dkmd/connector/panel


clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
