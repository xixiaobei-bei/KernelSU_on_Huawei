KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/service
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/prof
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_PROF_DEVDRV)   += drv_prof.o
drv_prof-objs := prof_drv_dev.o
