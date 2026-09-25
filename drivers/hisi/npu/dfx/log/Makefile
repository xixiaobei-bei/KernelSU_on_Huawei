ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
		CHIP_VERSION=hi3690_es
	else ifeq ($(chip_type),cs2)
		CHIP_VERSION=kirin990_cs2
	else
		CHIP_VERSION=hi3690
	endif

else ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	CHIP_VERSION=hi6280

endif

$(warning chip version $(CHIP_VERSION))
ifeq ($(fpga),true)
EXTRA_CFLAGS += -DTEMP_FPGA_B100
endif

KERNEL_DIR :=$(srctree)
$(warning KERNEL_DIR $(srctree))

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_conversion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/lowpower
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/log
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/npu_v100

ifeq ($(CONFIG_PLATFORM_HI6280),y)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/hi6280
obj-$(CONFIG_NPU_LOG_DEVDRV)   += npu_log.o
npu_log-objs := npu_dfx_log.o
else
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)
obj-$(CONFIG_NPU_LOG_DEVDRV)   += npu_log.o
npu_log-objs := npu_dfx_log.o
endif

