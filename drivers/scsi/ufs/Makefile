# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/hisi/tzdriver

# UFSHCD makefile
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs-kirin.o ufs-hisi.o
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs-kirin-hisi-mphy.o
obj-$(CONFIG_SCSI_UFS_KIRIN) += ufs_mphy_firmware.o
obj-$(CONFIG_SCSI_UFS_GEMINI) += ufs-gemini.o
obj-$(CONFIG_SCSI_UFS_ARIES) += ufs-aries.o
obj-$(CONFIG_SCSI_UFS_CANCER) += ufs-cancer.o
obj-$(CONFIG_SCSI_UFS_TAURUS) += ufs-taurus.o
obj-$(CONFIG_SCSI_UFS_PISCES) += ufs-pisces.o
obj-$(CONFIG_SCSI_UFS_LIBRA) += ufs-libra.o
obj-$(CONFIG_SCSI_UFS_CAPRICORN) += ufs-capricorn.o
obj-$(CONFIG_SCSI_UFS_LEO) += ufs-leo.o
obj-$(CONFIG_SCSI_UFS_DWC_TC_PCI) += tc-dwc-g210-pci.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_DWC_TC_PLATFORM) += tc-dwc-g210-pltfrm.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_QCOM) += ufs-qcom.o
obj-$(CONFIG_SCSI_UFSHCD) += ufshcd.o
obj-$(CONFIG_HISI_BOOTDEVICE) += ufs-rpmb.o
obj-$(CONFIG_HUAWEI_UFS_VENDOR_MODE) += ufs_vendor_mode.o
obj-$(CONFIG_SCSI_UFSHCD_PCI) += ufshcd-pci.o
obj-$(CONFIG_SCSI_UFSHCD_PLATFORM) += ufshcd-pltfrm.o
obj-$(CONFIG_SCSI_UFS_FAULT_INJECT) += ufs_fault_inject.o
obj-$(CONFIG_SCSI_UFS_BLOCK_TEST) += ufs_block_test.o
obj-$(CONFIG_SCSI_UFS_TEST) += ufs_test.o
obj-$(CONFIG_HISI_DEBUG_FS) += ufs_debugfs.o
obj-$(CONFIG_HUAWEI_UFS_DSM) += dsm_ufs.o
obj-$(CONFIG_HISI_AB_PARTITION) += ufs-kirin-lib.o
obj-$(CONFIG_HISI_BOOTDEVICE) += ufs-rpmb.o
obj-$(CONFIG_HISI_UFS_MANUAL_BKOPS) += hisi_ufs_bkops.o hisi_ufs_bkops_interface.o

ifeq ($(TARGET_VERSION_MODE),factory)
    EXTRA_CFLAGS += -DHEALTH_INFO_FACTORY_MODE
endif
