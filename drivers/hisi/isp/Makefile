#ifneq ($(TARGET_BUILD_VARIANT),user)
#ifneq ($(kcov),true)
ifeq ($(CONFIG_HISP_DEBUG), y)
subdir-ccflags-y += -DDEBUG_HISP
#endif
endif

ifeq ($(es_low_freq),true)
subdir-ccflags-y += -DCONFIG_ES_ISP_LOW_FREQ
endif

ifneq ($(chip_type),es)
subdir-ccflags-y += -DCONFIG_CS_ISP
endif

obj-$(CONFIG_HISP_RDR) += mntn/

obj-$(CONFIG_HISTAR_ISP) += hisp_rpm.o nonsec.o sec.o \
                                cma.o smc.o  qos.o hisp_dsm.o \
                                hisp_mempool.o hisp_dtsi.o \
                                hisp_pwr.o sensor.o isp.o \
                                secisp_ca.o hisp_rsc.o hisp_vdev.o \
                                hisp_loadbin.o

ifeq ($(CONFIG_HISTAR_ISP), y)
subdir-ccflags-y += -Wall
subdir-ccflags-y += -I$(srctree)/drivers/hisi/tzdriver \
                    -I$(srctree)/drivers/hisi/load_image \
                    -I$(srctree)/drivers/media/huawei/camera \
                    -I$(srctree)/drivers/media/huawei/camera/sensor \
                    -I$(srctree)/drivers/media/huawei/camera/platform
endif

obj-$(CONFIG_HISP_DPM) += hisp_dpm.o
