

#ifndef __PCIE_PM_HISI_H
#define __PCIE_PM_HISI_H

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "pcie_host.h"

int32_t oal_pcie_pm_chip_init(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_hi1103(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_hi1105(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t pcie_pm_chip_init_hi1106(oal_pcie_res *pst_pci_res, int32_t device_id);
#endif

#endif
