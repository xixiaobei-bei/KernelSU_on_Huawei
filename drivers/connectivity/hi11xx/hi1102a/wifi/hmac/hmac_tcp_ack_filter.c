

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tcp_ack_filter.h"
#include "hmac_edca_opt.h"
#include "hmac_statistic_data_flow.h"
#include "oam_wdk.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_ACK_FILTER_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_bool_enum_uint8 g_en_tcp_ack_filter_switch = OAL_TRUE;
oal_bool_enum_uint8 g_tcp_ack_filter_enable = OAL_FALSE;
oal_uint8 g_uc_filter_switch_cnt = 0;

oal_uint32 g_ul_2g_tx_large_pps_th = 4000;
oal_uint32 g_ul_5g_tx_large_pps_th = 20000;

/* {2.4G, 5G} */
/* {tx_large_pps, rx_large_pps, tx_small_pps, rx_small_pps} */
oal_uint16 g_aus_tcp_ack_filter_th[WLAN_BAND_BUTT][4] = {{4000, 500, 200, 500}, {20000, 1500, 500, 1500}};
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void hmac_set_filter_switch_cnt(oal_uint8 uc_filter_switch_cnt)
{
    g_uc_filter_switch_cnt = uc_filter_switch_cnt;
}


OAL_STATIC oal_void hmac_rx_tcp_ack_update_pkt(mac_tcp_ack_record_stru *pst_tcp_ack_record)
{
    hmac_wifi_statistic_rx_tcp_ack(pst_tcp_ack_record->uc_rx_filtered_small_pkt,
                                   pst_tcp_ack_record->uc_rx_filtered_large_pkt,
                                   pst_tcp_ack_record->ul_rx_filtered_bytes);
}


OAL_STATIC oal_void hmac_rx_tcp_ack_update_ba(mac_tcp_ack_record_stru *pst_tcp_ack_record)
{
    oal_uint8 uc_tid = WLAN_TIDNO_BEST_EFFORT;
    hmac_user_stru  *pst_hmac_user;
    hmac_ba_rx_stru *pst_ba_rx_hdl = OAL_PTR_NULL;

    pst_hmac_user = (hmac_user_stru *) mac_res_get_hmac_user(pst_tcp_ack_record->uc_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        return;
    }

    pst_ba_rx_hdl = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    if (pst_ba_rx_hdl == OAL_PTR_NULL) {
        return;
    }

    if (pst_ba_rx_hdl->en_ba_status != DMAC_BA_COMPLETE) {
        oam_warning_log2(0, OAM_SF_BA, "{hmac_rx_tcp_ack_update_ba::tid[%d] ba_status[%d]", uc_tid,
                         pst_ba_rx_hdl->en_ba_status);
        return;
    }

    hmac_ba_update_reorder_queue(pst_ba_rx_hdl, pst_tcp_ack_record);
}


OAL_STATIC mac_tcp_ack_record_stru *hmac_tcp_ack_record_get(oal_netbuf_stru *pst_netbuf)
{
    oal_uint16 us_frame_len;
    oal_uint16 us_netbuf_origin_len;
    oal_uint8  uc_tcp_ack_info_size;
    mac_rx_ctl_stru *pst_rx_info;
    mac_tcp_ack_record_stru *pst_tcp_ack_info = OAL_PTR_NULL;

    pst_rx_info = (mac_rx_ctl_stru *) oal_netbuf_cb(pst_netbuf);
    if (!pst_rx_info->bit_has_tcp_ack_info) {
        return OAL_PTR_NULL;
    }

    uc_tcp_ack_info_size = OAL_SIZEOF(mac_tcp_ack_record_stru);
    us_frame_len = (oal_uint16) oal_netbuf_len(pst_netbuf);
    if (us_frame_len < uc_tcp_ack_info_size) {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_tcp_ack_record_get::us_frame_len[%d] ERROR!}", us_frame_len);
        return OAL_PTR_NULL;
    }

    us_netbuf_origin_len = us_frame_len - uc_tcp_ack_info_size;

    /* tcp ack info位于netbuf data末尾 */
    pst_tcp_ack_info = (mac_tcp_ack_record_stru *)((oal_uint8 *) oal_netbuf_data(pst_netbuf) + us_netbuf_origin_len);

    return pst_tcp_ack_info;
}


oal_void hmac_rx_process_tcp_ack_record(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_tcp_ack_record_stru *pst_tcp_ack_record = OAL_PTR_NULL;

    if (pst_netbuf == OAL_PTR_NULL) {
        return;
    }

    pst_tcp_ack_record = hmac_tcp_ack_record_get(pst_netbuf);
    if (pst_tcp_ack_record == OAL_PTR_NULL) {
        return;
    }

    /* 更新BA */
    hmac_rx_tcp_ack_update_ba(pst_tcp_ack_record);

    /* 更新小包rx pkt数量 */
    hmac_rx_tcp_ack_update_pkt(pst_tcp_ack_record);

    /* tcp ack过滤信息更新收尾操作 */
    hmac_rx_process_tcp_ack_record_post_do(pst_netbuf);
}

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

