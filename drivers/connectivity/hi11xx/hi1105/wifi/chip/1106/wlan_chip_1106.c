

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
#include "hmac_stat.h"
#include "hmac_scan.h"


#include "hisi_customize_wifi.h"
#include "hisi_customize_wifi_hi1106.h"

#include "hmac_config.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
#include "hmac_csi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "plat_pm_wlan.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#include "mac_mib.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WLAN_CHIP_1106_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC void hwifi_cfg_host_global_switch_init_1106(void)
{
#ifndef _PRE_LINUX_TEST
    /*************************** 低功耗定制化 *****************************/
    /* 由于device 低功耗开关不是true false,而host是,先取定制化的值赋给device开关,再根据此值给host低功耗开关赋值 */
    g_wlan_device_pm_switch = g_cust_cap.wlan_device_pm_switch;
    g_wlan_ps_mode = g_cust_host.wlan_ps_mode;
    g_wlan_fast_ps_dyn_ctl = ((MAX_FAST_PS == g_wlan_ps_mode) ? 1 : 0);
    g_wlan_min_fast_ps_idle = g_cust_cap.fast_ps.wlan_min_fast_ps_idle;
    g_wlan_max_fast_ps_idle = g_cust_cap.fast_ps.wlan_max_fast_ps_idle;
    g_wlan_auto_ps_screen_on = g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_on;
    g_wlan_auto_ps_screen_off = g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_off;
    g_wlan_host_pm_switch = (g_wlan_device_pm_switch == WLAN_DEV_ALL_ENABLE ||
        g_wlan_device_pm_switch == WLAN_DEV_LIGHT_SLEEP_SWITCH_EN) ? OAL_TRUE : OAL_FALSE;

    g_feature_switch[HMAC_MIRACAST_SINK_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH] == 1);
    g_feature_switch[HMAC_MIRACAST_REDUCE_LOG_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH] == 1);
    g_feature_switch[HMAC_CORE_BIND_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH] == 1);
#endif
}

OAL_STATIC void hwifi_cfg_host_global_init_sounding_1106(void)
{
    g_pst_mac_device_capability[0].en_rx_stbc_is_supp = g_cust_cap.en_rx_stbc_is_supp;
    g_pst_mac_device_capability[0].en_tx_stbc_is_supp = g_cust_cap.en_tx_stbc_is_supp;
    g_pst_mac_device_capability[0].en_su_bfmer_is_supp = g_cust_cap.en_su_bfmer_is_supp;
    g_pst_mac_device_capability[0].en_su_bfmee_is_supp = g_cust_cap.en_su_bfmee_is_supp;
    g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = g_cust_cap.en_mu_bfmer_is_supp;
    g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = g_cust_cap.en_mu_bfmee_is_supp;
}

OAL_STATIC void hwifi_cfg_host_global_init_param_extend_1106(void)
{
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_LOCK_CPU_FREQ)) {
        g_freq_lock_control.uc_lock_max_cpu_freq = g_cust_host.lock_max_cpu_freq;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_HIEX_CAP)) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &g_cust_cap.hiex_cap,
            sizeof(mac_hiex_cap_stru)) != EOK) {
            oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap memcpy_s fail!");
        }
    }
    oal_io_print("hwifi_cfg_host_global_init_param_extend_1106:hiex cap[0x%X]\r\n",
        *(uint32_t *)&g_st_default_hiex_cap);
#endif
    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_OPTIMIZED_FEATURE_SWITCH)) {
        g_optimized_feature_switch_bitmap = g_cust_cap.optimized_feature_mask;
    }

    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_TRX_SWITCH)) {
        hmac_set_trx_switch(g_cust_cap.trx_switch);
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_FTM)) {
        g_mac_ftm_cap = g_cust_cap.ftm_cap;
    }
#endif
}

OAL_STATIC void hwifi_cfg_host_global_init_mcm_1106(void)
{
    uint8_t chan_radio_cap, chan_num_2g, chan_num_5g;

    chan_radio_cap = g_cust_cap.chn_radio_cap;
    /* 获取通道数目,刷新nss信息到mac_device */
    chan_num_2g = chan_radio_cap & 0x0F;
    chan_num_5g = chan_radio_cap & 0xF0;

    g_pst_mac_device_capability[0].en_nss_num =
        oal_max(oal_bit_get_num_one_byte(chan_num_2g), oal_bit_get_num_one_byte(chan_num_5g)) - 1;

    g_mcm_mask_custom = g_cust_cap.mcm_custom_func_mask;
}

