# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the kernel mmc core.
#

obj-$(CONFIG_MMC)		+= mmc_core.o
mmc_core-y			:= core.o bus.o host.o \
				   mmc.o mmc_ops.o sd.o sd_ops.o \
				   sdio.o sdio_ops.o sdio_bus.o \
				   sdio_cis.o sdio_io.o sdio_irq.o \
				   slot-gpio.o \
				   cache_enable.o

mmc_core-$(CONFIG_MMC_PASSWORDS) += lock.o sysfs.o
mmc_core-$(CONFIG_OF)		+= pwrseq.o
obj-$(CONFIG_PWRSEQ_SIMPLE)	+= pwrseq_simple.o
obj-$(CONFIG_PWRSEQ_SD8787)	+= pwrseq_sd8787.o
obj-$(CONFIG_PWRSEQ_EMMC)	+= pwrseq_emmc.o
mmc_core-$(CONFIG_DEBUG_FS)	+= debugfs.o
obj-$(CONFIG_MMC_BLOCK)		+= mmc_block.o
mmc_block-objs			:= block.o queue.o
obj-$(CONFIG_MMC_TEST)		+= mmc_test.o
obj-$(CONFIG_SDIO_UART)		+= sdio_uart.o
obj-$(CONFIG_HISI_DEBUG_FS)	+= hisi_mmc_sysfs.o
obj-$(CONFIG_HISI_BOOTDEVICE) 	+= emmc-rpmb.o
obj-$(CONFIG_MMC_CQ_HCI) 	+= hisi_cmdq_core.o
obj-$(CONFIG_HISI_MMC) 		+= hisi_core_mmc.o
obj-$(CONFIG_HISI_AB_PARTITION) += mmc-kirin-lib.o
obj-$(CONFIG_HISI_MMC_MANUAL_BKOPS) += hisi_mmc_bkops.o

obj-$(CONFIG_MMC_FFU)		+= ffu.o

obj-$(CONFIG_HW_SD_HEALTH_DETECT)	+= mmc_health_diag.o
obj-$(CONFIG_HW_MMC_TEST_MODULE)	+= hw_mmc_test.o
obj-$(CONFIG_HW_SYSTEM_HW_WR_PROTECT) += emmc_system_wp.o
obj-$(CONFIG_HUAWEI_EMMC_DSM) += dsm_emmc.o
obj-$(CONFIG_HUAWEI_SDCARD_DSM) += dsm_sdcard.o
obj-$(CONFIG_MMC_CQ_HCI) += hisi_cmdq_card.o
obj-$(CONFIG_HISI_RPMB_MMC) += hisi_rpmb_ioctl.o
ifeq ($(TARGET_VERSION_MODE),factory)
	EXTRA_CFLAGS += -DBYPASS_AUTHORITY_VERIFY
endif
