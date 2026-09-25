KERNEL_DIR := $(srctree)

CHIP_VERSION1=npu_v100
ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
		CHIP_VERSION=hi3690_es
	else ifeq ($(chip_type),cs2)
		CHIP_VERSION=kirin990_cs2
	else
		CHIP_VERSION=hi3690
	endif
	obj-$(CONFIG_PLATFORM_HI3690)   += npu_adapt.o

else ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	CHIP_VERSION=hi6280
	obj-$(CONFIG_PLATFORM_HI6280)   += npu_adapt.o

endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/model/$(CHIP_VERSION1)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/format_conversion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/heartbeat
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/stream
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/facility/memory
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/dts
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)

obj-$(CONFIG_NPU_DEVDRV_DEVPLATFORM) += npu_platform_module.o
npu_platform_module-objs := npu_platform.o npu_adapter_pm.o
npu_platform_module-objs += dts/npu_dfx.o dts/npu_feature.o dts/npu_gic.o dts/npu_irq.o dts/npu_reg.o dts/npu_resmem.o
npu_adapt-objs := $(CHIP_VERSION)/npu_adapter.o
ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),cs2)
		npu_adapt-objs += $(CHIP_VERSION)/npu_chip_cfg.o
	endif
endif
