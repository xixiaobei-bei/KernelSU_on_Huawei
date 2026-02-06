
#ifdef _PRE_WLAN_FEATURE_ROAM
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "oal_schedule.h"
#include "mac_ie.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "dmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sae.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_ALG_C

#define current_2g_target_5g(_pst_mac_vap, _pst_bss_dscr) (((_pst_mac_vap->st_channel.en_band) == WLAN_BAND_2G) && ((_pst_bss_dscr->st_channel.en_band) == WLAN_BAND_5G))
#define current_5g_target_2g(_pst_mac_vap, _pst_bss_dscr) (((_pst_mac_vap->st_channel.en_band) == WLAN_BAND_5G) && ((_pst_bss_dscr->st_channel.en_band) == WLAN_BAND_2G))

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_roam_rssi_capacity_stru g_st_rssi_table_11a_ofdm[ROAM_RSSI_LEVEL] = {
    { -75, 29300 },
    { -82, 17300 },
    { -90, 5400 }
};
hmac_roam_rssi_capacity_stru g_st_rssi_table_11b[ROAM_RSSI_LEVEL] = {
    { -88, 8100 },
    { -92, 4900 },
    { -94, 900 }

};
hmac_roam_rssi_capacity_stru g_st_rssi_table_11g_ofdm[ROAM_RSSI_LEVEL] = {
    { -75, 29300 },
    { -82, 17300 },
    { -90, 5400 }
};
hmac_roam_rssi_capacity_stru g_st_rssi_table_ht20_ofdm[ROAM_RSSI_LEVEL] = {
    { -72, 58800 },
    { -80, 35300 },
    { -90, 5800 }
};
hmac_roam_rssi_capacity_stru g_st_rssi_table_ht40_ofdm[ROAM_RSSI_LEVEL] = {
    { -75, 128100 },
    { -77, 70500 },
    { -87, 11600 }
};
hmac_roam_rssi_capacity_stru g_st_rssi_table_vht80_ofdm[ROAM_RSSI_LEVEL] = {
    { -72, 256200 },
    { -74, 141000 },
    { -84, 23200 }
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void hmac_roam_alg_init(hmac_roam_info_stru *pst_roam_info, oal_int8 c_current_rssi)
{
    hmac_roam_alg_stru *pst_roam_alg;

    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_alg_init::param null.}");
        return;
    }
    /*lint -e571*/
    OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_alg_init c_current_rssi = %d.}", c_current_rssi);
    /*lint +e571*/
    pst_roam_alg = &(pst_roam_info->st_alg);
    if (c_current_rssi == ROAM_RSSI_LINKLOSS_TYPE) {
        pst_roam_info->st_static.ul_trigger_linkloss_cnt++;
    } else {
        pst_roam_info->st_static.ul_trigger_rssi_cnt++;
    }

    pst_roam_alg->ul_max_capacity = 0;
    pst_roam_alg->pst_max_capacity_bss = OAL_PTR_NULL;
    pst_roam_alg->c_current_rssi = c_current_rssi;
    pst_roam_alg->c_max_rssi = 0;
    pst_roam_alg->uc_another_bss_scaned = 0;
    /* 首次关联时初始化 pst_roam_alg->uc_invalid_scan_cnt   = 0x0; */
    pst_roam_alg->pst_max_rssi_bss = OAL_PTR_NULL;

    return;
}


OAL_STATIC oal_int8 hmac_roam_alg_adjust_5g_rssi_weight(oal_int8 c_original_rssi)
{
    oal_int8 c_current_rssi = c_original_rssi;
    if (c_current_rssi >= ROAM_RSSI_NE65_DB) {
        c_current_rssi += ROAM_RSSI_DIFF_20_DB;
    } else if ((c_current_rssi < ROAM_RSSI_NE65_DB) && (c_current_rssi >= ROAM_RSSI_NE72_DB)) {
        c_current_rssi += ROAM_RSSI_DIFF_10_DB;
    } else {
        c_current_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    return c_current_rssi;
}


OAL_STATIC oal_int16 hmac_roam_alg_compare_rssi_increase(hmac_roam_info_stru *pst_roam_info,
                                                         mac_bss_dscr_stru *pst_bss_dscr, oal_int8 c_target_weight_rssi)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    oal_int16 s_current_rssi;
    oal_int16 s_target_rssi;
    oal_uint8 uc_delta_rssi;

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        return -1;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    s_target_rssi = pst_bss_dscr->c_rssi;
    if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        uc_delta_rssi = pst_roam_info->st_config.uc_delta_rssi_5g;
    } else {
        uc_delta_rssi = pst_roam_info->st_config.uc_delta_rssi_2g;
    }

    /* current AP is 2G, target AP is 5G */
    if (current_2g_target_5g(pst_mac_vap, pst_bss_dscr)) {
        s_target_rssi = c_target_weight_rssi;
    }
    s_current_rssi = pst_roam_info->st_alg.c_current_rssi;
    /* current AP is 5G, target AP is 2G */
    if (current_5g_target_2g(pst_mac_vap, pst_bss_dscr)) {
        s_current_rssi = (oal_int16)hmac_roam_alg_adjust_5g_rssi_weight((oal_int8)s_current_rssi);
    }

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if ((pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) && (pst_bss_dscr->c_rssi >= ROAM_RSSI_NE70_DB)) {
        return 1;
    }
