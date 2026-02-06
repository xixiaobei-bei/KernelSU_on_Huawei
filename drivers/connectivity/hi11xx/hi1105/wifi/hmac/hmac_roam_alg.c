

#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_device.h"
#include "mac_resource.h"

#include "hmac_dfx.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blacklist.h"
#include "mac_mib.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_ALG_C

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

void hmac_roam_alg_init(hmac_roam_info_stru *roam_info, int8_t c_current_rssi)
{
    hmac_roam_alg_stru *roam_alg = NULL;

    if (roam_info == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_init::param null.}");
        return;
    }
    oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_init c_current_rssi = %d.}", c_current_rssi);
    roam_alg = &(roam_info->st_alg);
    if (c_current_rssi == ROAM_RSSI_LINKLOSS_TYPE) {
        roam_info->st_static.trigger_linkloss_cnt++;
    } else {
        roam_info->st_static.trigger_rssi_cnt++;
    }

    roam_alg->max_capacity = 0;
    roam_alg->pst_max_capacity_bss = NULL;
    roam_alg->c_current_rssi = c_current_rssi;
    roam_alg->c_max_rssi = 0;
    roam_alg->uc_another_bss_scaned = 0;
    /* 首次关联时初始化 roam_alg->uc_invalid_scan_cnt   = 0x0; */
    roam_alg->pst_max_rssi_bss = NULL;
#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        if (mac_is_ftm_enable(&roam_info->pst_hmac_vap->st_vap_base_info)) {
            roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_1;
        }
    }
#endif

    return;
}


void hmac_roam_alg_init_rssi(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    if (oal_unlikely(hmac_vap == NULL)) {
        return;
    }

    if (oal_unlikely(roam_info == NULL)) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        hmac_roam_alg_init(roam_info, roam_info->st_bsst_rsp_info.c_rssi);
    } else
#endif
        if (roam_info->en_roam_trigger == ROAM_TRIGGER_HOME_NETWORK) {
            hmac_roam_alg_init(roam_info, hmac_vap->station_info.signal);
        } else {
            hmac_roam_alg_init(roam_info, ROAM_RSSI_CMD_TYPE);
        }

    return;
}


OAL_STATIC int8_t hmac_roam_alg_adjust_5g_rssi_weight(int8_t orginal_rssi)
{
    int8_t current_rssi = orginal_rssi;

    if (current_rssi >= ROAM_RSSI_NE65_DB) {
        current_rssi += ROAM_RSSI_DIFF_20_DB;
    } else if (current_rssi < ROAM_RSSI_NE65_DB && current_rssi >= ROAM_RSSI_NE72_DB) {
        current_rssi += ROAM_RSSI_DIFF_10_DB;
    } else {
        current_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    return current_rssi;
}


OAL_STATIC int8_t hmac_roam_alg_adjust_ax_rssi_weight(int8_t orginal_rssi)
{
    int8_t current_rssi = orginal_rssi;

    if (current_rssi >= ROAM_RSSI_NE65_DB) {
        current_rssi += ROAM_RSSI_DIFF_6_DB;
    } else if (current_rssi < ROAM_RSSI_NE65_DB && current_rssi > ROAM_RSSI_NE75_DB) {
        current_rssi += ROAM_RSSI_DIFF_2_DB;
    }

    return current_rssi;
}

OAL_STATIC int8_t hmac_roam_alg_adjust_sap_rssi_weight(int8_t orginal_rssi)
{
    int8_t current_rssi = orginal_rssi;

    if (current_rssi >= ROAM_RSSI_NE65_DB) {
        current_rssi += ROAM_RSSI_DIFF_8_DB;
    } else if (current_rssi < ROAM_RSSI_NE65_DB && current_rssi >= ROAM_RSSI_NE72_DB) {
        current_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    return current_rssi;
}


OAL_STATIC int16_t hmac_roam_alg_compare_rssi_increase(hmac_roam_info_stru *roam_info,
    mac_bss_dscr_stru *bss_dscr, int8_t c_target_weight_rssi)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    int16_t s_current_rssi;
    int16_t s_target_rssi;
    uint8_t delta_rssi;

    hmac_vap = roam_info->pst_hmac_vap;
    if (oal_unlikely(hmac_vap == NULL)) {
        return -1;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    s_target_rssi = bss_dscr->c_rssi;
    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        delta_rssi = roam_info->st_config.uc_delta_rssi_5g;
    } else {
        delta_rssi = roam_info->st_config.uc_delta_rssi_2g;
    }

    if ((bss_dscr->st_channel.en_band == WLAN_BAND_5G) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G))  { // current AP is 2G, target AP is 5G
        s_target_rssi = c_target_weight_rssi;
    }

    s_current_rssi = roam_info->st_alg.c_current_rssi;
    if ((mac_vap->st_channel.en_band == WLAN_BAND_5G) &&
        (bss_dscr->st_channel.en_band == WLAN_BAND_2G)) {  // current AP is 5G, target AP is 2G
        s_current_rssi = (int16_t)hmac_roam_alg_adjust_5g_rssi_weight((int8_t)s_current_rssi);
    }

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V &&
        (bss_dscr->c_rssi >= ROAM_RSSI_NE70_DB)) {
        return 1;
    }
