
#include "hmac_multi_netbuf_amsdu.h"
#include "hmac_config.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MULTI_NETBUF_AMSDU_C

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
static inline uint8_t wlan_amsdu_level_to_num(uint8_t level)
{
    return (((level) == WLAN_TX_AMSDU_NONE) ? 0 : \
        /* level number 2 */
        ((level) == WLAN_TX_AMSDU_BY_2) ? 2 : \
        /* level number 3 */
        ((level) == WLAN_TX_AMSDU_BY_3) ? 3 : \
        /* level number 4 */
        ((level) == WLAN_TX_AMSDU_BY_4) ? 4 : \
        /* level number 8 */
        ((level) == WLAN_TX_AMSDU_BY_8) ? 8 : 0);
}


uint8_t hmac_set_amsdu_num_based_protocol(mac_vap_stru *mac_vap, wlan_tx_amsdu_enum_uint8 tx_amsdu_level_type)
{
    /* 协议规定HT mpdu长度不得超过4095字节 */
    if (oal_value_eq_any3(mac_vap->en_protocol, WLAN_HT_ONLY_MODE, WLAN_HT_11G_MODE, WLAN_HT_MODE)) {
        return  WLAN_TX_AMSDU_BY_2;
    } else if ((mac_vap->en_protocol == WLAN_VHT_MODE) ||
               (mac_vap->en_protocol == WLAN_VHT_ONLY_MODE) ||
               (g_wlan_spec_cfg->feature_11ax_is_open && (mac_vap->en_protocol == WLAN_HE_MODE))) {
        return  tx_amsdu_level_type;
    } else {
        return  WLAN_TX_AMSDU_NONE;
    }
}


uint8_t hmac_update_amsdu_num_1103(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high;
    uint32_t limit_throughput_low;
    uint32_t tx_throughput_mbps = performance_stat_params->tx_throughput_mbps;
    /* 每秒吞吐量门限 */
    limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    limit_throughput_low  = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;

    if (tx_throughput_mbps > limit_throughput_high) {
        /* 高于高门限,切换amsdu大包聚合 */
        return WLAN_TX_AMSDU_BY_2;
    } else if (tx_throughput_mbps < limit_throughput_low) {
        /* 低于低门限,关闭amsdu大包聚合 */
        return WLAN_TX_AMSDU_NONE;
    } else {
        /* 介于低门限-高门限之间,不作切换 */
        return g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id];
    }
}

#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
static void hmac_update_amsdu_num_vap_info(void)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    uint32_t up_vap_num = mac_device_calc_up_vap_num(mac_device);
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;

    oam_warning_log2(0, OAM_SF_ANY, "{hmac_update_amsdu_num_vap_info::up_vap_num[%d], dev vap cnt[%d]!}", up_vap_num,
        mac_device->uc_vap_num);

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (mac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_update_amsdu_num_vap_info::vap null! id[%d]}", vap_idx);
            continue;
        }

        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_update_amsdu_num_vap_info::up vap mode[%d], p2p mode[%d], vap state[%d], user_nums[%d]!}",
            mac_vap->en_vap_mode, mac_vap->en_p2p_mode, mac_vap->en_vap_state, mac_vap->us_user_nums);
    }
}
#endif

uint8_t hmac_adjust_tx_amsdu_num_1105(mac_vap_stru *mac_vap, uint32_t tx_throughput_mbps,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    uint32_t limit_throughput_middle = g_st_tx_large_amsdu.us_amsdu_throughput_middle >> mu_vap_flag;
    uint32_t limit_throughput_low = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;
    wlan_tx_amsdu_enum_uint8 max_amsdu;

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 解决与1103的sdio兼容性问题：11ax门限比11ac低100M */
        if (mac_vap->en_protocol == WLAN_HE_MODE) {
            limit_throughput_high -= WLAN_AMSDU_THROUGHPUT_TH_HE_VHT_DIFF;
        }
    }
#endif

    /* 打开AMSDU, middle门限是开启amsdu的起始门限 */
    if (tx_throughput_mbps > limit_throughput_middle) {
        if (tx_throughput_mbps > limit_throughput_high) {
            /* 高于高门限, 则开启4xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_4;
        } else if (tx_throughput_mbps > (limit_throughput_middle << 1)) {
            /* 起始门限的两倍以上并且小于高门限, 则开启3xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_3;
        } else {
            /* 高于起始门限,则开启2xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_2;
        }

        /* 已经考虑对端的AMSDU聚合长度 */
        max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, max_amsdu);
    } else if (tx_throughput_mbps > limit_throughput_low) {
        /* 低门限-中门限, 若上次是AMSDU聚合的，则聚合2个; 否则不聚合 */
        if (g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id] != WLAN_TX_AMSDU_NONE) {
            max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, WLAN_TX_AMSDU_BY_2);
        } else {
            max_amsdu = WLAN_TX_AMSDU_NONE;
        }
    } else {
        /* 低于低门限, 关闭amsdu大包聚合 */
        max_amsdu = WLAN_TX_AMSDU_NONE;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        max_amsdu = oal_min(oal_max(max_amsdu, mac_vap->bit_ofdma_aggr_num), WLAN_TX_AMSDU_BY_8);
    }