#endif
    /* 指定BSSID漫游 */
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_BSSID) {
        return 1;
    }

    if (uc_delta_rssi < ROAM_RSSI_DIFF_4_DB) {
        uc_delta_rssi = ROAM_RSSI_DIFF_4_DB;
    }

    if (s_current_rssi >= ROAM_RSSI_NE70_DB) {
        return (s_target_rssi - s_current_rssi - (oal_int16)uc_delta_rssi);
    }

    if (uc_delta_rssi >= ROAM_RSSI_DIFF_4_DB + 2) { /* 步进2DB至4DB */
        uc_delta_rssi -= 2;
    }

    if (s_current_rssi >= ROAM_RSSI_NE75_DB) {
        return (s_target_rssi - s_current_rssi - (oal_int16)uc_delta_rssi);
    }

    if (uc_delta_rssi >= ROAM_RSSI_DIFF_4_DB + 2) { /* 步进2DB至4DB */
        uc_delta_rssi -= 2;
    }

    if (s_current_rssi >= ROAM_RSSI_NE80_DB) {
        return (s_target_rssi - s_current_rssi - (oal_int16)uc_delta_rssi);
    }

    return (s_target_rssi - s_current_rssi - ROAM_RSSI_DIFF_4_DB);
}


OAL_STATIC oal_uint32 hmac_roam_alg_add_bsslist(hmac_roam_bss_list_stru *pst_roam_bss_list, oal_uint8 *puc_bssid,
                                                roam_blacklist_type_enum_uint8 list_type)
{
    hmac_roam_bss_info_stru *pst_cur_bss;
    hmac_roam_bss_info_stru *pst_oldest_bss;
    hmac_roam_bss_info_stru *pst_zero_bss;
    oal_uint8 auc_mac_zero[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_uint32 ul_current_index;
    oal_uint32 ul_now;
    oal_uint32 ul_timeout;

    pst_oldest_bss = OAL_PTR_NULL;
    pst_zero_bss = OAL_PTR_NULL;
    ul_now = (oal_uint32)oal_time_get_stamp_ms();

    for (ul_current_index = 0; ul_current_index < ROAM_LIST_MAX; ul_current_index++) {
        pst_cur_bss = &pst_roam_bss_list->ast_bss[ul_current_index];
        ul_timeout = (oal_uint32)pst_cur_bss->ul_timeout;
        if (oal_compare_mac_addr(pst_cur_bss->auc_bssid, puc_bssid) == 0) {
            /* 优先查找已存在的记录，如果名单超时更新时间戳，否则更新count */
            if (oal_time_get_runtime(pst_cur_bss->ul_time_stamp, ul_now) > ul_timeout) {
                pst_cur_bss->ul_time_stamp = ul_now;
                pst_cur_bss->us_count = 1;
            } else {
                pst_cur_bss->us_count++;
                if (pst_cur_bss->us_count == pst_cur_bss->us_count_limit) {
                    pst_cur_bss->ul_time_stamp = ul_now;
                }
            }
            return OAL_SUCC;
        }

        /* 记录第一个空记录 */
        if (pst_zero_bss != OAL_PTR_NULL) {
            continue;
        }

        if (oal_compare_mac_addr(pst_cur_bss->auc_bssid, auc_mac_zero) == 0) {
            pst_zero_bss = pst_cur_bss;
            continue;
        }

        /* 记录一个非空最老记录 */
        if (pst_oldest_bss == OAL_PTR_NULL) {
            pst_oldest_bss = pst_cur_bss;
        } else {
            if (oal_time_get_runtime(pst_cur_bss->ul_time_stamp, ul_now) >
                oal_time_get_runtime(pst_oldest_bss->ul_time_stamp, ul_now)) {
                pst_oldest_bss = pst_cur_bss;
            }
        }
    }

    if (pst_zero_bss == OAL_PTR_NULL) {
        pst_zero_bss = pst_oldest_bss;
    }

    if (pst_zero_bss != OAL_PTR_NULL) {
        oal_set_mac_addr(pst_zero_bss->auc_bssid, puc_bssid);
        pst_zero_bss->ul_time_stamp = ul_now;
        pst_zero_bss->us_count = 1;
        return OAL_SUCC;
    }
    return OAL_FAIL;
}


OAL_STATIC oal_bool_enum_uint8 hmac_roam_alg_find_in_bsslist(hmac_roam_bss_list_stru *pst_roam_bss_list,
                                                             oal_uint8 *puc_bssid)
{
    hmac_roam_bss_info_stru *pst_cur_bss;
    oal_uint32 ul_current_index;
    oal_uint32 ul_now;
    oal_uint32 ul_timeout;
    oal_uint32 ul_delta_time;
    oal_uint16 us_count_limit;

    ul_now = (oal_uint32)oal_time_get_stamp_ms();

    for (ul_current_index = 0; ul_current_index < ROAM_LIST_MAX; ul_current_index++) {
        pst_cur_bss = &pst_roam_bss_list->ast_bss[ul_current_index];
        ul_timeout = pst_cur_bss->ul_timeout;
        us_count_limit = pst_cur_bss->us_count_limit;

        if (oal_compare_mac_addr(pst_cur_bss->auc_bssid, puc_bssid) == 0) {
            /* 如果在超时时间内出现count_limit次以上记录 */
            ul_delta_time = oal_time_get_runtime(pst_cur_bss->ul_time_stamp, ul_now);
            if ((ul_delta_time <= ul_timeout) &&
                (pst_cur_bss->us_count >= us_count_limit)) {
                return OAL_TRUE;
            }
            return OAL_FALSE;
        }
    }

    return OAL_FALSE;
}


oal_uint32 hmac_roam_alg_add_blacklist(hmac_roam_info_stru *pst_roam_info, oal_uint8 *puc_bssid,
                                       roam_blacklist_type_enum_uint8 list_type)
{
    oal_uint32 ul_ret;
    if ((pst_roam_info == OAL_PTR_NULL) || (puc_bssid == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_roam_alg_add_bsslist(&pst_roam_info->st_alg.st_blacklist, puc_bssid, list_type);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ROAM, "{hmac_roam_alg_add_blacklist::hmac_roam_alg_add_list failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_roam_alg_find_in_blacklist(hmac_roam_info_stru *pst_roam_info, oal_uint8 *puc_bssid)
{
    if ((pst_roam_info == OAL_PTR_NULL) || (puc_bssid == OAL_PTR_NULL)) {
        return OAL_FALSE;
    }

    return hmac_roam_alg_find_in_bsslist(&pst_roam_info->st_alg.st_blacklist, puc_bssid);
}


oal_uint32 hmac_roam_alg_add_history(hmac_roam_info_stru *pst_roam_info, oal_uint8 *puc_bssid)
{
    oal_uint32 ul_ret;

    if ((pst_roam_info == OAL_PTR_NULL) || (puc_bssid == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_roam_alg_add_bsslist(&pst_roam_info->st_alg.st_history, puc_bssid, ROAM_BLACKLIST_TYPE_NORMAL_AP);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ROAM, "{hmac_roam_alg_add_history::hmac_roam_alg_add_list failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_roam_alg_find_in_history(hmac_roam_info_stru *pst_roam_info, oal_uint8 *puc_bssid)
{
    if ((pst_roam_info == OAL_PTR_NULL) || (puc_bssid == OAL_PTR_NULL)) {
        return OAL_FALSE;
    }

    return hmac_roam_alg_find_in_bsslist(&pst_roam_info->st_alg.st_history, puc_bssid);
}
OAL_STATIC oal_void hmac_roam_alg_scan_channel_2g_init(hmac_roam_info_stru *pst_roam_info,
    mac_scan_req_stru *pst_scan_params)
{
    oal_uint8 uc_chan_idx;
    oal_uint8 uc_chan_number;
    oal_uint32 ul_ret;

    if (pst_roam_info->st_config.uc_scan_band & ROAM_BAND_2G_BIT) {
        switch (pst_roam_info->st_config.uc_scan_orthogonal) {
            case ROAM_SCAN_CHANNEL_ORG_3: {
                pst_scan_params->ast_channel_list[0].uc_chan_number = 1;
                pst_scan_params->ast_channel_list[0].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[0].uc_idx = 0;

                pst_scan_params->ast_channel_list[1].uc_chan_number = 6; /* 第1组信道列表的主20MHz信道号是6 */
                pst_scan_params->ast_channel_list[1].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[1].uc_idx = 5; /* 第1组信道列表的信道索引号是5 */

                pst_scan_params->ast_channel_list[2].uc_chan_number = 11; /* 第2组信道列表的主20MHz信道号是11 */
                pst_scan_params->ast_channel_list[2].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[2].uc_idx = 10; /* 第2组信道列表的信道索引号是10 */

                pst_scan_params->uc_channel_nums = 3; /* 信道列表中信道的个数是3 */
                pst_scan_params->uc_max_scan_count_per_channel = 1;
                break;
            }
            case ROAM_SCAN_CHANNEL_ORG_4: {
                pst_scan_params->ast_channel_list[0].uc_chan_number = 1;
                pst_scan_params->ast_channel_list[0].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[0].uc_idx = 0;

                pst_scan_params->ast_channel_list[1].uc_chan_number = 5; /* 第1组信道列表的主20MHz信道号是5 */
                pst_scan_params->ast_channel_list[1].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[1].uc_idx = 4; /* 第1组信道列表的信道索引号是4 */

                pst_scan_params->ast_channel_list[2].uc_chan_number = 7; /* 第2组信道列表的主20MHz信道号是7 */
                pst_scan_params->ast_channel_list[2].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[2].uc_idx = 6; /* 第2组信道列表的信道索引号是6 */

                pst_scan_params->ast_channel_list[3].uc_chan_number = 13; /* 第3组信道列表的主20MHz信道号是13 */
                pst_scan_params->ast_channel_list[3].en_band = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[3].uc_idx = 12; /* 第3组信道列表的信道索引号是12 */

                pst_scan_params->uc_channel_nums = 4; /* 信道列表中信道的个数是4 */
                pst_scan_params->uc_max_scan_count_per_channel = 1;
                break;
            }
            default:
            {
                for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
                    ul_ret = mac_is_channel_idx_valid(WLAN_BAND_2G, uc_chan_idx);
                    if (ul_ret != OAL_SUCC) {
                        continue;
                    }
                    mac_get_channel_num_from_idx(WLAN_BAND_2G, uc_chan_idx, &uc_chan_number);
                    pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].uc_chan_number = uc_chan_number;
                    pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].en_band = WLAN_BAND_2G;
                    pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].uc_idx = uc_chan_idx;
                    pst_scan_params->uc_channel_nums++;
                }
                break;
            }
        }
    }
}
OAL_STATIC oal_void hmac_roam_alg_scan_channel_5g_init(hmac_roam_info_stru *pst_roam_info,
    mac_scan_req_stru *pst_scan_params)
{
    oal_uint8 uc_chan_idx;
    oal_uint8 uc_chan_number;
    oal_uint32 ul_ret;

    if (pst_roam_info->st_config.uc_scan_band & ROAM_BAND_5G_BIT) {
        for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
            ul_ret = mac_is_channel_idx_valid(WLAN_BAND_5G, uc_chan_idx);
            if (ul_ret != OAL_SUCC) {
                continue;
            }
            mac_get_channel_num_from_idx(WLAN_BAND_5G, uc_chan_idx, &uc_chan_number);
            pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].uc_chan_number = uc_chan_number;
            pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].en_band = WLAN_BAND_5G;
            pst_scan_params->ast_channel_list[pst_scan_params->uc_channel_nums].uc_idx = uc_chan_idx;
            pst_scan_params->uc_channel_nums++;
        }
    }
}

