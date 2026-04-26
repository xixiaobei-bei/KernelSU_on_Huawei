

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME     "[PCIEC]"
#define HI11XX_LOG_MODULE_NAME_VAR pciec_loglevel
#define HISI_LOG_TAG               "[PCIEC]"

#include "pcie_chip.h"
#include "chip/hi1103/pcie_chip_hi1103.h"
#include "chip/hi1106/pcie_chip_hi1106.h"

#include "pcie_linux.h"
#include "pcie_pm.h"

static pcie_chip_id_stru g_pcie_chipinfo_id[] = {
    {PCIE_HISI_DEVICE_ID_HI1103,     oal_pcie_chip_info_init_hi1103},
    {PCIE_HISI_DEVICE_ID_HI1105,     oal_pcie_chip_info_init_hi1103},
    {PCIE_HISI_DEVICE_ID_HI1106,     oal_pcie_chip_info_init_hi1106},
    {PCIE_HISI_DEVICE_ID_HI1106FPGA, oal_pcie_chip_info_init_hi1106}
};

static int32_t oal_pcie_chip_info_res_init(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    int32_t i;
    int32_t chip_num = oal_array_size(g_pcie_chipinfo_id);
    for (i = 0; i < chip_num; i++) {
        if (g_pcie_chipinfo_id[i].device_id == device_id) {
            if (g_pcie_chipinfo_id[i].func != NULL) {
                return g_pcie_chipinfo_id[i].func(pst_pci_res, device_id);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie chip init 0x%x init null func", device_id);
                return OAL_SUCC;
            }
        }
    }
    oal_print_hi11xx_log(HI11XX_LOG_ERR, "not implement chip info init device_id=0x%x", device_id);
    return -OAL_ENODEV;
}

int32_t oal_pcie_chip_info_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    uint32_t device_id;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    size_t len = sizeof(pst_pci_res->chip_info.addr_info);

    (void)memset_s(&pst_pci_res->chip_info.addr_info, len, 0, len);

    device_id = (uint32_t)pst_pci_dev->device;
    ret = oal_pcie_pm_chip_init(pst_pci_res, device_id);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_chip_info_res_init(pst_pci_res, device_id);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return ret;
}

unsigned long oal_pcie_slt_get_deivce_ram_cpuaddr(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.cb.get_test_ram_address == NULL) {
        return 0xffffffff; // invalid address
    }
    return (unsigned long)pst_pci_res->chip_info.cb.get_test_ram_address();
}

int32_t oal_pcie_host_slave_address_switch(oal_pcie_res *pst_pci_res, uint64_t src_addr,
                                           uint64_t* dst_addr, int32_t is_host_iova)
{
    if (oal_warn_on(pst_pci_res->chip_info.cb.pcie_host_slave_address_switch == NULL)) {
        *dst_addr = 0;
        return -OAL_ENODEV; // needn't address switch
    }
    return pst_pci_res->chip_info.cb.pcie_host_slave_address_switch(pst_pci_res,
                                                                    src_addr, dst_addr, is_host_iova);
}
#endif