/* 将定制化数据配置到host业务使用的全局变量 */
OAL_STATIC void hwifi_cfg_host_global_init_param_1106(void)
{
    uint8_t device_idx;

    hwifi_cfg_host_global_switch_init_1106();
    /*************************** 私有定制化 *******************************/
    for (device_idx = 0; device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; device_idx++) {
        /* 定制化 RADIO_0高4bit 给dbdc软件开关用 */
        g_wlan_service_device_per_chip[device_idx] = g_cust_cap.radio_cap[device_idx] & 0x0F;
    }
    /* 同步host侧业务device */
    memcpy_s(g_auc_mac_device_radio_cap, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP,
             g_wlan_service_device_per_chip, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);

    g_pst_mac_device_capability[0].en_channel_width = g_cust_cap.en_channel_width;
    g_pst_mac_device_capability[0].en_ldpc_is_supp = g_cust_cap.en_ldpc_is_supp;

    hwifi_cfg_host_global_init_sounding_1106();

    g_pst_mac_device_capability[0].en_1024qam_is_supp = g_cust_cap.en_1024qam_is_supp;
    g_pst_mac_device_capability[0].hisi_priv_cap.hisi_priv_value = g_cust_cap.hisi_priv_cap;
    g_download_rate_limit_pps = g_cust_cap.download_rate_limit_pps;
    hwifi_cfg_host_global_init_mcm_1106();

    hwifi_cfg_host_global_init_param_extend_1106();
    return;
}

OAL_STATIC oal_bool_enum_uint8 wlan_first_powon_mark_1106(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;

    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_CALI_DATA_MASK)) {
        cali_first_pwr_on = !!(CALI_FIST_POWER_ON_MASK & g_cust_cap.cali_data_mask);
        oal_io_print("wlan_first_powon_mark_1106:cali_first_pwr_on [%d]\r\n",
                     cali_first_pwr_on);
    }
    return cali_first_pwr_on;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_is_aput_support_160m_1106(void)
{
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_APUT_160M_ENABLE)) {
        return !!g_cust_cap.aput_160m_switch;
    }
    return OAL_FALSE;
}

OAL_STATIC void wlan_chip_get_flow_ctrl_used_mem_1106(struct wlan_flow_ctrl_params *flow_ctrl)
{
    flow_ctrl->start = g_cust_cap.used_mem_for_start;
    flow_ctrl->stop  = g_cust_cap.used_mem_for_stop;
}

OAL_STATIC uint32_t hwifi_force_update_rf_params_1106(void)
{
    return hwifi_config_init_1106(CUS_TAG_POW);
}

OAL_STATIC uint8_t wlan_chip_get_selfstudy_country_flag_1106(void)
{
    return g_cust_cap.country_self_study;
}

OAL_STATIC uint32_t wlan_chip_get_11ax_switch_mask_1106(void)
{
    return g_cust_cap.wifi_11ax_switch_mask;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_get_11ac2g_enable_1106(void)
{
    return g_cust_host.wlan_11ac2g_switch;
}

OAL_STATIC uint32_t wlan_chip_get_probe_resp_mode_1106(void)
{
    return g_cust_host.wlan_probe_resp_mode;
}
OAL_STATIC uint32_t wlan_chip_get_trx_switch_1106(void)
{
    return g_cust_cap.trx_switch;
}

#endif // _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC const uint32_t g_cmd_need_filter_1106[] = {
    WLAN_CFGID_ETH_SWITCH,
    WLAN_CFGID_80211_UCAST_SWITCH,
    WLAN_CFGID_80211_MCAST_SWITCH,
    WLAN_CFGID_PROBE_SWITCH,
    WLAN_CFGID_OTA_BEACON_SWITCH,
    WLAN_CFGID_OTA_RX_DSCR_SWITCH,
    WLAN_CFGID_VAP_STAT,
    WLAN_CFGID_REPORT_VAP_INFO,
    WLAN_CFGID_USER_INFO,
    WLAN_CFGID_ALL_STAT,
    WLAN_CFGID_BSS_TYPE,
    WLAN_CFGID_GET_HIPKT_STAT,
    WLAN_CFGID_PROT_MODE,
    WLAN_CFGID_BEACON_INTERVAL,
    WLAN_CFGID_NO_BEACON,
    WLAN_CFGID_DTIM_PERIOD,
    WLAN_CFGID_SET_POWER_TEST,
    WLAN_CFGID_GET_ANT,
    WALN_CFGID_SET_P2P_SCENES,
    WLAN_CFGID_QUERY_RSSI,
    WLAN_CFGID_QUERY_PSST,
    WLAN_CFGID_QUERY_RATE,
    WLAN_CFGID_QUERY_ANI,
    WLAN_CFGID_SET_P2P_MIRACAST_STATUS,
    WLAN_CFGID_SET_CUS_DYN_CALI_PARAM,
    WLAN_CFGID_SET_ALL_LOG_LEVEL,
    WLAN_CFGID_SET_CUS_RF,
    WLAN_CFGID_SET_CUS_DTS_CALI,
    WLAN_CFGID_SET_CUS_NVRAM_PARAM,
    WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS,
    WLAN_CFGID_SEND_BAR,
    WLAN_CFGID_RESET_HW,
    WLAN_CFGID_GET_USER_RSSBW,
    WLAN_CFGID_ENABLE_ARP_OFFLOAD,
    WLAN_CFGID_SHOW_ARPOFFLOAD_INFO,
    WLAN_CFGID_SET_AUTO_PROTECTION,
    WLAN_CFGID_SET_BW_FIXED,
    WLAN_CFGID_SET_FEATURE_LOG,
    WLAN_CFGID_WMM_SWITCH,
    WLAN_CFGID_NARROW_BW,
};

