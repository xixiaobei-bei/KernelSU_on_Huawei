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

KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_conversion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/$(CHIP_VERSION1)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/log
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)

obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += npu_manager_module.o

npu_manager_module-objs := npu_firmware.o npu_manager.o npu_manager_ioctl_services.o npu_manager_common.o
npu_manager_module-objs += ../dfx/bbox/npu_dfx_black_box.o
