ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_LCD_KIT_QCOM)   += lcd_kit_factory.o
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif

obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_sysfs.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_common.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_parse.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_core.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_bl.o
obj-$(CONFIG_LCD_KIT_QCOM)	+= lcd_kit_bias.o
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/common/include
