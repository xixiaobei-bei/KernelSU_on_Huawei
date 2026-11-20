

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

int32_t oal_pcie_chiptest_init_hi1103(oal_pcie_res *pst_pci_res, int32_t device_id);
int32_t oal_pcie_chiptest_init_hi1106(oal_pcie_res *pst_pci_res, int32_t device_id);

int32_t oal_pcie_ip_factory_test(hcc_bus *pst_bus, int32_t test_count)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(pst_pci_lres->pst_pci_res == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(pst_pci_lres->pst_pci_res->chip_info.cb.pcie_ip_factory_test == NULL)) {
        return -OAL_EIO;
    }

    return pst_pci_lres->pst_pci_res->chip_info.cb.pcie_ip_factory_test(pst_bus, test_count);
}


int32_t oal_pcie_rc_slt_chip_transfer(hcc_bus *pst_bus, void *ddr_address,
                                      uint32_t data_size, int32_t direction)
{
    oal_pcie_linux_res *pst_pci_lres = NULL;
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_ENODEV;
    }

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pci_lres is null");
        return -OAL_ENODEV;
    }

    if (direction == 1) { /* 1表示 Host to Device */
        return oal_pcie_copy_to_device_by_dword(pst_pci_lres->pst_pci_res, ddr_address,
                                                oal_pcie_slt_get_deivce_ram_cpuaddr(pst_pci_lres->pst_pci_res),
                                                data_size);
    } else if (direction == 2) { /* 2表示 Device to Host */
        return oal_pcie_copy_from_device_by_dword(pst_pci_lres->pst_pci_res, ddr_address,
                                                  oal_pcie_slt_get_deivce_ram_cpuaddr(pst_pci_lres->pst_pci_res),
                                                  data_size);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invaild direction:%d", direction);
        return -OAL_EINVAL;
    }
}

static pcie_chip_id_stru g_pcie_chiptest_id[] = {
    {PCIE_HISI_DEVICE_ID_HI1103,     oal_pcie_chiptest_init_hi1103},
    {PCIE_HISI_DEVICE_ID_HI1105,     oal_pcie_chiptest_init_hi1103},
    {PCIE_HISI_DEVICE_ID_HI1106,     oal_pcie_chiptest_init_hi1106},
    {PCIE_HISI_DEVICE_ID_HI1106FPGA, oal_pcie_chiptest_init_hi1106}
};

int32_t oal_pcie_chiptest_init(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    int32_t i;
    int32_t chip_num = oal_array_size(g_pcie_chiptest_id);
    for (i = 0; i < chip_num; i++) {
        if (g_pcie_chiptest_id[i].device_id == device_id) {
            if (g_pcie_chiptest_id[i].func != NULL) {
                return g_pcie_chiptest_id[i].func(pst_pci_res, device_id);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "chiptest init 0x%x init null func", device_id);
                return OAL_SUCC;
            }
        }
    }
    oal_print_hi11xx_log(HI11XX_LOG_ERR, "not implement chiptest device_id=0x%x", device_id);
    return -OAL_ENODEV;
}

#endif
