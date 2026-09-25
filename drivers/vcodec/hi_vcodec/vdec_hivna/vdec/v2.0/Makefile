VFMW_DIR := drivers/vcodec/hi_vcodec/vdec_hivna/vfmw/v6.2

VFMW_INC_DIR := $(VFMW_DIR)

LINUX_VER := linux
CHIP_VER := hi36a0

EXTRA_CFLAGS += -DENV_ARMLINUX_KERNEL

EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/include
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/common
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/v2.0
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/v2.0/smmu
EXTRA_CFLAGS += -I$(VFMW_INC_DIR)/include
EXTRA_CFLAGS += -I$(VFMW_INC_DIR)/intf

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV600), y)
EXTRA_CFLAGS += -DPLATFORM_HIVCODECV600
endif

#MOD_NAME := hisi_vcodec_dec_ctrl
#build in
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_dec_ctrl.o
#obj-m += $(MOD_NAME).o
hisi_vcodec_dec_ctrl-objs += hisi_vcodec_vdec.o			\
			smmu/smmu.o				\
			../common/hisi_vcodec_vdec_regulator.o  \
			../common/hisi_vcodec_vdec_memory.o	\
			../common/hisi_vcodec_vdec_plat.o
