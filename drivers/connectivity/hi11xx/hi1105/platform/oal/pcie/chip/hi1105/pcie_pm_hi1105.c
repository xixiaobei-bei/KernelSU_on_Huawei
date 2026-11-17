

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"

int32_t oal_pcie_device_auxclk_init_hi1103(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_aspm_init_hi1103(oal_pcie_res *pst_pci_res);
void oal_pcie_device_aspm_ctrl_hi1103(oal_pcie_res *pst_pci_res, oal_bool_enum clear);

/*
 * Prototype    : oal_pcie_devce_l1_disable
 * Description  : 关闭L1
 * Input        : oal_pcie_res *
 * Output       : true means l1 disabled before we set
 */
OAL_STATIC oal_bool_enum oal_pcie_devce_l1_disable(oal_pcie_res *pst_pci_res)
{
    low_power_cfg cfg;

    cfg.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
    if (cfg.bits.soc_app_xfer_pending == 1) {
        /* l1 disabled */
        return OAL_TRUE;
    }

    cfg.bits.soc_app_xfer_pending = 1;
    oal_writel(cfg.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);

    return OAL_FALSE;
}

/*
 * Prototype    : oal_pcie_devce_l1_restore
 * Description  : 恢复L1配置
 * Input        : oal_pcie_res *,  l1_disable : true means disable L1
 * Output       : void
 */
OAL_STATIC void oal_pcie_devce_l1_restore(oal_pcie_res *pst_pci_res, oal_bool_enum l1_disable)
{
    low_power_cfg cfg;

    cfg.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
    cfg.bits.soc_app_xfer_pending = (l1_disable == OAL_TRUE) ? 1 : 0;
    oal_writel(cfg.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_LOW_POWER_CFG_OFF);
}

/*
 * Prototype    : oal_pcie_device_phy_addr_range_vaild
 * Description  : 判断pcie phy地址范围合法性,IP强相关
 * Input        : addr:phy地址，16bit位宽
 * Output       : oal_bool_enum true means vaild
 */
OAL_STATIC oal_bool_enum oal_pcie_device_phy_addr_range_vaild(uint16_t addr)
{
    if (addr <= PCIE_EP_PHY_SUP_ADDR_END) {
        return OAL_TRUE;
    }

    if ((addr >= PCIE_EP_PHY_LANEN_ADDR_BEGIN) && (addr <= PCIE_EP_PHY_LANEN_ADDR_END)) {
        return OAL_TRUE;
    }

    oal_print_hi11xx_log(HI11XX_LOG_ERR, "invaild device phy addr:0x%x", addr);

    return OAL_FALSE;
}

/*
 * Prototype    : oal_pcie_device_phy_write
 * Description  : 写pcie device phy 寄存器，必须保证PHY在上电状态
 * Input        : addr:phy地址，16bit位宽 , value:写入值
 * Output       : int32_t : OAL_TURE means write succ
 */
OAL_STATIC int32_t oal_pcie_device_phy_write(oal_pcie_res *pst_pci_res,
                                             uint16_t addr, uint16_t value)
{
    int32_t i;
    pcie_phy_cfg phy_cfg;
    pcie_phy_cfg_addr phy_cfg_addr;
    pcie_phy_wr_data phy_wr_data;

    if (oal_pcie_device_phy_addr_range_vaild(addr) != OAL_TRUE) {
        return -OAL_EINVAL;
    }

    /* set write address */
    phy_cfg_addr.as_dword = 0;
    phy_cfg_addr.bits.phy_cr_addr = (uint32_t)addr;
    oal_writel(phy_cfg_addr.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_ADDR_OFF);

    /* set write value */
    phy_wr_data.as_dword = 0;
    phy_wr_data.bits.phy_cr_wr_data = (uint32_t)value;
    oal_writel(phy_wr_data.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_WR_DATA_OFF);

    /* trigger write */
    phy_cfg.as_dword = 0;
    phy_cfg.bits.phy_write = 1;     /* 1 means write */
    phy_cfg.bits.phy_config_en = 1; /* pulse */
    oal_writel(phy_cfg.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_OFF);

    /* check config ready */
    for (i = 0; i < PCIE_EP_PHY_RW_TIMEOUT_CNT; i++) {
        phy_cfg.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_OFF);
        if (phy_cfg.bits.phy_config_rdy == 1) { /* write done */
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "write phy done addr:0x%x value=0x%x loop=%d", addr, value, i);
            return OAL_SUCC;
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_ERR, "write phy addr:0x%x value=0x%x timeout=%d", addr, value, i);
    return -OAL_ETIMEDOUT;
}