#endif

    if (roam_info->en_roam_trigger == ROAM_TRIGGER_BSSID) {
        return 1;
    }

    if (delta_rssi < ROAM_RSSI_DIFF_4_DB) {
        delta_rssi = ROAM_RSSI_DIFF_4_DB;
    }

    if (s_current_rssi >= ROAM_RSSI_NE70_DB) {
        return (s_target_rssi - s_current_rssi - (int16_t)delta_rssi);
    }

    if (delta_rssi >= ROAM_RSSI_DIFF_6_DB) {
        /* 步进2DB至4DB */
        delta_rssi -= 2;
    }

    if (s_current_rssi >= ROAM_RSSI_NE75_DB) {
        return (s_target_rssi - s_current_rssi - (int16_t)delta_rssi);
    }

    if (delta_rssi >= ROAM_RSSI_DIFF_6_DB) {
        /* 步进2DB至4DB */
        delta_rssi -= 2;
    }

    if (s_current_rssi >= ROAM_RSSI_NE80_DB) {
        return (s_target_rssi - s_current_rssi - (int16_t)delta_rssi);
    }

    return (s_target_rssi - s_current_rssi - ROAM_RSSI_DIFF_4_DB);
}


OAL_STATIC uint32_t hmac_roam_alg_add_bsslist(hmac_roam_bss_list_stru *roam_bss_list,
                                              uint8_t *puc_bssid, roam_blacklist_type_enum list_type)
{
    hmac_roam_bss_info_stru *cur_bss = NULL;
    hmac_roam_bss_info_stru *oldest_bss = NULL;
    hmac_roam_bss_info_stru *zero_bss = NULL;
    uint8_t auc_mac_zero[WLAN_MAC_ADDR_LEN] = { 0 };
    uint32_t current_index, now, timeout;

    now = (uint32_t)oal_time_get_stamp_ms();

    for (current_index = 0; current_index < ROAM_LIST_MAX; current_index++) {
        cur_bss = &roam_bss_list->ast_bss[current_index];
        timeout = (uint32_t)cur_bss->timeout;
        if (oal_compare_mac_addr(cur_bss->auc_bssid, puc_bssid) == 0) {
            /* 优先查找已存在的记录，如果名单超时更新时间戳，否则更新count */
            if (oal_time_get_runtime(cur_bss->time_stamp, now) > timeout) {
                cur_bss->time_stamp = now;
                cur_bss->us_count = 1;
                return OAL_SUCC;
            }
            cur_bss->us_count++;
            if (cur_bss->us_count == cur_bss->us_count_limit) {
                cur_bss->time_stamp = now;
            }
            return OAL_SUCC;
        }

        /* 记录第一个空记录 */
        if (zero_bss != NULL) {
            continue;
        }

        if (oal_compare_mac_addr(cur_bss->auc_bssid, auc_mac_zero) == 0) {
            zero_bss = cur_bss;
            continue;
        }

        /* 记录一个非空最老记录 */
        if (oldest_bss == NULL) {
            oldest_bss = cur_bss;
        } else if (oal_time_get_runtime(cur_bss->time_stamp, now) >
            oal_time_get_runtime(oldest_bss->time_stamp, now)) {
            oldest_bss = cur_bss;
        }
    }

    if (zero_bss == NULL) {
        zero_bss = oldest_bss;
    }

    if (zero_bss != NULL) {
        oal_set_mac_addr(zero_bss->auc_bssid, puc_bssid);
        zero_bss->time_stamp = now;
        zero_bss->us_count = 1;
        return OAL_SUCC;
    }
    return OAL_FAIL;
}


OAL_STATIC oal_bool_enum_uint8 hmac_roam_alg_find_in_bsslist(hmac_roam_bss_list_stru *roam_bss_list,
                                                             uint8_t *puc_bssid)
{
    hmac_roam_bss_info_stru *cur_bss = NULL;
    uint32_t current_index;
    uint32_t now;
    uint32_t timeout;
    uint32_t delta_time;
    uint16_t us_count_limit;

    now = (uint32_t)oal_time_get_stamp_ms();

    for (current_index = 0; current_index < ROAM_LIST_MAX; current_index++) {
        cur_bss = &roam_bss_list->ast_bss[current_index];
        timeout = cur_bss->timeout;
        us_count_limit = cur_bss->us_count_limit;

        if (oal_compare_mac_addr(cur_bss->auc_bssid, puc_bssid) == 0) {
            /* 如果在超时时间内出现count_limit次以上记录 */
            delta_time = oal_time_get_runtime(cur_bss->time_stamp, now);
            if ((delta_time <= timeout) &&
                (cur_bss->us_count >= us_count_limit)) {
                return OAL_TRUE;
            }
            return OAL_FALSE;
        }
    }

    return OAL_FALSE;
}