OAL_STATIC oal_bool_enum_uint8 wlan_chip_h2d_cmd_need_filter_1106(uint32_t cmd_id)
{
    uint32_t idx;

    for (idx = 0; idx < oal_array_size(g_cmd_need_filter_1106); idx++) {
        if (cmd_id == g_cmd_need_filter_1106[idx]) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE; /* 不用过滤 */
}

OAL_STATIC uint32_t wlan_chip_update_cfg80211_mgmt_tx_wait_time_1106(uint32_t wait_time)
{
    if (wait_time <= 100) { // 100 1106 FPGA 切换信道时间较长，对于等待时间小于100ms 发帧，修改为150ms
        wait_time = 150;    // 小于100ms 监听时间，增加为150ms，增加发送时间。避免管理帧未来得及发送，超时定时器就结束。
    }
    return wait_time;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_setup_ba_session_1106(void)
{
    /* 06 device tx由于硬件约束不允许建立聚合 */
    return (hmac_get_tx_switch() == DEVICE_TX) ? OAL_FALSE : OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_process_bar_1106(void)
{
    /* 1106 硬件处理bar，软件不需要处理 */
    return OAL_FALSE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_ba_need_check_lut_idx_1106(void)
{
    /* 1106无lut限制，不用检查LUT idx */
    return OAL_FALSE;
}

OAL_STATIC void wlan_chip_mac_mib_set_auth_rsp_time_out_1106(mac_vap_stru *mac_vap)
{
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_AP_TIMEOUT);
    } else {
        /* 1106FPGA阶段 入网auth时间加长为1105一倍 */
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_TIMEOUT * 2); /* 入网时间增加2倍 */
    }
}

static void wlan_chip_proc_query_station_packets_1106(hmac_vap_stru *hmac_vap,
    dmac_query_station_info_response_event *response_event)
{
    hmac_vap_stat_stru *hmac_vap_stat = hmac_stat_get_vap_stat(hmac_vap);
    /* 1106 tx/rx帧统计从host获取 */
    hmac_vap->station_info.rx_packets = hmac_vap_stat->rx_packets;
    hmac_vap->station_info.rx_bytes = hmac_vap_stat->rx_bytes;
    hmac_vap->station_info.rx_dropped_misc = response_event->rx_dropped_misc + hmac_vap_stat->rx_dropped_misc;

    hmac_vap->station_info.tx_packets = hmac_vap_stat->tx_packets;
    hmac_vap->station_info.tx_bytes = hmac_vap_stat->tx_bytes;
    hmac_vap->station_info.tx_retries = response_event->tx_retries + hmac_vap_stat->tx_retries;
    hmac_vap->station_info.tx_failed = response_event->tx_failed + hmac_vap_stat->tx_failed;
}

static uint32_t wlan_chip_scan_req_alloc_and_fill_netbuf_1106(frw_event_mem_stru *event_mem, hmac_vap_stru *hmac_vap,
                                                              oal_netbuf_stru **netbuf_scan_req, void *params)
{
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *scan_req_event = NULL;
    mac_scan_req_h2d_stru *scan_req_h2d = (mac_scan_req_h2d_stru *)params;
    mac_scan_req_ex_stru *scan_req_ex = NULL;

    /* 申请netbuf内存  */
    (*netbuf_scan_req) = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        (sizeof(mac_scan_req_ex_stru)), OAL_NETBUF_PRIORITY_MID);
    if ((*netbuf_scan_req) == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_req_post_event_msg_info_set_1106::netbuf_scan_req alloc failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************** copy data **************************/
    memset_s(oal_netbuf_cb(*netbuf_scan_req), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    scan_req_ex = (mac_scan_req_ex_stru *)(oal_netbuf_data(*netbuf_scan_req));
    /* 拷贝扫描请求参数到netbuf data区域 */
    scan_req_ex->scan_flag = scan_req_h2d->scan_flag;
    hmac_scan_param_convert_ex(scan_req_ex, &(scan_req_h2d->st_scan_params));

    /* 拷贝netbuf 到事件数据区域 */
    scan_req_event = (dmac_tx_event_stru *)event->auc_event_data;
    scan_req_event->pst_netbuf = (*netbuf_scan_req);
    scan_req_event->us_frame_len = sizeof(mac_scan_req_ex_stru);
    scan_req_event->us_remain = 0;
    return OAL_SUCC;
}

const struct wlan_chip_ops g_wlan_chip_ops_1106 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_1106,
    .first_power_on_mark = wlan_first_powon_mark_1106,
    .first_powon_cali_completed = hmac_first_powon_cali_completed,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_1106,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_1106,
    .force_update_custom_params = hwifi_force_update_rf_params_1106,
    .init_nvram_main = hwifi_config_init_nvram_main_1106,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_1106,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_1106,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_1106,
    .custom_cali = wal_custom_cali_1106,
    .custom_cali_data_host_addr_init = hwifi_rf_cali_data_host_addr_init_1106,
    .send_20m_all_chn_cali_data = wal_send_cali_data_1106,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_1106,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_1106,
    .show_customize_info = hwifi_show_customize_info_1106,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_1106,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_1106,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_1106,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_1106,
    .get_trx_switch = wlan_chip_get_trx_switch_1106,
#endif
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_1106,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_1106,
    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_win_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_1106,
    .tx_update_amsdu_num = hmac_update_amsdu_num_1106,
    .check_need_process_bar = wlan_chip_check_need_process_bar_1106,
    .ba_send_reorder_timeout = hmac_ba_send_ring_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_1106,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_1106,
#endif
    .get_6g_flag = mac_get_rx_6g_flag_1106,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_1106,
    .save_cali_event = hmac_save_cali_event_1106,
    .update_cur_chn_cali_data = hmac_update_cur_chn_cali_data_1106,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_1106,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_1106,
    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_1106,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_1106, /* 大包AMDPU+大包AMSDU入口06不生效 */
#endif
    .check_headroom_len = check_headroom_length,
    .adjust_netbuf_data = hmac_format_netbuf_header,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_1106,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_1106,
#ifdef _PRE_WLAN_FEATURE_MONITOR
    .set_sniffer_config = hmac_config_set_sniffer_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .csi_config = hmac_csi_config_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .ftm_config = hmac_config_ftm_dbg_1106,
    .rrm_proc_rm_request = hmac_rrm_proc_rm_request_1106,
    .config_wifi_rtt_config = hmac_config_wifi_rtt_config_1106,
    .ftm_vap_init = hmac_ftm_vap_init_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = hmac_config_start_zero_wait_dfs_handle,
#endif
};

const struct wlan_chip_ops g_wlan_chip_ops_bisheng = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_1106,
    .first_power_on_mark = wlan_first_powon_mark_1106,
    .first_powon_cali_completed = hmac_first_powon_cali_completed,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_1106,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_1106,
    .force_update_custom_params = hwifi_force_update_rf_params_1106,
    .init_nvram_main = hwifi_config_init_nvram_main_1106,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_1106,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_1106,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_1106,
    .custom_cali = wal_custom_cali_1106,
    .custom_cali_data_host_addr_init = hwifi_rf_cali_data_host_addr_init_1106,
    .send_cali_data = wal_send_cali_data_1106,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_1106,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_1106,
    .show_customize_info = hwifi_show_customize_info_1106,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_1106,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_1106,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_1106,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_1106,
    .get_trx_switch = wlan_chip_get_trx_switch_1106,
#endif
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_1106,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_1106,
    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_win_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_1106,
    .tx_update_amsdu_num = hmac_update_amsdu_num_1106,
    .check_need_process_bar = wlan_chip_check_need_process_bar_1106,
    .ba_send_reorder_timeout = hmac_ba_send_ring_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_1106,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_1106,
#endif
    .get_6g_flag = mac_get_rx_6g_flag_1106,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_1106,
    .save_cali_event = NULL,
    .update_cur_chn_cali_data = hmac_update_cur_chn_cali_data_1106,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_1106,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_1106,
    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_1106,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_1106, /* 大包AMDPU+大包AMSDU入口06不生效 */
#endif
    .check_headroom_len = check_headroom_length,
    .adjust_netbuf_data = hmac_format_netbuf_header,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_1106,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_1106,
#ifdef _PRE_WLAN_FEATURE_MONITOR
    .set_sniffer_config = hmac_config_set_sniffer_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .csi_config = hmac_csi_config_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .ftm_config = hmac_config_ftm_dbg_1106,
    .rrm_proc_rm_request = hmac_rrm_proc_rm_request_1106,
    .config_wifi_rtt_config = hmac_config_wifi_rtt_config_1106,
    .ftm_vap_init = hmac_ftm_vap_init_1106,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = hmac_config_start_zero_wait_dfs_handle,
#endif
};
