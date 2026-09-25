EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

subdir-ccflags-y              += -Wall -Werror

obj-$(CONFIG_HISI_HIEPS)  += hisi_hieps.o
obj-$(CONFIG_HISI_HIEPS)  += hieps_power.o

ifeq ($(CONFIG_HISI_PERIDVFS),y)
    EXTRA_CFLAGS += -Idrivers/clk/hisi/dvfs/
endif

