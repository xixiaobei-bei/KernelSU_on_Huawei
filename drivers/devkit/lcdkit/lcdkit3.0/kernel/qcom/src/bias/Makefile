obj-$(CONFIG_LCD_KIT_QCOM) 	+= tps65132.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= rt4801h.o
obj-$(CONFIG_LCD_KIT_QCOM) 	+= icn68116.o

EXTRA_CFLAGS +=-Idrivers/staging/android \
		-Idrivers/atfdriver \
		-Idrivers/devkit/lcdkit/lcdkit3.0 \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/common/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/include \
		-Idrivers/devkit/lcdkit/lcdkit3.0/kernel/qcom/adapt \
		-I$(srctree)/include/drm \
		-I$(srctree)/drivers/gpu/drm \
		-I$(srctree)/techpack/display/msm \
		-I$(srctree)/techpack/display/msm/dsi
