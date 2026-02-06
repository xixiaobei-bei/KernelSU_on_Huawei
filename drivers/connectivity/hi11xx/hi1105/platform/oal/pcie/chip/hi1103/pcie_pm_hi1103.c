

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"

/* 时钟分频要在低功耗关闭下配置 */
int32_t oal_pcie_device_auxclk_init_hi1103(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    uint32_t value;
    pci_addr_map st_map;

    value = oal_readl(pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
    /* aux_clk 1M, synophys set */
    value &= (~((1 << 10) - 1));
    value |= 0x1;
    oal_writel(value, pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);

    /* tcxo 38.4M 39分频 = 0.98M 接近1M */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, (0x50000000 + 0x2c), &st_map);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wcpu address  convert failed, ret=%d", ret);
        return ret;
    }

    value = oal_readl((void *)st_map.va);
    value &= (~(((1 << 6) - 1) << 8));
    value |= (0x27 << 8);
    oal_writel(value, (void *)st_map.va);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "clk_freq reg:0x%x, freq_div reg:0x%x",
                         oal_readl(pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF),
                         oal_readl((void *)st_map.va));
    return OAL_SUCC;
}

int32_t oal_pcie_device_aspm_init_hi1103(oal_pcie_res *pst_pci_res)
{
    uint32_t value;

    value = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_CFG_OFF);
    value |= (1 << 1);
    value |= (1 << 3);
    oal_writel(value, pst_pci_res->pst_pci_ctrl_base + PCIE_CFG_OFF);

    oal_pcie_device_xfer_pending_sig_clr(pst_pci_res, OAL_TRUE);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "cfg reg:0x%x, low_power_cfg reg:0x%x",
                         oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_CFG_OFF),
                         oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF));

    return OAL_SUCC;
}

void oal_pcie_device_aspm_ctrl_hi1103(oal_pcie_res *pst_pci_res, oal_bool_enum clear)
{
    uint32_t value;
    if (clear != 0) {
        /* clear xfer pending signal, allow L1 enter/exist */
        value = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
        value &= ~(1 << 7);
        oal_writel(value, pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
    } else {
        /* set xfer pending signal, wakeup L1 & disallow enter/exist */
        value = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
        value |= (1 << 7);
        oal_writel(value, pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
    }

    /* flush pcie bus */
    oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
}

int32_t oal_pcie_device_phy_config_hi1103(oal_pcie_res *pst_pci_res)
{
    oal_reference(pst_pci_res);
    return OAL_SUCC;
}

int32_t pcie_pm_chip_init_hi1103(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init = oal_pcie_device_aspm_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init = oal_pcie_device_auxclk_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl = oal_pcie_device_aspm_ctrl_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config = oal_pcie_device_phy_config_hi1103;
    return OAL_SUCC;
}

#endif

