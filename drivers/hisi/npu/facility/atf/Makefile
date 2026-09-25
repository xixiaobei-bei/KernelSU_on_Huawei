KERNEL_DIR := $(srctree)

ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
	EXTRA_CFLAGS += -DKIRIN990_ES
	endif
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/services/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc

obj-$(CONFIG_NPU_HISI_ATF_SEVICES) += npu_atf_subsys.o