OAL_STATIC oal_uint32 hmac_sync_tcp_ack_filter_switch(mac_vap_stru *pst_mac_vap,
                                                      oal_bool_enum_uint8 en_tcp_ack_filter)
{
    oal_uint32 ul_ret;

    g_tcp_ack_filter_enable = en_tcp_ack_filter;
    g_uc_filter_switch_cnt = 0;

    /***************************************************************************
                    抛事件到DMAC层, 开启/关闭tcp ack过滤功能
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TCP_ACK_FILTER_SWITCH, OAL_SIZEOF(oal_bool_enum_uint8),
                                    &en_tcp_ack_filter);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_sync_tcp_ack_filter_switch::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_bool_enum_uint8 hmac_get_tcp_ack_filter_status_pps(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_large_pps,
    oal_uint32 ul_rx_large_pps, oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    wlan_channel_bandwidth_enum_uint8 en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    wlan_channel_band_enum_uint8 en_band = pst_mac_vap->st_channel.en_band;

    /* 目前只考虑2.4G 20M和5G 80M两种场景 */
    if (((en_band == WLAN_BAND_2G) && (en_bandwidth == WLAN_BAND_WIDTH_20M)) ||
        ((en_band == WLAN_BAND_5G) && (en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS))) {
        /* 不同频段下tcp tx跑流标准不同 */
        if ((ul_tx_large_pps > g_aus_tcp_ack_filter_th[en_band][0]) &&
            /* 判断tx_small_pps是否小于g_aus_tcp_ack_filter_th[en_band][2]（2.4G 200,5G 500） */
            (ul_tx_small_pps < g_aus_tcp_ack_filter_th[en_band][2]) &&
            (ul_rx_large_pps + ul_rx_small_pps > g_aus_tcp_ack_filter_th[en_band][1] +
            /* g_aus_tcp_ack_filter_th[en_band][3]表示rx_small_pps（2.4G 500,5G 1500） */
            g_aus_tcp_ack_filter_th[en_band][3])) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


OAL_STATIC oal_bool_enum_uint8 hmac_get_tcp_ack_filter_status_sta(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_large_pps,
    oal_uint32 ul_rx_large_pps, oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    return hmac_get_tcp_ack_filter_status_pps(pst_mac_vap, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps,
                                              ul_rx_small_pps);
}


OAL_STATIC oal_bool_enum_uint8 hmac_get_tcp_ack_filter_status_ap(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_large_pps,
    oal_uint32 ul_rx_large_pps, oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    return pst_mac_vap->us_user_nums == 1 &&
           (g_st_wifi_tx_opt.uc_tx_opt_switch_ap & HMAC_TX_OPT_APUT_TCP_ACK_FILTER) != 0 &&
           hmac_get_tcp_ack_filter_status_pps(pst_mac_vap, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps,
                                              ul_rx_small_pps);
}


OAL_STATIC oal_bool_enum_uint8 hmac_tcp_ack_filter_enable(mac_device_stru *pst_mac_device,
                                                          oal_uint32 ul_tx_large_pps, oal_uint32 ul_rx_large_pps,
                                                          oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    if ((mac_device_calc_up_vap_num(pst_mac_device) != 1) ||
        (mac_device_find_up_vap(pst_mac_device, &pst_mac_vap) != OAL_SUCC)) {
        return OAL_FALSE;
    }

    if (IS_LEGACY_STA(pst_mac_vap)) {
        return hmac_get_tcp_ack_filter_status_sta(
               pst_mac_vap, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps, ul_rx_small_pps);
    } else if (IS_LEGACY_AP(pst_mac_vap)) {
        return hmac_get_tcp_ack_filter_status_ap(
               pst_mac_vap, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps, ul_rx_small_pps);
    } else {
        return OAL_FALSE;
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_tcp_ack_filter_change_state(oal_bool_enum_uint8 en_tcp_ack_filter)
{
    if (g_tcp_ack_filter_enable == en_tcp_ack_filter) {
        g_uc_filter_switch_cnt = 0;
        return OAL_FALSE;
    }

    g_uc_filter_switch_cnt++;

    /* 尝试开启时直接开启; 尝试关闭时连续3次与原状态不一致才关闭 */
    return en_tcp_ack_filter || (g_uc_filter_switch_cnt >= MAX_TCP_ACK_FILTER_SWITCH_CNT);
}


oal_void hmac_rx_tcp_ack_filter_switch(oal_uint32 ul_tx_large_pps, oal_uint32 ul_rx_large_pps,
                                       oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    oal_bool_enum_uint8 en_tcp_ack_filter;
    mac_vap_stru       *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru    *pst_mac_device = OAL_PTR_NULL;

    if (!g_en_tcp_ack_filter_switch) {
        return;
    }

    pst_mac_device = mac_res_get_dev(0);
    en_tcp_ack_filter = hmac_tcp_ack_filter_enable(pst_mac_device, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps,
                                                   ul_rx_small_pps);
    if (hmac_tcp_ack_filter_change_state(en_tcp_ack_filter)) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
        if (pst_mac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_rx_tcp_ack_filter_switch::mac_vap NULL}");
            return;
        }

        hmac_sync_tcp_ack_filter_switch(pst_mac_vap, en_tcp_ack_filter);
    }
}
#endif