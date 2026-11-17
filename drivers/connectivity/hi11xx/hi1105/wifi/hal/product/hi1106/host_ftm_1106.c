
#ifdef _PRE_WLAN_FEATURE_FTM
/* 1 头文件包含 */
#include "mac_data.h"
#include "mac_ftm.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "host_mac_1106.h"
#include "host_ftm_1106.h"
#include "plat_pm_wlan.h"
#include "hmac_tx_msdu_dscr.h"

#include "securec.h"
#include "securectype.h"


static uint32_t hi1106_regs_ftm_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset  = hi1106_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(0, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hi1106_regs_ftm_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case HI1106_MAC_FTM_PROCESS_REG:
            hal_device->ftm_regs.ftm_pro_reg = host_va;
            break;
        case HI1106_MAC_FTM_BUF_BASE_ADDR_LSB_REG:
            hal_device->ftm_regs.base_lsb = host_va;
            break;
        case HI1106_MAC_FTM_BUF_BASE_ADDR_MSB_REG:
            hal_device->ftm_regs.base_msb = host_va;
            break;
        case HI1106_MAC_FTM_WHITELIST_ADDR_LSB_0_REG:
            hal_device->ftm_regs.white_addr_lsb = host_va;
            break;
        case HI1106_MAC_FTM_WHITELIST_ADDR_MSB_0_REG:
            hal_device->ftm_regs.white_addr_msb = host_va;
            break;
        case HI1106_PHY_GLB_REG_BANK_FTM_CFG_REG:
            hal_device->ftm_regs.ftm_cfg_reg = host_va;
            break;
        case HI1106_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG:
            hal_device->ftm_regs.ftm_cali_reg = host_va;
            break;
        case HI1106_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG:
            hal_device->ftm_regs.fifo_force_reg = host_va;
            break;
        case HI1106_MAC_FTM_INFO_ADDR_MSB_REG:
            hal_device->ftm_regs.ftm_info_msb = host_va;
            break;
        case HI1106_MAC_FTM_INFO_ADDR_LSB_REG:
            hal_device->ftm_regs.ftm_info_lsb = host_va;
            break;
        case HI1106_PHY_GLB_REG_BANK_PHY_BW_MODE_REG:
            hal_device->ftm_regs.ftm_phy_bw_mode = host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM, "{hi1106_regs_ftm_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

uint32_t hi1106_host_ftm_reg_init(uint8_t hal_dev_id)
{
    uint32_t idx;
    uint32_t hal_regs[] = {
        HI1106_MAC_FTM_PROCESS_REG,
        HI1106_MAC_FTM_BUF_BASE_ADDR_LSB_REG,
        HI1106_MAC_FTM_BUF_BASE_ADDR_MSB_REG,
        HI1106_MAC_FTM_WHITELIST_ADDR_LSB_0_REG,
        HI1106_MAC_FTM_WHITELIST_ADDR_MSB_0_REG,
        HI1106_PHY_GLB_REG_BANK_FTM_CFG_REG,
        HI1106_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG,
        HI1106_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG,
        HI1106_MAC_FTM_INFO_ADDR_LSB_REG,
        HI1106_MAC_FTM_INFO_ADDR_MSB_REG,
        HI1106_PHY_GLB_REG_BANK_PHY_BW_MODE_REG,
    };
    hal_host_device_stru *hal_device  = hal_get_host_device(hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "hi1106_host_ftm_reg_init::haldev null!");
        return OAL_FAIL;
    }
    if (hal_device->device_id >= HAL_DEVICE_ID_BUTT) {
        oam_error_log1(0, OAM_SF_FTM, "{hi1106_host_ftm_reg_init::device_id[%d].}", hal_device->device_id);
        return OAL_FAIL;
    }
    if (!hal_device->ftm_regs.inited) {
        for (idx = 0; idx < sizeof(hal_regs) / sizeof(uint32_t); idx++) {
            if (hi1106_regs_ftm_transfer(hal_device, hal_regs[idx]) != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    }
    hal_device->ftm_regs.inited = OAL_TRUE;
    oam_warning_log0(0, OAM_SF_ANY, "{hi1106_csi_reg_init::regs addr trans succ.}");

    return OAL_SUCC;
}


static void hi1106_ftm_get_dscr_addr(uint64_t value, uint8_t *addr)
{
    addr[MAC_ADDR_0] = (uint8_t)(value & 0xff);                    /* macaddr arr idx 0 */
    addr[MAC_ADDR_1] = (uint8_t)((value >> BIT_OFFSET_8) & 0xff);  /* macaddr arr idx 1 */
    addr[MAC_ADDR_2] = (uint8_t)((value >> BIT_OFFSET_16) & 0xff); /* macaddr arr idx 2 */
    addr[MAC_ADDR_3] = (uint8_t)((value >> BIT_OFFSET_24) & 0xff); /* macaddr arr idx 3 */
    addr[MAC_ADDR_4] = (uint8_t)((value >> BIT_OFFSET_32) & 0xff); /* macaddr arr idx 4 */
    addr[MAC_ADDR_5] = (uint8_t)((value >> BIT_OFFSET_40) & 0xff); /* macaddr arr idx 5 */
}


hmac_vap_stru *hi1106_host_vap_get_by_hal_vap_id(uint8_t hal_device_id, uint8_t hal_vap_id)
{
    uint8_t mac_vap_id;
    hal_host_device_stru *hal_device = hal_get_host_device(hal_device_id);

    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{hi1106_host_vap_get_by_hal_vap_id::hal_device null.}");
        return NULL;
    }
    if ((hal_vap_id == HI1106_OTHER_BSS_ID) || (hal_vap_id < HI1106_HAL_VAP_OFFSET)) {
        return NULL;
    }

    hal_vap_id -= HI1106_HAL_VAP_OFFSET;
    mac_vap_id = hal_device->hal_vap_sts_info[hal_vap_id].mac_vap_id;
    /* 2. 获取hMAC层VAP信息 */
    return (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id);
}

uint32_t hi1106_host_ftm_get_info(hal_ftm_info_stru *hal_ftm_info, uint8_t *addr)
{
    uint8_t idx;
    uint32_t *ftm_power_output = NULL;
    volatile uint32_t *ftm_power_input = NULL;
    hi1106_ftm_dscr_stru *hi1106_ftm_dscr = NULL;

    /* 获取hal_ftm_info的地址 */

    hi1106_ftm_dscr = (hi1106_ftm_dscr_stru *)addr;
    if (hi1106_ftm_dscr == NULL) {
        return OAL_FAIL;
    }

    /* 2. 存储部分结果 */
    hal_ftm_info->vap_index = hi1106_ftm_dscr->word4.bit_vap_index;
    hal_ftm_info->ftm_init_resp_flag = hi1106_ftm_dscr->word10.bit_init_resp_flag;
    hal_ftm_info->ftm_intp_time = hi1106_ftm_dscr->word10.bit_intp_time;
    hal_ftm_info->ftm_rx_time = hi1106_ftm_dscr->word9.rx_time;
    hal_ftm_info->ftm_tx_time = hi1106_ftm_dscr->word8.tx_time;
    hal_ftm_info->ftm_dialog_token = hi1106_ftm_dscr->word4.bit_dialog_token;
    hal_ftm_info->ftm_timestamp = hi1106_ftm_dscr->word7.timestamp;

    hi1106_ftm_get_dscr_addr(hi1106_ftm_dscr->word5.ra, &hal_ftm_info->ftm_ra[MAC_ADDR_0]);
    hi1106_ftm_get_dscr_addr(hi1106_ftm_dscr->word6.ta, &hal_ftm_info->ftm_ta[MAC_ADDR_0]);

    for (idx = 0; idx < HAL_HOST_MAX_RF_NUM; idx++) {
        hal_ftm_info->ftm_snr_ant[idx] = hi1106_ftm_dscr->word1.snr_ant[idx];
        hal_ftm_info->ftm_snr_ant[idx] = hi1106_ftm_dscr->word1.rssi_hltf[idx];
    }
    hal_ftm_info->ftm_phase_incr_ant = hi1106_ftm_dscr->word4.bit_phase_incr;
    ftm_power_input = (uint32_t *)((uint8_t *)hi1106_ftm_dscr + sizeof(hi1106_ftm_dscr_stru));
    ftm_power_output = &hal_ftm_info->ftm_power[0].reg_value;

     /* 直接存储ftm ppu字段里面的相关峰信息，
       相关峰字段的顺序，位宽是16bit：max_next, max, max_pre1, max_pre2, max_pre3……max_pre62，
       先收的数据放低位，后收到的数据放高位组成一个word */
    for (idx = 0; idx < FTM_MAX_POWER_NUM; idx++) {
        *ftm_power_output++ = *ftm_power_input++;
    }

    return OAL_SUCC;
}


void hi1106_host_ftm_get_divider(hal_host_device_stru *hal_device, uint8_t *divider)
{
    uint32_t reg_value;

    reg_value = oal_readl((void *)(uintptr_t)hal_device->ftm_regs.ftm_phy_bw_mode);

    switch (reg_value) {
        case 0x0:  // phymode 20, clk = 2x 160
        case 0x1:  // phymode 40, clk = 2x 160
            *divider = DIVIDER_2CLK; // 分频系数2
            break;

        case 0x2:  // phymode 80, clk = 4x 160
        case 0x3:  // phymode 160, clk = 4x 160
            *divider = DIVIDER_4CLK; // 分频系数4
            break;

        default:
            *divider = DIVIDER_2CLK; // 分频系数2
            break;
    }
}


void hi1106_host_ftm_set_sample(hal_host_device_stru *hal_device, oal_bool_enum_uint8 ftm_status)
{
    /* 设置FTM采集使能开关 */

    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_pro_reg,
                  HI1106_MAC_CFG_FTM_EN_OFFSET, HI1106_MAC_CFG_FTM_EN_LEN,
                  ftm_status);
}


