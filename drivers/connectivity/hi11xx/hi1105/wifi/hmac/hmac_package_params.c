/*
 * 版权所有 (c) 华为技术有限公司 2020-2020
 * 功能说明 :
 * 创建日期 : 2020年6月17日
 */
#include "hmac_package_params.h"
#include "oal_ext_if.h"
#include "hmac_vap.h"
#include "mac_vap_common.h"
#include "hmac_main.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PACKAGE_PARAMS_C
uint32_t hmac_package_uint8_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    new_params[0] = (uint8_t)params[0];
    *new_params_len = sizeof(uint8_t);
    return OAL_SUCC;
}
uint32_t hmac_package_uint16_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    *(uint16_t *)new_params = (uint16_t)params[0];
    *new_params_len = sizeof(uint16_t);
    return OAL_SUCC;
}
uint32_t hmac_package_uint32_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    *(uint32_t*)new_params = params[0];
    *new_params_len = sizeof(uint32_t);
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE uint32_t hmac_package_check_stru_size(uint32_t stru_size)
{
    if (stru_size > CMD_DEVICE_PARAMS_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_package_check_stru_size:stru_size = %d is too long", stru_size);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

uint32_t hmac_package_tlv_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_set_tlv_stru *tlv_params = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_set_tlv_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    tlv_params = (mac_cfg_set_tlv_stru *)new_params;
    tlv_params->uc_cmd_type = (uint8_t)params[0];
    tlv_params->value = (uint32_t)params[1];
    *new_params_len = sizeof(mac_cfg_set_tlv_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_sta_ps_mode_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_ps_mode_param_stru *ps_mode_params = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_ps_mode_param_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    ps_mode_params = (mac_cfg_ps_mode_param_stru *)new_params;
    ps_mode_params->uc_vap_ps_mode = (uint8_t)params[0];
    *new_params_len = sizeof(mac_cfg_ps_mode_param_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_sta_ps_info_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_ps_info_stru *ps_info_params = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_ps_info_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    ps_info_params = (mac_cfg_ps_info_stru *)new_params;
    ps_info_params->uc_psm_info_enable = (uint8_t)params[0];
    ps_info_params->uc_psm_debug_mode = (uint8_t)params[1];
    *new_params_len = sizeof(mac_cfg_ps_info_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_fasts_sleep_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    uint8_t idx;
    /* 获取携带的4个参数<min listen时间><max listen时间><亮屏收发包门限><暗屏收发包门限> */
    for (idx = 0; idx < 4; idx++) {
        new_params[idx] = (uint8_t)params[idx];
    }
    *new_params_len = 4 * sizeof(uint8_t); // 4个参数
    return OAL_SUCC;
}

uint32_t hmac_package_uapsd_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    uint8_t idx;
    mac_cfg_uapsd_sta_stru *uapsd_param = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_uapsd_sta_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    uapsd_param = (mac_cfg_uapsd_sta_stru *)new_params;
    uapsd_param->uc_max_sp_len = (uint8_t)params[0];
    for (idx = 0; idx < WLAN_WME_AC_BUTT; idx++) {
        uapsd_param->uc_delivery_enabled[idx] = (uint8_t)params[idx + 1];
        uapsd_param->uc_trigger_enabled[idx] = (uint8_t)params[idx + 1];
    }
    *new_params_len = sizeof(mac_cfg_uapsd_sta_stru);
    return OAL_SUCC;
}
uint32_t hmac_package_btcoex_preempt_type_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_btcoex_preempt_mgr_stru *btcoex_preempt_mgr = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_btcoex_preempt_mgr_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    btcoex_preempt_mgr = (mac_btcoex_preempt_mgr_stru *)new_params;
    btcoex_preempt_mgr->uc_cfg_preempt_mode = (uint8_t)params[0];
    btcoex_preempt_mgr->uc_cfg_preempt_type = (uint8_t)params[1];
    *new_params_len = sizeof(mac_btcoex_preempt_mgr_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_ota_switch_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    oam_ota_switch_param_stru *ota_swicth_stru = NULL;
    if (hmac_package_check_stru_size(sizeof(oam_ota_switch_param_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    ota_swicth_stru = (oam_ota_switch_param_stru *)new_params;
    ota_swicth_stru->ota_type = (uint8_t)params[0];
    ota_swicth_stru->ota_switch = (uint8_t)params[1];
    *new_params_len = sizeof(oam_ota_switch_param_stru);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
uint32_t hmac_phy_event_rpt_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_phy_event_rpt_stru *phy_event_cfg = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_phy_event_rpt_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    phy_event_cfg = (mac_cfg_phy_event_rpt_stru *)new_params;
    phy_event_cfg->event_rpt_en = params[0];
    phy_event_cfg->wp_mem_num = params[1];
    phy_event_cfg->wp_event0_type_sel = params[BYTE_OFFSET_2];
    phy_event_cfg->wp_event1_type_sel = params[BYTE_OFFSET_3];

    return OAL_SUCC;
}
#endif
uint32_t hmac_package_80211_mcast_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_80211_mcast_switch_stru *mcast_switch = NULL;
    uint8_t idx = 0;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_80211_mcast_switch_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    mcast_switch = (mac_cfg_80211_mcast_switch_stru *)new_params;
    mcast_switch->en_frame_direction = (uint8_t)params[idx++]; /* 获取80211帧方向 */
    mcast_switch->en_frame_type      = (uint8_t)params[idx++]; /* 获取帧类型 */
    mcast_switch->en_frame_switch    = (uint8_t)params[idx++]; /* 获取帧内容打印开关 */
    mcast_switch->en_cb_switch       = (uint8_t)params[idx++]; /* 获取帧CB字段打印开关 */
    mcast_switch->en_dscr_switch     = (uint8_t)params[idx++]; /* 获取描述符打印开关 */
    *new_params_len = sizeof(mac_cfg_80211_mcast_switch_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_probe_switch_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_probe_switch_stru *probe_switch = NULL;
    uint8_t idx = 0;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_probe_switch_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    probe_switch = (mac_cfg_probe_switch_stru *)new_params;
    probe_switch->en_frame_direction = (uint8_t)params[idx++]; /* 获取帧方向 */
    probe_switch->en_frame_switch    = (uint8_t)params[idx++]; /* 获取帧内容打印开关 */
    probe_switch->en_cb_switch       = (uint8_t)params[idx++]; /* 获取帧CB字段打印开关 */
    probe_switch->en_dscr_switch     = (uint8_t)params[idx++]; /* 获取描述符打印开关 */
    *new_params_len = sizeof(mac_cfg_probe_switch_stru);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_TXOPPS
uint32_t hmac_package_txop_ps_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_txopps_machw_param_stru *txopps = NULL;
    uint8_t idx = 0;
    if (hmac_package_check_stru_size(sizeof(mac_txopps_machw_param_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    txopps = (mac_txopps_machw_param_stru *)new_params;
    txopps->en_machw_txopps_en         = (oal_switch_enum_uint8)params[idx++]; /* 获取txop ps使能开关 */
    txopps->en_machw_txopps_condition1 = (oal_switch_enum_uint8)params[idx++]; /* 获取txop ps condition1使能开关 */
    txopps->en_machw_txopps_condition2 = (oal_switch_enum_uint8)params[idx++]; /* 获取txop ps condition2使能开关 */
    *new_params_len = sizeof(mac_txopps_machw_param_stru);
    return OAL_SUCC;
}
#endif
uint32_t hmac_package_sta_psm_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_ps_param_stru *ps_para = NULL;
    uint8_t idx = 0;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_ps_param_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    ps_para = (mac_cfg_ps_param_stru *)new_params;
    ps_para->us_beacon_timeout  = (uint16_t)params[idx++];
    ps_para->us_tbtt_offset     = (uint16_t)params[idx++];
    ps_para->us_ext_tbtt_offset = (uint16_t)params[idx++];
    ps_para->us_dtim3_on        = (uint16_t)params[idx++];
    *new_params_len = sizeof(mac_cfg_ps_param_stru);
    return OAL_SUCC;
}
uint32_t hmac_package_show_arpoffload_info_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_arpoffload_info_stru *arpoffload_info = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_arpoffload_info_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    arpoffload_info = (mac_cfg_arpoffload_info_stru *)new_params;
    arpoffload_info->uc_show_ip_addr = (uint8_t)params[0];
    arpoffload_info->uc_show_arpoffload_info = (uint8_t)params[1];
    *new_params_len = sizeof(mac_cfg_arpoffload_info_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_icmp_filter_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_assigned_filter_cmd_stru *assigned_filter_cmd = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_assigned_filter_cmd_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    assigned_filter_cmd = (mac_assigned_filter_cmd_stru *)new_params;
    assigned_filter_cmd->en_filter_id   = (mac_assigned_filter_id_enum_uint8)params[0]; /* 获取filter id */
    assigned_filter_cmd->en_enable      = (oal_switch_enum_uint8)params[1];             /* 获取开关 */
    *new_params_len = sizeof(mac_assigned_filter_cmd_stru);
    return OAL_SUCC;
}

uint32_t hmac_package_default_key_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    uint8_t idx = 0;
    mac_setdefaultkey_param_stru *defaultkey_params = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_setdefaultkey_param_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    defaultkey_params = (mac_setdefaultkey_param_stru *)new_params;
    defaultkey_params->uc_key_index   = (uint8_t)params[idx++]; /* 获取key_index */
    defaultkey_params->en_unicast     = (uint8_t)params[idx++]; /* 获取en_unicast */
    defaultkey_params->en_multicast   = (uint8_t)params[idx++]; /* 获取multicast */
    *new_params_len = sizeof(mac_setdefaultkey_param_stru);
    return OAL_SUCC;
}
uint32_t hmac_package_cac_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_cfg_cac_stru *cac_param = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_cfg_cac_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    cac_param = (mac_cfg_cac_stru *)new_params;
    cac_param->cac_channel = (uint8_t)params[0];
    cac_param->cac_bandwidth = (uint8_t)params[1];

    *new_params_len = sizeof(mac_cfg_cac_stru);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_FTM
uint32_t hmac_package_ftm_params(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len)
{
    mac_ftm_debug_switch_stru *ftm_param = NULL;
    if (hmac_package_check_stru_size(sizeof(mac_ftm_debug_switch_stru)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    ftm_param = (mac_ftm_debug_switch_stru *)new_params;
    ftm_param->cmd_bit_map = BIT6;
    ftm_param->st_ftm_time_bit6.ftm_correct_time3 = (uint32_t)params[0];

    *new_params_len = sizeof(mac_ftm_debug_switch_stru);
    return OAL_SUCC;
}
#endif
