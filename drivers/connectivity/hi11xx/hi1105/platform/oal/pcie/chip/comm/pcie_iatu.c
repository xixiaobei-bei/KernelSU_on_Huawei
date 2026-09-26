

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_linux.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "securec.h"
#include "plat_pm.h"

int32_t oal_pcie_get_bar_region_info(oal_pcie_res *pst_pci_res,
                                     oal_pcie_region **region_base, uint32_t *region_num)
{
    if (pst_pci_res->chip_info.cb.pcie_get_bar_region_info == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_get_bar_region_info is null");
        return -OAL_ENODEV;
    }
    return pst_pci_res->chip_info.cb.pcie_get_bar_region_info(pst_pci_res, region_base, region_num);
}

int32_t oal_pcie_set_outbound_membar(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar *pst_iatu_bar)
{
    if (pst_pci_res->chip_info.cb.pcie_set_outbound_membar == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_set_outbound_membar is null");
        return -OAL_ENODEV;
    }
    return pst_pci_res->chip_info.cb.pcie_set_outbound_membar(pst_pci_res, pst_iatu_bar);
}

#endif

