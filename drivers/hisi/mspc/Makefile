EXTRA_CFLAGS += -Idrivers/hisi/rpmb
EXTRA_CFLAGS += -Idrivers/hisi/mntn/hisee
EXTRA_CFLAGS += -Idrivers/hisi/hisee
EXTRA_CFLAGS += -Idrivers/hisi/mspc/mspc_at

subdir-ccflags-y              += -Wall -Werror

obj-$(CONFIG_MSPC)  += mspc.o \
                            mspc_fs.o \
                            mspc_power.o \
                            mspc_smc.o \
                            mspc_upgrade.o \
                            mspc_factory.o \
                            mspc_flash.o \
                            mspc_smx.o \
                            mspc_at/mspc_at.o \
                            mspc_at/mspc_at_install.o \
                            mspc_at/mspc_at_smx.o \
                            mspc_at/mspc_at_socslt.o \
                            mspc_at/mspc_at_test.o

obj-$(CONFIG_HISI_DEBUG_FS)  += mspc_autotest.o

obj-$(CONFIG_MSPC_SUPPORT_DCS) += mspc_dcs.o

