# Hisilicon IPP Makefile

obj-$(CONFIG_HISPIPP) += ipp-core.o ipp-com.o memory.o smc.o
obj-$(CONFIG_HISPIPP) += v200/ device/
obj-$(CONFIG_HISPIPP) += adapter_common.o \
                            adapter_mcf.o \
                            adapter_mfnr.o \
                            adapter_slam.o \
                            adapter_slamtest.o

subdir-ccflags-$(CONFIG_HISPIPP) += -Wall -Werror
subdir-ccflags-$(CONFIG_HISPIPP) += -I$(srctree)/drivers/hisi/ipp \
                                    -I$(srctree)/drivers/hisi/ipp/v200 \
                                    -I$(srctree)/drivers/hisi/ipp/device \
                                    -I$(srctree)/drivers/hisi/ipp/include \
                                    -I$(srctree)/drivers/hisi/ipp/v200/include \
                                    -I$(srctree)/drivers/hisi/ipp/device/include \
                                    -I$(srctree)/drivers/hisi/tzdriver
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP) += -DCONFIG_IPP_DEBUG
endif
