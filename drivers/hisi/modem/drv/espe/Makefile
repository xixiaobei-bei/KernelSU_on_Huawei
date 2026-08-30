# balong spe module
subdir-ccflags-y := -Werror

obj-$(CONFIG_BALONG_ESPE) 		+= espe_objs.o
#obj-$(CONFIG_BALONG_ESPE)			+= func/

espe_objs-y				:= espe_core.o espe_platform_balong.o espe_desc.o espe_ip_entry.o espe_mac_entry.o espe_port.o espe_dbg.o 
espe_objs-$(CONFIG_DEBUG_FS)		+= espe_debugfs.o
#espe_objs-$(CONFIG_BALONG_ESPE_XFRM)	+= espe_xfrm.o

