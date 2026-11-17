KERNEL_DIR :=$(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEF_ENG
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/agent_rpmb/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/auth
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/ion
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/tlogger
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/tui
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/core

obj-$(CONFIG_TEECD_AUTH) += teec_daemon_auth.o
obj-$(CONFIG_KERNEL_CLIENT) += teek_client_api.o
obj-y += smc_smp.o tc_client_driver.o session_manager.o mailbox_mempool.o
obj-y += agent.o gp_ops.o mem.o cmdmonitor.o tzdebug.o tz_spi_notify.o tz_pm.o
