ifeq ($(NPU_CHIP_VERSION),hi3690)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC

else ifeq ($(NPU_CHIP_VERSION),kirin990_cs2)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC

else ifeq ($(NPU_CHIP_VERSION),hi6280)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC

else ifeq ($(NPU_CHIP_VERSION),hi6290)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC

else ifeq ($(NPU_CHIP_VERSION),hi36a0)
	EXTRA_CFLAGS += -DCONFIG_NPU_NOC
endif

KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_convertor
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream_manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/log
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)

obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += npu_manager_module.o

npu_manager_module-objs := npu_manager.o npu_manager_ioctl_services.o npu_manager_common.o
npu_manager_module-objs += ../dfx/bbox/npu_dfx_black_box.o

ifeq ($(NPU_ARCH_VERSION),npu_v100)
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager/v100
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/bbox/v100
else
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/bbox/adapter/$(NPU_CHIP_VERSION)/
endif
