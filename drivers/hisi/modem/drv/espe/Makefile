# balong spe module
subdir-ccflags-y := -Werror

obj-$(CONFIG_BALONG_ESPE) 		+= espe_objs.o
obj-$(CONFIG_BALONG_ESPE)			+= nic/

espe_objs-y				:= spe_core.o spe_platform_balong.o spe_desc.o spe_entry.o spe_port.o spe_dbg.o 
espe_objs-$(CONFIG_DEBUG_FS)		+= spe_debugfs.o
espe_objs-$(CONFIG_BALONG_ESPE_XFRM)	+= spe_xfrm.o

