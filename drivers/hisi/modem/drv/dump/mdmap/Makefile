#
# Makefile for the memory technology device drivers.
#
# Core functionality.


ifeq ($(strip $(CONFIG_HISI_DUMP)),y)
else
obj-y				+= dump_baseinfo.o
endif
obj-y				+= dump_mdmap_core.o
obj-y				+= dump_area.o
obj-y				+= dump_hook.o
obj-y                           += dump_ko.o
