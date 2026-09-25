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

KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/services/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_conversion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/$(CHIP_VERSION1)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/id_allocator
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/communication
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

obj-$(CONFIG_NPU_DEVDRV_DEVFACILITY) += npu_facility.o

npu_facility-objs := communication/npu_doorbell.o communication/npu_mailbox_msg.o
npu_facility-objs += memory/npu_ts_subsys.o memory/npu_pool.o memory/npu_shm.o
npu_facility-objs += id_allocator/npu_calc_cq.o id_allocator/npu_calc_sq.o id_allocator/npu_dfx_cq.o id_allocator/npu_dfx_sq.o id_allocator/npu_event.o \
                     id_allocator/npu_hwts_cq.o id_allocator/npu_hwts_sq.o id_allocator/npu_mailbox.o id_allocator/npu_model.o id_allocator/npu_notify.o id_allocator/npu_task.o
