
ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DLCD_KIT_DEBUG_ENABLE
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
	obj-y += lcd_kit_debug.o
endif

ifeq ($(TARGET_VERSION_MODE),factory)
obj-y   += lcd_kit_factory.o
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif

ifeq ($(CONFIG_HUAWEI_DEVKIT_HISI_3_0),y)
	EXTRA_CFLAGS += -DLV_GET_LCDBK_ON
endif

obj-y	+= lcd_kit_sysfs.o
obj-y	+= lcd_kit_common.o
obj-y	+= lcd_kit_parse.o
obj-y	+= lcd_kit_core.o
obj-y	+= lcd_kit_bl.o
obj-y	+= lcd_kit_bias.o
obj-y	+= bias_ic_common.o
obj-y	+= bias_bl_common.o
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include
EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dss
EXTRA_CFLAGS += -Idrivers/video/hisi/dss \
		-Idrivers/base
