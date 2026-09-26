

#ifndef __WLAN_CHIP_I_H__
#define __WLAN_CHIP_I_H__

#include "wlan_chip.h"
#include "oneimage.h"
#include "oal_main.h"

uint32_t wlan_chip_ops_init(void);

OAL_STATIC OAL_INLINE void wlan_chip_host_global_init_param(void)
{
    if (g_wlan_chip_ops->host_global_init_param) {
        g_wlan_chip_ops->host_global_init_param();
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_first_power_on_mark(void)
{
    if (g_wlan_chip_ops->first_power_on_mark) {
        return g_wlan_chip_ops->first_power_on_mark();
    }
    return OAL_TRUE;
}
OAL_STATIC OAL_INLINE void wlan_chip_first_powon_cali_completed(void)
{
    if (g_wlan_chip_ops->first_powon_cali_completed) {
        g_wlan_chip_ops->first_powon_cali_completed();
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_is_aput_support_160m(void)
{
    if (g_wlan_chip_ops->is_aput_support_160M) {
        return g_wlan_chip_ops->is_aput_support_160M();
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE void wlan_chip_get_flow_ctrl_used_mem(struct wlan_flow_ctrl_params *flow_ctrl)
{
    if (g_wlan_chip_ops->get_flow_ctrl_used_mem) {
        g_wlan_chip_ops->get_flow_ctrl_used_mem(flow_ctrl);
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_h2d_cmd_need_filter(uint32_t cmd_id)
{
    if (g_wlan_chip_ops->h2d_cmd_need_filter) {
        return g_wlan_chip_ops->h2d_cmd_need_filter(cmd_id);
    }
    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_force_update_custom_params(void)
{
    if (g_wlan_chip_ops->force_update_custom_params) {
        return g_wlan_chip_ops->force_update_custom_params();
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_init_nvram_main(oal_net_device_stru *cfg_net_dev)
{
    if (g_wlan_chip_ops->init_nvram_main) {
        return g_wlan_chip_ops->init_nvram_main(cfg_net_dev);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE void wlan_chip_cpu_freq_ini_param_init(void)
{
    if (g_wlan_chip_ops->cpu_freq_ini_param_init) {
        g_wlan_chip_ops->cpu_freq_ini_param_init();
    }
}

OAL_STATIC OAL_INLINE void wlan_chip_host_global_ini_param_init(void)
{
    if (g_wlan_chip_ops->host_global_ini_param_init) {
        g_wlan_chip_ops->host_global_ini_param_init();
    }
}

OAL_STATIC OAL_INLINE uint8_t wlan_chip_get_selfstudy_country_flag(void)
{
    if (g_wlan_chip_ops->get_selfstudy_country_flag) {
        return g_wlan_chip_ops->get_selfstudy_country_flag();
    }
    return 0;
}
OAL_STATIC OAL_INLINE uint32_t wlan_chip_custom_cali(void)
{
    if (g_wlan_chip_ops->custom_cali) {
        return g_wlan_chip_ops->custom_cali();
    }
    return OAL_FAIL;
}
OAL_STATIC OAL_INLINE void wlan_chip_custom_host_cali_data_init(void)
{
    if (g_wlan_chip_ops->custom_cali_data_host_addr_init) {
        g_wlan_chip_ops->custom_cali_data_host_addr_init();
    }
}
OAL_STATIC OAL_INLINE uint32_t wlan_chip_custom_host_read_cfg_init(void)
{
    if (g_wlan_chip_ops->custom_host_read_cfg_init) {
        return g_wlan_chip_ops->custom_host_read_cfg_init();
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_hcc_customize_h2d_data_cfg(void)
{
    if (g_wlan_chip_ops->hcc_customize_h2d_data_cfg) {
        return g_wlan_chip_ops->hcc_customize_h2d_data_cfg();
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE void wlan_chip_show_customize_info(void)
{
    if (g_wlan_chip_ops->show_customize_info) {
        g_wlan_chip_ops->show_customize_info();
    }
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_get_sar_ctrl_params(uint8_t lvl_num,
    uint8_t *data_addr, uint16_t *data_len, uint16_t dest_len)
{
    if (g_wlan_chip_ops->get_sar_ctrl_params) {
        return g_wlan_chip_ops->get_sar_ctrl_params(lvl_num, data_addr, data_len, dest_len);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_get_11ax_switch_mask(void)
{
    if (g_wlan_chip_ops->get_11ax_switch_mask) {
        return g_wlan_chip_ops->get_11ax_switch_mask();
    }
    return 0;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_get_11ac2g_enable(void)
{
    if (g_wlan_chip_ops->get_11ac2g_enable) {
        return g_wlan_chip_ops->get_11ac2g_enable();
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_get_probe_resp_mode(void)
{
    if (g_wlan_chip_ops->get_probe_resp_mode) {
        return g_wlan_chip_ops->get_probe_resp_mode();
    }
    return 0;
}
OAL_STATIC OAL_INLINE uint32_t wlan_chip_get_trx_switch(void)
{
    if (g_wlan_chip_ops->get_trx_switch) {
        return g_wlan_chip_ops->get_trx_switch();
    }
    return DEVICE_RX;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_update_cfg80211_mgmt_tx_wait_time(uint32_t wait_time)
{
    if (g_wlan_chip_ops->update_cfg80211_mgmt_tx_wait_time) {
        return g_wlan_chip_ops->update_cfg80211_mgmt_tx_wait_time(wait_time);
    }
    return wait_time;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_check_need_setup_ba_session(void)
{
    if (g_wlan_chip_ops->check_need_setup_ba_session) {
        return g_wlan_chip_ops->check_need_setup_ba_session();
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE uint8_t wlan_chip_tx_update_amsdu_num(mac_vap_stru *mac_vap,
    hmac_performance_stat_stru *performance_stat_params, oal_bool_enum_uint8 mu_vap_flag)
{
    if (g_wlan_chip_ops->tx_update_amsdu_num) {
        return g_wlan_chip_ops->tx_update_amsdu_num(mac_vap, performance_stat_params, mu_vap_flag);
    }
    return WLAN_TX_AMSDU_BUTT;
}

OAL_STATIC OAL_INLINE void wlan_chip_ba_rx_hdl_init(hmac_vap_stru *pst_hmac_vap,
    hmac_user_stru *pst_hmac_user, uint8_t uc_tid)
{
    if (g_wlan_chip_ops->ba_rx_hdl_init) {
        g_wlan_chip_ops->ba_rx_hdl_init(pst_hmac_vap, pst_hmac_user, uc_tid);
    }
}
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
OAL_STATIC OAL_INLINE uint32_t wlan_chip_config_tcp_ack_buf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_chip_ops->tcp_ack_buff_config) {
        return g_wlan_chip_ops->tcp_ack_buff_config(mac_vap, len, param);
    }
    return OAL_FAIL;
}
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
OAL_STATIC OAL_INLINE uint32_t wlan_chip_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_chip_ops->csi_config) {
        return g_wlan_chip_ops->csi_config(mac_vap, len, param);
    }
    return OAL_FAIL;
}
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC OAL_INLINE uint32_t wlan_chip_config_wifi_rtt_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_chip_ops->ftm_config) {
        return g_wlan_chip_ops->config_wifi_rtt_config(mac_vap, len, param);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_ftm_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_chip_ops->ftm_config) {
        return g_wlan_chip_ops->ftm_config(mac_vap, len, param);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_rrm_proc_rm_request(hmac_vap_stru *hmac_vap_sta,
    oal_netbuf_stru *netbuf)
{
    if (g_wlan_chip_ops->rrm_proc_rm_request) {
        return g_wlan_chip_ops->rrm_proc_rm_request(hmac_vap_sta, netbuf);
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE void wlan_chip_ftm_vap_init(hmac_vap_stru *hmac_vap)
{
    if (g_wlan_chip_ops->ftm_vap_init) {
        g_wlan_chip_ops->ftm_vap_init(hmac_vap);
    }
}
#endif
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_check_need_process_bar(void)
{
    if (g_wlan_chip_ops->check_need_process_bar) {
        return g_wlan_chip_ops->check_need_process_bar();
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_ba_send_reorder_timeout(hmac_ba_rx_stru *rx_ba, hmac_vap_stru *hmac_vap,
    hmac_ba_alarm_stru *alarm_data, uint32_t *timeout)
{
    if (g_wlan_chip_ops->ba_send_reorder_timeout) {
        return g_wlan_chip_ops->ba_send_reorder_timeout(rx_ba, hmac_vap, alarm_data, timeout);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_ba_need_check_lut_idx(void)
{
    if (g_wlan_chip_ops->ba_need_check_lut_idx) {
        return g_wlan_chip_ops->ba_need_check_lut_idx();
    }
    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_send_cali_matrix_data(mac_vap_stru *mac_vap)
{
    if (g_wlan_chip_ops->send_cali_matrix_data) {
        return g_wlan_chip_ops->send_cali_matrix_data(mac_vap);
    }
    return OAL_FAIL;
}

OAL_STATIC OAL_INLINE void wlan_chip_send_cali_data(oal_net_device_stru *cfg_net_dev)
{
    if (g_wlan_chip_ops->send_cali_data) {
        return g_wlan_chip_ops->send_cali_data(cfg_net_dev);
    }
}

OAL_STATIC OAL_INLINE void wlan_chip_send_20m_all_chn_cali_data(oal_net_device_stru *cfg_net_dev)
{
    if (g_wlan_chip_ops->send_20m_all_chn_cali_data) {
        return g_wlan_chip_ops->send_20m_all_chn_cali_data(cfg_net_dev);
    }
}
OAL_STATIC OAL_INLINE uint32_t wlan_chip_save_cali_event(frw_event_mem_stru *event_mem)
{
    if (g_wlan_chip_ops->save_cali_event) {
        return g_wlan_chip_ops->save_cali_event(event_mem);
    }
    return OAL_FAIL;
}
OAL_STATIC OAL_INLINE void wlan_chip_update_cur_chn_cali_data(mac_vap_stru *mac_vap,
    wlan_h2d_cali_data_type_enum_uint8 data_type)
{
    if (g_wlan_chip_ops->update_cur_chn_cali_data) {
        g_wlan_chip_ops->update_cur_chn_cali_data(mac_vap, data_type);
    }
}

OAL_STATIC OAL_INLINE void wlan_chip_mac_vap_init_mib_11ax(mac_vap_stru *mac_vap, uint32_t nss_num)
{
    if (g_wlan_chip_ops->mac_vap_init_mib_11ax) {
        g_wlan_chip_ops->mac_vap_init_mib_11ax(mac_vap, nss_num);
    }
}

OAL_STATIC OAL_INLINE void wlan_chip_mac_mib_set_auth_rsp_time_out(mac_vap_stru *mac_vap)
{
    if (g_wlan_chip_ops->mac_mib_set_auth_rsp_time_out) {
        g_wlan_chip_ops->mac_mib_set_auth_rsp_time_out(mac_vap);
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_mac_vap_need_set_user_htc_cap(mac_vap_stru *mac_vap)
{
    if (g_wlan_chip_ops->mac_vap_need_set_user_htc_cap) {
        return g_wlan_chip_ops->mac_vap_need_set_user_htc_cap(mac_vap);
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_mac_get_6g_flag(dmac_rx_ctl_stru *rx_ctrl)
{
    if (g_wlan_chip_ops->get_6g_flag) {
        return g_wlan_chip_ops->get_6g_flag(rx_ctrl);
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE void wlan_chip_tx_encap_large_skb_amsdu(hmac_vap_stru *hmac_vap,
    hmac_user_stru *user, oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl)
{
    if (g_wlan_chip_ops->tx_encap_large_skb_amsdu) {
        g_wlan_chip_ops->tx_encap_large_skb_amsdu(hmac_vap, user, buf, tx_ctl);
    }
}

OAL_STATIC OAL_INLINE uint32_t wlan_chip_check_headroom_len(mac_tx_ctl_stru *tx_ctl,
    uint8_t nest_type, uint8_t nest_sub_type, uint8_t cb_len)
{
    if (g_wlan_chip_ops->check_headroom_len) {
        return g_wlan_chip_ops->check_headroom_len(tx_ctl, nest_type, nest_sub_type, cb_len);
    }
    return 0;
}

OAL_STATIC OAL_INLINE void wlan_chip_adjust_netbuf_data(oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctrl,
    uint8_t nest_type, uint8_t nest_sub_type)
{
    if (g_wlan_chip_ops->adjust_netbuf_data) {
        g_wlan_chip_ops->adjust_netbuf_data(netbuf, tx_ctrl, nest_type, nest_sub_type);
    }
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wlan_chip_wmmac_need_degrade_for_ts(uint8_t tid,
    uint8_t need_degrade, uint8_t wmmac_auth_flag)
{
    if (g_wlan_chip_ops->wmmac_need_degrade_for_ts) {
        return g_wlan_chip_ops->wmmac_need_degrade_for_ts(tid, need_degrade, wmmac_auth_flag);
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE void wlan_chip_proc_query_station_packets(hmac_vap_stru *hmac_vap,
    dmac_query_station_info_response_event *response_event)
{
    if (g_wlan_chip_ops->proc_query_station_packets) {
        g_wlan_chip_ops->proc_query_station_packets(hmac_vap, response_event);
    }
}
OAL_STATIC OAL_INLINE uint32_t wlan_chip_scan_req_alloc_and_fill_netbuf(frw_event_mem_stru *event_mem,
    hmac_vap_stru *hmac_vap, oal_netbuf_stru **netbuf_scan_req, void *params)
{
    if (g_wlan_chip_ops->scan_req_alloc_and_fill_netbuf) {
        return g_wlan_chip_ops->scan_req_alloc_and_fill_netbuf(event_mem, hmac_vap, netbuf_scan_req, params);
    }
    return OAL_FAIL;
}
#ifdef _PRE_WLAN_FEATURE_MONITOR
OAL_STATIC OAL_INLINE uint32_t wlan_chip_set_sniffer_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_chip_ops->set_sniffer_config) {
        return g_wlan_chip_ops->set_sniffer_config(mac_vap, len, param);
    }
    return OAL_FAIL;
}
#endif

OAL_STATIC OAL_INLINE void wlan_chip_start_zero_wait_dfs(mac_vap_stru *mac_vap,
    mac_cfg_channel_param_stru *channel_info)
{
    if (g_wlan_chip_ops->start_zero_wait_dfs) {
        return g_wlan_chip_ops->start_zero_wait_dfs(mac_vap, channel_info);
    }
}
#endif /* end of wlan_chip_i.h */