uint32_t hmac_roam_alg_add_blacklist(hmac_roam_info_stru *roam_info,
                                     uint8_t *puc_bssid,
                                     roam_blacklist_type_enum list_type)
{
    uint32_t ret;
    if (oal_any_null_ptr2(roam_info, puc_bssid)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_roam_alg_add_bsslist(&roam_info->st_alg.st_blacklist, puc_bssid, list_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
                       "{hmac_roam_alg_add_blacklist::hmac_roam_alg_add_list failed[%d].}",
                       ret);
        return ret;
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_roam_alg_find_in_blacklist(hmac_roam_info_stru *roam_info, uint8_t *puc_bssid)
{
    if (oal_any_null_ptr2(roam_info, puc_bssid)) {
        return OAL_FALSE;
    }

    return hmac_roam_alg_find_in_bsslist(&roam_info->st_alg.st_blacklist, puc_bssid);
}


uint32_t hmac_roam_alg_add_history(hmac_roam_info_stru *roam_info, uint8_t *puc_bssid)
{
    uint32_t ret;

    if (oal_any_null_ptr2(roam_info, puc_bssid)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_roam_alg_add_bsslist(&roam_info->st_alg.st_history,
                                    puc_bssid, ROAM_BLACKLIST_TYPE_NORMAL_AP);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
                       "{hmac_roam_alg_add_history::hmac_roam_alg_add_list failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_roam_alg_find_in_history(hmac_roam_info_stru *roam_info, uint8_t *puc_bssid)
{
    if (oal_any_null_ptr2(roam_info, puc_bssid)) {
        return OAL_FALSE;
    }

    return hmac_roam_alg_find_in_bsslist(&roam_info->st_alg.st_history, puc_bssid);
}

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)

void hmac_roam_neighbor_report_add_chan(mac_scan_req_stru *scan_params,
    uint8_t channum, wlan_channel_band_enum_uint8 en_band, uint8_t channel_idx)
{
    uint8_t   loop;

    if (scan_params == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_neighbor_report_add_chan::scan_params is NULL}");
        return;
    }

    if (scan_params->uc_channel_nums >= ROAM_NEIGHBOR_RPT_LIST_CHN_MAX_NUM) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_neighbor_report_add_chan::exceed max neighbor list}");
        return;
    }

    /* 找到第一个比channum大的scan_params->ast_channel_list索引 */
    for (loop = 0; loop < scan_params->uc_channel_nums; loop++) {
        if (scan_params->ast_channel_list[loop].uc_chan_number == channum) {
            return;
        } else if (scan_params->ast_channel_list[loop].uc_chan_number > channum) {
            memmove_s(&(scan_params->ast_channel_list[loop + 1]),
                sizeof(mac_channel_stru) * (scan_params->uc_channel_nums - loop),
                &(scan_params->ast_channel_list[loop]),
                sizeof(mac_channel_stru) * (scan_params->uc_channel_nums - loop));
            break;
        }
    }

    scan_params->ast_channel_list[loop].uc_chan_number  = channum;
    scan_params->ast_channel_list[loop].en_band         = en_band;
    scan_params->ast_channel_list[loop].uc_chan_idx     = channel_idx;
    scan_params->uc_channel_nums++;
}


void hmac_roam_neighbor_report_add_bssid(hmac_roam_info_stru *roam_info, uint8_t *puc_bssid)
{
    uint8_t *puc_idex = NULL;

    if (roam_info == NULL || puc_bssid == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_neighbor_report_add_bssid::NULL pointer}");
        return;
    }

    puc_idex = &roam_info->uc_neighbor_rpt_bssid_num;
    if (*puc_idex >= ROAM_NEIGHBOR_RPT_LIST_BSSID_MAX_NUM) {
        return;
    }

    if (memcpy_s(roam_info->auc_neighbor_rpt_bssid[*puc_idex], WLAN_MAC_ADDR_LEN,
                 puc_bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_roam_neighbor_report_add_bssid:memcopy fail!");
        return;
    }
    (*puc_idex)++;
    return;
}
#endif

OAL_STATIC uint32_t hmac_roam_alg_get_capacity_by_rssi(wlan_protocol_enum_uint8 en_protocol,
                                                       wlan_bw_cap_enum_uint8 en_bw_cap, int8_t c_rssi)
{
    hmac_roam_rssi_capacity_stru *rssi_table = NULL;
    uint8_t index;

    switch (en_protocol) {
        case WLAN_LEGACY_11A_MODE:
            rssi_table = g_st_rssi_table_11a_ofdm;
            break;

        case WLAN_LEGACY_11B_MODE:
            rssi_table = g_st_rssi_table_11b;
            break;

        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            rssi_table = g_st_rssi_table_11g_ofdm;
            break;

        case WLAN_HT_MODE:
        case WLAN_VHT_MODE:
            if (en_bw_cap == WLAN_BW_CAP_20M) {
                rssi_table = g_st_rssi_table_ht20_ofdm;
            } else if (en_bw_cap == WLAN_BW_CAP_40M) {
                rssi_table = g_st_rssi_table_ht40_ofdm;
            } else {
                rssi_table = g_st_rssi_table_vht80_ofdm;
            }
            break;

        default:
            break;
    }

    if (rssi_table == NULL) {
        return 0;
    }

    for (index = 0; index < ROAM_RSSI_LEVEL; index++) {
        if (c_rssi >= rssi_table[index].c_rssi) {
            return rssi_table[index].capacity_kbps;
        }
    }

    return 0;
}

