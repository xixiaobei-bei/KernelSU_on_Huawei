obj-$(CONFIG_DKMD_DPU_MIPI_DSI) += dpu_dsi.o

dpu_dsi-objs := \
	mipi_dsi_drv.o \
	mipi_dsi_dev.o \
	mipi_dsi_host.o \
	mipi_cdphy_utils.o

EXTRA_CFLAGS += \
	-Iinclude/ \
	-Iarch/arm64/include/ \
	-Idrivers/hisi/ap/platform/$(TARGET_PRODUCT)_es \
	-Idrivers/video/fbdev/dkmd/dksm \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/connector/panel \
	-Idrivers/video/fbdev/dkmd/connector/mipi \

# ifeq ($(CONFIG_DPU_CONNECTOR_TEST),m)
# 	dpu_dsi-objs += ../test/dsi_fake_composer.o
# endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
