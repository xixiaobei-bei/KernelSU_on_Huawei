
#ifdef _PRE_WLAN_FEATURE_CSI

/* 1 头文件包含 */
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "host_mac_1106.h"
#include "host_csi_1106.h"
#include "plat_pm_wlan.h"
#include "hmac_tx_msdu_dscr.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CSI_C

#include "securec.h"
#include "securectype.h"


static uint32_t hi1106_regs_csi_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset = hi1106_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(0, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hi1106_regs_csi_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case HI1106_MAC_CSI_BUF_BASE_ADDR_LSB_REG:
            hal_device->csi_regs.base_lsb = host_va;
            break;
        case HI1106_MAC_CSI_BUF_BASE_ADDR_MSB_REG:
            hal_device->csi_regs.base_msb = host_va;
            break;
        case HI1106_MAC_CSI_BUF_SIZE_REG:
            hal_device->csi_regs.size = host_va;
            break;
        case HI1106_MAC_CSI_PROCESS_REG:
            hal_device->csi_regs.csi_pro_reg = host_va;
            break;
        case HI1106_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG:
            hal_device->csi_regs.chn_set = host_va;
            break;
        case HI1106_MAC_RX_CTRL_REG:
            hal_device->csi_regs.rx_ctrl_reg = host_va;
            break;
        case HI1106_MAC_CSI_WHITELIST_ADDR_LSB_0_REG:
            hal_device->csi_regs.white_addr_lsb = host_va;
            break;
        case HI1106_MAC_CSI_WHITELIST_ADDR_MSB_0_REG:
            hal_device->csi_regs.white_addr_msb = host_va;
            break;
        case HI1106_MAC_LOCATION_INFO_MASK_REG:
            hal_device->csi_regs.location_info = host_va;
            break;
        case HI1106_MAC_CSI_INFO_ADDR_LSB_REG:
            hal_device->csi_regs.csi_info_lsb = host_va;
            break;
        case HI1106_MAC_CSI_INFO_ADDR_MSB_REG:
            hal_device->csi_regs.csi_info_msb = host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{hi1106_regs_csi_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}


void hi1106_csi_base_addr_init(hal_host_device_stru *hal_device)
{
    hal_device->csi_regs.devva = 0;
    hal_device->csi_regs.inited = OAL_FALSE;
    hal_device->csi_regs.rbase_dma_addr = 0;
    hal_device->csi_regs.rbase_vaddr = NULL;
}

uint32_t hi1106_csi_reg_init(hal_host_device_stru *hal_device)
{
    uint32_t idx;
    uint32_t hal_regs[] = {
        HI1106_MAC_CSI_BUF_BASE_ADDR_LSB_REG,
        HI1106_MAC_CSI_BUF_BASE_ADDR_MSB_REG,
        HI1106_MAC_CSI_BUF_SIZE_REG,
        HI1106_MAC_CSI_PROCESS_REG,
        HI1106_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG,
        HI1106_MAC_RX_CTRL_REG,
        HI1106_MAC_CSI_WHITELIST_ADDR_LSB_0_REG,
        HI1106_MAC_CSI_WHITELIST_ADDR_MSB_0_REG,
        HI1106_MAC_LOCATION_INFO_MASK_REG,
        HI1106_MAC_CSI_INFO_ADDR_LSB_REG,
        HI1106_MAC_CSI_INFO_ADDR_MSB_REG,
    };

    if (hal_device->device_id >= HAL_DEVICE_ID_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "{hi1106_csi_reg_init::device_id[%d].}", hal_device->device_id);
        return OAL_FAIL;
    }

    if (hal_device->csi_regs.inited == OAL_FALSE) {
        for (idx = 0; idx < sizeof(hal_regs) / sizeof(uint32_t); idx++) {
            if (OAL_SUCC != hi1106_regs_csi_transfer(hal_device, hal_regs[idx])) {
                hal_device->csi_regs.inited = OAL_FALSE;
                return OAL_FAIL;
            }
        }
        hal_device->csi_regs.inited = OAL_TRUE;
        oam_warning_log0(0, OAM_SF_ANY, "{hi1106_csi_reg_init::regs addr trans succ.}");
    }
    return OAL_SUCC;
}


