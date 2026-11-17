ccflags-$(CONFIG_IVP_IVPV120) += -Idrivers/hisi/ivp/ivpv120
ccflags-$(CONFIG_IVP_IVPV160) += -Idrivers/hisi/ivp/ivpv160
ccflags-$(CONFIG_IVP_IVPV200) += -Idrivers/hisi/ivp/ivpv200
ccflags-$(CONFIG_IVP_IVPV210) += -Idrivers/hisi/ivp/ivpv210
ccflags-$(CONFIG_IVP_IVPV250) += -Idrivers/hisi/ivp/ivpv250
ccflags-$(CONFIG_IVP_IVPV300) += -Idrivers/hisi/ivp/ivpv300
ccflags-$(CONFIG_IVP_IVPV310) += -Idrivers/hisi/ivp/ivpv310
ccflags-$(CONFIG_IVP_IVPV320) += -Idrivers/hisi/ivp/ivpv320

obj-$(CONFIG_IVP_SMMU)        += ivp_smmu.o
obj-$(CONFIG_IVP_SMMU_V3)     += ivp_smmuv3.o
obj-$(CONFIG_DEV_IVP)        += ivp_ipc.o
obj-$(CONFIG_DEV_IVP)        += ivp_main.o
obj-$(CONFIG_DEV_IVP)        += ivp_ioctl.o
obj-$(CONFIG_DEV_IVP)        += ivp_manager.o
obj-$(CONFIG_DEV_IVP)        += ivp_map.o

obj-$(CONFIG_IVP_IVPV300)     += ivp_ca.o
obj-$(CONFIG_IVP_IVPV300)     += ivp_multi_algo.o

obj-$(CONFIG_IVP_IVPV200)     += ivp_sec.o
obj-$(CONFIG_IVP_IVPV250)     += ivp_sec.o
obj-$(CONFIG_IVP_IVPV300)     += ivp_sec.o

obj-$(CONFIG_IVP_RDR_SUPPORT) += ivp_rdr.o
obj-$(CONFIG_IVP_IVPV200)     += ivp_atf.o
obj-$(CONFIG_IVP_IVPV250)     += ivp_atf.o
obj-$(CONFIG_IVP_IVPV300)     += ivp_atf.o
obj-$(CONFIG_IVP_IVPV320)     += ivp_atf.o
obj-$(CONFIG_IVP_IVPV320)     += ivp_multi_algo.o

obj-$(CONFIG_IVP_IVPV120)     += ivpv120/
obj-$(CONFIG_IVP_IVPV160)     += ivpv160/
obj-$(CONFIG_IVP_IVPV200)     += ivpv200/
obj-$(CONFIG_IVP_IVPV210)     += ivpv210/
obj-$(CONFIG_IVP_IVPV250)     += ivpv250/
obj-$(CONFIG_IVP_IVPV300)     += ivpv300/
obj-$(CONFIG_IVP_IVPV310)     += ivpv310/
obj-$(CONFIG_IVP_IVPV320)     += ivpv320/

ifneq (, $(filter eng, $(TARGET_BUILD_VARIANT)))
    EXTRA_CFLAGS += -DENG_BUILDMODE
endif

ifeq ($(CONFIG_IVP_RDR_SUPPORT), y)
subdir-ccflags-y += -DIVP_RDR_SUPPORT
endif

ifeq ($(CONFIG_IVP_IVPV200), y)
subdir-ccflags-y += -DSEC_IVP_ENABLE \
                    -DSEC_IVP_V200
endif

ifeq ($(CONFIG_IVP_IVPV210), y)
subdir-ccflags-y += -DIVP_QOS_SUPPORT
endif

ifeq ($(CONFIG_IVP_IVPV250), y)
subdir-ccflags-y += -DIVP_QOS_SUPPORT \
                    -DSEC_IVP_ENABLE  \
                    -DSEC_IVP_V250 \
                    -DIVP_CHIPTYPE_SUPPORT
endif

ifeq ($(CONFIG_IVP_IVPV300), y)
subdir-ccflags-y += -DIVP_DUAL_CORE \
                    -DMULTIPLE_ALGO \
                    -DSEC_IVP_ENABLE \
                    -DSEC_IVP_V300
endif

ifeq ($(CONFIG_IVP_IVPV310), y)
subdir-ccflags-y += -DIVP_QOS_SUPPORT
endif

ifeq ($(CONFIG_IVP_IVPV320), y)
subdir-ccflags-y += -DMULTIPLE_ALGO \
                    -DIVP_QOS_SUPPORT
endif

ifeq ($(CONFIG_IVP_IVPV160), y)
subdir-ccflags-y += -DIVP_FAMA_SUPPORT
endif

subdir-ccflags-y += -I$(srctree)/drivers/hisi/tzdriver/ \
                    -I$(srctree)/lib/libc_sec/securec_v2/include/ \
                    -I$(srctree)/drivers/hisi/load_image