OAL_STATIC uint32_t hmac_roam_alg_calc_avail_channel_capacity(mac_bss_dscr_stru *bss_dscr)
{
    uint32_t capacity;
    uint32_t avail_channel_capacity;
    uint32_t ret;
    uint8_t channel_utilization = 0;
    uint8_t *puc_obss_ie = NULL;
    uint8_t ie_offset;
    wlan_protocol_enum_uint8 en_protocol = 0;

    ret = hmac_sta_get_user_protocol_etc(bss_dscr, &en_protocol);
    if (ret != OAL_SUCC) {
        return 0;
    }
    /***************************************************************************
        ------------------------------------------------------------------------
        |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
        ------------------------------------------------------------------------
        |1   |1   |2            |1                  |2                         |
        ------------------------------------------------------------------------
        ***************************************************************************/
    ie_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    puc_obss_ie = mac_find_ie(MAC_EID_QBSS_LOAD, (uint8_t *)(bss_dscr->auc_mgmt_buff + ie_offset),
                              (int32_t)(bss_dscr->mgmt_len - ie_offset));
    /* 长度要到达ChannelUtilization这个域，至少为3 */
    if (puc_obss_ie && (puc_obss_ie[1] >= 3)) {
        channel_utilization = *(puc_obss_ie + BYTE_OFFSET_4);
    }

    capacity = hmac_roam_alg_get_capacity_by_rssi(en_protocol, bss_dscr->en_bw_cap, bss_dscr->c_rssi);
    /* 255用于计算avail_channel_capacity值 */
    avail_channel_capacity = capacity * (255 - channel_utilization) / 255 / ROAM_CONCURRENT_USER_NUMBER;

    return avail_channel_capacity;
}


OAL_STATIC void hmac_roam_alg_record_bss_info(hmac_roam_record_stru *roam_record,
                                              mac_bss_dscr_stru *bss_dscr)
{
    mac_channel_stru *channel = NULL;

    if (oal_any_null_ptr2(roam_record, bss_dscr)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_record_bss_info::param null.}");
        return;
    }

    if (roam_record->uc_bssid_num >= MAX_RECORD_BSSID_NUM) {
        return;
    }

    /* update channel info if bssid is already recorded */
    if (roam_record->uc_bssid_num == 1 &&
        oal_memcmp(roam_record->auc_bssid[0], bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN) == 0) {
        if (bss_dscr->st_channel.uc_chan_number != roam_record->st_channel[0].uc_chan_number) {
            roam_record->st_channel[0] = bss_dscr->st_channel;
        }
        return;
    }

    memcpy_s(roam_record->auc_bssid[roam_record->uc_bssid_num], WLAN_MAC_ADDR_LEN,
             bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN);

    channel = &(roam_record->st_channel[roam_record->uc_bssid_num]);
    memcpy_s(channel, sizeof(mac_channel_stru), &(bss_dscr->st_channel), sizeof(mac_channel_stru));

    roam_record->uc_bssid_num++;

    return;
}

OAL_STATIC void hmac_roam_record_chan_info(hmac_roam_record_chan_stru *chan_info, mac_bss_dscr_stru *bss_dscr)
{
    int8_t chan_idx;
    int8_t chan_next;
    uint8_t stored = 0;

    for (chan_idx = 0; chan_idx < MAX_RECORD_CHAN_NUM; chan_idx++) {
        if (bss_dscr->st_channel.uc_chan_number == chan_info->channel[chan_idx].uc_chan_number) {
            stored = 1;
            break;
        } else if (bss_dscr->st_channel.uc_chan_number < chan_info->channel[chan_idx].uc_chan_number) {
            for (chan_next = MAX_RECORD_CHAN_NUM - 2; chan_next >= chan_idx; chan_next--) { /* 扫描信道信息减2 */
                chan_info->channel[chan_next + 1] = chan_info->channel[chan_next]; // 从小到大排序，插入排序
            }
            memset_s(&(chan_info->channel[chan_idx]), sizeof(mac_channel_stru), 0, sizeof(mac_channel_stru));
        }

        if (chan_info->channel[chan_idx].uc_chan_number == 0) {
            chan_info->channel[chan_idx] = bss_dscr->st_channel;
            oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_record_chan_info::idx=%d, ChanNum=%d mac:%02x::%02x}",
                chan_info->chan_num, chan_info->channel[chan_idx].uc_chan_number,
                bss_dscr->auc_bssid[BYTE_OFFSET_4], bss_dscr->auc_bssid[BYTE_OFFSET_5]);
            chan_info->chan_num++;
            stored = 1;
            break;
        }
    }

    if (stored == 0 && chan_info->chan_num == MAX_RECORD_CHAN_NUM) {
        chan_info->chan_num++; // 标记超过3 个信道
    }
}

OAL_STATIC void hmac_roam_alg_record_bss(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_roam_alg_stru *roam_alg = &(roam_info->st_alg);

    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        hmac_roam_record_chan_info(&(roam_info->scan_info.chan_5g), bss_dscr);
        hmac_roam_alg_record_bss_info(&(roam_alg->st_home_network.st_5g_bssid), bss_dscr);
    } else if (bss_dscr->st_channel.en_band == WLAN_BAND_2G) {
        hmac_roam_record_chan_info(&(roam_info->scan_info.chan_2g), bss_dscr);
        hmac_roam_alg_record_bss_info(&(roam_alg->st_home_network.st_2g_bssid), bss_dscr);
    } else {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess::en_band[%d].}", bss_dscr->st_channel.en_band);
    }
}

