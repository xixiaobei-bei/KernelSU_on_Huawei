

#include "wlan_chip.h"
#include "oal_main.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#include "hmac_ext_if.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_blockack.h"
#include "hmac_cali_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_tx_amsdu.h"
#include "hmac_hcc_adapt.h"
#include "hmac_scan.h"
#include "wal_linux_ioctl.h"
#include "hmac_config.h"
#include "mac_mib.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "plat_pm_wlan.h"
#endif
#include "hisi_customize_wifi.h"
#include "hisi_customize_wifi_hi1103.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WLAN_CHIP_1103_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC void hwifi_cfg_host_global_switch_init(void)
{
#ifndef _PRE_LINUX_TEST
    /*************************** 低功耗定制化 *****************************/
    /* 由于device 低功耗开关不是true false,而host是,先取定制化的值赋给device开关,再根据此值给host低功耗开关赋值 */
    g_wlan_device_pm_switch = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);
    g_wlan_ps_mode = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_PS_MODE);
    g_wlan_fast_ps_dyn_ctl = ((MAX_FAST_PS == g_wlan_ps_mode) ? 1 : 0);
    g_wlan_min_fast_ps_idle = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MIN_FAST_PS_IDLE);
    g_wlan_max_fast_ps_idle = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MAX_FAST_PS_IDLE);
    g_wlan_auto_ps_screen_on = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON);
    g_wlan_auto_ps_screen_off = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF);
    g_wlan_host_pm_switch = (g_wlan_device_pm_switch == WLAN_DEV_ALL_ENABLE ||
        g_wlan_device_pm_switch == WLAN_DEV_LIGHT_SLEEP_SWITCH_EN) ? OAL_TRUE : OAL_FALSE;

    g_feature_switch[HMAC_MIRACAST_SINK_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MIRACAST_SINK_ENABLE) == 1);
    g_feature_switch[HMAC_MIRACAST_REDUCE_LOG_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_REDUCE_MIRACAST_LOG) == 1);
    g_feature_switch[HMAC_CORE_BIND_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CORE_BIND_CAP) == 1);
#endif
}

OAL_STATIC void hwifi_cfg_host_global_init_sounding(void)
{
    int32_t l_priv_value = 0;
    int32_t l_ret;

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RX_STBC, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: rx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_rx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TX_STBC, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: tx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_tx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFER, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: su bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_su_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFEE, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: su bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_su_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFER, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: mu bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFEE, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: mu bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }
}


OAL_STATIC void hwifi_cfg_host_global_init_param_extend(void)
{
#if (defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)) || \
    defined(_PRE_WLAN_FEATURE_HIEX)
    int32_t priv_value = 0;
    int32_t l_ret;
#endif

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LOCK_CPU_FREQ, &priv_value);
    if (l_ret == OAL_SUCC) {
        g_freq_lock_control.uc_lock_max_cpu_freq = (oal_bool_enum_uint8) !!priv_value;
    }
    oal_io_print("hwifi_cfg_host_global_init_param_extend:lock_max_cpu_freq[%d]\r\n",
        g_freq_lock_control.uc_lock_max_cpu_freq);
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HIEX_CAP, &priv_value);
    if (l_ret == OAL_SUCC) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &priv_value,
            sizeof(mac_hiex_cap_stru)) != EOK) {
            oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap memcpy_s fail!");
        }
    }
    oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap[0x%X]\r\n", *(uint32_t *)&g_st_default_hiex_cap);
#endif
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH, &priv_value);
    if (l_ret == OAL_SUCC) {
        g_optimized_feature_switch_bitmap = (uint8_t)priv_value;
    }

#ifdef _PRE_WLAN_FEATURE_FTM
        l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_FTM_CAP, &priv_value);
        if (l_ret == OAL_SUCC) {
            g_mac_ftm_cap = (uint8_t)priv_value;
        }
#endif
}