oal_uint32 hmac_roam_alg_scan_channel_init(hmac_roam_info_stru *pst_roam_info, mac_scan_req_stru *pst_scan_params)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_channel_stru *pst_channel;
    oal_uint32 ul_ret;

    if ((pst_roam_info == OAL_PTR_NULL) || (pst_scan_params == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_alg_scan_channel_init::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_scan_params->uc_channel_nums = 0;
    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "hmac_roam_alg_scan_channel_init:pst_hmac_vap is null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_channel = &(pst_hmac_vap->st_vap_base_info.st_channel);

    if (pst_roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
        oam_info_log3(0, OAM_SF_ROAM,
                      "{hmac_roam_alg_scan_channel_init::scan one channel: ChanNum[%d], Band[%d], ChanIdx[%d]}",
                      pst_channel->uc_chan_number, pst_channel->en_band, pst_channel->uc_idx);
        if (pst_roam_info->en_roam_trigger != ROAM_TRIGGER_11V) {
            pst_scan_params->ast_channel_list[0].uc_chan_number = pst_channel->uc_chan_number;
            pst_scan_params->ast_channel_list[0].en_band = pst_channel->en_band;
            pst_scan_params->ast_channel_list[0].uc_idx = pst_channel->uc_idx;
        }
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        /* 支持11v协议时，在备选列表所在信道扫描 */
        else {
            pst_scan_params->ast_channel_list[0].uc_chan_number = pst_roam_info->st_bsst_rsp_info.uc_chl_num;
            pst_scan_params->ast_channel_list[0].en_band =
                mac_get_band_by_channel_num(pst_roam_info->st_bsst_rsp_info.uc_chl_num);
            ul_ret = mac_get_channel_idx_from_num(pst_scan_params->ast_channel_list[0].en_band,
                                                  pst_scan_params->ast_channel_list[0].uc_chan_number,
                                                  &(pst_scan_params->ast_channel_list[0].uc_idx));
            /* 维测 */
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_roam_alg_scan_channel_init::get_channel_idx_from_num fail=%d",
                                 ul_ret);
            }
        }
#endif
        pst_scan_params->uc_channel_nums = 1;
        pst_scan_params->uc_max_scan_count_per_channel = 1;
        return OAL_SUCC;
    }
    hmac_roam_alg_scan_channel_2g_init(pst_roam_info, pst_scan_params);
    hmac_roam_alg_scan_channel_5g_init(pst_roam_info, pst_scan_params);
    pst_scan_params->uc_max_scan_count_per_channel = 2;  /* 每个信道的扫描次数是2 */

    return OAL_SUCC;
}

