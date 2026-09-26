# balong espe module
subdir-ccflags-y := -Werror


obj-$(CONFIG_BALONG_ESPE) 		+= espe_objs.o


espe_objs-y	:= espe_core.o espe_platform_balong.o espe_interrupt.o espe_desc.o espe_ip_entry.o espe_mac_entry.o espe_port.o espe_dbg.o \
               espe_modem_reset.o espe_entry_hook.o


obj-$(CONFIG_BALONG_ESPE)			+= func/