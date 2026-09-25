CHIP_VERSION1=npu_v100
ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
		CHIP_VERSION=hi3690_es
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990_ES
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	else ifeq ($(chip_type),cs2)
		CHIP_VERSION=kirin990_cs2
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	else
		CHIP_VERSION=hi3690
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	endif

else ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	CHIP_VERSION=hi6280

endif

ifeq ($(fpga),true)
EXTRA_CFLAGS += -DTEMP_FPGA_B100
endif

KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/$(CHIP_VERSION1)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)

ifeq ($(TARGET_PRODUCT),cloud)
	obj-m += npu_dbg.o
else
	obj-$(CONFIG_NPU_DBG_DEVDRV)   += npu_dbg.o
endif
npu_dbg-objs := npu_debug_resource_count.o
