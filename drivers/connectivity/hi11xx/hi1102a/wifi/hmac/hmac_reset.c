

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_device.h"
#include "mac_resource.h"
#include "hmac_vap.h"
#include "oal_cfg80211.h"
#include "hmac_reset.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RESET_C
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_reset_sys_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_dev;
    mac_reset_sys_stru *pst_reset_sys;

    pst_reset_sys = (mac_reset_sys_stru *)puc_param;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_reset_sys_event::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (pst_reset_sys->en_reset_sys_type) {
        case MAC_RESET_STATUS_SYS_TYPE:
            if ((pst_mac_dev->uc_device_reset_in_progress == OAL_TRUE) && (pst_reset_sys->uc_value == OAL_FALSE)) {
                pst_mac_dev->us_device_reset_num++;
            }

            hmac_config_set_reset_state(pst_mac_vap, uc_len, puc_param);

            break;
        case MAC_RESET_SWITCH_SYS_TYPE:
            pst_mac_dev->en_reset_switch = pst_reset_sys->uc_value;

            break;
        default:
            break;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_config_query_chr_ext_info_rsp_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                    oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    chr_wifi_ext_info_stru *pst_query_chr_ext_info = OAL_PTR_NULL;
    oal_uint8 uc_idx;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_query_chr_ext_info_rsp_event::pst_hmac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 用全局变量保存通过device侧查询上报的chr info */
    pst_query_chr_ext_info = (chr_wifi_ext_info_stru *)puc_param;
    pst_hmac_vap->st_chr_wifi_ext_info_query.st_chr_wifi_ext_info.ul_tbtt_cnt = pst_query_chr_ext_info->ul_tbtt_cnt;
    pst_hmac_vap->st_chr_wifi_ext_info_query.st_chr_wifi_ext_info.ul_rx_beacon_cnt =
        pst_query_chr_ext_info->ul_rx_beacon_cnt;
    pst_hmac_vap->st_chr_wifi_ext_info_query.st_chr_wifi_ext_info.en_distance_id =
        pst_query_chr_ext_info->en_distance_id;
    pst_hmac_vap->st_chr_wifi_ext_info_query.st_chr_wifi_ext_info.ul_upc1_01_data =
        pst_query_chr_ext_info->ul_upc1_01_data;
    for (uc_idx = 0; uc_idx < WLAN_TID_MAX_NUM; uc_idx++) {
        pst_hmac_vap->st_chr_wifi_ext_info_query.st_chr_wifi_ext_info.auc_is_paused[uc_idx] =
            pst_query_chr_ext_info->auc_is_paused[uc_idx];
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_chr_wifi_ext_info_query.en_chr_info_ext_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->st_chr_wifi_ext_info_query.st_query_chr_wait_q));

    return OAL_SUCC;
}

OAL_STATIC oal_uint8 hmac_proc_query_response_flag(hmac_vap_stru *pst_hmac_vap,
    dmac_query_station_info_response_event *pst_rsp_event)
{
    oal_uint8 uc_flag = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_MCS) ? RATE_INFO_FLAGS_MCS : 0);
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH) ?
        RATE_INFO_FLAGS_40_MHZ_WIDTH : 0);
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_SHORT_GI) ?
        RATE_INFO_FLAGS_SHORT_GI : 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_60G) ? RATE_INFO_FLAGS_60G : 0);
#endif
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    uc_flag = pst_rsp_event->rate_info.txrx_rate.tx_rate.flags;
#else
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_MCS) ?
        RATE_INFO_FLAGS_MCS : 0);
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_VHT_MCS) ?
        RATE_INFO_FLAGS_VHT_MCS : 0);
    uc_flag |= ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_SHORT_GI) ?
        RATE_INFO_FLAGS_SHORT_GI : 0);
    uc_flag |=
        ((pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_60G) ? RATE_INFO_FLAGS_60G : 0);
    pst_hmac_vap->station_info.txrate.bw =
        pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH ? RATE_INFO_BW_40 :
        (pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH ? RATE_INFO_BW_80 :
        (pst_rsp_event->rate_info.txrx_rate.tx_rate.flags & MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH ? RATE_INFO_BW_160 :
        RATE_INFO_BW_20));
