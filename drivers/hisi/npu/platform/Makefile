KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/config
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_convertor
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream_manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/bbit_debugfs
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/lowpower

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager/v100
else
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager
endif

obj-$(CONFIG_NPU_DEVDRV_DEVPLATFORM) += npu_adapt.o
obj-$(CONFIG_NPU_DEVDRV_DEVPLATFORM) += npu_platform_module.o
npu_platform_module-objs := npu_platform.o npu_adapter_pm.o
npu_platform_module-objs += dts/npu_dfx.o dts/npu_feature.o dts/npu_gic.o dts/npu_irq.o dts/npu_reg.o npu_iova_dev.o
npu_adapt-objs := $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_adapter.o
npu_adapt-objs += $(NPU_ARCH_VERSION)/npu_resmem.o

ifeq ($(NPU_ARCH_VERSION),npu_v100)
npu_adapt-objs += $(NPU_ARCH_VERSION)/npu_sink_task_verify.o
npu_adapt-objs += $(NPU_ARCH_VERSION)/npu_sink_sqe_fmt.o
endif

ifeq ($(NPU_ARCH_VERSION),npu_v200)
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_hwts_plat.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_sink_task_verify.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_sink_sqe_fmt.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_platform_pm.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_aicore_plat.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_sdma_plat.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_platform_exception.o
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_chip_cfg.o
endif

ifeq ($(NPU_CHIP_VERSION),kirin990_cs2)
	npu_adapt-objs += $(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)/npu_chip_cfg.o
endif
