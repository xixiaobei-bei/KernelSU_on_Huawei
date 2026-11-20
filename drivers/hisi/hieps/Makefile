EXTRA_CFLAGS += -Idrivers/tzdriver
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
EXTRA_CFLAGS += -Idrivers/hisi/hieps/mspe_at
EXTRA_CFLAGS += -Idrivers/hisi/hieps/power

subdir-ccflags-y              += -Wall -Werror

obj-$(CONFIG_HIEPS)  += hieps.o

ifeq ($(CONFIG_HIEPS_POWERON_WITH_SMMU),y)
obj-$(CONFIG_HIEPS)  += power/tcu_power.o
else
obj-$(CONFIG_HIEPS)  += power/hieps_power.o
endif

obj-$(CONFIG_HIEPS_SMX_ATCMD)  += \
	mspe_at/mspe_at.o \
	mspe_at/mspe_at_smx.o

ifeq ($(CONFIG_HISI_PERIDVFS),y)
    EXTRA_CFLAGS += -Idrivers/clk/hisi/dvfs/
endif