oal_uint32 hmac_roam_alg_check_bss_is_valid(hmac_roam_info_stru *pst_roam_info, mac_bss_dscr_stru *pst_bss_dscr)
{
    oal_int8 c_tmp_rssi;
    oal_int16 s_delta_rssi;

    c_tmp_rssi = pst_bss_dscr->c_rssi;

    if (hmac_roam_check_cipher_limit(pst_roam_info, pst_bss_dscr) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM, "hmac_roam_alg_check_bss_is_valid:0x%02x:0x%02x cipher limit, ignore roaming",
                         pst_bss_dscr->auc_bssid[4], pst_bss_dscr->auc_bssid[5]); /* auc_bssid第4、5byte为参数输出打印 */
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 终端评审:AP信号很弱时，忽略漫游。避免漫游时间太长 */
    /* 2.4G候选AP小于-80db不再漫游，5G候选AP小于-78dB不再漫游 */
    if (((c_tmp_rssi < ROAM_RSSI_NE80_DB) && (pst_bss_dscr->st_channel.en_band == WLAN_BAND_2G)) ||
        ((c_tmp_rssi < ROAM_RSSI_NE78_DB) && (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G))) {
        /*lint -e571*/
        oam_warning_log3(0, OAM_SF_ROAM,
                         "hmac_roam_alg_check_bss_is_valid:candidate 0x%02x:0x%02x rssi=%d, ignore roaming.}",
                         /* auc_bssid第4、5byte为参数输出打印 */
                         pst_bss_dscr->auc_bssid[4], pst_bss_dscr->auc_bssid[5], c_tmp_rssi);
        /*lint +e571*/
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        c_tmp_rssi = hmac_roam_alg_adjust_5g_rssi_weight(c_tmp_rssi);
    }

    /* c_current_rssi为0时，表示linkloss上报的触发，不需要考虑rssi增益 */
    s_delta_rssi = hmac_roam_alg_compare_rssi_increase(pst_roam_info, pst_bss_dscr, c_tmp_rssi);
    if (s_delta_rssi <= 0) {
        /*lint -e571*/
        oam_warning_log3(0, OAM_SF_ROAM, "hmac_roam_alg_check_bss_is_valid:0x%02x:0x%02xdelta rssi[%d], ignore roaming",
                         /* auc_bssid第4、5byte为参数输出打印 */
                         pst_bss_dscr->auc_bssid[4], pst_bss_dscr->auc_bssid[5], s_delta_rssi);
        /*lint +e571*/
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_roam_alg_bss_in_ess(hmac_roam_info_stru *pst_roam_info, mac_bss_dscr_stru *pst_bss_dscr)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    hmac_roam_alg_stru *pst_roam_alg;
    mac_cfg_ssid_param_stru st_cfg_ssid;
    oal_uint8 uc_stru_len;
    oal_uint32 ul_ret;

    if ((pst_roam_info == OAL_PTR_NULL) || (pst_bss_dscr == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    mac_mib_get_ssid (pst_mac_vap, &uc_stru_len, (oal_uint8 *)(&st_cfg_ssid));

    if ((OAL_STRLEN(pst_bss_dscr->ac_ssid) != st_cfg_ssid.uc_ssid_len) ||
        (oal_memcmp(st_cfg_ssid.ac_ssid, pst_bss_dscr->ac_ssid, st_cfg_ssid.uc_ssid_len) != 0)) {
        return OAL_SUCC;
    }
    pst_roam_alg = &(pst_roam_info->st_alg);

    /* 是否扫描到了当前关联的 bss, 仅置位，不过滤 */
    if (oal_compare_mac_addr(pst_mac_vap->auc_bssid, pst_bss_dscr->auc_bssid) != 0) {
        pst_roam_alg->uc_another_bss_scaned = 1;

        if (pst_roam_info->uc_rssi_ignore == OAL_TRUE) {
            ul_ret = hmac_roam_alg_check_bss_is_valid(pst_roam_info, pst_bss_dscr);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess::no bss valid, ignore roaming.}");
            } else {
                hmac_roam_ignore_rssi_trigger(pst_hmac_vap, OAL_FALSE);
            }
        } else {
            hmac_roam_ignore_rssi_trigger(pst_hmac_vap, OAL_FALSE);
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名 : hmac_roam_check_cipher_limit
 * 功能描述 : 检测目标BSS加密套件是否支持漫游
 */
oal_uint32 hmac_roam_check_cipher_limit(hmac_roam_info_stru *pst_roam_info, mac_bss_dscr_stru *pst_bss_dscr)
{
    mac_vap_stru *pst_mac_vap;
    mac_cap_info_stru *pst_cap_info;
#if defined(_PRE_WLAN_FEATURE_SAE) && defined(_PRE_WLAN_FEATURE_11R)
    oal_uint8 *puc_mde = OAL_PTR_NULL;
#endif

    pst_mac_vap = &pst_roam_info->pst_hmac_vap->st_vap_base_info;
    pst_cap_info = (mac_cap_info_stru *)&pst_bss_dscr->us_cap_info;

    /* wep的bss直接过滤掉 */
    pst_cap_info = (mac_cap_info_stru *)&pst_bss_dscr->us_cap_info;
    if ((pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode == 0) &&
        (pst_cap_info->bit_privacy != 0)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* open加密方式到wpa/wpa2直接过滤掉 */
    if ((pst_cap_info->bit_privacy == 0) != (mac_mib_get_privacyinvoked(pst_mac_vap) != OAL_TRUE)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    /* 不支持SAE与其他任何非SAE认证方式的漫游 */
    if (hmac_check_illegal_sae_roam(pst_mac_vap, pst_bss_dscr->st_bss_sec_info.auc_rsn_auth_policy)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_check_cipher_limit::Roaming between SAE-only AP and non-SAE AP is not allowed}");

        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

#ifdef _PRE_WLAN_FEATURE_11R
    /* SAE不支持11R漫游 */
    if ((pst_roam_info->pst_hmac_vap->en_auth_mode == WLAN_WITP_AUTH_SAE) &&
        (pst_bss_dscr->ul_mgmt_len > (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET))) {
        puc_mde = mac_find_ie(MAC_EID_MOBILITY_DOMAIN,
                              pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET, /*lint !e416*/
                              pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET);
        if (puc_mde) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM,
                             "hmac_roam_check_cipher_limit::SAE do not support 11r roam");

            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
#endif
#endif
    return OAL_SUCC;
}

oal_uint32 hmac_roam_alg_check_bssid_limit(hmac_roam_info_stru *pst_roam_info, oal_uint8 *puc_mac_addr)
{
    oal_uint32 ul_ret;

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        if (oal_memcmp(pst_roam_info->st_bsst_rsp_info.auc_target_bss_addr,
                       puc_mac_addr,
                       WLAN_MAC_ADDR_LEN)) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */

    /* 检查黑名单 */
    ul_ret = hmac_roam_alg_find_in_blacklist(pst_roam_info, puc_mac_addr);
    if (ul_ret == OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ROAM, "{hmac_roam_alg_check_bssid_limit:: [%02X:XX:XX:XX:%02X:%02X] in blacklist!}",
                         puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]); /* puc_mac_addr第0、4、5byte为参数输出打印 */
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_roam_alg_bss_check(hmac_roam_info_stru *pst_roam_info, mac_bss_dscr_stru *pst_bss_dscr)
{
    hmac_vap_stru              *pst_hmac_vap;
    mac_vap_stru               *pst_mac_vap;
    hmac_roam_alg_stru         *pst_roam_alg;
    oal_uint8                  *puc_pmkid;
    mac_cfg_ssid_param_stru     st_cfg_ssid;
    oal_uint32                  ul_ret;
    oal_uint8                   uc_stru_len;
    oal_int8                    c_tmp_rssi;

    if (oal_any_null_ptr3(pst_roam_info, pst_bss_dscr, pst_roam_info->pst_hmac_vap)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_check::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    mac_mib_get_ssid (pst_mac_vap, &uc_stru_len, (oal_uint8 *)(&st_cfg_ssid));

    if ((OAL_STRLEN(pst_bss_dscr->ac_ssid) != st_cfg_ssid.uc_ssid_len) ||
        (oal_memcmp(st_cfg_ssid.ac_ssid, pst_bss_dscr->ac_ssid, st_cfg_ssid.uc_ssid_len) != 0)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }
    pst_roam_alg = &(pst_roam_info->st_alg);

    /* 检查漫游到自己:未扫描到自己，返回失败 */
    if ((pst_roam_info->en_current_bss_ignore == OAL_FALSE)
        && oal_memcmp(pst_mac_vap->auc_bssid, pst_bss_dscr->auc_bssid, OAL_MAC_ADDR_LEN)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 检查特定BSSID的漫游 */
    if ((oal_memcmp(pst_roam_info->auc_target_bssid, pst_bss_dscr->auc_bssid, OAL_MAC_ADDR_LEN) != 0) &&
        (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_BSSID)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    if (pst_roam_info->en_current_bss_ignore == OAL_TRUE) {
        /* 非voe认证挑选HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE时间以内的有效bss */
        /* 11R时，漫游命令不扫描时，bss上报到wpas和漫游bss选择都会只选择当前时间之前最近5s内的，该逻辑会导致漫游时找不到可用的bss，最终漫游失败 */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        if (pst_hmac_vap->bit_voe_11r_auth == 0)
#endif
        {
            if (oal_time_after32(oal_time_get_stamp_ms(),
                                 (pst_bss_dscr->ul_timestamp + HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE))) {
                oam_warning_log2(0, OAM_SF_ROAM,
                                 "{hmac_roam_alg_bss_check::Over the expire time!ul_timestamp[%d],now time[%d].}",
                                 pst_bss_dscr->ul_timestamp, oal_time_get_stamp_ms());
                return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            }
        }
    }
    ul_ret = hmac_roam_alg_check_bssid_limit(pst_roam_info, pst_bss_dscr->auc_bssid);
    if (ul_ret != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 检查漫游到其它BSSID */
    if (pst_roam_info->en_current_bss_ignore == OAL_TRUE) {
        /* 排除当前bss的rssi值计算，本地已经保存了dmac上报的rssi */
        if (oal_compare_mac_addr(pst_mac_vap->auc_bssid, pst_bss_dscr->auc_bssid) == 0) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    ul_ret = hmac_roam_alg_check_bss_is_valid(pst_roam_info, pst_bss_dscr);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_check::no bss valid, ignore roaming.}");
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    c_tmp_rssi = pst_bss_dscr->c_rssi;
    if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        c_tmp_rssi = hmac_roam_alg_adjust_5g_rssi_weight(c_tmp_rssi);
    }
    /* 对于已存在pmk缓存的bss进行加分处理 */
    puc_pmkid = hmac_vap_get_pmksa(pst_hmac_vap, pst_bss_dscr->auc_bssid);
    if (puc_pmkid != OAL_PTR_NULL) {
        c_tmp_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    if ((pst_roam_alg->pst_max_rssi_bss == OAL_PTR_NULL) ||
        (c_tmp_rssi > pst_roam_alg->c_max_rssi)) {
        pst_roam_alg->c_max_rssi = c_tmp_rssi;
        pst_roam_alg->pst_max_rssi_bss = pst_bss_dscr;
    }

    /* 当2G AP RSSI和5G AP加权RSSI相等时，优选5G低band AP */
    if ((c_tmp_rssi == pst_roam_alg->c_max_rssi) &&
        (pst_roam_alg->pst_max_rssi_bss->st_channel.en_band == WLAN_BAND_2G) &&
        (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G)) {
        pst_roam_alg->c_max_rssi = c_tmp_rssi;
        pst_roam_alg->pst_max_rssi_bss = pst_bss_dscr;
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_roam_alg_need_to_stop_roam_trigger(hmac_roam_info_stru *pst_roam_info)
{
    hmac_vap_stru *pst_hmac_vap;
    hmac_roam_alg_stru *pst_roam_alg;

    if (pst_roam_info == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    pst_roam_alg = &(pst_roam_info->st_alg);

    if (pst_roam_alg->uc_another_bss_scaned) {
        pst_roam_alg->uc_invalid_scan_cnt = 0xff;
        return OAL_FALSE;
    }

    if (pst_roam_alg->uc_invalid_scan_cnt == 0xff) {
        return OAL_FALSE;
    }

    if (pst_roam_alg->uc_invalid_scan_cnt++ > 4) { /* 连续扫描到当前关联且首次关联的 bss 的次数是否大于4 */
        pst_roam_alg->uc_invalid_scan_cnt = 0xff;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


mac_bss_dscr_stru *hmac_roam_alg_select_bss(hmac_roam_info_stru *pst_roam_info)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    mac_bss_dscr_stru *pst_bss_dscr;
    hmac_roam_alg_stru *pst_roam_alg;
    oal_int16 s_delta_rssi;

    if (pst_roam_info == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    pst_roam_alg = &(pst_roam_info->st_alg);

    /* 取得最大 rssi 的 bss */
    pst_bss_dscr = pst_roam_alg->pst_max_rssi_bss;

    if ((pst_roam_alg->pst_max_capacity_bss != OAL_PTR_NULL) &&
        (pst_roam_alg->ul_max_capacity >= ROAM_THROUGHPUT_THRESHOLD)) {
        /* 取得最大 capacity 的 bss */
        pst_bss_dscr = pst_roam_alg->pst_max_capacity_bss;
    }

    if (pst_bss_dscr == OAL_PTR_NULL) {
        /* should not be here */
        return OAL_PTR_NULL;
    }

    /* rssi增益处理 */
    s_delta_rssi = hmac_roam_alg_compare_rssi_increase(pst_roam_info, pst_bss_dscr, pst_roam_alg->c_max_rssi);
    if (s_delta_rssi <= 0) {
        return OAL_PTR_NULL;
    }

    /*lint -e571*/
    if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        oam_warning_log4(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_select_bss::roam_to 5G candidate AP, max_rssi=%d, [%02X:XX:XX:XX:%02X:%02X]}",
                         pst_roam_alg->c_max_rssi, pst_bss_dscr->auc_bssid[0],
                         pst_bss_dscr->auc_bssid[4], pst_bss_dscr->auc_bssid[5]); /* auc_bssid第4、5byte为参数输出打印 */
    } else {
        oam_warning_log4(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_select_bss::roam_to candidate AP, max_rssi=%d, [%02X:XX:XX:XX:%02X:%02X]}",
                         pst_roam_alg->c_max_rssi, pst_bss_dscr->auc_bssid[0],
                         pst_bss_dscr->auc_bssid[4], pst_bss_dscr->auc_bssid[5]); /* auc_bssid第4、5byte为参数输出打印 */
    }
    /*lint +e571*/
    return pst_bss_dscr;
}

#endif  // _PRE_WLAN_FEATURE_ROAM