uint32_t hmac_roam_alg_bss_in_ess(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_roam_alg_stru *roam_alg = NULL;
    mac_cfg_ssid_param_stru cfg_ssid;
    uint8_t stru_len;
    uint32_t roam_timeout;

    if (oal_any_null_ptr2(roam_info, bss_dscr)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    mac_mib_get_ssid (mac_vap, &stru_len, (uint8_t *)(&cfg_ssid));

    if ((OAL_STRLEN(bss_dscr->ac_ssid) != cfg_ssid.uc_ssid_len) ||
        (oal_memcmp(cfg_ssid.ac_ssid, bss_dscr->ac_ssid, cfg_ssid.uc_ssid_len) != 0)) {
        return OAL_SUCC;
    }

    /* 挑选HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE时间以内的有效bss */
    roam_timeout = ((g_pd_bss_expire_time * HMAC_S_TO_MS) < HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE) ?
        HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE : (g_pd_bss_expire_time * HMAC_S_TO_MS);
    if (oal_time_after32(oal_time_get_stamp_ms(),
                         (bss_dscr->timestamp + roam_timeout))) {
        return OAL_SUCC;
    }

    roam_alg = &(roam_info->st_alg);
    /* 是否扫描到了当前关联的 bss, 仅置位，不过滤 */
    if (oal_compare_mac_addr(mac_vap->auc_bssid, bss_dscr->auc_bssid) != 0) {
        roam_alg->uc_another_bss_scaned = 1;

        /* Roaming Scenario Recognition
         * dense AP standard: RSSI>=-60dB, candidate num>=5;
         *                    RSSI<-60dB && RSSI >=-75dB, candidate num>=10;
         */
        roam_alg->uc_candidate_bss_num++;
        if (bss_dscr->c_rssi >= roam_info->st_config.c_candidate_good_rssi) {
            roam_alg->uc_candidate_good_rssi_num++;
        } else if ((bss_dscr->c_rssi < roam_info->st_config.c_candidate_good_rssi) &&
                   bss_dscr->c_rssi >= ROAM_RSSI_NE75_DB) {
            roam_alg->uc_candidate_weak_rssi_num++;
        }

        if (bss_dscr->c_rssi > roam_alg->c_max_rssi) {
            roam_alg->c_max_rssi = bss_dscr->c_rssi;
        }
    } else {
        roam_alg->c_current_rssi = bss_dscr->c_rssi;
    }

    hmac_roam_alg_record_bss(roam_info, bss_dscr);

    return OAL_SUCC;
}


uint32_t hmac_roam_check_cipher_limit(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    mac_vap_stru *mac_vap = NULL;
    mac_cap_info_stru *cap_info = NULL;
#ifdef _PRE_WLAN_FEATURE_SAE
    mac_crypto_settings_stru crypto;
    uint32_t match_suite;
    uint32_t aul_rsn_akm_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
#ifdef _PRE_WLAN_FEATURE_11R
    wlan_auth_alg_mode_enum_uint8 en_auth_mode;
    uint8_t *puc_mde = NULL;
#endif
#endif

    mac_vap = &roam_info->pst_hmac_vap->st_vap_base_info;
    cap_info = (mac_cap_info_stru *)&bss_dscr->us_cap_info;

    /*  wep的bss直接过滤掉 */
    if ((bss_dscr->puc_rsn_ie == NULL) &&
        (bss_dscr->puc_wpa_ie == NULL) &&
        (cap_info->bit_privacy != 0)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /*  open加密方式到wpa/wpa2直接过滤掉 */
    if ((cap_info->bit_privacy == 0) != (OAL_TRUE != mac_mib_get_privacyinvoked(mac_vap))) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    /* 不支持SAE与其他任何非SAE认证方式的漫游 */
    memset_s(&crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));
    mac_ie_get_rsn_cipher(bss_dscr->puc_rsn_ie, &crypto);
    match_suite = mac_mib_rsn_akm_match_suites_s(mac_vap, crypto.aul_akm_suite,
                                                 sizeof(crypto.aul_akm_suite));
    if (match_suite == 0) {
        mac_mib_get_rsn_akm_suites_s(mac_vap, aul_rsn_akm_suites, sizeof(aul_rsn_akm_suites));
        if (is_only_support_sae(aul_rsn_akm_suites) || is_only_support_sae(crypto.aul_akm_suite) ||
            is_only_support_tbpeke(aul_rsn_akm_suites) || is_only_support_tbpeke(crypto.aul_akm_suite)) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ROAM,
                             "hmac_roam_check_cipher_limit::forbid roam between sae and others");
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_11R
    if (roam_info->pst_hmac_vap->bit_11r_enable) {
        en_auth_mode = mac_mib_get_AuthenticationMode(mac_vap);
        puc_mde = mac_find_ie(MAC_EID_MOBILITY_DOMAIN,
                              bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                              bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET);
        /* SAE不支持11R漫游 */
        if ((roam_info->pst_hmac_vap->en_sae_connect == OAL_TRUE) && (puc_mde != NULL)) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ROAM,
                             "hmac_roam_check_cipher_limit::SAE do not support 11r roam");
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            /* 不支持ft到非ft */
        } else if ((en_auth_mode == WLAN_WITP_AUTH_FT) && (puc_mde == NULL)) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ROAM,
                             "hmac_roam_check_cipher_limit::not support roam from FT  to Non_FT");
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE

