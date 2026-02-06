obj-y	+= lcd_kit_adapt.o
obj-y	+= lcd_kit_sysfs_hs.o
obj-y	+= bias_bl_utils.o
ifneq ($(TARGET_BUILD_VARIANT),user)
obj-y += lcd_kit_debug_hs.o
endif
ifeq ($(TARGET_VERSION_MODE),factory)
obj-y   += lcd_kit_factory_hs.o
EXTRA_CFLAGS += -DLCD_FACTORY_MODE
endif
EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dss \
		-Idrivers/video/hisi/dss \
		-Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/hisi/tzdriver \
		-Idrivers/base \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include
