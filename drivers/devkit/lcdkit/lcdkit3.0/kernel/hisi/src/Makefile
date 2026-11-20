ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DLCD_KIT_DEBUG_ENABLE
endif

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif
ifeq ($(CONFIG_HUAWEI_DEVKIT_HISI_3_0),y)
EXTRA_CFLAGS += -DLV_GET_LCDBK_ON
endif
obj-y	+= lcd_kit_disp.o
obj-y	+= lcd_kit_power.o
obj-y 	+= lcd_kit_utils.o
obj-y 	+= lcd_kit_panel.o
obj-y 	+= lcd_kit_ext_disp.o
obj-y	+= lcd_kit_ext_power.o

EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dss \
		-Idrivers/video/hisi/dss \
		-Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/hisi/tzdriver \
		-Idrivers/devkit/lcdkit/lcdkit3.0 \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/adapt \
		-Iinclude/chipset_common/log