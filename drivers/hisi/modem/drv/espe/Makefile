# balong espe module
subdir-ccflags-y := -Werror


obj-$(CONFIG_BALONG_ESPE) 		+= espe_objs.o


espe_objs-y				:= espe_core.o espe_platform_balong.o espe_desc.o espe_ip_entry.o espe_mac_entry.o espe_port.o espe_dbg.o 

ifeq ($(strip $(CONFIG_ESPE_DEBUG_FS)),YES)
espe_objs-$(CONFIG_DEBUG_FS)		+= espe_debugfs.o
endif

obj-$(CONFIG_BALONG_ESPE)			+= func/
