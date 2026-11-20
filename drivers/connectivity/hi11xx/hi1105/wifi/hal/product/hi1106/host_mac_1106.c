

#include "host_mac_1106.h"
#include "pcie_host.h"
#include "oal_util.h"
#include "oal_ext_if.h"
#include "host_hal_ring.h"
#include "host_hal_device.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_HOST_MAC_C

void hi1106_get_host_phy_int_status(hal_host_device_stru *pst_hal_device, uint32_t *pul_status)
{
    *pul_status = oal_readl((void*)(uintptr_t)pst_hal_device->phy_regs.irq_status_reg);
}


void hi1106_get_host_phy_int_mask(hal_host_device_stru *hal_device, uint32_t *mask)
{
    *mask = oal_readl((void*)(uintptr_t)hal_device->phy_regs.irq_status_mask_reg);
}


void hi1106_clear_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t status)
{
    oal_writel(status, (uintptr_t)hal_device->phy_regs.irq_clr_reg);
}



void hi1106_get_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t *status)
{
    *status = oal_readl((void*)(uintptr_t)hal_device->mac_regs.irq_status_reg);
}


void hi1106_get_host_mac_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask)
{
    *p_mask = oal_readl((uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
}


uint32_t hi1106_get_dev_rx_filt_mac_status(uint64_t *reg_status)
{
    uint32_t reg_addr;
    uint64_t host_va = 0;

    if (reg_status == NULL) {
        return OAL_FAIL;
    }
    reg_addr = HI1106_MAC_RX_FRAMEFILT2_REG;
    if (oal_pcie_devca_to_hostva(0, reg_addr, &host_va) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "{hi1106_get_dev_rx_filt_mac_status::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
    *reg_status = ((uint64_t)oal_readl((void*)(uintptr_t)host_va)) << BIT_OFFSET_32;
    reg_addr = HI1106_MAC_RX_FRAMEFILT_REG;
    if (oal_pcie_devca_to_hostva(0, reg_addr, &host_va) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "{hi1106_get_dev_rx_filt_mac_status::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
    *reg_status = (*reg_status | oal_readl((void*)(uintptr_t)host_va));
    return OAL_SUCC;
}


void hi1106_clear_host_mac_int_status(hal_host_device_stru *pst_hal_device, uint32_t status)
{
    oal_writel(status, (uintptr_t)pst_hal_device->mac_regs.irq_clr_reg);
}


void  hi1106_host_mac_irq_unmask(hal_host_device_stru *hal_device, uint32_t irq)
{
    uint32_t intr_mask;

    intr_mask = oal_readl((void*)(uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
    intr_mask &= (~((uint32_t)1 << irq));

    oal_writel(intr_mask, (void*)(uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
}


void  hi1106_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t irq)
{
    uint32_t intr_mask;

    intr_mask = oal_readl((void*)(uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
    intr_mask |= (((uint32_t)1 << irq));
    oal_writel(intr_mask, (uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
}

uint32_t hi1106_regs_addr_get_offset(uint8_t device_id, uint32_t reg_addr)
{
    uint32_t offset = 0;
    uint32_t mac_reg_offset[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {0, HI1106_MAC_BANK_REG_OFFSET};
    uint32_t phy_reg_offset[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {0, HI1106_PHY_BANK_REG_OFFSET};
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case HI1106_MAC_HOST_INTR_CLR_REG:
        case HI1106_MAC_HOST_INTR_MASK_REG:
        case HI1106_MAC_HOST_INTR_STATUS_REG:
        case HI1106_MAC_CSI_BUF_BASE_ADDR_LSB_REG:
        case HI1106_MAC_CSI_BUF_BASE_ADDR_MSB_REG:
        case HI1106_MAC_CSI_BUF_SIZE_REG:
        case HI1106_MAC_CSI_PROCESS_REG:
        case HI1106_MAC_RX_CTRL_REG:
        case HI1106_MAC_CSI_WHITELIST_ADDR_LSB_0_REG:
        case HI1106_MAC_CSI_WHITELIST_ADDR_MSB_0_REG:
        case HI1106_MAC_LOCATION_INFO_MASK_REG:
        case HI1106_MAC_CSI_INFO_ADDR_LSB_REG:
        case HI1106_MAC_CSI_INFO_ADDR_MSB_REG:
        case HI1106_MAC_FTM_INFO_ADDR_MSB_REG:
        case HI1106_MAC_FTM_INFO_ADDR_LSB_REG:
        case HI1106_MAC_DMAC_INTR_MASK_REG:
        case HI1106_MAC_FTM_PROCESS_REG:
        case HI1106_MAC_FTM_BUF_BASE_ADDR_LSB_REG:
        case HI1106_MAC_FTM_BUF_BASE_ADDR_MSB_REG:
        case HI1106_MAC_FTM_WHITELIST_ADDR_LSB_0_REG:
        case HI1106_MAC_FTM_WHITELIST_ADDR_MSB_0_REG:
            offset = mac_reg_offset[device_id];
            break;
        case HI1106_DFX_REG_BANK_CFG_INTR_CLR_REG:
        case HI1106_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG:
        case HI1106_DFX_REG_BANK_PHY_INTR_RPT_REG:
        case HI1106_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG:
        case HI1106_PHY_GLB_REG_BANK_FTM_CFG_REG:
        case HI1106_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG:
        case HI1106_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG:
        case HI1106_PHY_GLB_REG_BANK_PHY_BW_MODE_REG:
            offset = phy_reg_offset[device_id];
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hi1106_regs_addr_get_offset::reg addr[%x] not used.}", reg_addr);
    }
#endif
    return offset;
}
uint32_t hi1106_regs_addr_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset;

    if (hal_device->device_id >= HAL_DEVICE_ID_BUTT) {
        oam_warning_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::device_id[%x]!}", hal_device->device_id);
        return OAL_FAIL;
    }

    offset = hi1106_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(0, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case HI1106_MAC_HOST_INTR_CLR_REG:
            hal_device->mac_regs.irq_clr_reg = host_va;
            break;
        case HI1106_MAC_HOST_INTR_MASK_REG:
            hal_device->mac_regs.irq_status_mask_reg = host_va;
            break;
        case HI1106_MAC_HOST_INTR_STATUS_REG:
            hal_device->mac_regs.irq_status_reg = host_va;
            break;
        case HI1106_DFX_REG_BANK_CFG_INTR_CLR_REG:
            hal_device->phy_regs.irq_clr_reg = host_va;
            break;
        case HI1106_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG:
            hal_device->phy_regs.irq_status_mask_reg = host_va;
            break;
        case HI1106_DFX_REG_BANK_PHY_INTR_RPT_REG:
            hal_device->phy_regs.irq_status_reg = host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}


int32_t hi1106_host_regs_addr_init(hal_host_device_stru *hal_device)
{
    uint32_t idx;
    uint32_t hal_regs[] = {
        HI1106_MAC_HOST_INTR_CLR_REG,
        HI1106_MAC_HOST_INTR_MASK_REG,
        HI1106_MAC_HOST_INTR_STATUS_REG,
        HI1106_DFX_REG_BANK_CFG_INTR_CLR_REG,
        HI1106_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG,
        HI1106_DFX_REG_BANK_PHY_INTR_RPT_REG,
    };

    for (idx = 0; idx < sizeof(hal_regs) / sizeof(uint32_t); idx++) {
        if (OAL_SUCC != hi1106_regs_addr_transfer(hal_device, hal_regs[idx])) {
            return OAL_FAIL;
        }
    }
    oam_warning_log0(0, OAM_SF_ANY, "{hal_host_regs_addr_init::regs addr trans succ.}");
    return OAL_SUCC;
}

/* 功能描述 : 常收维测信息输出 */
void hi1106_host_al_rx_fcs_info(hmac_vap_stru *hmac_vap)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{hi1106_host_al_rx_fcs_info::hal_device null.}");
        return;
    }

    oam_warning_log3(0, OAM_SF_CFG, "host_al_rx_fcs_info:packets info, mpdu succ[%d], ampdu succ[%d],fail[%d]",
        hal_device->st_alrx.rx_normal_mdpu_succ_num,
        hal_device->st_alrx.rx_ampdu_succ_num,
        hal_device->st_alrx.rx_ppdu_fail_num);

    hal_device->st_alrx.rx_normal_mdpu_succ_num = 0;
    hal_device->st_alrx.rx_ampdu_succ_num = 0;
    hal_device->st_alrx.rx_ppdu_fail_num = 0;
    return;
}

/* 功能描述 : 常收维测信息输出 */
void hi1106_host_get_rx_pckt_info(hmac_vap_stru *hmac_vap,
    dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event_info)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{hi1106_host_get_rx_pckt_info::hal_device null.}");
        return;
    }

    rx_pkcg_event_info->event_para = hal_device->st_alrx.rx_normal_mdpu_succ_num;
    rx_pkcg_event_info->s_always_rx_rssi = rx_pkcg_event_info->s_always_rx_rssi;
    return;
}


/* 功能描述 : 初始化host通用定时器，填写转换地址 */
int32_t hi1106_host_init_common_timer(hal_mac_common_timer *mac_timer)
{
    uint64_t host_va = 0;
    uint8_t timer_id = mac_timer->timer_id;
    uint32_t mask_dev_reg = HI1106_MAC_DMAC_COMMON_TIMER_MASK_REG;
    uint32_t timer_ctrl = HI1106_MAC_COMMON_TIMER_CTRL_0_REG + HI1106_MAC_COMMON_TIMER_OFFSET * timer_id;
    uint32_t timer_val = HI1106_MAC_COMMON_TIMER_VAL_0_REG + HI1106_MAC_COMMON_TIMER_OFFSET * timer_id;

    if (timer_id > HI1106_MAC_MAX_COMMON_TIMER) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::timer id[%d] error.}", timer_id);
        return OAL_FAIL;
    }

    if (oal_pcie_devca_to_hostva(0, mask_dev_reg, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::regaddr[%x] devca2hostva fail.}", mask_dev_reg);
        return OAL_FAIL;
    }
    mac_timer->mask_reg_addr = host_va;

    if (oal_pcie_devca_to_hostva(0, timer_ctrl, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::regaddr[%x] devca2hostva fail.}", timer_ctrl);
        return OAL_FAIL;
    }
    mac_timer->timer_ctrl_addr = host_va;

    if (oal_pcie_devca_to_hostva(0, timer_val, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::regaddr[%x] devca2hostva fail.}", timer_val);
        return OAL_FAIL;
    }
    mac_timer->timer_val_addr = host_va;
    return OAL_SUCC;
}


void hi1106_host_set_mac_common_timer(hal_mac_common_timer *mac_common_timer)
{
    uint8_t timer_id = mac_common_timer->timer_id;
    uint32_t timer_mask;
    uint32_t timer_ctrl;

    if (timer_id > HI1106_MAC_MAX_COMMON_TIMER) {
        oam_error_log1(0, OAM_SF_ANY, "hi1106_host_set_mac_common_timer :: timer[%d] id error.", timer_id);
        return;
    }
    timer_mask = oal_readl((uintptr_t)mac_common_timer->mask_reg_addr);
    timer_mask = timer_mask & (~BIT(timer_id)); /* 打开timer_id对应的mask */
    oal_writel(timer_mask, (uintptr_t)mac_common_timer->mask_reg_addr); /* 打开中断处理 */
    timer_ctrl = ((mac_common_timer->timer_en) | (mac_common_timer->timer_mode << 1) |
        (mac_common_timer->timer_unit << 2)); /* bit0: timer en bit1: timer_mode bit2~3: unit */
    oal_writel(timer_ctrl, (uintptr_t)mac_common_timer->timer_ctrl_addr);
    oal_writel(mac_common_timer->common_timer_val, (uintptr_t)mac_common_timer->timer_val_addr);
}
