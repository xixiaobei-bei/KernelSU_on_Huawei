$(warning chip version $(CHIP_VERSION))

KERNEL_DIR :=$(srctree)
$(warning KERNEL_DIR $(srctree))

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/service
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/logdrv

ifeq ($(CONFIG_PLATFORM_HI6280),y)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/hi6280
obj-$(CONFIG_NPU_LOG_DEVDRV)   += drv_log.o
drv_log-objs := log_drv_dev.o log_cache_flush.o
endif