void hi1106_host_set_csi_buf_base_addr(hal_host_device_stru *hal_device, uint64_t devva)
{
    if (devva == 0) {
        oam_error_log0(0, 0, "hi1106_host_set_csi_buf_base_addr::devva is empty");
        return;
    }
    oal_writel(get_low_32_bits(devva), (uintptr_t)hal_device->csi_regs.base_lsb);
    oal_writel(get_high_32_bits(devva), (uintptr_t)hal_device->csi_regs.base_msb);
}


void hi1106_host_set_csi_buf_size(hal_host_device_stru *hal_device, uint16_t cfg_csi_buf_size)
{
    oal_writel(cfg_csi_buf_size, (uintptr_t)hal_device->csi_regs.size);
}


void hi1106_host_set_csi_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_EN_OFFSET,
                  HI1106_MAC_CFG_CSI_EN_LEN, reg_value);            /* CSI采集使能 */
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_H_MATRIX_RPT_EN_OFFSET,
                  HI1106_MAC_CFG_CSI_H_MATRIX_RPT_EN_LEN, reg_value); /* 上报h矩阵 */
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.chn_set,
                  HI1106_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_OFFSET,
                  HI1106_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_LEN, reg_value);
}


void hi1106_host_set_phy_info_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.rx_ctrl_reg,
                  HI1106_MAC_CFG_RX_PPDU_DESC_MODE_OFFSET, HI1106_MAC_CFG_RX_PPDU_DESC_MODE_LEN, reg_value);
}


void hi1106_host_set_csi_bw(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_BAND_WIDTH_MASK_OFFSET, HI1106_MAC_CFG_CSI_BAND_WIDTH_MASK_LEN,
                  reg_value);
}


void hi1106_host_set_csi_frame_type(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_FRAME_TYPE_MASK_OFFSET, HI1106_MAC_CFG_CSI_FRAME_TYPE_MASK_LEN,
                  reg_value);
}

void hi1106_host_set_csi_extrat_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_SAMPLE_PERIOD_OFFSET, HI1106_MAC_CFG_CSI_SAMPLE_PERIOD_LEN,
                  reg_value);
}


void hi1106_host_set_csi_sample_period(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.csi_pro_reg,
                  HI1106_MAC_CFG_CSI_SAMPLE_PERIOD_OFFSET, HI1106_MAC_CFG_CSI_SAMPLE_PERIOD_LEN,
                  reg_value);
}


/* 当前只配置到csi_num=0的寄存器中，总共有32对TA寄存器 */
void hi1106_host_set_csi_ta(hal_host_device_stru *hal_device, uint8_t *mac_addr)
{
    uint32_t bssid_msb; /* MAC地址高16位 */
    uint32_t bssid_lsb; /* MAC地址低32位 */

    bssid_msb = mac_addr[MAC_ADDR_0];
    bssid_msb = (bssid_msb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_1];

    bssid_lsb = mac_addr[MAC_ADDR_2];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_3];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_4];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_5];

    oal_writel(bssid_msb, (uintptr_t)hal_device->csi_regs.white_addr_msb);
    oal_writel(bssid_lsb, (uintptr_t)hal_device->csi_regs.white_addr_lsb);
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