/*
 * Prototype    : oal_pcie_device_phy_read
 * Description  : 读pcie device phy 寄存器，必须保证PHY在上电状态
 * Input        : addr:phy地址，16bit位宽
 * Output       : int32_t : OAL_TURE means write succ , *pst_value:回读值
 */
OAL_STATIC int32_t oal_pcie_device_phy_read(oal_pcie_res *pst_pci_res,
                                            uint16_t addr, uint16_t *pst_value)
{
    int32_t i;
    pcie_phy_cfg phy_cfg;
    pcie_phy_cfg_addr phy_cfg_addr;
    pcie_phy_rd_data phy_rd_data;

    if (oal_unlikely(pst_value == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(oal_pcie_device_phy_addr_range_vaild(addr) != OAL_TRUE)) {
        return -OAL_EINVAL;
    }

    /* set read address */
    phy_cfg_addr.as_dword = 0;
    phy_cfg_addr.bits.phy_cr_addr = (uint32_t)addr;
    oal_writel(phy_cfg_addr.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_ADDR_OFF);

    /* trigger read */
    phy_cfg.as_dword = 0;
    phy_cfg.bits.phy_write = 0;     /* 0 means read */
    phy_cfg.bits.phy_config_en = 1; /* pulse */
    oal_writel(phy_cfg.as_dword, pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_OFF);

    /* check config ready */
    for (i = 0; i < PCIE_EP_PHY_RW_TIMEOUT_CNT; i++) {
        phy_cfg.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_CFG_OFF);
        if (phy_cfg.bits.phy_config_rdy == 1) { /* read done */
            phy_rd_data.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_PHY_RD_DATA_OFF);
            *pst_value = (uint16_t)phy_rd_data.bits.phy_cr_rd_data;
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "read phy done addr:0x%x value=0x%x loop=%d", addr, *pst_value, i);
            return OAL_SUCC;
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_ERR, "read phy addr:0x%x timeout=%d", addr, i);
    return -OAL_ETIMEDOUT;
}

/*
* Prototype    : oal_pcie_device_phy_los_en_ovrd
* Description  : 设置PCIE PHY  LOS_EN=1, bypass rx_los_en信号
* Output       : int32_t : OAL_TURE means set succ
*/
OAL_STATIC int32_t oal_pcie_device_phy_los_en_ovrd(oal_pcie_res *pst_pci_res)
{
    uint16_t value;
    int32_t ret;
    oal_bool_enum l1_disabled;

    /* wakeup phy */
    l1_disabled = oal_pcie_devce_l1_disable(pst_pci_res);

    ret = oal_pcie_device_phy_read(pst_pci_res, PCIE_PHY_LANEN_DIG_RX_OVRD_IN_LO_OFF, &value);
    if (ret != OAL_SUCC) {
        oal_pcie_devce_l1_restore(pst_pci_res, l1_disabled);
        return ret;
    }

    /* 13:RX_LOS_EN_OVRD  12:RX_LOS_EN */
    value &= ~((1 << 12) | (1 << 13));
    value |= ((1 << 12) | (1 << 13));

    ret = oal_pcie_device_phy_write(pst_pci_res, PCIE_PHY_LANEN_DIG_RX_OVRD_IN_LO_OFF, value);
    if (ret != OAL_SUCC) {
        oal_pcie_devce_l1_restore(pst_pci_res, l1_disabled);
        return ret;
    }

    ret = oal_pcie_device_phy_read(pst_pci_res, PCIE_PHY_LANEN_DIG_RX_OVRD_IN_LO_OFF, &value);
    if (ret == OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCIE_PHY_LANEN_DIG_RX_OVRD_IN_LO=0x%x", value);
    }

    oal_pcie_devce_l1_restore(pst_pci_res, l1_disabled);

    return OAL_SUCC;
}

/*
* Prototype    : oal_pcie_device_phy_config
* Description  : PCIE PHY配置
* Output       : int32_t : OAL_TURE means set succ
*/
OAL_STATIC int32_t oal_pcie_device_phy_config_bugfix(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    oal_pci_dev_stru *pst_pci_dev;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    if (pst_pci_dev->device == PCIE_DEVICE_ID_1105) { /* 1105 soc bugfix */
        ret = oal_pcie_device_phy_los_en_ovrd(pst_pci_res);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie phy los en ovrd failed=%d", ret);
            return ret;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie phy los en ovrd succ");
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "pcie device id:0x%x", pst_pci_dev->device);

    return OAL_SUCC;
}

int32_t pcie_pm_chip_init_hi1105(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init = oal_pcie_device_aspm_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init = oal_pcie_device_auxclk_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl = oal_pcie_device_aspm_ctrl_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config = oal_pcie_device_phy_config_bugfix;
    return OAL_SUCC;
}
#endif

