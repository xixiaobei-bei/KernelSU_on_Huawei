KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_DEVDRV_DEVDRIVER) += npu_devdrv.o
npu_devdrv-objs := common/npu_common.o common/npu_shm.o common/npu_devinit.o common/npu_doorbell.o common/npu_pm.o common/npu_mailbox_msg.o
npu_devdrv-objs += resource/npu_calc_sq.o resource/npu_calc_cq.o  resource/npu_stream.o  resource/npu_dfx_sq.o resource/npu_dfx_cq.o resource/npu_mailbox.o resource/npu_event.o resource/npu_model.o resource/npu_task.o resource/npu_sink_stream.o
npu_devdrv-objs += service/npu_calc_channel.o