static uint32_t hi1106_config_csi_buffer_addr(hal_host_device_stru *hal_device, uint32_t cfg_csi_on)
{
    int32_t ret;
    oal_pcie_linux_res  *pcie_linux_res = oal_get_pcie_linux_res();

    if (cfg_csi_on == OAL_TRUE) {
        if (hal_device->csi_regs.rbase_vaddr == NULL) {
            hal_device->csi_regs.rbase_vaddr =
                dma_alloc_coherent(&pcie_linux_res->pst_pcie_dev->dev, HMAC_CSI_BUFF_SIZE,
                                   &hal_device->csi_regs.rbase_dma_addr, GFP_KERNEL);

            if (hal_device->csi_regs.rbase_vaddr == NULL) {
                oam_error_log2(0, 0, "hmac_config_csi_buffer_addr:dma_alloc fail[%p]size[%d]",
                               &pcie_linux_res->pst_pcie_dev->dev, HMAC_CSI_BUFF_SIZE);
                return OAL_FAIL;
            }
            memset_s(hal_device->csi_regs.rbase_vaddr, HMAC_CSI_BUFF_SIZE,
                     0, HMAC_CSI_BUFF_SIZE);

            ret = pcie_if_hostca_to_devva(0, (uint64_t)hal_device->csi_regs.rbase_dma_addr,
                                          &hal_device->csi_regs.devva);
            if (ret != OAL_SUCC) {
                oam_error_log0(0, 0, "hmac_config_csi_buffer_addr alloc pcie_if_hostca_to_devva fail.");
                dma_free_coherent(&pcie_linux_res->pst_pcie_dev->dev,
                                  HMAC_CSI_BUFF_SIZE,
                                  hal_device->csi_regs.rbase_vaddr,
                                  (dma_addr_t)hal_device->csi_regs.rbase_dma_addr);
                hal_device->csi_regs.rbase_vaddr = NULL;
                return OAL_FAIL;
            }

            hi1106_host_set_csi_buf_base_addr(hal_device, hal_device->csi_regs.devva);
            hi1106_host_set_csi_buf_size(hal_device, HMAC_CSI_BUFF_SIZE);
        }
    } else {
        if (hal_device->csi_regs.rbase_vaddr != NULL) {
            dma_free_coherent(&pcie_linux_res->pst_pcie_dev->dev,
                              HMAC_CSI_BUFF_SIZE,
                              hal_device->csi_regs.rbase_vaddr,
                              (dma_addr_t)hal_device->csi_regs.rbase_dma_addr);
            hal_device->csi_regs.rbase_vaddr = NULL;
        }
    }
    return OAL_SUCC;
}
#else
static uint32_t hi1106_config_csi_buffer_addr(hal_host_device_stru *hal_device, uint32_t cfg_csi_on)
{
    int32_t ret;
    oal_pcie_linux_res  *pcie_linux_res = oal_get_pcie_linux_res();

    if (cfg_csi_on == OAL_TRUE) {
        if (hal_device->csi_regs.rbase_vaddr == NULL) {
            hal_device->csi_regs.rbase_vaddr = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, HMAC_CSI_BUFF_SIZE, OAL_FALSE);

            if (hal_device->csi_regs.rbase_vaddr == NULL) {
                oam_error_log2(0, 0, "hmac_config_csi_buffer_addr:dma_alloc fail[%p]size[%d]",
                               &pcie_linux_res->pst_pcie_dev->dev, HMAC_CSI_BUFF_SIZE);
                return OAL_FAIL;
            }
            memset_s(hal_device->csi_regs.rbase_vaddr, HMAC_CSI_BUFF_SIZE,
                     0, HMAC_CSI_BUFF_SIZE);

            ret = pcie_if_hostca_to_devva(0, (uint64_t)hal_device->csi_regs.rbase_dma_addr,
                                          &hal_device->csi_regs.devva);
            if (ret != OAL_SUCC) {
                oam_error_log0(0, 0, "hmac_config_csi_buffer_addr alloc pcie_if_hostca_to_devva fail.");
                oal_mem_free_m(hal_device->csi_regs.rbase_vaddr, OAL_TRUE);
                hal_device->csi_regs.rbase_vaddr = NULL;
                return OAL_FAIL;
            }

            hi1106_host_set_csi_buf_base_addr(hal_device, hal_device->csi_regs.devva);
            hi1106_host_set_csi_buf_size(hal_device, HMAC_CSI_BUFF_SIZE);
        }
    } else {
        if (hal_device->csi_regs.rbase_vaddr != NULL) {
            oal_mem_free_m(hal_device->csi_regs.rbase_vaddr, OAL_TRUE);
            hal_device->csi_regs.rbase_vaddr = NULL;
        }
    }
    return OAL_SUCC;
}
#endif

uint32_t hi1106_get_host_ftm_csi_locationinfo(hal_host_device_stru *hal_device)
{
    return oal_readl((void *)(uintptr_t)hal_device->csi_regs.location_info);
}

