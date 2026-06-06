KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/service
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/logdrv
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/bbox

obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += npu_manager_module.o

npu_manager_module-objs := npu_proc_ctx.o npu_manager.o npu_ioctl_services.o
npu_manager_module-objs += npu_firmware.o npu_secure.o
npu_manager_module-objs += npu_recycle.o npu_manager_ioctl_services.o
npu_manager_module-objs += npu_heart_beat.o
npu_manager_module-objs += npu_manager_common.o
npu_manager_module-objs += bbox/npu_black_box.o
