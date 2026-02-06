obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_adapt.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_sysfs_qcom.o
ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif
EXTRA_CFLAGS += -Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/include \
		-I$(srctree)/include/drm \
		-I$(srctree)/drivers/gpu/drm \
		-I$(srctree)/techpack/display/msm/dsi \
		-I$(srctree)/techpack/display/msm \
		-I$(srctree)/techpack/display/sde