uint32_t hi1106_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_csi_param_stru *cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    hmac_vap_stru *hmac_vap = NULL;
    hal_host_device_stru *hal_device = NULL;
    oal_bool_enum_uint8 cfg_csi_on;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }
    hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (oal_any_null_ptr2(cfg_csi_param, hal_device)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_csi_config_1106::cfg_csi_param and hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_csi_on = cfg_csi_param->en_cfg_csi_on;

    if (hi1106_csi_reg_init(hal_device) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_csi_config_1106::csi init regs failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 唤醒低功耗内存 */
    wlan_pm_wakeup_dev_lock();
    hi1106_host_set_csi_en(hal_device, OAL_FALSE);
    hi1106_host_set_phy_info_en(hal_device, OAL_FALSE);

    /* 设置带宽 */
    hi1106_host_set_csi_bw(hal_device, (uint32_t)cfg_csi_param->uc_csi_bw);

    /* 设置CSI采集的帧类型 */
    hi1106_host_set_csi_frame_type(hal_device, (uint32_t)cfg_csi_param->uc_csi_frame_type);

    /* 设置CSI采样周期 */
    hi1106_host_set_csi_sample_period(hal_device, (uint32_t)cfg_csi_param->uc_csi_sample_period);

    if (!mac_addr_is_zero(cfg_csi_param->auc_mac_addr)) {
        /* 设置上报csi帧的TA mac地址 */
        hi1106_host_set_csi_ta(hal_device, cfg_csi_param->auc_mac_addr);
    }

    if (hi1106_config_csi_buffer_addr(hal_device, !!cfg_csi_on) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hi1106_host_set_csi_en(hal_device, !!cfg_csi_on); /* !! 非零传入1, 0传入0 */
    hi1106_host_set_phy_info_en(hal_device, !!cfg_csi_on);

    oam_warning_log4(0, OAM_SF_CFG,
        "{hmac_csi_config_1106::cfg_csi_on:%d, bw:%d, csi_frame_type:%d, "
        "csi_sample_period:%d.}", cfg_csi_on, cfg_csi_param->uc_csi_bw,
        cfg_csi_param->uc_csi_frame_type, cfg_csi_param->uc_csi_sample_period);

    return OAL_SUCC;
}

uint32_t hi1106_get_csi_info(hmac_csi_info_stru *hmac_csi_info, uint8_t *addr)
{
    hi1106_csi_ppu_stru *csi_ppu = NULL;
    uint32_t timestamp;
    csi_ppu = (hi1106_csi_ppu_stru *)addr;

    timestamp = csi_ppu->word7.timestamp;

    hmac_csi_info->csi_data[BYTE_OFFSET_0] = (uint8_t)((timestamp & 0xFF000000) >> BIT_OFFSET_24);
    hmac_csi_info->csi_data[BYTE_OFFSET_1] = (uint8_t)((timestamp & 0xFF0000) >> BIT_OFFSET_16);
    hmac_csi_info->csi_data[BYTE_OFFSET_2] = (uint8_t)((timestamp & 0xFF00) >> BIT_OFFSET_8);
    hmac_csi_info->csi_data[BYTE_OFFSET_3] = (uint8_t)(timestamp & 0xFF);
    hmac_csi_info->csi_data[BYTE_OFFSET_4] = (uint8_t)(csi_ppu->word4.bit_nss_mcs_rate);
    hmac_csi_info->csi_data[BYTE_OFFSET_5] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_0]);
    hmac_csi_info->csi_data[BYTE_OFFSET_6] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_1]);
    hmac_csi_info->csi_data[BYTE_OFFSET_7] = (uint8_t)(csi_ppu->word0.bit_freq_bw);
    hmac_csi_info->csi_data[BYTE_OFFSET_8] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_0]);
    hmac_csi_info->csi_data[BYTE_OFFSET_9] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_1]);

    hmac_csi_info->rpt_info_len = csi_ppu->word0.bit_rpt_info_len;
    hmac_csi_info->vap_index = csi_ppu->word4.bit_vap_index;
    return OAL_SUCC;
}

uint32_t hi1106_host_ftm_csi_init(hal_host_device_stru *hal_device)
{
    hi1106_csi_base_addr_init(hal_device);

    return OAL_SUCC;
}

#endif
