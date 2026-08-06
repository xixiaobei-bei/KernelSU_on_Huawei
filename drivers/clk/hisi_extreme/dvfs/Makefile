obj-$(CONFIG_HISI_PERIDVFS)		+= peri-dvfs-volt.o
ifeq ($(CONFIG_HISI_HW_PERI_DVS),y)
obj-$(CONFIG_HISI_PERIDVFS)		+= hw-peri-dvs.o
else
obj-$(CONFIG_HISI_PERIDVFS)		+= plat-peri-dvfs-vlot.o
endif
