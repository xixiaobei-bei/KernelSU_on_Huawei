NPU_GLOBAL_CFLAGS :=

ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	NPU_ARCH_VERSION=npu_v100
	ifeq ($(chip_type),es)
		NPU_CHIP_VERSION=hi3690_es
	else ifeq ($(chip_type),cs2)
		NPU_CHIP_VERSION=kirin990_cs2
	else
		NPU_CHIP_VERSION=hi3690
	endif

else ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	NPU_CHIP_VERSION=hi6280
	NPU_ARCH_VERSION=npu_v100

else ifeq ($(TARGET_BOARD_PLATFORM),denver)
	NPU_CHIP_VERSION=hi6290
	NPU_ARCH_VERSION=npu_v100

	ifeq ($(product_type), lite)
	NPU_CUT_MAKEFILE := yes
	endif

else ifeq ($(TARGET_BOARD_PLATFORM),baltimore)
	NPU_CHIP_VERSION=hi36a0
	NPU_ARCH_VERSION=npu_v200
endif

export NPU_CHIP_VERSION NPU_ARCH_VERSION NPU_GLOBAL_CFLAGS

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V100
else ifeq ($(NPU_ARCH_VERSION),npu_v200)
	NPU_GLOBAL_CFLAGS += -DNPU_ARCH_V200
	NPU_GLOBAL_CFLAGS += -DCONFIG_NPU_HWTS
endif

ifneq ($(NPU_CUT_MAKEFILE), yes)
obj-$(CONFIG_NPU_DEVDRV) += platform/
obj-$(CONFIG_NPU_DEVDRV) += manager/
obj-$(CONFIG_NPU_DEVDRV) += device/
obj-$(CONFIG_NPU_DEVDRV) += dfx/log/
obj-$(CONFIG_NPU_DEVDRV) += dfx/dbg/
obj-$(CONFIG_NPU_DEVDRV) += dfx/profiling/
obj-$(CONFIG_NPU_DEVDRV) += facility/
obj-$(CONFIG_NPU_BBIT_DEBUGFS) += bbit_debugfs/
endif