void hi1106_host_ftm_set_enable(hal_host_device_stru *hal_device, oal_bool_enum_uint8 ftm_status)
{
     /* 置位 MAC FTM enable位 */

    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_pro_reg,
                  HI1106_MAC_CFG_FTM_EN_OFFSET, HI1106_MAC_CFG_FTM_EN_LEN,
                  ftm_status);

    /* 置位 PHY FTM enable位 */
    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_cfg_reg,
                  HI1106_PHY_GLB_REG_BANK_CFG_FTM_EN_OFFSET, HI1106_PHY_GLB_REG_BANK_CFG_FTM_EN_LEN,
                  ftm_status);

    /* 置位 PHY FTM cali enable位 */
    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_cali_reg,
                  HI1106_RF_CTRL_REG_BANK_0_CFG_FTM_CALI_WR_RF_REG_EN_OFFSET,
                  HI1106_RF_CTRL_REG_BANK_0_CFG_FTM_CALI_WR_RF_REG_EN_LEN,
                  ftm_status);
}


void hi1106_host_ftm_set_m2s_phy(hal_host_device_stru *hal_device, uint8_t tx_chain_selection,
                                 uint8_t tx_rssi_selection)
{
    uint8_t tx_selection;
    uint32_t ftm_cap = mac_ftm_get_cap();

    tx_selection = ((ftm_cap & FTM_SWITCH_CAP) == 0 ? tx_chain_selection : tx_rssi_selection);
    if (tx_selection >= HAL_HOST_MAX_RF_NUM) {  /* 最大4通道 */
        return;
    }

    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_cfg_reg,
                  HI1106_PHY_GLB_REG_BANK_CFG_FTM_CALI_CH_SEL_OFFSET,
                  HI1106_PHY_GLB_REG_BANK_CFG_FTM_CALI_CH_SEL_LEN,
                  tx_selection);
    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.fifo_force_reg,
                  HI1106_CALI_TEST_REG_BANK_0_CFG_REG_TX_FIFO_EN_OFFSET,
                  HI1106_CALI_TEST_REG_BANK_0_CFG_REG_TX_FIFO_EN_LEN, BIT(tx_selection));
    oam_warning_log1(0, OAM_SF_ANY,
                     "hi1106_set_ftm_m2s_phy:: tx_selection=[%d]", tx_selection);
}


