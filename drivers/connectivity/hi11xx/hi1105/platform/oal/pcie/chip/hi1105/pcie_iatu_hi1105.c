

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

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

OAL_STATIC oal_pcie_region g_hi1105_pcie_asic_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000BFFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000BFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_WCPU_ITCM"
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_WCPU_DTCM"
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x600FFFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x600FFFFF, /* 512*2KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_W_PKTRAM"
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_W_PERP_AHB"
    },
    {   .pci_start = 0x80000000,
        .pci_end = 0x8014FFFF,
        .cpu_start = 0x80000000,
        .cpu_end = 0x8014FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_BCPU_ITCM"
    },
    {   .pci_start = 0x80200000,
        .pci_end = 0x8030FFFF,
        .cpu_start = 0x80200000,
        .cpu_end = 0x8030FFFF, /* 1088KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_BCPU_DTCM"
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48001FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x483FFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_B_PERP_AHB"
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000FFFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_AON_APB"
    },
};

OAL_STATIC oal_pcie_region g_hi1105_pcie_fpga_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000BFFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000BFFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_WCPU_ITCM"
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_WCPU_DTCM"
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6007FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6007FFFF, /* 512KB */
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_W_PKTRAM"
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_W_PERP_AHB"
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000FFFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000FFFF,
        .flag = OAL_IORESOURCE_REG,
        .name = "HI1105_REGION_AON_APB"
    },
};

int32_t oal_pcie_get_bar_region_info_hi1105(oal_pcie_res *pst_pci_res,
                                            oal_pcie_region **region_base, uint32_t *region_num)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "1105 %s region map", hi110x_is_asic() ? "asic" : "fpga");
    if (hi110x_is_asic()) {
        /* 1105 Asic */
        *region_num = oal_array_size(g_hi1105_pcie_asic_regions);
        *region_base = &g_hi1105_pcie_asic_regions[0];
    } else {
        /* 1105 Fpga */
        *region_num = oal_array_size(g_hi1105_pcie_fpga_regions);
        *region_base = &g_hi1105_pcie_fpga_regions[0];
    }
    return OAL_SUCC;
}

#endif