oal_bool_enum_uint8 hmac_roam_alg_check_bsst_bssid_in_scan_list(hmac_roam_info_stru *roam_info,
                                                                uint8_t *puc_mac_addr)
{
    uint8_t bss_list_idx = 0;
    oal_bool_enum_uint8 bssid_find = OAL_FALSE;
    uint8_t *puc_dst_mac_addr = NULL;

    if (roam_info == NULL || puc_mac_addr == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_roam_alg_check_bsst_bssid_in_scan_list:: NULL pointer}");
        return OAL_FALSE;
    }

    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        uint8_t neighbor_bssid_num = roam_info->uc_neighbor_rpt_bssid_num;
        for (bss_list_idx = 0; bss_list_idx < neighbor_bssid_num; bss_list_idx++) {
            puc_dst_mac_addr = roam_info->auc_neighbor_rpt_bssid[bss_list_idx];
            if (!oal_memcmp(roam_info->auc_neighbor_rpt_bssid[bss_list_idx],
                            puc_mac_addr, WLAN_MAC_ADDR_LEN)) {
                bssid_find = OAL_TRUE;
                break;
            }
        }
    }
    return bssid_find;
}
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */


uint32_t hmac_roam_alg_check_bssid_limit(hmac_roam_info_stru *roam_info, uint8_t *puc_mac_addr)
{
    uint32_t ret;

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        ret = hmac_roam_alg_check_bsst_bssid_in_scan_list(roam_info, puc_mac_addr);
        if (ret != OAL_TRUE) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
        oam_warning_log3(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_check_bssid_limit :: [%02X:XX:XX:XX:%02X:%02X] is in scan list}",
                         puc_mac_addr[MAC_ADDR_0], puc_mac_addr[MAC_ADDR_4], puc_mac_addr[MAC_ADDR_5]);
    }
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */

    /* 检查黑名单 */
    ret = hmac_roam_alg_find_in_blacklist(roam_info, puc_mac_addr);
    if (ret == OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_check_bssid_limit:: [%02X:XX:XX:XX:%02X:%02X] in blacklist!}",
                         puc_mac_addr[MAC_ADDR_0], puc_mac_addr[MAC_ADDR_4], puc_mac_addr[MAC_ADDR_5]);
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    return OAL_SUCC;
}

#define is_roam_homenetwork_2gto5g_condition(hmac_vap, roam_info, bss_dscr) \
    (((roam_info)->st_alg.st_home_network.uc_is_homenetwork == OAL_TRUE) &&           \
        ((hmac_vap)->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) &&          \
        ((bss_dscr)->st_channel.en_band == WLAN_BAND_5G) &&                           \
        ((bss_dscr)->c_rssi > (roam_info)->st_alg.st_home_network.c_5g_rssi_th))

#define is_roam_weak_rssi_condition(c_tmp_rssi, bss_dscr)                                    \
    ((((c_tmp_rssi) < ROAM_RSSI_NE80_DB) && ((bss_dscr)->st_channel.en_band == WLAN_BAND_2G)) ||  \
        (((c_tmp_rssi) < ROAM_RSSI_NE78_DB) && ((bss_dscr)->st_channel.en_band == WLAN_BAND_5G)))

