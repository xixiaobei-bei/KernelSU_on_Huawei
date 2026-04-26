KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_convertor
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/lowpower
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream_manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/log
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_DEVDRV_DEVDRIVER) += npu_devdrv.o
npu_devdrv-objs := npu_common.o npu_devinit.o core/npu_ioctl_services.o core/npu_proc_ctx.o core/npu_recycle.o
npu_devdrv-objs += format_convertor/npu_comm_sqe_fmt.o format_convertor/npu_comm_task_verify.o
npu_devdrv-objs += heartbeat/npu_heart_beat.o
npu_devdrv-objs += stream_manager/npu_calc_channel.o stream_manager/npu_hwts.o stream_manager/npu_sink_stream.o stream_manager/npu_stream.o

ifeq ($(CONFIG_HISI_NPU_DPM_ENABLED), y)
	EXTRA_CFLAGS += -DCONFIG_NPU_DPM_ENABLED
	npu_devdrv-objs += lowpower/npu_dpm.o
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager/v100

	npu_devdrv-objs += power_manager/v100/npu_pm_framework.o
else
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager
	EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/config

	npu_devdrv-objs += core/npu_hw_exp_irq.o
	npu_devdrv-objs += power_manager/npu_pm_framework.o
	npu_devdrv-objs += config/npu_pm_config_aicore.o config/npu_pm_config_npusubsys.o
	npu_devdrv-objs += config/npu_pm_config_nputop.o config/npu_pm_config_smmutbu.o config/npu_pm_config_tscpu.o
	npu_devdrv-objs += lowpower/npu_powercapping.o lowpower/npu_autofs.o
endif
