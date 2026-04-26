

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


typedef enum {
    HI1103_REGION_WCPU_ITCM,
    HI1103_REGION_WCPU_DTCM,
    HI1103_REGION_W_PKTRAM,
    HI1103_REGION_W_PERP_AHB,
    HI1103_REGION_BCPU_ITCM,
    HI1103_REGION_BCPU_DTCM,
    HI1103_REGION_B_PERP_AHB,
    HI1103_REGION_AON_APB,
    HI1103_REGION_BUTT
} hi1103_regions;

/* Region大小必须为4KB的倍数，iATU要求 */
/* 这里的分段都是对应iATU inbound */
OAL_STATIC oal_pcie_region g_hi1103_pcie_mpw2_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x0008BFFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x0008BFFF, /* 560KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_WCPU_ITCM)
    },
    {   .pci_start = 0x20000000,
        .pci_end = 0x20067FFF,
        .cpu_start = 0x20000000,
        .cpu_end = 0x20067FFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_WCPU_DTCM)
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6008FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6008FFFF, /* 576KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_W_PKTRAM)
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40102FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40102FFF, /* 1036KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_W_PERP_AHB)
    },
    {   .pci_start = 0x80000000,
        .pci_end = 0x800FFFFF,
        .cpu_start = 0x80000000,
        .cpu_end = 0x800FFFFF, /* 1024KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_BCPU_ITCM)
    },
    {   .pci_start = 0x80100000,
        .pci_end = 0x801DFFFF,
        .cpu_start = 0x80100000,
        .cpu_end = 0x801DFFFF, /* 896KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_BCPU_DTCM)
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48122FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x48122FFF, /* 1164KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_B_PERP_AHB)
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000EDFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000EDFF, /* 59KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_AON_APB)
    },
};

OAL_STATIC oal_pcie_region g_hi1103_pcie_pilot_regions[] = {
    {   .pci_start = 0x00000000,
        .pci_end = 0x000A7FFF,
        .cpu_start = 0x00000000,
        .cpu_end = 0x000A7FFF, /* 592KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_WCPU_ITCM)
    },
    {   .pci_start = 0x20018000,
        .pci_end = 0x2007FFFF,
        .cpu_start = 0x20018000,
        .cpu_end = 0x2007FFFF, /* 416KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_WCPU_DTCM)
    },
    {   .pci_start = 0x60000000,
        .pci_end = 0x6007FFFF,
        .cpu_start = 0x60000000,
        .cpu_end = 0x6007FFFF, /* 512KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_W_PKTRAM)
    },
    {   .pci_start = 0x40000000,
        .pci_end = 0x40107FFF,
        .cpu_start = 0x40000000,
        .cpu_end = 0x40107FFF,
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_W_PERP_AHB)
    },
    {   .pci_start = 0x80040000,
        .pci_end = 0x8010FFFF,
        .cpu_start = 0x80040000,
        .cpu_end = 0x8010FFFF, /* 832KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_BCPU_ITCM)
    },
    {   .pci_start = 0x80200000,
        .pci_end = 0x8030FFFF,
        .cpu_start = 0x80200000,
        .cpu_end = 0x8030FFFF, /* 1088KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_BCPU_DTCM)
    },
    {   .pci_start = 0x48000000,
        .pci_end = 0x48122FFF,
        .cpu_start = 0x48000000,
        .cpu_end = 0x48122FFF, /* 1164KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_B_PERP_AHB)
    },
    {   .pci_start = 0x50000000,
        .pci_end = 0x5000EDFF,
        .cpu_start = 0x50000000,
        .cpu_end = 0x5000EDFF, /* 59KB */
        .flag = OAL_IORESOURCE_REG,
        .name = oal_pcie_to_name(HI1103_REGION_AON_APB)
    },
};


int32_t oal_pcie_get_bar_region_info_hi1103(oal_pcie_res *pst_pci_res,
                                            oal_pcie_region **region_base, uint32_t *region_num)
{
    if (pst_pci_res->revision == PCIE_REVISION_4_70A) {
        *region_num = oal_array_size(g_hi1103_pcie_mpw2_regions);
        *region_base = &g_hi1103_pcie_mpw2_regions[0];
    } else if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
        *region_num = oal_array_size(g_hi1103_pcie_pilot_regions);
        *region_base = &g_hi1103_pcie_pilot_regions[0];
    } else {
        pci_print_log(PCI_LOG_ERR, "unkown pcie ip revision :0x%x\n", pst_pci_res->revision);
        return -OAL_ENODEV;
    }
    return OAL_SUCC;
}

int32_t oal_pcie_set_outbound_membar_hi1103(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar)
{
    pci_print_log(PCI_LOG_INFO, "no outbound!");
    return OAL_SUCC;
}

#endif

