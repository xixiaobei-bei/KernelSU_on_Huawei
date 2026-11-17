ifeq ($(CONFIG_ADSP_HUAWEI_SHARED_MEM),y)
ifeq ($(CONFIG_MACH_MT6893),y)
MTK_ADSP_PLATFORM := mt6885
else
MTK_ADSP_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
endif
ccflags-y += -I$(srctree)/drivers/misc/mediatek/adsp/$(MTK_ADSP_PLATFORM)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common/framework
obj-$(CONFIG_ADSP_HUAWEI_SHARED_MEM) += adsp_huawei_shared_mem.o
endif

obj-$(CONFIG_HUAWEI_ADSP_MISC) += adsp_misc.o