void hwifi_cfg_host_global_init_dbdc_param(void)
{
    uint8_t uc_cmd_idx;
    uint8_t uc_device_idx;
    int32_t l_priv_value = 0;
    int32_t l_ret;

    uc_cmd_idx = WLAN_CFG_PRIV_DBDC_RADIO_0;
    for (uc_device_idx = 0; uc_device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_idx++) {
        l_ret = hwifi_get_init_priv_value(uc_cmd_idx, &l_priv_value);
        if (l_ret == OAL_SUCC) {
            /* 定制化 RADIO_0高4bit 给dbdc软件开关用 */
            l_priv_value = (int32_t)((uint32_t)l_priv_value & 0x0F);
            g_wlan_service_device_per_chip[uc_device_idx] = (uint8_t)(uint32_t)l_priv_value;
        }

        uc_cmd_idx++;
    }
    /* 同步host侧业务device */
    memcpy_s(g_auc_mac_device_radio_cap, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP,
             g_wlan_service_device_per_chip, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
}

/*
 * 函 数 名  : hwifi_cfg_host_global_init_param_1103
 * 功能描述  : 初始化定制化ini文件host侧全局变量
 */
OAL_STATIC void hwifi_cfg_host_global_init_param_1103(void)
{
    int32_t l_priv_value = 0;
    int32_t l_ret;

    hwifi_cfg_host_global_switch_init();
    /*************************** 私有定制化 *******************************/
    hwifi_cfg_host_global_init_dbdc_param();
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BW_MAX_WITH, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: bw max with[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_channel_width = (wlan_bw_cap_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LDPC_CODING, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: ldpc coding[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_ldpc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    hwifi_cfg_host_global_init_sounding();

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_1024_QAM, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: 1024 qam[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_1024qam_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DOWNLOAD_RATE_LIMIT_PPS, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param:download rx drop threshold[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_download_rate_limit_pps = (uint16_t)l_priv_value;
    }

    hwifi_cfg_host_global_init_param_extend();

    return;
}

OAL_STATIC oal_bool_enum_uint8 wlan_first_powon_mark_1103(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;
    int32_t l_priv_value = 0;

    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &l_priv_value) == OAL_SUCC) {
        cali_first_pwr_on = !!(CALI_FIST_POWER_ON_MASK & (uint32_t)l_priv_value);
        oal_io_print("host_module_init:cali_first_pwr_on pri_val[0x%x]first_pow[%d]\r\n",
            l_priv_value, cali_first_pwr_on);
    }
    return cali_first_pwr_on;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_is_aput_support_160m_1103(void)
{
    int32_t l_priv_value;
    if (hwifi_get_init_priv_value(WLAN_CFG_APUT_160M_ENABLE, &l_priv_value) == OAL_SUCC) {
        return !!l_priv_value;
    }
    return OAL_FALSE;
}

OAL_STATIC void wlan_chip_get_flow_ctrl_used_mem_1103(struct wlan_flow_ctrl_params *flow_ctrl)
{
    flow_ctrl->start = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START);
    flow_ctrl->stop  = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP);
}

OAL_STATIC uint32_t hwifi_force_update_rf_params_1103(void)
{
    return hwifi_config_init(CUS_TAG_NV);
}

OAL_STATIC uint8_t wlan_chip_get_selfstudy_country_flag_1103(void)
{
    int32_t l_priv_value = 0;
    hwifi_get_init_priv_value(WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG, &l_priv_value);
    return (uint8_t)l_priv_value;
}

OAL_STATIC uint32_t wlan_chip_get_11ax_switch_mask_1103(void)
{
    int32_t l_priv_value = 0;
    hwifi_get_init_priv_value(WLAN_CFG_PRIV_11AX_SWITCH, &l_priv_value);
    return l_priv_value;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_get_11ac2g_enable_1103(void)
{
    return !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
}

OAL_STATIC uint32_t wlan_chip_get_probe_resp_mode_1103(void)
{
    return hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_PROBE_RESP_MODE);
}
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC oal_bool_enum_uint8 wlan_chip_h2d_cmd_need_filter_1103(uint32_t cmd_id)
{
    /* 对1103/1105产品，1106独有事件不需要下发device */
    return oal_value_in_valid_range(cmd_id, WLAN_CFGID_HI1106_PRIV_START, WLAN_CFGID_HI1106_PRIV_END);
}

OAL_STATIC uint32_t wlan_chip_update_cfg80211_mgmt_tx_wait_time_1103(uint32_t wait_time)
{
    return wait_time;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_setup_ba_session_1103(void)
{
    return OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_process_bar_1103(void)
{
    /* 1103/1105需要软件处理bar */
    return OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_ba_need_check_lut_idx_1103(void)
{
    /* 1103/1105需要检查LUT idx */
    return OAL_TRUE;
}

OAL_STATIC void wlan_chip_mac_mib_set_auth_rsp_time_out_1103(mac_vap_stru *mac_vap)
{
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_AP_TIMEOUT);
    } else {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_TIMEOUT);
    }
}

static void wlan_chip_proc_query_station_packets_1103(hmac_vap_stru *hmac_vap,
    dmac_query_station_info_response_event *response_event)
{
    /* 1103 tx/rx帧统计从host获取 */
    hmac_vap->station_info.rx_packets = response_event->rx_packets;
    hmac_vap->station_info.rx_bytes = response_event->rx_bytes;
    hmac_vap->station_info.rx_dropped_misc = response_event->rx_dropped_misc;

    hmac_vap->station_info.tx_packets = response_event->tx_packets;
    hmac_vap->station_info.tx_bytes = response_event->tx_bytes;
    hmac_vap->station_info.tx_retries = response_event->tx_retries;
    hmac_vap->station_info.tx_failed = response_event->tx_failed;
}
static uint32_t wlan_chip_scan_req_alloc_and_fill_netbuf_1103(frw_event_mem_stru *event_mem, hmac_vap_stru *hmac_vap,
                                                              oal_netbuf_stru **netbuf_scan_req, void *params)
{
    int32_t ret;
    uint8_t *netbuf_data = NULL;
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *scan_req_event = NULL;

    /* 申请netbuf内存  */
    (*netbuf_scan_req) = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        (sizeof(mac_scan_req_h2d_stru)), OAL_NETBUF_PRIORITY_MID);
    if ((*netbuf_scan_req) == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_req_post_event_msg_info_set_1103::netbuf_scan_req alloc failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************** copy data **************************/
    memset_s(oal_netbuf_cb(*netbuf_scan_req), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    netbuf_data = (uint8_t *)(oal_netbuf_data(*netbuf_scan_req));
    /* 拷贝扫描请求参数到netbuf data区域 */
    ret = memcpy_s(netbuf_data, sizeof(mac_scan_req_h2d_stru), (uint8_t *)params, sizeof(mac_scan_req_h2d_stru));
    if (ret != EOK) {
        oal_netbuf_free(*netbuf_scan_req);
        oam_error_log0(0, OAM_SF_ANY, "wlan_chip_scan_req_alloc_and_fill_netbuf_1103::memcpy fail!");
        return OAL_FAIL;
    }

    /* 拷贝netbuf 到事件数据区域 */
    scan_req_event = (dmac_tx_event_stru *)event->auc_event_data;
    scan_req_event->pst_netbuf = (*netbuf_scan_req);
    scan_req_event->us_frame_len = sizeof(mac_scan_req_h2d_stru);
    scan_req_event->us_remain = 0;
    return OAL_SUCC;
}
const struct wlan_chip_ops g_wlan_chip_ops_1103 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_1103,
    .first_power_on_mark = wlan_first_powon_mark_1103,
    .first_powon_cali_completed = NULL,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_1103,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_1103,
    .force_update_custom_params = hwifi_force_update_rf_params_1103,
    .init_nvram_main = hwifi_config_init_nvram_main_1103,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_1103,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_1103,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_1103,
    .custom_cali = wal_custom_cali_1103,
    .custom_cali_data_host_addr_init = NULL,
    .send_cali_data = wal_send_cali_data_1103,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_1103,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_1103,
    .show_customize_info = hwifi_get_cfg_params,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_1103,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_1103,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_1103,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_1103,
#endif
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_1103,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_1103,

    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_hdl_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_1103,
    .tx_update_amsdu_num = hmac_update_amsdu_num_1103, /* 注意：该函数1103/1105实现不同 */
    .check_need_process_bar = wlan_chip_check_need_process_bar_1103,
    .ba_send_reorder_timeout = hmac_ba_send_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_1103,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_1103,
#endif
    .get_6g_flag = mac_get_rx_6g_flag_1103,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_1103,
    .save_cali_event = hmac_save_cali_event_1103,
    .update_cur_chn_cali_data = NULL,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_1103,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_1103,

    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_1103,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_1103, /* 大包AMDPU+大包AMSDU入口03 05生效 */
#endif
    .check_headroom_len = check_headroom_add_length,
    .adjust_netbuf_data = hmac_adjust_netbuf_data,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_1103,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_1103,
#ifdef _PRE_WLAN_FEATURE_MONITOR
    .set_sniffer_config = hmac_config_set_sniffer_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .csi_config = hmac_csi_config_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .ftm_config = hmac_config_ftm_dbg_1103,
    .rrm_proc_rm_request = NULL,
    .config_wifi_rtt_config = hmac_config_wifi_rtt_config_1103,
    .ftm_vap_init = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = NULL,
#endif
};

const struct wlan_chip_ops g_wlan_chip_ops_1105 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_1103,
    .first_power_on_mark = wlan_first_powon_mark_1103,
    .first_powon_cali_completed = NULL,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_1103,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_1103,
    .force_update_custom_params = hwifi_force_update_rf_params_1103,
    .init_nvram_main = hwifi_config_init_nvram_main_1103,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_1103,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_1103,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_1103,
    .custom_cali = wal_custom_cali_1103,
    .custom_cali_data_host_addr_init = NULL,
    .send_cali_data = wal_send_cali_data_1105,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_1103,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_1103,
    .show_customize_info = hwifi_get_cfg_params,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_1103,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_1103,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_1103,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_1103,
#endif
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_1103,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_1103,

    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_hdl_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_1103,
    .tx_update_amsdu_num = hmac_update_amsdu_num_1105, /* 注意：该函数1103/1105实现不同 */
    .check_need_process_bar = wlan_chip_check_need_process_bar_1103,
    .ba_send_reorder_timeout = hmac_ba_send_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_1103,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_1103,
#endif
    .get_6g_flag = mac_get_rx_6g_flag_1103,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_1105,
    .save_cali_event = hmac_save_cali_event_1105,
    .update_cur_chn_cali_data = NULL,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_1105,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_1103,

    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_1103,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_1103, /* 大包AMDPU+大包AMSDU入口03 05生效 */
#endif
    .check_headroom_len = check_headroom_add_length,
    .adjust_netbuf_data = hmac_adjust_netbuf_data,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_1103,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_1103,
#ifdef _PRE_WLAN_FEATURE_MONITOR
    .set_sniffer_config = hmac_config_set_sniffer_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .csi_config = hmac_csi_config_1103,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .ftm_config = hmac_config_ftm_dbg_1103,
    .rrm_proc_rm_request = NULL,
    .config_wifi_rtt_config = hmac_config_wifi_rtt_config_1103,
    .ftm_vap_init = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = NULL,
#endif
};

