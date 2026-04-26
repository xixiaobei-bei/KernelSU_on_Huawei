

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

#include "chip/hi1106/pcie_soc_hi1106.h"


OAL_STATIC oal_pcie_region g_hi1106_pcie_fpga_regions[] = {
    {   .pci_start = 0x00000000 + 0x3000000,  /* 0x3000000 : 外设ACP地址固定偏移 */
        .pci_end   = 0x00003FFF + 0x3000000,
        .cpu_start = 0x00000000,
        .cpu_end   = 0x00003FFF, /* 16K */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BOOT"
    },
    {   .pci_start = 0x00040000 + 0x3000000,
        .pci_end   = 0x001bFFFF + 0x3000000,
        .cpu_start = 0x00040000,
        .cpu_end   = 0x001bFFFF, /* 1536KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_WRAM_NON_ACP"
    },
    {   .pci_start = 0x02000000 + 0x3000000,
        .pci_end   = 0x020A1FFF + 0x3000000,
        .cpu_start = 0x02000000,
        .cpu_end   = 0x020A1FFF, /* 648KB */
        .flag      = OAL_IORESOURCE_REG,
        /* 和芯片约束，PCIE不使用S7端口 */
        .name      = "HI1106_SHARE_MEM_S6_NON_ACP"
    },
    {   .pci_start = 0x40000000,
        .pci_end   = 0x4013FFFF,
        .cpu_start = 0x40000000,
        .cpu_end   = 0x4013FFFF, /* 1280KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_AON_APB"
    },
    {   .pci_start = 0x44000000,
        .pci_end   = 0x44121FFF,
        .cpu_start = 0x44000000,
        .cpu_end   = 0x44121FFF, /* 1160KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BITCM"
    },
    {   .pci_start = 0x44200000,
        .pci_end   = 0x442FFFFF,
        .cpu_start = 0x44200000,
        .cpu_end   = 0x442FFFFF, /* 1024KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BDTCM"
    },
    {   .pci_start = 0x44400000,
        .pci_end   = 0x4446FFFF,
        .cpu_start = 0x44400000,
        .cpu_end   = 0x4446FFFF, /* 448KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_GITCM"
    },
    {   .pci_start = 0x44500000,
        .pci_end   = 0x44601FFF,
        .cpu_start = 0x44500000,
        .cpu_end   = 0x44601FFF, /* 1032KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_GDTCM"
    },
};

OAL_STATIC oal_pcie_region g_hi1106_pcie_asic_regions[] = {
    {   .pci_start = 0x00000000 + 0x3000000,  /* 0x3000000 : 外设ACP地址固定偏移 */
        .pci_end   = 0x00003FFF + 0x3000000,
        .cpu_start = 0x00000000,
        .cpu_end   = 0x00003FFF, /* 16K */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BOOT"
    },
    {   .pci_start = 0x00040000 + 0x3000000,
        .pci_end   = 0x001bFFFF + 0x3000000,
        .cpu_start = 0x00040000,
        .cpu_end   = 0x001bFFFF, /* 1536KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_WRAM_NON_ACP"
    },
    {   .pci_start = 0x02000000 + 0x3000000,
        .pci_end   = 0x020A1FFF + 0x3000000,
        .cpu_start = 0x02000000,
        .cpu_end   = 0x020A1FFF, /* 648KB */
        .flag      = OAL_IORESOURCE_REG,
        /* 和芯片约束，PCIE不使用S7端口 */
        .name      = "HI1106_SHARE_MEM_S6_NON_ACP"
    },
    {   .pci_start = 0x40000000,
        .pci_end   = 0x4013FFFF,
        .cpu_start = 0x40000000,
        .cpu_end   = 0x4013FFFF, /* 1280KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_AON_APB"
    },
    {   .pci_start = 0x44000000,
        .pci_end   = 0x44121FFF,
        .cpu_start = 0x44000000,
        .cpu_end   = 0x44121FFF, /* 1160KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BITCM"
    },
    {   .pci_start = 0x44200000,
        .pci_end   = 0x442FFFFF,
        .cpu_start = 0x44200000,
        .cpu_end   = 0x442FFFFF, /* 1024KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_BDTCM"
    },
    {   .pci_start = 0x44400000,
        .pci_end   = 0x4446FFFF,
        .cpu_start = 0x44400000,
        .cpu_end   = 0x4446FFFF, /* 448KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_GITCM"
    },
    {   .pci_start = 0x44500000,
        .pci_end   = 0x44601FFF,
        .cpu_start = 0x44500000,
        .cpu_end   = 0x44601FFF, /* 1032KB */
        .flag      = OAL_IORESOURCE_REG,
        .name      = "HI1106_GDTCM"
    },
};

int32_t oal_pcie_get_bar_region_info_hi1106(oal_pcie_res *pst_pci_res,
                                            oal_pcie_region **region_base, uint32_t *region_num)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "1106 %s region map", hi110x_is_asic() ? "asic" : "fpga");
    if (hi110x_is_asic()) {
        /* 1106 Asic, same as 1106 Fpga */
        *region_num = oal_array_size(g_hi1106_pcie_asic_regions);
        *region_base = &g_hi1106_pcie_asic_regions[0];
    } else {
        /* 1106 Fpga */
        *region_num = oal_array_size(g_hi1106_pcie_fpga_regions);
        *region_base = &g_hi1106_pcie_fpga_regions[0];
    }
    return OAL_SUCC;
}

int32_t oal_pcie_set_outbound_membar_hi1106(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar)
{
    int32_t ret;
    ret = oal_pcie_set_outbound_iatu_by_membar(pst_iatu_bar->st_region.vaddr,
                                               0, HISI_PCIE_SLAVE_START_ADDRESS,
                                               HISI_PCIE_MASTER_START_ADDRESS,
                                               HISI_PCIE_IP_REGION_SIZE);
    if (ret) {
        return ret;
    }
    return OAL_SUCC;
}

#endif