OAL_STATIC uint32_t hmac_roam_alg_check_aging_time(hmac_roam_info_stru *roam_info,
                                                   mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru *hmac_vap;
    uint32_t roam_timeout;

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_timeout = ((g_pd_bss_expire_time * HMAC_S_TO_MS) < HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE) ?
        HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE : (g_pd_bss_expire_time * HMAC_S_TO_MS);

    if (roam_info->en_current_bss_ignore == OAL_TRUE) {
        /* 非voe认证, 挑选HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE时间以内的有效bss */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        if (hmac_vap->bit_voe_11r_auth != 0) {
            return OAL_SUCC;
        }
#endif
        if (oal_time_after32(oal_time_get_stamp_ms(), (bss_dscr->timestamp + roam_timeout))) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_alg_check_channel(mac_vap_stru *mac_vap, hmac_roam_info_stru *roam_info,
                                                mac_bss_dscr_stru *bss_dscr)
{
    hmac_roam_alg_stru *roam_alg = &(roam_info->st_alg);
    mac_vap_stru *p2p_vap = mac_vap_find_another_up_vap_by_mac_vap(mac_vap);

    /* check dbac channel compared with p2p up channel */
    if (hmac_roam_check_dbac_channel(p2p_vap, bss_dscr->st_channel.uc_chan_number) == OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ROAM,
            "{hmac_roam_check_dbac_channel::dbac channel ignored, %0x:%0x chanNum=%d}",
            bss_dscr->auc_mac_addr[MAC_ADDR_4], bss_dscr->auc_mac_addr[MAC_ADDR_5],
            bss_dscr->st_channel.uc_chan_number);
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 非双sta模式，不做判断 */
    if (!mac_is_dual_sta_mode()) {
        return OAL_SUCC;
    }

    if (mac_is_primary_legacy_vap(mac_vap)) {
        /* wlan0已经找到了同频段可漫游的bss, 待处理bss是异频，不处理该bss。否则需要继续处理 */
        if (roam_alg->pst_max_rssi_bss &&
            roam_alg->pst_max_rssi_bss->st_channel.en_band == mac_vap->st_channel.en_band &&
            bss_dscr->st_channel.en_band != mac_vap->st_channel.en_band) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    } else {
        /* wlan1漫游不考虑异频的bss */
        if (bss_dscr->st_channel.en_band != mac_vap->st_channel.en_band) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint8_t hmac_roam_alg_compare_bss(hmac_vap_stru *hmac_vap, hmac_roam_alg_stru *roam_alg,
                                             mac_bss_dscr_stru *bss_dscr, int8_t tmp_rssi)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;

    if (roam_alg->pst_max_rssi_bss == NULL) {
        return OAL_TRUE;
    }

    /* 双sta模式下，如果已找到的bss是异频的，当前处理的bss是同频。不比较rssi，优先同频。 */
    if (mac_is_dual_sta_mode() &&
        roam_alg->pst_max_rssi_bss->st_channel.en_band != mac_vap->st_channel.en_band &&
        bss_dscr->st_channel.en_band == mac_vap->st_channel.en_band) {
        oam_warning_log1(0, OAM_SF_ROAM,
            "{hmac_roam_compare_bss::dual sta mode find first same band[%d] bss, replace the result!}",
            bss_dscr->st_channel.en_band);
        return OAL_TRUE;
    }

    return tmp_rssi > roam_alg->c_max_rssi;
}

static uint32_t hmac_roam_alg_check_common_info(mac_vap_stru *mac_vap,
                                                hmac_roam_info_stru *roam_info,
                                                mac_bss_dscr_stru *bss_dscr)
{
    uint32_t ret;
    /* 检查漫游到自己 */
    if ((roam_info->en_current_bss_ignore == OAL_FALSE) &&
        oal_memcmp(mac_vap->auc_bssid, bss_dscr->auc_bssid, OAL_MAC_ADDR_LEN)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 检查特定BSSID的漫游 */
    if ((roam_info->en_roam_trigger == ROAM_TRIGGER_BSSID) &&
        oal_memcmp(roam_info->auc_target_bssid, bss_dscr->auc_bssid, OAL_MAC_ADDR_LEN)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    ret = hmac_roam_alg_check_channel(mac_vap, roam_info, bss_dscr);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    ret = hmac_roam_alg_check_aging_time(roam_info, bss_dscr);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    ret = hmac_roam_alg_check_bssid_limit(roam_info, bss_dscr->auc_bssid);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* check bssid blacklist from Framework setting */
    ret = hmac_blacklist_filter(mac_vap, bss_dscr->auc_bssid);
    if (ret == OAL_TRUE) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 检查漫游到其它BSSID */
    if (roam_info->en_current_bss_ignore == OAL_TRUE) {
        /* 排除当前bss的rssi值计算，本地已经保存了dmac上报的rssi */
        if (oal_compare_mac_addr(mac_vap->auc_bssid, bss_dscr->auc_bssid) == 0) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    if (hmac_roam_check_cipher_limit(roam_info, bss_dscr) != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }
    return OAL_SUCC;
}

uint32_t hmac_roam_alg_bss_check(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_roam_alg_stru *roam_alg = NULL;
    uint8_t *puc_pmkid = NULL;
    mac_cfg_ssid_param_stru cfg_ssid;

    uint32_t avail_channel_capacity;
    uint8_t stru_len;
    int16_t s_delta_rssi;
    int8_t c_tmp_rssi;

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    mac_mib_get_ssid (mac_vap, &stru_len, (uint8_t *)(&cfg_ssid));

    if ((OAL_STRLEN(bss_dscr->ac_ssid) != cfg_ssid.uc_ssid_len) ||
        (0 != oal_memcmp(cfg_ssid.ac_ssid, bss_dscr->ac_ssid, cfg_ssid.uc_ssid_len))) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }
    roam_alg = &(roam_info->st_alg);

    if (hmac_roam_alg_check_common_info(mac_vap, roam_info, bss_dscr) != OAL_SUCC) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    if (is_roam_homenetwork_2gto5g_condition(hmac_vap, roam_info, bss_dscr)) {
        roam_alg->c_max_rssi = bss_dscr->c_rssi;
        roam_alg->pst_max_rssi_bss = bss_dscr;
        return OAL_SUCC;
    }

    c_tmp_rssi = bss_dscr->c_rssi;
    // 终端评审: 2.4G候选AP小于-80dB不再漫游，5G候选AP小于-78dB不再漫游
    if (is_roam_weak_rssi_condition(c_tmp_rssi, bss_dscr)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        c_tmp_rssi = hmac_roam_alg_adjust_5g_rssi_weight(c_tmp_rssi);
    }

    /* 支持ax 加分处理 */
    if (bss_dscr->en_he_capable == OAL_TRUE) {
        c_tmp_rssi = hmac_roam_alg_adjust_ax_rssi_weight(c_tmp_rssi);
    }

    if (bss_dscr->sap_mode == MAC_SAP_MASTER) {
        c_tmp_rssi = hmac_roam_alg_adjust_sap_rssi_weight(c_tmp_rssi);
    }

    /* c_current_rssi为0时，表示linkloss上报的触发，不需要考虑rssi增益 */
    s_delta_rssi = hmac_roam_alg_compare_rssi_increase(roam_info, bss_dscr, c_tmp_rssi);
    if (s_delta_rssi <= 0) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    avail_channel_capacity = hmac_roam_alg_calc_avail_channel_capacity(bss_dscr);
    if ((avail_channel_capacity != 0) &&
        ((roam_alg->pst_max_capacity_bss == NULL) ||
         (avail_channel_capacity > roam_alg->max_capacity))) {
    }

    /* 对于已存在pmk缓存的bss进行加分处理 */
    puc_pmkid = hmac_vap_get_pmksa(hmac_vap, bss_dscr->auc_bssid);
    if (puc_pmkid != NULL) {
        c_tmp_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    if (hmac_roam_alg_compare_bss(hmac_vap, roam_alg, bss_dscr, c_tmp_rssi)) {
        roam_alg->c_max_rssi = c_tmp_rssi;
        roam_alg->pst_max_rssi_bss = bss_dscr;
    }

    /* 当2G AP RSSI和5G AP加权RSSI相等时，优选5G低band AP */
    if ((c_tmp_rssi == roam_alg->c_max_rssi) &&
        (roam_alg->pst_max_rssi_bss->st_channel.en_band == WLAN_BAND_2G) &&
        (bss_dscr->st_channel.en_band == WLAN_BAND_5G)) {
        roam_alg->c_max_rssi = c_tmp_rssi;
        roam_alg->pst_max_rssi_bss = bss_dscr;
    }

    return OAL_SUCC;
}

oal_bool_enum_uint8 hmac_roam_alg_need_to_stop_roam_trigger(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_roam_alg_stru *roam_alg = NULL;

    if (roam_info == NULL) {
        return OAL_FALSE;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        return OAL_FALSE;
    }

    roam_alg = &(roam_info->st_alg);

    if (roam_alg->uc_another_bss_scaned) {
        roam_alg->uc_invalid_scan_cnt = 0xff;
        return OAL_FALSE;
    }

    if (roam_alg->uc_invalid_scan_cnt == 0xff) {
        return OAL_FALSE;
    }

    if (roam_alg->uc_invalid_scan_cnt++ > 4) { /* 连续扫描到当前关联且首次关联的 bss 的次数大于4次 */
        roam_alg->uc_invalid_scan_cnt = 0xff;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

mac_bss_dscr_stru *hmac_roam_alg_select_bss(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    hmac_roam_alg_stru *roam_alg = NULL;
    int16_t delta_rssi;

    if (oal_unlikely(roam_info == NULL || roam_info->pst_hmac_vap == NULL)) {
        return NULL;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    mac_vap = &(hmac_vap->st_vap_base_info);
    roam_alg = &(roam_info->st_alg);

    /* 取得最大 rssi 的 bss */
    bss_dscr = roam_alg->pst_max_rssi_bss;

    if ((roam_alg->pst_max_capacity_bss != NULL) &&
        (roam_alg->max_capacity >= ROAM_THROUGHPUT_THRESHOLD)) {
        /* 取得最大 capacity 的 bss */
        bss_dscr = roam_alg->pst_max_capacity_bss;
    }

    if (bss_dscr == NULL) {
        /* should not be here */
        return NULL;
    }

    if (roam_info->st_alg.st_home_network.uc_is_homenetwork == OAL_TRUE &&
        roam_info->en_roam_trigger == ROAM_TRIGGER_HOME_NETWORK) {
        /* 5g优选，选中的5g bssid */
        if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
            oam_warning_log4(0, OAM_SF_ROAM,
                "{hmac_roam_alg_select_bss::roam_to 5G home network AP, max_rssi=%d, [%02X:XX:XX:XX:%02X:%02X]}",
                roam_alg->c_max_rssi, bss_dscr->auc_bssid[BYTE_OFFSET_0],
                bss_dscr->auc_bssid[BYTE_OFFSET_4], bss_dscr->auc_bssid[BYTE_OFFSET_5]);
            return bss_dscr;
        }
    }

    /* rssi增益处理 */
    delta_rssi = hmac_roam_alg_compare_rssi_increase(roam_info, bss_dscr, roam_alg->c_max_rssi);
    if (delta_rssi <= 0) {
        return NULL;
    }

    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        oam_warning_log4(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_select_bss::roam_to 5G candidate AP, max_rssi=%d, [%02X:XX:XX:XX:%02X:%02X]}",
                         roam_alg->c_max_rssi, bss_dscr->auc_bssid[BYTE_OFFSET_0],
                         bss_dscr->auc_bssid[BYTE_OFFSET_4], bss_dscr->auc_bssid[BYTE_OFFSET_5]);
    } else {
        oam_warning_log4(0, OAM_SF_ROAM,
                         "{hmac_roam_alg_select_bss::roam_to candidate AP, max_rssi=%d, [%02X:XX:XX:XX:%02X:%02X]}",
                         roam_alg->c_max_rssi, bss_dscr->auc_bssid[BYTE_OFFSET_0],
                         bss_dscr->auc_bssid[BYTE_OFFSET_4], bss_dscr->auc_bssid[BYTE_OFFSET_5]);
    }

    return bss_dscr;
}
