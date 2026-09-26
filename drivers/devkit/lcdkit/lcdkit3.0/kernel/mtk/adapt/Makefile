obj-$(CONFIG_LCD_KIT_MTK)	+= lcd_kit_adapt.o
obj-$(CONFIG_LCD_KIT_MTK)	+= lcd_kit_sysfs_mtk.o
obj-$(CONFIG_LCD_KIT_MTK)	+= bias_bl_utils.o
ifneq ($(TARGET_BUILD_VARIANT),user)
obj-$(CONFIG_LCD_KIT_MTK)   += lcd_kit_debug_mtk.o
endif
ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif
EXTRA_CFLAGS += -Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/mtk/include \
		-Idrivers/misc/mediatek/lcm/inc \
		-Idrivers/base
ccflags-$(CONFIG_DRM_MEDIATEK) += -I$(srctree)/drivers/gpu/drm/ \
		-I$(srctree)/drivers/gpu/drm/mediatek/
