KERNEL_DIR :=$(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEBUG_TUI
endif

ifeq ($(CONFIG_TEE_TUI_MTK), y)
EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/tzdriver/core
EXTRA_CFLAGS += -Idrivers/gpu/drm/mediatek
EXTRA_CFLAGS += -Idrivers/misc/mediatek/memory-ssmr
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/mtk/include
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/mtk/adapt
else
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/video/hisi/dss
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/video/fbdev/hisi/dss
endif

ifeq ($(CONFIG_HW_SECMEM), y)
EXTRA_CFLAGS += -Idrivers/uni_drivers/secmem
endif

obj-$(CONFIG_TEE_TUI) += tui.o