# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/hisi/tzdriver

# UFSHCD makefile
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs-kirin.o ufshcd-kirin-extend.o
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs-kirin-hisi-mphy.o
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs_mphy_firmware.o
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs-hpb.o
obj-$(CONFIG_SCSI_UFS_ARIES) += ufs-aries.o
obj-$(CONFIG_SCSI_UFS_CANCER) += ufs-cancer.o
obj-$(CONFIG_SCSI_UFS_TAURUS) += ufs-taurus.o
obj-$(CONFIG_SCSI_UFS_PISCES) += ufs-pisces.o
obj-$(CONFIG_SCSI_UFS_SCORPIO) += ufs-scorpio.o
obj-$(CONFIG_HISI_UFS_HC) += ufs-hisi.o ufs-hisi-dump.o
obj-$(CONFIG_SCSI_UFS_LIBRA) += ufs-libra.o
obj-$(CONFIG_SCSI_UFS_CAPRICORN) += ufs-capricorn.o
obj-$(CONFIG_SCSI_UFS_LEO) += ufs-leo.o
obj-$(CONFIG_SCSI_UFS_AQUARIUS) += ufs-leo.o
obj-$(CONFIG_SCSI_UFS_VIRGO) += ufs-virgo.o
obj-$(CONFIG_SCSI_UFS_SGR) += ufs-sgr.o
obj-$(CONFIG_SCSI_UFS_DWC_TC_PCI) += tc-dwc-g210-pci.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_DWC_TC_PLATFORM) += tc-dwc-g210-pltfrm.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_QCOM) += ufs-qcom.o
obj-$(CONFIG_SCSI_UFSHCD) += ufshcd.o
obj-$(CONFIG_RPMB_UFS) += ufs_rpmb.o
obj-$(CONFIG_HUAWEI_UFS_VENDOR_MODE) += ufs-vendor-mode.o
obj-$(CONFIG_SCSI_UFSHCD_PCI) += ufshcd-pci.o
obj-$(CONFIG_SCSI_UFSHCD_PLATFORM) += ufshcd-pltfrm.o
obj-$(CONFIG_SCSI_UFS_FAULT_INJECT) += ufs-fault-inject.o
obj-$(CONFIG_SCSI_UFS_BLOCK_TEST) += ufs_block_test.o
obj-$(CONFIG_SCSI_UFS_TEST) += ufs_test.o
obj-$(CONFIG_HISI_DEBUG_FS) += ufs_debugfs.o
obj-$(CONFIG_HUAWEI_UFS_DSM) += dsm_ufs.o
obj-$(CONFIG_AB_PARTITION_TABLE) += ufs-kirin-lib.o
obj-$(CONFIG_HISI_UFS_MANUAL_BKOPS) += hisi-ufs-bkops.o hisi_ufs_bkops_interface.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufs-vendor-cmd.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufs_vcmd_proc.o
obj-$(CONFIG_SCSI_UFS_HI1861_VCMD) += ufstt.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_core.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_read.o
obj-$(CONFIG_SCSI_UFS_UNISTORE) += ufs_unistore_write.o

ifeq ($(CONFIG_HISI_DEBUG_FS),y)
obj-$(CONFIG_HISI_UFS_HC) += ufs_hisi_test.o
endif

ifeq ($(TARGET_VERSION_MODE),factory)
    obj-$(CONFIG_HUAWEI_UFS_TRACE) += ufs_trace.o
    EXTRA_CFLAGS += -DHEALTH_INFO_FACTORY_MODE
    EXTRA_CFLAGS += -DUFS_TRACE_FLAG
endif
