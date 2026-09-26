KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/bbit_debugfs
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/bbit_debugfs/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/bbit_debugfs/kprobe
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/bbit_debugfs/hiail

LOCAL_PATH := $(call my-dir)

obj-$(CONFIG_NPU_BBIT_DEBUGFS) += npu_bbit_debugfs_module.o
npu_bbit_debugfs_module-objs :=npu_bbit_debugfs.o core/npu_bbit_core.o core/npu_kprobe_core.o kprobe/npu_kprobe_debugfs.o kprobe/npu_kprobe.o

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager/v100
	npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_stub.o
else
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager
	npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_log.o
	npu_bbit_debugfs_module-objs += hiail/npu_bbit_hwts_config.o
endif