#endif

    return max_amsdu;
}


uint8_t hmac_update_amsdu_num_1105(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    hmac_vap_stru   *hmac_vap;

    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_update_amsdu_num_1105::mac_vap null!}");
        hmac_update_amsdu_num_vap_info(); // vap异常为空，打印出当前device挂接所有vap状态
        return WLAN_TX_AMSDU_BUTT;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap != NULL) {
        if (hmac_vap->en_is_hid2d_state == OAL_TRUE) {
            return WLAN_TX_AMSDU_NONE;
        }
    }
#endif

    return hmac_adjust_tx_amsdu_num_1105(mac_vap, performance_stat_params->tx_throughput_mbps, mu_vap_flag);
}
uint8_t hmac_adjust_rx_amsdu_num_1106(mac_vap_stru *mac_vap, uint32_t rx_throughput_mbps, uint32_t tx_pps,
    wlan_tx_amsdu_enum_uint8 tx_amsdu)
{
    uint32_t limit_throughput_high;
    uint32_t limit_throughput_low;
    uint32_t limit_pps_high;
    uint32_t limit_pps_low;
    wlan_tx_amsdu_enum_uint8 amsdu_level_rx, amsdu_level;

    /* 每秒吞吐量门限，rx吞吐量判决amsdu_level */
    hmac_tx_small_amsdu_get_limit_throughput(&limit_throughput_high, &limit_throughput_low);
    /* 大于low才开启聚合 */
    if (rx_throughput_mbps > limit_throughput_low) {
        if (rx_throughput_mbps > (limit_throughput_high << 1)) {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_8;
        } else if (rx_throughput_mbps > limit_throughput_high) {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_4;
        } else {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_2;
        }
    } else {
        amsdu_level_rx = WLAN_TX_AMSDU_NONE;
    }
    amsdu_level_rx = oal_max(amsdu_level_rx, tx_amsdu);
    /* 每秒PPS门限，pps判决amsdu_level */
    hmac_tx_small_amsdu_get_limit_pps(&limit_pps_high, &limit_pps_low);
    if (tx_pps > limit_pps_low) {
        if (tx_pps > (limit_pps_high << 1)) {
            amsdu_level = WLAN_TX_AMSDU_BY_8;
        } else if (tx_pps > limit_pps_high) {
            amsdu_level = WLAN_TX_AMSDU_BY_4;
        } else {
            amsdu_level = WLAN_TX_AMSDU_BY_2;
        }
    } else {
        amsdu_level = WLAN_TX_AMSDU_NONE;
    }
    amsdu_level = oal_max(amsdu_level_rx, amsdu_level);

    return amsdu_level;
}

uint8_t hmac_adjust_tx_amsdu_num_1106(mac_vap_stru *mac_vap, uint32_t tx_throughput_mbps,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    uint32_t limit_throughput_middle = g_st_tx_large_amsdu.us_amsdu_throughput_middle >> mu_vap_flag;
    uint32_t limit_throughput_low = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;
    wlan_tx_amsdu_enum_uint8 max_amsdu;

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 解决与1103的sdio兼容性问题：11ax门限比11ac低100M */
        if (mac_vap->en_protocol == WLAN_HE_MODE) {
            limit_throughput_high -= WLAN_AMSDU_THROUGHPUT_TH_HE_VHT_DIFF;
        }
    }
#endif

    /* 打开AMSDU, middle门限是开启amsdu的起始门限 */
    if (tx_throughput_mbps > limit_throughput_middle) {
        if (tx_throughput_mbps > (limit_throughput_high << 1)) {
            /* 高于2倍高门限, 则开启8xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_8;
        } else if (tx_throughput_mbps > limit_throughput_high) {
            /* 高于高门限, 则开启4xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_4;
        } else if (tx_throughput_mbps > (limit_throughput_middle << 1)) {
            /* 起始门限的两倍以上并且小于高门限, 则开启3xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_3;
        } else {
            /* 高于起始门限,则开启2xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_2;
        }
        /* 已经考虑对端的AMSDU聚合长度 */
        max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, max_amsdu);
    } else if (tx_throughput_mbps > limit_throughput_low) {
        /* 低门限-中门限, 若上次是AMSDU聚合的，则聚合2个; 否则不聚合 */
        if (g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id] != WLAN_TX_AMSDU_NONE) {
            max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, WLAN_TX_AMSDU_BY_2);
        } else {
            max_amsdu = WLAN_TX_AMSDU_NONE;
        }
    } else {
        /* 低于低门限, 关闭amsdu大包聚合 */
        max_amsdu = WLAN_TX_AMSDU_NONE;
    }

    return max_amsdu;
}


