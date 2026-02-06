

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIEC]"
#define HISI_LOG_TAG           "[PCIEC]"

#include "oal_hcc_host_if.h"
#include "hi1106/host_ctrl_rb_regs.h"
#include "hi1106/pcie_ctrl_rb_regs.h"

#include "chip/hi1106/pcie_soc_hi1106.h"

/* HI1106 Registers */
#define HI1106_PA_PCIE0_CTRL_BASE_ADDR      PCIE_CTRL_RB_BASE /* 对应PCIE_CTRL页 */
#define HI1106_PA_PCIE1_CTRL_BASE_ADDR      0x4010E000 /* 对应PCIE_CTRL页 */
#define HI1106_PA_PCIE0_DBI_BASE_ADDRESS    0x40107000
#define HI1106_PA_PCIE1_DBI_BASE_ADDRESS    0x4010D000
#define HI1106_PA_ETE_CTRL_BASE_ADDRESS    HOST_CTRL_RB_BASE
#define HI1106_PA_GLB_CTL_BASE_ADDR        0x40000000
#define HI1106_PA_PMU_CMU_CTL_BASE         0x40002000
#define HI1106_PA_PMU2_CMU_IR_BASE         0x4000E000
#define HI1106_PA_W_CTL_BASE               0x40105000

#define HI1106_DEV_VERSION_CPU_ADDR 0x0000003c

int32_t g_dual_pci_support = OAL_FALSE; /* 0 means don't support */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_dual_pci_support, int, S_IRUGO | S_IWUSR);
#endif

int32_t oal_pcie_get_bar_region_info_hi1106(oal_pcie_res *pst_pci_res, oal_pcie_region **, uint32_t *);
int32_t oal_pcie_set_outbound_membar_hi1106(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar);

static uintptr_t oal_pcie_get_test_ram_address(void)
{
    return 0x2000000; // pkt mem for test, by acp port
}

static int32_t oal_pcie_voltage_bias_init_hi1106(oal_pcie_res *pst_pci_res)
{
    oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_pcie_chip_info_init_hi1106 is not implement!");
    return OAL_SUCC;
}

static int32_t oal_pcie_host_slave_address_switch_hi1106(oal_pcie_res *pst_pci_res, uint64_t src_addr,
                                                         uint64_t* dst_addr, int32_t is_host_iova)
{
    if (is_host_iova == OAL_TRUE) {
        if (oal_likely((src_addr < (HISI_PCIE_MASTER_END_ADDRESS)))) {
            *dst_addr = src_addr + HISI_PCIE_IP_REGION_OFFSET;
            pci_print_log(PCI_LOG_DBG, "pcie_if_hostca_to_devva ok, hostca=0x%llx\n", *dst_addr);
            return OAL_SUCC;
        }
    } else {
        if (oal_likely((((src_addr >= HISI_PCIE_SLAVE_START_ADDRESS)
                       && (src_addr < (HISI_PCIE_SLAVE_END_ADDRESS)))))) {
            *dst_addr = src_addr - HISI_PCIE_IP_REGION_OFFSET;
            pci_print_log(PCI_LOG_DBG, "pcie_if_devva_to_hostca ok, devva=0x%llx\n", *dst_addr);
            return OAL_SUCC;
        }
    }

    pci_print_log(PCI_LOG_ERR, "pcie_slave_address_switch %s failed, src_addr=0x%llx\n",
                  (is_host_iova == OAL_TRUE) ? "iova->slave" : "slave->iova", src_addr);
    return -OAL_EFAIL;
}

static void oal_pcie_chip_info_cb_init(pcie_chip_cb *cb, int32_t device_id)
{
    cb->get_test_ram_address = oal_pcie_get_test_ram_address;
    cb->pcie_voltage_bias_init = oal_pcie_voltage_bias_init_hi1106;
    cb->pcie_get_bar_region_info = oal_pcie_get_bar_region_info_hi1106;
    cb->pcie_set_outbound_membar = oal_pcie_set_outbound_membar_hi1106;
    cb->pcie_host_slave_address_switch = oal_pcie_host_slave_address_switch_hi1106;
}

int32_t oal_pcie_chip_info_init_hi1106(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_chip_info_init_hi1106");
    pst_pci_res->chip_info.dual_pci_support = (uint32_t)g_dual_pci_support;
    pst_pci_res->chip_info.ete_support = OAL_TRUE;
    pst_pci_res->chip_info.membar_support = OAL_TRUE;
    pst_pci_res->chip_info.addr_info.pcie_ctrl = HI1106_PA_PCIE0_CTRL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.dbi = HI1106_PA_PCIE0_DBI_BASE_ADDRESS;
    pst_pci_res->chip_info.addr_info.ete_ctrl = HI1106_PA_ETE_CTRL_BASE_ADDRESS;
    pst_pci_res->chip_info.addr_info.glb_ctrl = HI1106_PA_GLB_CTL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.pmu_ctrl = HI1106_PA_PMU_CMU_CTL_BASE;
    pst_pci_res->chip_info.addr_info.pmu2_ctrl = HI1106_PA_PMU2_CMU_IR_BASE;
    pst_pci_res->chip_info.addr_info.boot_version = HI1106_DEV_VERSION_CPU_ADDR;
    pst_pci_res->chip_info.addr_info.sharemem_addr = PCIE_CTRL_RB_HOST_DEVICE_REG1_REG;
    oal_pcie_chip_info_cb_init(&pst_pci_res->chip_info.cb, device_id);
    return OAL_SUCC;
}

#endif
