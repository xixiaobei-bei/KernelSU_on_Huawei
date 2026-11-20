VFMW_DIR := drivers/vcodec/hi_vcodec/vdec_hivna/vfmw/vfmw_v6.2

VFMW_INC_DIR := $(VFMW_DIR)

LINUX_VER := linux
PRODUCT_VER := HiVCodecV600

EXTRA_CFLAGS += -DENV_ARMLINUX_KERNEL

EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/include
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/drv_v2.0
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/drv_v2.0/smmu
EXTRA_CFLAGS += -I$(VFMW_INC_DIR)/include
EXTRA_CFLAGS += -I$(VFMW_INC_DIR)/intf

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV600), y)
EXTRA_CFLAGS += -DPLATFORM_HIVCODECV600
endif

#MOD_NAME := hisi_vcodec_dec_ctrl
#build in
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_dec_ctrl.o
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_regulator.o
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_memory.o
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_plat.o
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_utils.o
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_dbg.o
ifeq ($(CONFIG_VCODEC_DPM_ENABLE),y)
EXTRA_CFLAGS += -DVDEC_DPM_ENABLE
obj-$(CONFIG_HI_VCODEC_VDEC_HIVNA) += hisi_vcodec_vdec_dpm.o
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DENABLE_VDEC_PROC
endif

#obj-m += $(MOD_NAME).o
hisi_vcodec_dec_ctrl-objs += hisi_vcodec_vdec.o			\
			smmu/smmu.o