uint8_t hmac_update_amsdu_num_1106(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
    wlan_tx_amsdu_enum_uint8 max_amsdu;
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    hmac_vap_stru   *hmac_vap;

    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_update_amsdu_num_1106::mac_vap null!}");
        hmac_update_amsdu_num_vap_info(); // vap异常为空，打印出当前device挂接所有vap状态
        return WLAN_TX_AMSDU_BUTT;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap != NULL) {
        if (hmac_vap->en_is_hid2d_state == OAL_TRUE) {
            return WLAN_TX_AMSDU_NONE;
        }
    }
#endif
    max_amsdu = hmac_adjust_tx_amsdu_num_1106(mac_vap, performance_stat_params->tx_throughput_mbps, mu_vap_flag);

    max_amsdu = hmac_adjust_rx_amsdu_num_1106(mac_vap, performance_stat_params->rx_throughput_mbps,
        performance_stat_params->tx_pps, max_amsdu);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        max_amsdu = oal_min(oal_max(max_amsdu, mac_vap->bit_ofdma_aggr_num), WLAN_TX_AMSDU_BY_8);
    }
#endif

    return max_amsdu;
}
OAL_STATIC uint32_t hmac_get_amsdu_judge_result(uint32_t ul_ret,
    oal_bool_enum_uint8 en_mu_vap_flag, mac_vap_stru *pst_vap1, mac_vap_stru *pst_vap2)
{
    if ((ul_ret != OAL_SUCC) || (pst_vap1 == NULL) || (en_mu_vap_flag && (pst_vap2 == NULL))) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
void hmac_cfg_amsdu_info_to_dmac(mac_vap_stru *mac_vap)
{
    uint32_t ret;
    uint8_t amsdu_maxnum;

    amsdu_maxnum = (uint8_t)wlan_amsdu_level_to_num(g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id]);
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_DMAC_SYNC_AMSDU_MAXNUM,
        sizeof(uint8_t), (uint8_t *)&amsdu_maxnum);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_cfg_amsdu_info_to_dmac::hmac_config_send_event failed[%d].}", ret);
        return;
    }
}

void hmac_tx_amsdu_ampdu_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    mac_vap_stru *mac_vap[2] = {NULL}; // 2代表2个vap
    wlan_tx_amsdu_enum_uint8 tx_amsdu = 0;
    uint32_t up_ap_num = mac_device_calc_up_vap_num(mac_device);
    oal_bool_enum_uint8 mu_vap = (up_ap_num > 1);
    uint32_t ret;
    uint8_t  idx;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_performance_stat_stru performance_stat_params = {tx_throughput_mbps, rx_throughput_mbps,
        g_freq_lock_control.tx_pps};
    /* 如果定制化不支持硬件聚合 */
    if (g_st_tx_large_amsdu.uc_host_large_amsdu_en == OAL_FALSE) {
        return;
    }

    if (mu_vap) {
        ret = mac_device_find_2up_vap(mac_device, &mac_vap[0], &mac_vap[1]);
    } else {
        ret = mac_device_find_up_vap(mac_device, &mac_vap[0]);
    }

    if (hmac_get_amsdu_judge_result(ret, mu_vap, mac_vap[0], mac_vap[1]) == OAL_FAIL) {
        return;
    }

    for (idx = 0; idx < up_ap_num; idx++) {
        tx_amsdu = wlan_chip_tx_update_amsdu_num(mac_vap[idx], (hmac_performance_stat_stru *)&performance_stat_params,
            mu_vap);
        /* 聚合异常或当前聚合方式相同,不处理 */
        if (tx_amsdu == WLAN_TX_AMSDU_BUTT ||
            g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] == tx_amsdu) {
            continue;
        }

        g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] = tx_amsdu;
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap[idx]->uc_vap_id);
        if ((hmac_vap != NULL) && (hmac_vap->d2h_amsdu_switch == OAL_FALSE)) {
            g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] = WLAN_TX_AMSDU_NONE;
        }

        oam_warning_log4(mac_vap[idx]->uc_vap_id, OAM_SF_ANY,
            "{hmac_tx_amsdu_ampdu_switch: up vap num[%d], mu_vap[%d], amsdu level[%d],tx_throught= [%d]!}",
            up_ap_num, mu_vap, tx_amsdu, tx_throughput_mbps);

        hmac_cfg_amsdu_info_to_dmac(mac_vap[idx]);
    }
}
#endif