void hi1106_host_ftm_set_buf_base_addr(hal_host_device_stru *hal_device, uint64_t devva)
{
    if (devva == 0) {
        oam_error_log0(0, 0, "hi1106_host_set_csi_buf_base_addr::devva is empty");
        return;
    }
    oal_writel(get_low_32_bits(devva), (uintptr_t)hal_device->ftm_regs.base_lsb);
    oal_writel(get_high_32_bits(devva), (uintptr_t)hal_device->ftm_regs.base_msb);
}


void hi1106_host_ftm_set_buf_size(hal_host_device_stru *hal_device, uint16_t cfg_ftm_buf_size)
{
    /* 设置FTM采样上报片外循环BUF深度 */
    oal_setl_bits((void *)(uintptr_t)hal_device->ftm_regs.ftm_pro_reg,
                  HI1106_MAC_CFG_FTM_BUF_SIZE_OFFSET,
                  HI1106_MAC_CFG_FTM_BUF_SIZE_LEN, cfg_ftm_buf_size);
}


uint32_t hi1106_host_ftm_set_white_list(hal_host_device_stru *hal_device, uint8_t idx,
                                        uint8_t *mac_addr)
{
    uint32_t reg_value;
    uint32_t addr_lsb = 0;
    uint32_t addr_msb = 0;

    if (idx >= FTM_WHITELIST_ADDR_SIZE) { /* 偏移31个以上寄存器地址，超出了ftm寄存器地址偏移范围 */
        return OAL_FAIL;
    }
    reg_value = (uint32_t)mac_addr[MAC_ADDR_5];
    addr_lsb |= (reg_value & 0x000000FF);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_4];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_8);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_3];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_16);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_2];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_24);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_1];
    addr_msb |= (reg_value & 0x000000FF);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_0];
    addr_msb |= ((reg_value & 0x000000FF) << BIT_OFFSET_8);

    /* 以FTM采样白名单低位寄存器起始地址为基地址，偏移8 * idx个字节 */
    oal_writel(addr_lsb, (uintptr_t)hal_device->ftm_regs.white_addr_lsb + 8 * idx);

    /* 以FTM采样白名单高位寄存器起始地址为基地址，偏移8 * idx个字节 */
    oal_writel(addr_msb, (uintptr_t)hal_device->ftm_regs.white_addr_msb + 8 * idx);

    return OAL_SUCC;
}
#endif