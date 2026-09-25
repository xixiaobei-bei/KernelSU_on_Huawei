KERNEL_DIR :=$(srctree)

CHIP_VERSION1=npu_v100
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


EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_conversion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/lowpower
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/$(CHIP_VERSION1)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/log
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/../platform/kirin990
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_DEVDRV_DEVDRIVER) += npu_devdrv.o
npu_devdrv-objs := npu_common.o npu_devinit.o core/npu_pm.o core/npu_ioctl_services.o core/npu_proc_ctx.o core/npu_recycle.o
ifeq ($(CHIP_VERSION1), npu_v100)
	npu_devdrv-objs += format_conversion/npu_ts_sqe.o
else
	npu_devdrv-objs += format_conversion/npu_hwts_sqe.o
endif
npu_devdrv-objs += heartbeat/npu_heart_beat.o
npu_devdrv-objs += stream/npu_calc_channel.o stream/npu_hwts.o stream/npu_sink_stream.o stream/npu_stream.o

ifeq ($(CHIP_VERSION),hi3690)
	EXTRA_CFLAGS += -DCONFIG_NPU_DPM_ENABLED
	npu_devdrv-objs += lowpower/npu_dpm.o
endif
