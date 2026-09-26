# Add your debugging flag (or not) to CFLAGS

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV200), y)
PLATFORM := HiVCodecV200
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV210), y)
EXTRA_CFLAGS += -DHIVCODEC_V210
EXTRA_CFLAGS += -DHIVCODEC_PLATFORM_ECONOMIC
PLATFORM := HiVCodecV210
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV300), y)
PLATFORM := HiVCodecV300
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV310), y)
EXTRA_CFLAGS += -DHIVCODEC_V310
EXTRA_CFLAGS += -DHIVCODEC_PLATFORM_ECONOMIC
PLATFORM := HiVCodecV310
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV500), y)
PLATFORM := HiVCodecV500
EXTRA_CFLAGS += -DHIVCODECV500
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV520), y)
PLATFORM := HiVCodecV500
EXTRA_CFLAGS += -DHIVCODECV520
EXTRA_CFLAGS += -DHIVCODECV500
EXTRA_CFLAGS += -DSMMU_V3
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV600), y)
PLATFORM := HiVCodecV600
EXTRA_CFLAGS += -DHIVCODECV600
EXTRA_CFLAGS += -DHIVCODECV500
EXTRA_CFLAGS += -DSMMU_V3
endif

include drivers/vcodec/hi_vcodec/venc_hivna/drv/venc/drv_venc_make.cfg

EXTRA_CFLAGS += $(VENC_CFLAGS) -fno-pic
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/include
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/include
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/venc
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/product/$(PLATFORM)
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/product/common

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV500), y)
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/product/HiSMMUV150
else ifneq ($(findstring $(CONFIG_VCODEC_PLATFORM_HIVCODECV600)$(CONFIG_VCODEC_PLATFORM_HIVCODECV520), y),)
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/product/HiSMMUV300
endif

ifeq ($(chip_type), )
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ap/platform/$(TARGET_BOARD_PLATFORM)
else
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ap/platform/$(TARGET_BOARD_PLATFORM)_$(chip_type)
endif

ifeq ($(es_low_freq),true)
EXTRA_CFLAGS += -DCONFIG_ES_VENC_LOW_FREQ
endif

ifneq ($(TARGET_BUILD_VARIANT), user)
EXTRA_CFLAGS += -DVENC_DEBUG_ENABLE
endif

obj-$(CONFIG_HI_VCODEC_VENC_HIVNA) += hi_omxvenc.o
hi_omxvenc-objs :=  venc_regulator.o     \
                    drv_venc_intf.o      \
                    drv_venc_efl.o       \
                    drv_venc_osal.o      \
                    drv_venc.o           \
                    drv_mem.o         \
                    power_manager.o   \
                    ../product/$(PLATFORM)/hal_venc.o

ifeq ($(CONFIG_VCODEC_DPM_ENABLE),y)
EXTRA_CFLAGS += -DVENC_DPM_ENABLE
hi_omxvenc-objs := $(hi_omxvenc-objs) drv_venc_dpm.o
endif

ifeq ($(CONFIG_TZDRIVER), y)
ifneq ($(filter HiVCodecV500 HiVCodecV600,$(PLATFORM)),)
hi_omxvenc-objs := $(hi_omxvenc-objs) tvp_adapter.o
EXTRA_CFLAGS += -DSUPPORT_SECURE_VENC
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
endif
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV500), y)
hi_omxvenc-objs := $(hi_omxvenc-objs) ../product/HiSMMUV150/smmu.o \
                   ../product/common/venc/hal_venc_common.o \
                   ../product/common/hal_common.o
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV520), y)
hi_omxvenc-objs := $(hi_omxvenc-objs) ../product/HiSMMUV300/smmu.o \
                   ../product/common/venc/hal_venc_common.o \
                   ../product/common/hal_common.o
endif

ifeq ($(CONFIG_VCODEC_PLATFORM_HIVCODECV600), y)
hi_omxvenc-objs := $(hi_omxvenc-objs) ../product/HiSMMUV300/smmu.o \
               ../product/common/venc/hal_venc_common.o \
               ../product/common/hal_common.o
endif

ifeq ($(SUPPORT_CMDLIST),YES)
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/venc_hivna/drv/product/common/cmdlist
hi_omxvenc-objs := $(hi_omxvenc-objs) ../product/common/cmdlist/hal_cmdlist.o \
                    drv_cmdlist.o
endif

#obj-m += venc.o
#venc-y :=          venc_regulator.o     \
                    drv_venc_intf.o      \
                    drv_venc_efl.o       \
                    drv_venc_osal.o      \
                    drv_venc.o           \
                    hal_venc.o           \
                    drv_mem.o