#endif
    return uc_flag;
}


oal_uint32 hmac_proc_query_response_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    station_info_extend_stru *pst_station_info_extend = OAL_PTR_NULL;
    dmac_query_station_info_response_event *pst_rsp_event = (dmac_query_station_info_response_event *)(puc_param);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_query_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_rsp_event->query_event == OAL_QUERY_STATION_INFO_EVENT) {
        pst_hmac_vap->station_info.signal = pst_rsp_event->c_signal;
        pst_hmac_vap->station_info.rx_packets = pst_rsp_event->ul_rx_packets;
        pst_hmac_vap->station_info.tx_packets = pst_rsp_event->ul_tx_packets;
        pst_hmac_vap->station_info.rx_bytes = pst_rsp_event->ul_rx_bytes;
        pst_hmac_vap->station_info.tx_bytes = pst_rsp_event->ul_tx_bytes;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
        pst_hmac_vap->station_info.tx_retries = pst_rsp_event->ul_tx_retries;
        pst_hmac_vap->station_info.rx_dropped_misc = pst_rsp_event->ul_rx_dropped_misc;
        pst_hmac_vap->station_info.tx_failed = pst_rsp_event->ul_tx_failed;
#endif
        pst_hmac_vap->station_info.txrate.mcs = pst_rsp_event->rate_info.txrx_rate.tx_rate.mcs;
        pst_hmac_vap->station_info.txrate.legacy = pst_rsp_event->rate_info.txrx_rate.tx_rate.legacy;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
        pst_hmac_vap->station_info.txrate.nss = pst_rsp_event->rate_info.txrx_rate.tx_rate.nss;
#endif
        pst_hmac_vap->station_info.rxrate.legacy = pst_rsp_event->rate_info.txrx_rate.rx_rate_legacy;
        hmac_vap_save_rx_rate(pst_hmac_vap);
        /*
         * 速率flag因内核版本而异，而DMAC依3.8.0为准给出全部结果，在此提取并转换flags
         * linux 小于 3.5.0
         *     RATE_INFO_FLAGS_MCS              1<<0,
         *     RATE_INFO_FLAGS_40_MHZ_WIDTH     1<<1,
         *     RATE_INFO_FLAGS_SHORT_GI         1<<2,
         *
         * linux 大于等于 3.5.0 && linux 小于 3.8.0
         *    RATE_INFO_FLAGS_MCS              1<<0,
         *    RATE_INFO_FLAGS_40_MHZ_WIDTH     1<<1,
         *    RATE_INFO_FLAGS_SHORT_GI         1<<2,
         *    RATE_INFO_FLAGS_60G              1<<3
         *
         * linux 大于等于 3.8.0&& linux 小于 4.0.0
         *    RATE_INFO_FLAGS_MCS              BIT(0),
         *    RATE_INFO_FLAGS_VHT_MCS          BIT(1),
         *    RATE_INFO_FLAGS_40_MHZ_WIDTH     BIT(2),
         *    RATE_INFO_FLAGS_80_MHZ_WIDTH     BIT(3),
         *    RATE_INFO_FLAGS_80P80_MHZ_WIDTH  BIT(4),
         *    RATE_INFO_FLAGS_160_MHZ_WIDTH    BIT(5),
         *    RATE_INFO_FLAGS_SHORT_GI         BIT(6),
         *    RATE_INFO_FLAGS_60G              BIT(7),
         *
         * linux 大于等于 4.0.0
         * rate_info_flags
         *    RATE_INFO_FLAGS_MCS              BIT(0),
         *    RATE_INFO_FLAGS_VHT_MCS          BIT(1),
         *    RATE_INFO_FLAGS_SHORT_GI         BIT(2),
         *    RATE_INFO_FLAGS_60G              BIT(3),
         * rate_info_bw
         *    RATE_INFO_BW_5
         *    RATE_INFO_BW_10
         *    RATE_INFO_BW_20
         *    RATE_INFO_BW_40
         *    RATE_INFO_BW_80
         *    RATE_INFO_BW_160
 */
        pst_hmac_vap->station_info.txrate.flags = hmac_proc_query_response_flag(pst_hmac_vap, pst_rsp_event);
        pst_hmac_vap->center_freq =
            oal_ieee80211_channel_to_frequency(pst_mac_vap->st_channel.uc_chan_number,
                                               (enum nl80211_band)pst_mac_vap->st_channel.en_band);
        pst_station_info_extend = &(pst_hmac_vap->st_station_info_extend);
        pst_hmac_vap->s_free_power = pst_rsp_event->s_free_power;
        pst_station_info_extend->uc_distance = pst_rsp_event->st_station_info_extend.uc_distance;
        pst_station_info_extend->uc_cca_intr = pst_rsp_event->st_station_info_extend.uc_cca_intr;
        pst_station_info_extend->ul_bcn_cnt = pst_rsp_event->st_station_info_extend.ul_bcn_cnt;
        pst_station_info_extend->ul_tbtt_cnt = pst_rsp_event->st_station_info_extend.ul_tbtt_cnt;
        pst_station_info_extend->us_chload = pst_rsp_event->st_station_info_extend.us_chload;
#ifdef _PRE_WLAN_DFT_STAT
        pst_hmac_vap->uc_device_distance = pst_rsp_event->st_station_info_extend.uc_distance;
        pst_hmac_vap->uc_intf_state_cca = pst_rsp_event->st_station_info_extend.uc_cca_intr;
        pst_hmac_vap->uc_intf_state_co = pst_rsp_event->rate_info.txrx_rate.tx_rate.en_co_intf_state;
#endif
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


oal_uint32 hmac_config_reset_operate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_dev;
    mac_reset_sys_stru st_reset_sys;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_ret;
#endif
    oal_int8 *pc_token = OAL_PTR_NULL;
    oal_int8 *pc_end = OAL_PTR_NULL;
    oal_int8 *pc_ctx = OAL_PTR_NULL;
    oal_int8 *pc_sep = " ";

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_config_reset_operate::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取复位信息 */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_reset_operate::pc_token null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 字符串转换为长整值(16表示十六进制) */
    st_reset_sys.en_reset_sys_type = (mac_reset_sys_type_enum_uint8)oal_strtol(pc_token, &pc_end, 16);

    if (st_reset_sys.en_reset_sys_type == MAC_RESET_SWITCH_SET_TYPE) {
        /* 获取Channel List */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (pc_token == NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_reset_operate::pc_token null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        st_reset_sys.uc_value = (oal_uint8)oal_strtol(pc_token, &pc_end, 16); /* 字符串转换为长整值(16表示十六进制) */
        pst_mac_dev->en_reset_switch = st_reset_sys.uc_value;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RESET_HW_OPERATE, us_len, (oal_uint8 *)&st_reset_sys);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_reset_operate::hmac_config_send_event failed[%d].}", ul_ret);
        }

        return ul_ret;
#endif
    } else if (st_reset_sys.en_reset_sys_type == MAC_RESET_SWITCH_GET_TYPE) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_reset_operate::the reset switch is %d.}",
                         pst_mac_dev->en_reset_switch);
    } else if (st_reset_sys.en_reset_sys_type == MAC_RESET_STATUS_GET_TYPE) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reset_operate::the reset status is %d, the reset num is %d.}",
                         pst_mac_dev->uc_device_reset_in_progress, pst_mac_dev->us_device_reset_num);
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reset_operate::the reset tpye is %d, out of limit.}",
                         st_reset_sys.en_reset_sys_type);
    }

    return OAL_SUCC;
}
oal_module_symbol(hmac_config_reset_operate);
