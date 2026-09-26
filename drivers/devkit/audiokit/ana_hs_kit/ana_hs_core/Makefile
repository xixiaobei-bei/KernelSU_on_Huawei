# invert headset driver
ifdef CONFIG_ANA_HS_CORE
	 ccflags-y += -Idrivers/devkit/audiokit/
endif

obj-$(CONFIG_ANA_HS_CORE) += ana_hs_core.o
ifdef CONFIG_ANA_HS_EXTERN_OPS
obj-$(CONFIG_ANA_HS_CORE) += ana_hs_i2c.o
endif
