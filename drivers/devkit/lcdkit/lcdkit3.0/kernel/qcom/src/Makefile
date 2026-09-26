obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_power.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= lcd_kit_utils.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= lcd_kit_panel.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= lcd_kit_drm_panel.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= bias/
obj-$(CONFIG_LCD_KIT_QCOM) 	+= backlight/

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif
EXTRA_CFLAGS +=-Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/devkit/lcdkit/lcdkit3.0 \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/adapt \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/src/backlight \
		-I$(srctree)/include/drm \
		-I$(srctree)/drivers/gpu/drm \
		-I$(srctree)/techpack/display/msm \
		-I$(srctree)/techpack/display/msm/dsi
