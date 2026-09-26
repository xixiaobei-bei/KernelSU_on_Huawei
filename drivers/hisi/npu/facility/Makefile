KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += $(NPU_GLOBAL_CFLAGS)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/iommu/hisi
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/services/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_convertor
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream_manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(NPU_ARCH_VERSION)/$(NPU_CHIP_VERSION)
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_DEVDRV_DEVFACILITY) += npu_facility.o

npu_facility-objs := communication/npu_doorbell.o communication/npu_mailbox_msg.o communication/npu_message.o
npu_facility-objs += atf/npu_atf_subsys.o
npu_facility-objs += memory/npu_iova.o memory/npu_pool.o memory/npu_shm.o memory/npu_ion.o memory/npu_svm.o
npu_facility-objs += id_allocator/npu_calc_cq.o id_allocator/npu_calc_sq.o id_allocator/npu_dfx_cq.o id_allocator/npu_dfx_sq.o id_allocator/npu_event.o \
                     id_allocator/npu_hwts_cq.o id_allocator/npu_hwts_sq.o id_allocator/npu_mailbox.o id_allocator/npu_model.o \
                     id_allocator/npu_task.o id_allocator/npu_id_allocator.o id_allocator/npu_hwts_event.o

ifeq ($(NPU_CHIP_VERSION),hi36b0)
    npu_facility-objs += communication/npu_intr_hub.o
endif

ifeq ($(NPU_ARCH_VERSION),npu_v100)
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager/v100
    IGNORE_SRCS := communication/npu_task_message.c
    npu_facility-objs += communication/v100/npu_task_message.o
else
    EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/power_manager
    IGNORE_SRCS := communication/v100/npu_task_message.c
    npu_facility-objs += communication/npu_task_message.o
endif
