# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_PCIE_DW) += pcie-designware.o
obj-$(CONFIG_PCIE_DW_HOST) += pcie-designware-host.o
obj-$(CONFIG_PCIE_DW_EP) += pcie-designware-ep.o
obj-$(CONFIG_PCIE_DW_PLAT) += pcie-designware-plat.o
ifneq ($(filter y,$(CONFIG_PCI_DRA7XX_HOST) $(CONFIG_PCI_DRA7XX_EP)),)
        obj-$(CONFIG_PCI_DRA7XX) += pci-dra7xx.o
endif
obj-$(CONFIG_PCI_EXYNOS) += pci-exynos.o
obj-$(CONFIG_PCI_IMX6) += pci-imx6.o
obj-$(CONFIG_PCIE_SPEAR13XX) += pcie-spear13xx.o
obj-$(CONFIG_PCI_KEYSTONE) += pci-keystone-dw.o pci-keystone.o
obj-$(CONFIG_PCI_LAYERSCAPE) += pci-layerscape.o
obj-$(CONFIG_PCIE_QCOM) += pcie-qcom.o
obj-$(CONFIG_PCIE_ARMADA_8K) += pcie-armada8k.o
obj-$(CONFIG_PCIE_ARTPEC6) += pcie-artpec6.o
obj-$(CONFIG_PCIE_KPORT) += pcie-kport.o
obj-$(CONFIG_PCIE_KPORT_V1) += pcie-kport-v1.o
obj-$(CONFIG_PCIE_KPORT) += pcie-kport-common.o
obj-$(CONFIG_PCIE_KPORT_PHY_XW) += pcie-kport-phy-xw.o
obj-$(CONFIG_PCIE_KPORT_PHY) += pcie-kport-phy.o
obj-$(CONFIG_PCIE_KPORT_MNTN) += pcie-mntn.o
obj-$(CONFIG_PCIE_KPORT_TEST) += pcie-kport-test.o
obj-$(CONFIG_PCIE_KPORT_SLT) += pcie-slt.o
obj-$(CONFIG_PCIE_KPORT_FEB) += pcie-feb.o
obj-$(CONFIG_PCIE_KPORT_MAR) += pcie-mar.o
obj-$(CONFIG_PCIE_KPORT_APR) += pcie-apr.o
obj-$(CONFIG_PCIE_KPORT_MAY) += pcie-may.o
obj-$(CONFIG_PCIE_KPORT_SEP) += pcie-sep.o
obj-$(CONFIG_PCIE_KPORT_BURB) += pcie-burb.o
obj-$(CONFIG_PCIE_KPORT_JUN) += pcie-jun.o
obj-$(CONFIG_PCIE_KPORT_IDLE) += pcie-kport-idle.o
# The following drivers are for devices that use the generic ACPI
# pci_root.c driver but don't support standard ECAM config access.
# They contain MCFG quirks to replace the generic ECAM accessors with
# device-specific ones that are shared with the DT driver.

# The ACPI driver is generic and should not require driver-specific
# config options to be enabled, so we always build these drivers on
# ARM64 and use internal ifdefs to only build the pieces we need
# depending on whether ACPI, the DT driver, or both are enabled.

obj-$(CONFIG_ARM64) += pcie-hisi.o
