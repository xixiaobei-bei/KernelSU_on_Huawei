obj-y += mdc.o

mdc-y := hisi_mdc_dev.o hisi_mdc_drv.o hisi_mdc_overlay.o

mdc-objs := $(mdc-y)

EXTRA_CFLAGS += \
    -Idrivers/video/fbdev/hisi/dss \
    -Idrivers/video/fbdev/hisi/dss/overlay

ifeq ($(CONFIG_HISI_PERIDVFS),y)
EXTRA_CFLAGS += -Idrivers/clk/hisi/dvfs
endif

clean:
	rm *.o .*cmd

