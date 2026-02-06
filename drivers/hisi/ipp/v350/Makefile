# Hisilicon IPP v350 Makefile
obj-$(CONFIG_HISPIPP_V350) += dataflow/ modules/ segment/
obj-$(CONFIG_HISPIPP_V350) += ipp_com.o ipp_core.o memory.o smc.o

subdir-ccflags-$(CONFIG_HISPIPP_V350) += -Wall -Werror
subdir-ccflags-$(CONFIG_HISPIPP_V350) += -I$(srctree)/drivers/hisi/ipp/v350 \
                                    -I$(srctree)/drivers/hisi/ipp/v350/dataflow \
                                    -I$(srctree)/drivers/hisi/ipp/v350/segment \
									-I$(srctree)/drivers/hisi/ipp/v350/modules \
                                    -I$(srctree)/drivers/hisi/ipp/v350/platform \
									-I$(srctree)/drivers/hisi/ipp/v350/platform/reg_offset_field \
                                    -I$(srctree)/drivers/hisi/ipp/v350/include \
                                    -I$(srctree)/drivers/hisi/ipp/v350/include/config_table \
                                    -I$(srctree)/drivers/hisi/ipp/v350/include/dataflow \
									-I$(srctree)/drivers/hisi/ipp/v350/include/modules \
                                    -I$(srctree)/drivers/hisi/ipp/v350/include/segment \
                                    -I$(srctree)/drivers/hisi/tzdriver
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V350) += -DCONFIG_IPP_DEBUG
endif
