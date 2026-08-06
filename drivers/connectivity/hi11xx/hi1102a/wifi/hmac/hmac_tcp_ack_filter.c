


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


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
oal_uint32 g_ul_2G_tx_large_pps_th = 4000;
oal_uint32 g_ul_5G_tx_large_pps_th = 20000;

/* {2.4G, 5G} */
/* {tx_large_pps, rx_large_pps, tx_small_pps, rx_small_pps}  */
oal_uint16 g_aus_tcp_tx_performance_test_th[WLAN_BAND_BUTT][4] = {{4000, 200, 200, 500}, {20000, 500, 500, 1500}};
/*****************************************************************************
  3 函数实现
*****************************************************************************/

OAL_STATIC oal_void hmac_rx_tcp_ack_update_pkt(mac_tcp_ack_record_stru *pst_tcp_ack_record)
{
    hmac_wifi_statistic_rx_tcp_ack(pst_tcp_ack_record->uc_rx_small_pkt, pst_tcp_ack_record->uc_rx_large_pkt);
}


OAL_STATIC oal_void hmac_rx_tcp_ack_update_ba(mac_tcp_ack_record_stru *pst_tcp_ack_record)
{
    oal_uint8 uc_tid = WLAN_TIDNO_BEST_EFFORT;
    hmac_user_stru  *pst_hmac_user;
    hmac_ba_rx_stru *pst_ba_rx_hdl;

    pst_hmac_user = (hmac_user_stru *) mac_res_get_hmac_user(pst_tcp_ack_record->uc_user_index);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        return;
    }

    pst_ba_rx_hdl = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    if (OAL_PTR_NULL == pst_ba_rx_hdl)
    {
        return;
    }

    if (DMAC_BA_COMPLETE != pst_ba_rx_hdl->en_ba_status)
    {
        OAM_WARNING_LOG2(0, OAM_SF_BA, "{hmac_rx_tcp_ack_update_ba::tid[%d] ba_status[%d]", uc_tid, pst_ba_rx_hdl->en_ba_status);
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
    if (!pst_rx_info->bit_has_tcp_ack_info)
    {
        return OAL_PTR_NULL;
    }

    uc_tcp_ack_info_size = OAL_SIZEOF(mac_tcp_ack_record_stru);
    us_frame_len = (oal_uint16) OAL_NETBUF_LEN(pst_netbuf);
    if (us_frame_len < uc_tcp_ack_info_size)
    {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_tcp_ack_record_get::us_frame_len[%d] ERROR!}", us_frame_len);
        return OAL_PTR_NULL;
    }

    us_netbuf_origin_len = us_frame_len - uc_tcp_ack_info_size;

    /* tcp ack info位于netbuf data末尾 */
    pst_tcp_ack_info = (mac_tcp_ack_record_stru *) ((oal_uint8 *) oal_netbuf_data(pst_netbuf) + us_netbuf_origin_len);

    return pst_tcp_ack_info;
}


oal_void hmac_rx_process_tcp_ack_record(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_tcp_ack_record_stru *pst_tcp_ack_record;

    if (OAL_PTR_NULL == pst_netbuf)
    {
        return;
    }

    pst_tcp_ack_record = hmac_tcp_ack_record_get(pst_netbuf);
    if (OAL_PTR_NULL == pst_tcp_ack_record)
    {
        return;
    }

    /* 更新BA */
    hmac_rx_tcp_ack_update_ba(pst_tcp_ack_record);

    /* 更新小包rx pkt数量 */
    hmac_rx_tcp_ack_update_pkt(pst_tcp_ack_record);

    /* tcp ack过滤信息更新收尾操作 */
    hmac_rx_process_tcp_ack_record_post_do(pst_netbuf);
}


OAL_STATIC oal_uint32 hmac_sync_tcp_ack_filter_switch(hmac_vap_stru *pst_hmac_vap, oal_bool_enum_uint8 en_tcp_ack_filter)
{
    oal_uint32 ul_ret;
    mac_vap_stru *pst_mac_vap;

    pst_hmac_vap->uc_tcp_ack_filter_switch_cnt = 0;
    pst_hmac_vap->en_tcp_ack_filter = en_tcp_ack_filter;

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    /***************************************************************************
                    抛事件到DMAC层, 开启/关闭tcp ack过滤功能
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TCP_ACK_FILTER_SWITCH, OAL_SIZEOF(oal_bool_enum_uint8), &en_tcp_ack_filter);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_sync_tcp_ack_filter_switch::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_bool_enum_uint8 hmac_in_tcp_tx_performace_test(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_large_pps, oal_uint32 ul_rx_large_pps, oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    wlan_channel_band_enum_uint8 en_band;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;

    en_band = pst_mac_vap->st_channel.en_band;
    en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

    /* 目前只考虑2.4G 20M和5G 80M两种场景 */
    if ((WLAN_BAND_2G == en_band && WLAN_BAND_WIDTH_20M == en_bandwidth) ||
        (WLAN_BAND_5G == en_band && WLAN_BAND_WIDTH_80PLUSPLUS <= en_bandwidth))
    {
        /* 不同频段下tcp tx跑流标准不同 */
        if (ul_tx_large_pps > g_aus_tcp_tx_performance_test_th[en_band][0] &&
            ul_rx_large_pps < g_aus_tcp_tx_performance_test_th[en_band][1] &&
            ul_tx_small_pps < g_aus_tcp_tx_performance_test_th[en_band][2] &&
            ul_rx_small_pps > g_aus_tcp_tx_performance_test_th[en_band][3])
        {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_void hmac_rx_tcp_ack_filter_switch(oal_uint32 ul_tx_large_pps, oal_uint32 ul_rx_large_pps, oal_uint32 ul_tx_small_pps, oal_uint32 ul_rx_small_pps)
{
    oal_bool_enum_uint8 en_tcp_ack_filter = OAL_FALSE;
    mac_vap_stru    *pst_mac_vap = OAL_PTR_NULL;
    hmac_vap_stru   *pst_hmac_vap;
    mac_device_stru *pst_mac_device;

    if (!g_en_tcp_ack_filter_switch)
    {
        return;
    }

    pst_mac_device = mac_res_get_dev(0);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        return;
    }

    /* 单非P2P STA跑流场景才开启device tcp ack过滤功能 */
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1 || OAL_SUCC != mac_device_find_legacy_sta(pst_mac_device, &pst_mac_vap))
    {
        return;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        return;
    }

    en_tcp_ack_filter = hmac_in_tcp_tx_performace_test(pst_mac_vap, ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps, ul_rx_small_pps);

    /* 开易关难, 避免开关反复跳变 */
    if (pst_hmac_vap->en_tcp_ack_filter != en_tcp_ack_filter)
    {
        if (!en_tcp_ack_filter)
        {
            /* 连续多次尝试关闭时才关闭 */
            pst_hmac_vap->uc_tcp_ack_filter_switch_cnt++;
            if (pst_hmac_vap->uc_tcp_ack_filter_switch_cnt == HMAC_TCP_ACK_FILTER_SWITCH_MAX_CNT)
            {
                hmac_sync_tcp_ack_filter_switch(pst_hmac_vap, en_tcp_ack_filter);
            }
        }
        else
        {
            /* 尝试开启时马上开启 */
            hmac_sync_tcp_ack_filter_switch(pst_hmac_vap, en_tcp_ack_filter);
        }
    }
    else
    {
        pst_hmac_vap->uc_tcp_ack_filter_switch_cnt = 0;
    }
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

