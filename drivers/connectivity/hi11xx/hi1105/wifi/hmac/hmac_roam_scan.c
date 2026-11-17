

/* 1 头文件包含 */
#include "hmac_roam_scan.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_roam_alg.h"
#include "hmac_resource.h"
#include "mac_mib.h"
#include "hmac_dfx.h"
#include "hmac_fsm.h"
#include "hmac_scan.h"
#include "hmac_sme_sta.h"
#include "hmac_p2p.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_SCAN_C

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#define BSS_TRANSITION_RESP_MSG_LEN     4
#endif

#define ROAM_SCAN_SPECIFIC_CHAN_NUM  3

OAL_STATIC void hmac_roam_renew_privacy_wpa(mac_conn_security_stru *conn_sec, mac_vap_stru *mac_vap,
    mac_bss_dscr_stru *bss_dscr)
{
    uint32_t match_suite;
    if (bss_dscr->puc_wpa_ie == NULL) {
        return;
    }
    conn_sec->en_privacy = OAL_TRUE;
    mac_ie_get_wpa_cipher(bss_dscr->puc_wpa_ie, &(conn_sec->st_crypto));

    match_suite = mac_mib_wpa_pair_match_suites_s(mac_vap, conn_sec->st_crypto.aul_pair_suite,
                                                  sizeof(conn_sec->st_crypto.aul_pair_suite));
    if (match_suite != 0) {
        conn_sec->st_crypto.aul_pair_suite[0] = match_suite;
    }
    conn_sec->st_crypto.aul_pair_suite[1] = 0;

    match_suite = mac_mib_wpa_akm_match_suites_s(mac_vap, conn_sec->st_crypto.aul_akm_suite,
                                                 sizeof(conn_sec->st_crypto.aul_akm_suite));
    if (match_suite != 0) {
        conn_sec->st_crypto.aul_akm_suite[0] = match_suite;
    }
    conn_sec->st_crypto.aul_akm_suite[1] = 0;
}

OAL_STATIC uint16_t hmac_roam_renew_privacy_rsn(mac_conn_security_stru *conn_sec, mac_vap_stru *mac_vap,
    mac_bss_dscr_stru *bss_dscr)
{
    uint32_t match_suite;
    uint16_t rsn_cap_info = 0;
    if (bss_dscr->puc_rsn_ie == NULL) {
        return rsn_cap_info;
    }
    conn_sec->en_privacy = OAL_TRUE;
    mac_ie_get_rsn_cipher(bss_dscr->puc_rsn_ie, &(conn_sec->st_crypto));

    match_suite = mac_mib_rsn_pair_match_suites_s(mac_vap, conn_sec->st_crypto.aul_pair_suite,
                                                  sizeof(conn_sec->st_crypto.aul_pair_suite));
    if (match_suite != 0) {
        conn_sec->st_crypto.aul_pair_suite[0] = match_suite;
    }
    conn_sec->st_crypto.aul_pair_suite[1] = 0;

    match_suite = mac_mib_rsn_akm_match_suites_s(mac_vap, conn_sec->st_crypto.aul_akm_suite,
                                                 sizeof(conn_sec->st_crypto.aul_akm_suite));
    if (match_suite != 0) {
        conn_sec->st_crypto.aul_akm_suite[0] = match_suite;
    }
    conn_sec->st_crypto.aul_akm_suite[1] = 0;
    /* 返回目的AP的RSN CAP信息 */
    return mac_get_rsn_capability(bss_dscr->puc_rsn_ie);
}

OAL_STATIC uint32_t hmac_roam_renew_privacy(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    uint32_t ret;
    mac_conn_security_stru conn_sec;
    mac_cap_info_stru *cap_info = NULL;
    uint16_t rsn_cap_info;
    oal_bool_enum_uint8 pmf_cap, pmf_require;

    cap_info = (mac_cap_info_stru *)&bss_dscr->us_cap_info;

    if (cap_info->bit_privacy == 0) {
        return OAL_SUCC;
    }

    if (mac_mib_get_AuthenticationMode(&hmac_vap->st_vap_base_info) != NL80211_AUTHTYPE_OPEN_SYSTEM) {
        return OAL_SUCC;
    }

    memset_s(&conn_sec, sizeof(mac_conn_security_stru), 0, sizeof(mac_conn_security_stru));

    hmac_roam_renew_privacy_wpa(&conn_sec, &hmac_vap->st_vap_base_info, bss_dscr);

    rsn_cap_info = hmac_roam_renew_privacy_rsn(&conn_sec, &hmac_vap->st_vap_base_info, bss_dscr);

    pmf_cap = mac_mib_get_dot11RSNAMFPC(&hmac_vap->st_vap_base_info);
    pmf_require = mac_mib_get_dot11RSNAMFPR(&hmac_vap->st_vap_base_info);
    if ((pmf_require == OAL_TRUE) && (!(rsn_cap_info & BIT7))) { /* 本地强制，对端没有MFP能力 */
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: vap required pmf and ap don't have pmf cap!}");
    }

    if ((pmf_cap == OAL_FALSE) && (rsn_cap_info & BIT6)) { /* 对端强制，本地没有MFP能力 */
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: vap no pmf cap and ap required!!}");
    }

    /* 当前驱动的pmf能力由wpa控制, 故漫游时不支持pmf到非pmf ap的漫游 */
    if (hmac_vap->st_vap_base_info.en_user_pmf_cap && (!(rsn_cap_info & BIT7))) { /* 原ap开pmf 目的ap无pmf */
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: roam to no pmf ap!!}");
    }

    /* 暂时赋值上次的PMF能力，在join阶段才会根据对端PMF能力来更新VAP的PMF能力并同步到device */
    conn_sec.en_mgmt_proteced = hmac_vap->st_vap_base_info.en_user_pmf_cap;
    conn_sec.en_pmf_cap = pmf_cap + pmf_require;

    ret = mac_vap_init_privacy(&hmac_vap->st_vap_base_info, &conn_sec);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: mac_11i_init_privacy failed[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
/* 漫游时，大屏如果有其他VAP处于P2P listen状态，则先取消listen，防止漫游扫描失败 */
OAL_STATIC void hmac_roam_stop_p2p_listen(mac_vap_stru *mac_vap)
{
    uint8_t vap_idx;
    uint32_t pedding_data = 0;
    mac_vap_stru *stop_mac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;
    uint8_t self_vap_id = mac_vap->uc_vap_id;

    if (!hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        return;
    }

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_roam_stop_p2p_listen::hmac_device null device_id = [%d]!}",
            mac_vap->uc_device_id);
        return;
    }

    mac_device = hmac_device->pst_device_base_info;
    if (mac_device == NULL) {
        return;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        stop_mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (stop_mac_vap == NULL) {
            continue;
        }

        if (self_vap_id == stop_mac_vap->uc_vap_id) {
            continue;
        }

        if (stop_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            oam_warning_log1(self_vap_id, OAM_SF_ROAM, "{hmac_roam_stop_p2p_listen::stop p2p listen vap id = [%d]!}",
                stop_mac_vap->uc_vap_id);
            hmac_config_scan_abort(stop_mac_vap, sizeof(uint32_t), (uint8_t*)&pedding_data);
        }
    }
}

void hmac_roam_scan_comp_cb(void *param)
{
    hmac_scan_record_stru *scan_record = (hmac_scan_record_stru *)param;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_roam_info_stru *roam_info = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *scan_bss_mgmt = NULL;
    uint32_t ret;

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_roam_scan_comp_cb::hmac_vap is null.");
        return;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::device null!}");
        return;
    }

    scan_bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (roam_info->uc_enable == 0) {
        return;
    }

    oam_info_log0(scan_record->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_scan_complete::handling scan result!}");

    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)scan_bss_mgmt);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::handling scan result fail! ret:%d}", ret);
    }
}

OAL_STATIC void hmac_roam_scan_params_init(mac_scan_req_stru *scan_params, uint8_t *cur_ssid, mac_vap_stru *mac_vap)
{
     /* 扫描参数初始化 */
    scan_params->en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->en_scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_params->us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    scan_params->uc_probe_delay = 0;
    scan_params->uc_scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
    scan_params->p_fn_cb = hmac_roam_scan_comp_cb;
    scan_params->uc_max_send_probe_req_count_per_channel = 2; /* 每个信道最多发送2次探测请求 */
    scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道最多扫描2次 */
    scan_params->en_scan_mode = WLAN_SCAN_MODE_ROAM_SCAN;
    scan_params->bit_desire_fast_scan = mac_vap->bit_roam_scan_valid_rslt;
    if (memcpy_s(scan_params->ast_mac_ssid_set[0].auc_ssid, WLAN_SSID_MAX_LEN, cur_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_roam_scan_params_init:memcopy fail!");
        return;
    }
    scan_params->uc_ssid_num = 1;

    /* 初始扫描请求只指定1个bssid，为广播地址 */
    memset_s(scan_params->auc_bssid, WLAN_MAC_ADDR_LEN, 0xff, WLAN_MAC_ADDR_LEN);
    scan_params->uc_bssid_num = 1;
}

OAL_STATIC void hmac_roam_scan_inc_one_channel(mac_scan_req_stru *scan_params,
                                               mac_channel_stru *channel)
{
    uint8_t index = scan_params->uc_channel_nums;

    oam_warning_log3(0, OAM_SF_ROAM,
        "{hmac_roam_scan_inc_one_channel::increase one channel, ChanNum[%d], Band[%d], ChanIdx[%d]}",
        channel->uc_chan_number, channel->en_band, channel->uc_chan_idx);

    scan_params->ast_channel_list[index].uc_chan_number = channel->uc_chan_number;
    scan_params->ast_channel_list[index].en_band = channel->en_band;
    scan_params->ast_channel_list[index].uc_chan_idx = channel->uc_chan_idx;

    scan_params->uc_channel_nums = (++index);
}

OAL_STATIC uint32_t hmac_roam_scan_one_channel_init(hmac_roam_info_stru *roam_info,
                                                    mac_scan_req_stru *scan_params)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_channel_stru *channel = NULL;

    if (oal_any_null_ptr2(roam_info, scan_params)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_one_channel_init::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->pst_hmac_vap;

    if (roam_info->en_roam_trigger == ROAM_TRIGGER_HOME_NETWORK) {  // scan 5G home network
        channel = &(roam_info->st_alg.st_home_network.st_5g_bssid.st_channel[0]);
    } else {
        channel = &(hmac_vap->st_vap_base_info.st_channel);
    }
    hmac_roam_scan_inc_one_channel(scan_params, channel);

    scan_params->uc_max_scan_count_per_channel = 1;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_scan_spec_2g_chan(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    uint32_t ret = OAL_FAIL;

    switch (roam_info->st_config.uc_scan_orthogonal) {
        case ROAM_SCAN_CHANNEL_ORG_3: {
            scan_params->ast_channel_list[0].uc_chan_number = 1;
            scan_params->ast_channel_list[0].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[0].uc_chan_idx = 0;
            scan_params->ast_channel_list[1].uc_chan_number = 6; /* 6信道 */
            scan_params->ast_channel_list[1].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[1].uc_chan_idx = 5; /* 信道索引号为5 */
            scan_params->ast_channel_list[BYTE_OFFSET_2].uc_chan_number = 11; /* 11信道 */
            scan_params->ast_channel_list[BYTE_OFFSET_2].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[BYTE_OFFSET_2].uc_chan_idx = 10; /* 信道索引号为10 */
            scan_params->uc_channel_nums = 3; /* 信道数为3 */
            scan_params->uc_max_scan_count_per_channel = 1;
            ret = OAL_SUCC;
            break;
        }
        case ROAM_SCAN_CHANNEL_ORG_4: {
            scan_params->ast_channel_list[0].uc_chan_number = 1;
            scan_params->ast_channel_list[0].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[0].uc_chan_idx = 0;
            scan_params->ast_channel_list[1].uc_chan_number = 5; /* 5信道 */
            scan_params->ast_channel_list[1].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[1].uc_chan_idx = 4; /* 信道索引号为4 */
            scan_params->ast_channel_list[BYTE_OFFSET_2].uc_chan_number = 9; /* 9信道 */
            scan_params->ast_channel_list[BYTE_OFFSET_2].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[BYTE_OFFSET_2].uc_chan_idx = 8; /* 信道索引号为8 */
            scan_params->ast_channel_list[BYTE_OFFSET_3].uc_chan_number = 13; /* 13信道 */
            scan_params->ast_channel_list[BYTE_OFFSET_3].en_band = WLAN_BAND_2G;
            scan_params->ast_channel_list[BYTE_OFFSET_3].uc_chan_idx = 12; /* 信道索引号为12 */
            scan_params->uc_channel_nums = 4; /* 信道数为4 */
            scan_params->uc_max_scan_count_per_channel = 1;
            ret = OAL_SUCC;
            break;
        }
        default: {
            break;
        }
    }
    return ret;
}

OAL_STATIC uint32_t hmac_roam_scan_specified_chan_num(hmac_roam_info_stru *roam_info)
{
    if (roam_info->scan_info.chan_2g.chan_num <= MAX_RECORD_CHAN_NUM &&
        roam_info->scan_info.chan_5g.chan_num <= MAX_RECORD_CHAN_NUM) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

uint32_t hmac_roam_check_dual_threshold(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;

    if (mac_is_secondary_sta(&hmac_vap->st_vap_base_info)) {
        return OAL_FALSE;
    }

    if (hmac_roam_scan_specified_chan_num(roam_info) == OAL_TRUE &&
        (hmac_vap->en_is_psk == OAL_TRUE || hmac_vap->en_sae_connect == OAL_TRUE)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

OAL_STATIC uint32_t hmac_roam_scan_specified_chan_condition(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;

    if (mac_is_secondary_sta(&hmac_vap->st_vap_base_info)) {
        return OAL_FALSE;
    }

    if (hmac_roam_scan_specified_chan_num(roam_info) == OAL_FALSE) {
        return OAL_FALSE;
    }

    if (hmac_vap->en_is_psk == OAL_FALSE && hmac_vap->en_sae_connect == OAL_FALSE) {
        return OAL_FALSE;
    }

    /* APP cmd: full channel scan */
    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_FULL ||
        roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_DBDC) {
        return OAL_FALSE;
    }

    if (roam_info->uc_rssi_type != ROAM_ENV_SPARSE_AP_TH1 &&
        roam_info->uc_rssi_type != ROAM_ENV_SPARSE_AP_TH2) {
        return OAL_FALSE;
    }

    /* RSSI TH1~TH2: scan specified channel */
    if ((hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G &&
        roam_info->st_alg.c_current_rssi >= roam_info->st_config.c_trigger_rssi_2g) ||
        (hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_5G &&
        roam_info->st_alg.c_current_rssi >= roam_info->st_config.c_trigger_rssi_5g)) {
        return OAL_TRUE;
    }

    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_SPECIFIED &&
        roam_info->en_roam_trigger != ROAM_TRIGGER_DMAC) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/* 对于2G和5G信道小于等于3+3 的ESS网络，漫游扫描指定的信道，非全信道扫描 */
OAL_STATIC void hmac_roam_scan_specified_chan(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    uint8_t i;
    uint8_t chan_num = oal_min(roam_info->scan_info.chan_2g.chan_num, MAX_RECORD_CHAN_NUM);

    for (i = 0; i < chan_num; i++) {
        scan_params->ast_channel_list[scan_params->uc_channel_nums].en_band =
            roam_info->scan_info.chan_2g.channel[i].en_band;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].ext6g_band =
            roam_info->scan_info.chan_2g.channel[i].ext6g_band;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_idx =
            roam_info->scan_info.chan_2g.channel[i].uc_chan_idx;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number =
            roam_info->scan_info.chan_2g.channel[i].uc_chan_number;
        oam_warning_log2(0, OAM_SF_ROAM,
            "{hmac_roam_scan_specified_chan::increase 2G one channel, ChanIdx[%d], ChanNumber[%d]}",
            scan_params->uc_channel_nums, scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number);
        scan_params->uc_channel_nums++;
    }

    chan_num = oal_min(roam_info->scan_info.chan_5g.chan_num, MAX_RECORD_CHAN_NUM);

    for (i = 0; i < chan_num; i++) {
        scan_params->ast_channel_list[scan_params->uc_channel_nums].en_band =
            roam_info->scan_info.chan_5g.channel[i].en_band;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].ext6g_band =
            roam_info->scan_info.chan_5g.channel[i].ext6g_band;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_idx =
            roam_info->scan_info.chan_5g.channel[i].uc_chan_idx;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number =
            roam_info->scan_info.chan_5g.channel[i].uc_chan_number;
        oam_warning_log2(0, OAM_SF_ROAM,
            "{hmac_roam_scan_specified_chan::increase 5G one channel, ChanIdx[%d], ChanNumber[%d]}",
            scan_params->uc_channel_nums, scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number);
        scan_params->uc_channel_nums++;
    }

    scan_params->uc_max_send_probe_req_count_per_channel = 1; /* 每个信道最多发送1次探测请求 */
    scan_params->uc_max_scan_count_per_channel = 1; /* 每个信道最多扫描1次，停留的总时间=20ms*扫描次数 */
}


OAL_STATIC void hmac_roam_scan_full_chan(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    uint32_t ret;
    uint8_t chan_idx;
    uint8_t chan_number;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
        ret = mac_is_channel_idx_valid(WLAN_BAND_2G, chan_idx, OAL_FALSE);
        if (ret != OAL_SUCC) {
            continue;
        }
        mac_get_channel_num_from_idx(WLAN_BAND_2G, chan_idx, OAL_FALSE, &chan_number);
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number = chan_number;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].en_band = WLAN_BAND_2G;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_idx = chan_idx;
        scan_params->ast_channel_list[scan_params->uc_channel_nums].ext6g_band = OAL_FALSE;
        scan_params->uc_channel_nums++;
    }
    if (mac_device_band_is_support(roam_info->pst_hmac_vap->st_vap_base_info.uc_device_id,
        MAC_DEVICE_CAP_5G) == OAL_TRUE) {
        for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
            ret = mac_is_channel_idx_valid(WLAN_BAND_5G, chan_idx, OAL_FALSE);
            if (ret != OAL_SUCC) {
                continue;
            }
            mac_get_channel_num_from_idx(WLAN_BAND_5G, chan_idx, OAL_FALSE, &chan_number);
            scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_number = chan_number;
            scan_params->ast_channel_list[scan_params->uc_channel_nums].en_band = WLAN_BAND_5G;
            scan_params->ast_channel_list[scan_params->uc_channel_nums].uc_chan_idx = chan_idx;
            scan_params->ast_channel_list[scan_params->uc_channel_nums].ext6g_band = OAL_FALSE;
            scan_params->uc_channel_nums++;
        }
    }
    scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道最多扫描2次 */
}

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE

OAL_STATIC uint32_t hmac_roam_scan_11v_chan(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    uint8_t chan_idx;
    uint8_t chan_nums;
    mac_scan_req_stru *src_scan_params = NULL;
    hmac_user_11v_ctrl_stru *ctrl_info_11v = NULL;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;

    if (hmac_user == NULL || roam_info->en_roam_trigger != ROAM_TRIGGER_11V) {
        return OAL_FAIL;
    }

    ctrl_info_11v = &hmac_user->st_11v_ctrl_info;
    src_scan_params = &ctrl_info_11v->st_scan_h2d_params.st_scan_params;

    chan_nums = src_scan_params->uc_channel_nums;

    for (chan_idx = 0; chan_idx < chan_nums ; chan_idx++) {
        scan_params->ast_channel_list[chan_idx].uc_chan_number =
            src_scan_params->ast_channel_list[chan_idx].uc_chan_number;
        scan_params->ast_channel_list[chan_idx].en_band =
            src_scan_params->ast_channel_list[chan_idx].en_band;
        scan_params->ast_channel_list[chan_idx].uc_chan_idx =
            src_scan_params->ast_channel_list[chan_idx].uc_chan_idx;
    }
    scan_params->uc_channel_nums = chan_nums;
    scan_params->uc_max_scan_count_per_channel = 1;
    scan_params->en_need_switch_back_home_channel = OAL_TRUE;
    scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE;
    scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
    oam_warning_log1(0, OAM_SF_ROAM, "hmac_roam_scan_11v_chan:: chan_nums %d", scan_params->uc_channel_nums);
    return OAL_SUCC;
}
#endif

OAL_STATIC uint32_t hmac_roam_single_band_channel_list(mac_scan_req_stru *scan_params,
                                                       wlan_channel_band_enum_uint8 band)
{
    uint8_t chan_idx;
    uint8_t chan_number;
    uint8_t chan_end_idx;
    mac_channel_stru *channel_item = NULL;
    if (band == WLAN_BAND_2G) {
        chan_end_idx = MAC_CHANNEL_FREQ_2_BUTT;
    } else {
        chan_end_idx = MAC_CHANNEL_FREQ_5_BUTT;
    }

    for (chan_idx = 0; chan_idx < chan_end_idx; chan_idx++) {
        /* 判断信道是不是在管制域内, 由于现在漫游还不支持6G，所以这里还是2G、5G */
        if (mac_is_channel_idx_valid(band, chan_idx, OAL_FALSE) == OAL_SUCC) {
            mac_get_channel_num_from_idx(band, chan_idx, OAL_FALSE, &chan_number);
            channel_item = &scan_params->ast_channel_list[scan_params->uc_channel_nums];
            channel_item->uc_chan_number = chan_number;
            channel_item->en_band = band;
            channel_item->uc_chan_idx = chan_idx;
            channel_item->ext6g_band = OAL_FALSE;
            scan_params->uc_channel_nums++;
        }
    }
    return OAL_SUCC;
}

uint32_t hmac_roam_scan_dbdc_channel_init(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    mac_vap_stru *other_vap = NULL;

    if (roam_info->st_config.uc_scan_orthogonal != ROAM_SCAN_CHANNEL_ORG_DBDC) {
        return OAL_FALSE;
    }

    /* check p2p up state: if there is no up p2p, will execute full scan by default */
    other_vap = mac_vap_find_another_up_vap_by_mac_vap(&roam_info->pst_hmac_vap->st_vap_base_info);
    if ((other_vap != NULL) && (hmac_p2p_get_scenes(other_vap) == MAC_P2P_SCENES_LOW_LATENCY)) {
        mac_channel_stru *channel = &(other_vap->st_channel);
        if (channel->en_band == WLAN_BAND_5G) {
            // P2P working at 5G band, scan 2G band and p2p channel
            hmac_roam_single_band_channel_list(scan_params, WLAN_BAND_2G);
            hmac_roam_scan_inc_one_channel(scan_params, channel);
        } else {
            // P2P working at 2G band, scan p2p channel and 5G band
            hmac_roam_scan_inc_one_channel(scan_params, channel);
            hmac_roam_single_band_channel_list(scan_params, WLAN_BAND_5G);
        }
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

uint32_t hmac_roam_scan_channel_init(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    uint32_t ret;

    if (oal_any_null_ptr2(roam_info, scan_params)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_scan_channel_init::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_params->uc_channel_nums = 0;

    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
        return hmac_roam_scan_one_channel_init(roam_info, scan_params);
    }

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_5) {
        return hmac_roam_scan_11v_chan(roam_info, scan_params);
    }
#endif

    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_3 ||
        roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_4) {
        return hmac_roam_scan_spec_2g_chan(roam_info, scan_params);
    }
    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_2G) {
        return hmac_roam_single_band_channel_list(scan_params, WLAN_BAND_2G);
    }
    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_5G) {
        return hmac_roam_single_band_channel_list(scan_params, WLAN_BAND_5G);
    }

    if (roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_DBDC) {
        ret = hmac_roam_scan_dbdc_channel_init(roam_info, scan_params);
        if (ret == OAL_TRUE) {
            return OAL_SUCC;
        }
    }

    /* 对于2G和5G信道小于等于3+3 的ESS网络，漫游扫描指定的信道，非全信道扫描 */
    if (hmac_roam_scan_specified_chan_condition(roam_info) == OAL_TRUE) {
        hmac_roam_scan_specified_chan(roam_info, scan_params);
    } else {
        hmac_roam_scan_full_chan(roam_info, scan_params);
    }

    return OAL_SUCC;
}

uint32_t hmac_roam_scan_init(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    mac_scan_req_stru *scan_params = NULL;
    uint8_t *cur_ssid = NULL;
    mac_vap_stru *mac_vap = NULL;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_init::check_state fail[%d]!}", ret);
        return ret;
    }

    if (mac_is_wep_enabled(&(roam_info->pst_hmac_vap->st_vap_base_info))) {
        hmac_roam_rssi_trigger_type((roam_info->pst_hmac_vap), ROAM_ENV_LINKLOSS);
        return OAL_SUCC;
    }

    mac_vap = &(roam_info->pst_hmac_vap->st_vap_base_info);
    hmac_roam_stop_p2p_listen(mac_vap);

    scan_params = &roam_info->st_scan_h2d_params.st_scan_params;
    cur_ssid = mac_mib_get_DesiredSSID(&(roam_info->pst_hmac_vap->st_vap_base_info));
    hmac_roam_scan_params_init(scan_params, cur_ssid, mac_vap);

#ifdef _PRE_WLAN_FEATURE_11K
    if (roam_info->pst_hmac_vap->bit_nb_rpt_11k
        && (roam_info->en_roam_trigger == ROAM_TRIGGER_DMAC || roam_info->is_roaming_trigged_by_cmd == OAL_TRUE)
        && roam_info->pst_hmac_vap->bit_11k_enable
        && hmac_roam_is_neighbor_report_allowed(roam_info->pst_hmac_vap)) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_NEIGHBOR_PROCESS);
        ret = hmac_roam_send_neighbor_req(roam_info, cur_ssid);
        if (ret == OAL_SUCC) {
            return ret;
        }
        scan_params->uc_neighbor_report_process_flag = OAL_FALSE;
        scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道最多扫描2次 */
        memset_s(scan_params->ast_mac_ssid_set[1].auc_ssid, WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_INIT);
    }
#endif

    roam_info->st_static.uc_roam_mode = HMAC_CHR_ROAM_NORMAL;
    roam_info->st_static.uc_scan_mode = HMAC_CHR_NORMAL_SCAN;

    ret = hmac_roam_scan_channel_init(roam_info, scan_params);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_init::hmac_roam_alg_scan_channel_init fail[%d]}", ret);
        return ret;
    }
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, (void *)roam_info);
    if (ret != OAL_SUCC) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        return ret;
    }

    return OAL_SUCC;
}

/*
* 功能描述  : update scan channel list params
*/
OAL_STATIC void hmac_scan_params_update(mac_scan_req_stru *scan_params, uint8_t channel_nums_2g,
    uint8_t channel_nums_5g, mac_channel_stru *channel_list)
{
    if (channel_nums_2g != 0) {
        if (memcpy_s(&scan_params->ast_channel_list[0], WLAN_MAX_CHANNEL_NUM * sizeof(mac_channel_stru),
            &channel_list[0], channel_nums_2g * sizeof(mac_channel_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hmac_scan_params_update::memcpy fail!");
        }
    }

    if (channel_nums_5g != 0) {
        if (memcpy_s(&scan_params->ast_channel_list[channel_nums_2g],
            (WLAN_MAX_CHANNEL_NUM - channel_nums_2g) * sizeof(mac_channel_stru),
            &channel_list[WLAN_2G_CHANNEL_NUM], channel_nums_5g * sizeof(mac_channel_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hmac_scan_params_update::memcpy fail!");
        }
    }
}


OAL_STATIC uint32_t hmac_roam_reorder_chan_list(mac_scan_req_h2d_stru *s2d_scan_req)
{
    mac_scan_req_stru *scan_params = NULL;
    mac_channel_stru channel_list[WLAN_MAX_CHANNEL_NUM];
    uint8_t channel_nums_2g = 0;
    uint8_t channel_nums_5g = 0;
    uint8_t channel_nums;
    uint8_t chan_idx;
    int32_t ret = EOK;

    scan_params = &s2d_scan_req->st_scan_params;
    channel_nums = scan_params->uc_channel_nums;

    /* only scan 1 channel, not need reorder */
    if (channel_nums == 1) {
        return OAL_SUCC;
    }
    memset_s(channel_list, WLAN_MAX_CHANNEL_NUM * sizeof(mac_channel_stru),
             0, WLAN_MAX_CHANNEL_NUM * sizeof(mac_channel_stru));
    for (chan_idx = 0; chan_idx < channel_nums; chan_idx++) {
        if (scan_params->ast_channel_list[chan_idx].en_band == WLAN_BAND_2G) {
            ret += memcpy_s(&channel_list[channel_nums_2g],
                            (WLAN_MAX_CHANNEL_NUM - channel_nums_2g) * sizeof(mac_channel_stru),
                            &scan_params->ast_channel_list[chan_idx], sizeof(mac_channel_stru));
            channel_nums_2g++;
        } else if (scan_params->ast_channel_list[chan_idx].en_band == WLAN_BAND_5G) {
            ret += memcpy_s(&channel_list[WLAN_2G_CHANNEL_NUM + channel_nums_5g],
                            (WLAN_MAX_CHANNEL_NUM - (WLAN_2G_CHANNEL_NUM + channel_nums_5g)) * sizeof(mac_channel_stru),
                            &scan_params->ast_channel_list[chan_idx],
                            sizeof(mac_channel_stru));
            channel_nums_5g++;
        } else {
            continue;
        }
        if (ret != EOK) {
            oam_warning_log0(0, 0, "{hmac_roam_reorder_chan_list::memcpy fail.}");
        }
    }

    if (channel_nums_2g > WLAN_2G_CHANNEL_NUM || channel_nums_5g > WLAN_5G_CHANNEL_NUM) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_reorder_chan_list:: invalid ch num!}");
        return OAL_FAIL;
    }

    /* update scan channel num params if need */
    if (scan_params->uc_channel_nums != channel_nums_2g + channel_nums_5g) {
        scan_params->uc_channel_nums = channel_nums_2g + channel_nums_5g;
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_reorder_chan_list:: chnum need update [%d]!}",
            scan_params->uc_channel_nums);
    }

    hmac_scan_params_update(scan_params, channel_nums_2g, channel_nums_5g, channel_list);
    return OAL_SUCC;
}

uint32_t hmac_roam_scan_channel(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::check_state fail[%d]!}", ret);
        return ret;
    }

    /* 下发信道列表的顺序需要满足2.4G, 5G */
    ret = hmac_roam_reorder_chan_list(&roam_info->st_scan_h2d_params);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::reorde channel list fail [%d]!}", ret);
        return ret;
    }

    roam_info->st_static.scan_cnt++;

    /* 更新扫描/关联时间戳 */
    roam_info->st_static.scan_start_timetamp = (uint32_t)oal_time_get_stamp_ms();
    roam_info->st_static.scan_end_timetamp = (uint32_t)oal_time_get_stamp_ms();
    roam_info->st_static.connect_start_timetamp = (uint32_t)oal_time_get_stamp_ms();
    roam_info->st_static.connect_end_timetamp = (uint32_t)oal_time_get_stamp_ms();

    /* 发起背景扫描 */
    ret = hmac_fsm_call_func_sta(roam_info->pst_hmac_vap, HMAC_FSM_INPUT_SCAN_REQ,
                                 (void *)(&roam_info->st_scan_h2d_params));
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::start scan failed!}");
    }

    /* 启动扫描超时定时器 */
    hmac_roam_main_start_timer(roam_info, ROAM_SCAN_TIME_MAX);

    return OAL_SUCC;
}
void hmac_roam_scene1_process(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device, hmac_roam_info_stru *roam_info)
{
    if ((roam_info->st_config.uc_scenario_enable == OAL_TRUE) &&
        (hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_chan_numbers >= WLAN_FULL_CHANNEL_NUM)) {
        /* 密集AP场景要求: RSSI>=-60dB 的AP个数>=5 或 RSSI在[-75dB,-60dB)的AP个数>=10 */
        if ((roam_info->st_alg.uc_candidate_good_rssi_num >= roam_info->st_config.uc_candidate_good_num) ||
            (roam_info->st_alg.uc_candidate_weak_rssi_num >= roam_info->st_config.uc_candidate_weak_num)) {
            roam_info->st_alg.uc_scan_period = 0;
            oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                "{hmac_roam_check_bkscan_result::roam_to dense AP env: candidate bss total[%d] good[%d] weak[%d]}",
                roam_info->st_alg.uc_candidate_bss_num,
                roam_info->st_alg.uc_candidate_good_rssi_num,
                roam_info->st_alg.uc_candidate_weak_rssi_num);
            if (roam_info->uc_rssi_type != ROAM_ENV_DENSE_AP) {
                hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_DENSE_AP);
            }
            return;
        }
        /* 连续5次扫描都不是密集AP环境，则切换回默认AP环境模式 */
        roam_info->st_alg.uc_scan_period++;
        if ((roam_info->st_alg.uc_another_bss_scaned) &&
            (roam_info->st_alg.uc_scan_period == ROAM_ENV_DENSE_TO_SPARSE_PERIOD)) {
            oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                "{hmac_roam_check_bkscan_result::roam_to sparse AP env: candidate bss total[%d] good[%d] weak[%d]}",
                roam_info->st_alg.uc_candidate_bss_num,
                roam_info->st_alg.uc_candidate_good_rssi_num,
                roam_info->st_alg.uc_candidate_weak_rssi_num);

            hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP_TH2);
        }
    } else if (roam_info->st_alg.uc_another_bss_scaned  && (roam_info->uc_rssi_type != ROAM_ENV_DENSE_AP)) {
        /* uc_chan_numbers < WLAN_FULL_CHANNEL_NUM */
        hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP_TH2);
    }
}
void hmac_roam_scene4_process(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    if ((roam_info->st_config.uc_scenario_enable == OAL_TRUE) &&
        (roam_info->st_alg.c_max_rssi - roam_info->st_alg.c_current_rssi >= ROAM_ENV_BETTER_RSSI_DISTANSE) &&
        (roam_info->st_alg.c_current_rssi < ROAM_RSSI_NE65_DB)) {
        if (roam_info->st_alg.uc_better_rssi_null_period >= (ROAM_ENV_BETTER_RSSI_NULL_PERIOD + 1)) {
            roam_info->st_alg.uc_better_rssi_scan_period = 0;  // restart scan period
            roam_info->st_alg.uc_better_rssi_null_period = 0;
        }
        roam_info->st_alg.uc_better_rssi_scan_period++;
        oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_check_bkscan_result::roam_to better AP env:[%d] max_rssi[%d] current_rssi[%d]}",
                         roam_info->st_alg.uc_better_rssi_scan_period,
                         roam_info->st_alg.c_max_rssi, roam_info->st_alg.c_current_rssi);

        if (roam_info->st_alg.uc_better_rssi_scan_period == ROAM_ENV_BETTER_RSSI_PERIOD) {
            roam_info->st_alg.uc_better_rssi_scan_period = 0;
            roam_info->st_alg.uc_better_rssi_null_period = 0;

            hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_TRUE, NULL, ROAM_TRIGGER_APP);
        }
    } else {
        roam_info->st_alg.uc_better_rssi_null_period++;
    }
}

uint32_t hmac_roam_check_bkscan_result(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
    hmac_roam_info_stru *roam_info = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *scan_bss_mgmt = NULL;
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    oal_bool_enum_uint8 en_resume_roaming;

    if (oal_any_null_ptr2(hmac_vap, scan_record)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result::device is null}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 暗屏不进行场景识别 */
    if (hmac_device->pst_device_base_info->uc_in_suspend == OAL_TRUE) {
        return OAL_SUCC;
    }

    if (mac_is_wep_enabled(&hmac_vap->st_vap_base_info)) {
        return OAL_SUCC;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL || roam_info->uc_enable == 0) {
        return OAL_SUCC;
    }

    scan_bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    roam_info->st_alg.uc_candidate_bss_num = 0;
    roam_info->st_alg.uc_candidate_good_rssi_num = 0;
    roam_info->st_alg.uc_candidate_weak_rssi_num = 0;
    roam_info->st_alg.c_max_rssi = ROAM_RSSI_CMD_TYPE;
    roam_info->st_alg.c_current_rssi = ROAM_RSSI_NE75_DB;  // in case current bss cannot scan

    oal_spin_lock(&(scan_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    oal_dlist_search_for_each(entry, &(scan_bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        hmac_roam_alg_bss_in_ess(roam_info, bss_dscr);

        bss_dscr = NULL;
    }

    oal_spin_unlock(&(scan_bss_mgmt->st_lock));

    /* 漫游场景识别1:密集AP场景/默认AP场景模式切换(风险:与扫描强相关,仅针对全信道扫描) */
    hmac_roam_scene1_process(hmac_vap, hmac_device, roam_info);

    /* 漫游场景识别2: 非漫游场景恢复，扫描到其他大于-80dB的AP立即恢复 */
    en_resume_roaming = (roam_info->st_alg.uc_another_bss_scaned) &&
                        (roam_info->st_alg.c_max_rssi >= ROAM_RSSI_NE80_DB) &&
                        (roam_info->st_alg.c_max_rssi - roam_info->st_alg.c_current_rssi > ROAM_RSSI_DIFF_4_DB);
    if ((roam_info->uc_rssi_type == ROAM_ENV_LINKLOSS) &&
        (en_resume_roaming == OAL_TRUE)) {
        return hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP_TH2);
    }

    /* 漫游场景识别3: 关闭漫游 */
    if (OAL_TRUE == hmac_roam_alg_need_to_stop_roam_trigger(roam_info)) {
        return hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_LINKLOSS);
    }

    /* 漫游场景识别4: 当前信号弱于-65dB，高概率(5次有4次)扫到比当前高30dB的AP即可触发漫游 */
    hmac_roam_scene4_process(hmac_vap, roam_info);

    /* 漫游场景识别5: 家庭双频路由器，关联2G AP，可以漫游到RSSI>=-50dB 5G AP，5G weight += 30dBm */
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE

OAL_STATIC uint32_t hmac_roam_11v_send_btm_resp(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_user_11v_ctrl_stru *ctrl_info_11v = NULL;

    hmac_vap = roam_info->pst_hmac_vap;
    if (oal_any_null_ptr2(roam_info, hmac_vap)) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_11v_send_btm_resp::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_11v_send_btm_resp::mac_res_get_hmac_user failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ctrl_info_11v = &(hmac_user->st_11v_ctrl_info);

    /* 找到目标AP, 更新BTM resp中携带目标AP的BSSID和Channel num */
    if (bss_dscr != NULL) {
        if (EOK != memcpy_s(roam_info->st_bsst_rsp_info.auc_target_bss_addr, WLAN_MAC_ADDR_LEN,
                            bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN)) {
            oam_warning_log0(0, OAM_SF_ROAM, "hmac_roam_11v_send_btm_resp::memcpy fail!");
            return OAL_FAIL;
        }
        roam_info->st_bsst_rsp_info.uc_chl_num = bss_dscr->st_channel.uc_chan_number;
    } else if (ctrl_info_11v->en_only_scan_one_time == OAL_FALSE &&
        ctrl_info_11v->uc_11v_roam_scan_times <= MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT) {
        /* bss_list_num只有一个时，策略是单信道扫描两次，全信道扫描一次，
         * 前面两次扫描不到时，不发送BTM resp reject帧，第三次才发送BTM resp帧 */
        return OAL_SUCC;
    }

    roam_info->st_bsst_rsp_info.uc_status_code =
        (bss_dscr == NULL) ? WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES : WNM_BSS_TM_ACCEPT;

    if (ctrl_info_11v->mac_11v_callback_fn == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "hmac_roam_11v_send_btm_resp::mac_11v_callback_fn is NULL");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_11v_send_btm_resp::start to send btm resp, status_code=%d}",
                     roam_info->st_bsst_rsp_info.uc_status_code);
    ctrl_info_11v->mac_11v_callback_fn(hmac_vap, hmac_user, &(roam_info->st_bsst_rsp_info));
    ctrl_info_11v->mac_11v_callback_fn = NULL;
    memset_s(&(roam_info->st_bsst_rsp_info), sizeof(roam_info->st_bsst_rsp_info),
        0, sizeof(roam_info->st_bsst_rsp_info));
    if (roam_info->st_bsst_rsp_info.uc_status_code != WNM_BSS_TM_ACCEPT) {
        roam_info->st_static.roam_11v_scan_fail++;
#ifdef PLATFORM_DEBUG_ENABLE
        hmac_config_reg_info(&(hmac_vap->st_vap_base_info), BSS_TRANSITION_RESP_MSG_LEN, "all");
#endif
    }
    roam_info->st_scan_h2d_params.st_scan_params.uc_bss_transition_process_flag = OAL_FALSE;
    return OAL_SUCC;
}
#endif


int8_t hmac_get_rssi_from_scan_result(hmac_vap_stru *hmac_vap, uint8_t *bssid)
{
    mac_bss_dscr_stru *bss_dscr = NULL;
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(&hmac_vap->st_vap_base_info, bssid);
    if (bss_dscr != NULL) {
        return bss_dscr->c_rssi;
    }

    return ROAM_RSSI_CMD_TYPE;
}


OAL_STATIC uint32_t hmac_roam_check_bss_in_scan_list(hmac_roam_info_stru *roam_info,
                                                     hmac_bss_mgmt_stru *bss_mgmt)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;

    if (oal_any_null_ptr2(roam_info, bss_mgmt)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_search_bss_in_scan_list::NULL pointer}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果扫描到的bss个数为0，退出 */
    if (bss_mgmt->bss_num == 0) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_search_bss_in_scan_list::no bss scanned}");
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    oal_spin_lock(&(bss_mgmt->st_lock));

    // 遍历扫描到的bss信息，查找可以漫游的bss
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        if (hmac_roam_alg_bss_check(roam_info, bss_dscr) != OAL_SUCC) {
            continue;
        }

        bss_dscr = NULL;
    }

    oal_spin_unlock(&(bss_mgmt->st_lock));
    return OAL_SUCC;
}

OAL_STATIC mac_bss_dscr_stru *hmac_roam_get_bss_dscr(hmac_roam_info_stru *roam_info, hmac_vap_stru *hmac_vap)
{
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint32_t ret;

    bss_dscr = hmac_roam_alg_select_bss(roam_info);
    if (bss_dscr == NULL) {
        /* 没有扫描到可用的bss，等待定时器超时即可 */
        roam_info->st_static.roam_scan_fail++;
        oam_warning_log1(0, OAM_SF_ROAM, "{roam_to::no bss valid fail=%d}", roam_info->st_static.roam_scan_fail);
#ifdef PLATFORM_DEBUG_ENABLE
        hmac_config_reg_info(&(hmac_vap->st_vap_base_info), 4, "all"); /* 4为"all"的长度 */
#endif
        hmac_vap->st_vap_base_info.bit_roam_scan_valid_rslt = OAL_FALSE;
        return bss_dscr;
    }

    ret = hmac_roam_renew_privacy(hmac_vap, bss_dscr);
    if (ret != OAL_SUCC) {
        return NULL;
    }

    ret = hmac_check_capability_mac_phy_supplicant(&(hmac_vap->st_vap_base_info), bss_dscr);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_get_bss_dscr::check mac and phy cap fail[%d]!}\r\n", ret);
    }

    return bss_dscr;
}

OAL_STATIC mac_bss_dscr_stru* hmac_roam_check_scan_result(hmac_roam_info_stru *roam_info,
                                                          void *param)
{
    uint32_t ret;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;

    hmac_vap = roam_info->pst_hmac_vap;
    bss_mgmt = (hmac_bss_mgmt_stru *)param;
    if (oal_any_null_ptr2(hmac_vap, bss_mgmt)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::vap invalid!}");
        return NULL;
    }

    roam_info->st_static.scan_result_cnt++;

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (ROAM_TRIGGER_11V == roam_info->en_roam_trigger) {
        /* 用最新的扫描结果更新当前关联AP的RSSI */
        roam_info->st_alg.c_current_rssi =
            hmac_get_rssi_from_scan_result(hmac_vap, hmac_vap->st_vap_base_info.auc_bssid);

        roam_info->st_bsst_rsp_info.c_rssi = roam_info->st_alg.c_current_rssi;
    }
#endif

    ret = hmac_roam_check_bss_in_scan_list(roam_info, bss_mgmt);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::hmac_roam_search_bss_in_scan_list() failed}");
        return NULL;
    }

    bss_dscr = hmac_roam_get_bss_dscr(roam_info, hmac_vap);

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    /* 发送携带状态码的 BTM resp */
    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        if (hmac_roam_11v_send_btm_resp(roam_info, bss_dscr) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::send btm resp failed}");
        }
    }
#endif

    if (bss_dscr == NULL) {
        hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_SCAN_FAIL);
        /* 防止一直选网失败，每3次选网失败关闭TH1门限漫游 */
        if ((roam_info->st_static.roam_scan_fail > 0 &&
            roam_info->st_static.roam_scan_fail % ROAM_SCAN_SPECIFIC_CHAN_NUM == 0) &&
            (roam_info->uc_rssi_type == ROAM_ENV_SPARSE_AP_TH1)) {
            hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP_TH2);
        }
    }

    return bss_dscr;
}

#ifdef _PRE_WINDOWS_SUPPORT
hmac_roam_info_context g_roam_info_context;

static void hmac_roam_info_context_clear()
{
    memset_s(&g_roam_info_context.roam_info, sizeof(hmac_roam_info_stru), 0, sizeof(hmac_roam_info_stru));
    memset_s(&g_roam_info_context.bss_dscr, sizeof(mac_bss_dscr_stru), 0, sizeof(mac_bss_dscr_stru));
    g_roam_info_context.scan_timeout = OAL_FALSE;
    return;
}


void hmac_roam_info_save_context(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    memcpy_s(&g_roam_info_context.roam_info, sizeof(hmac_roam_info_stru), roam_info, sizeof(hmac_roam_info_stru));
    memcpy_s(&g_roam_info_context.bss_dscr, sizeof(mac_bss_dscr_stru), bss_dscr, sizeof(mac_bss_dscr_stru));
    g_roam_info_context.scan_timeout = OAL_FALSE;
    return;
}


static void hmac_roam_debug_bss_info(mac_bss_dscr_stru *bss_dscr)
{
    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_debug_bss_info bssid=[%02X:%02X]}",
        bss_dscr->auc_bssid[4], bss_dscr->auc_bssid[5]);

    oam_warning_log4(0, OAM_SF_ROAM, "{band=%d, chan_number=%d, bw=%d, chan_idx=%d.}",
        bss_dscr->st_channel.en_band, bss_dscr->st_channel.uc_chan_number,
        bss_dscr->st_channel.en_bandwidth, bss_dscr->st_channel.uc_chan_idx);

    return;
}


uint32_t hmac_config_roam_task_indicate(uint8_t flag)
{
    uint32_t ret;
    if (flag == OAL_FALSE) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_config_roam_task_indicate::windows indicate wrong flag!}");
        return OAL_FAIL;
    }

    oam_warning_log0(0, OAM_SF_ROAM, "{Enter hmac_config_roam_task_indicate!}");
    hmac_roam_debug_bss_info(&g_roam_info_context.bss_dscr);

    // 超时下发，驱动重置roam fsm且返回漫游失败
    if (g_roam_info_context.scan_timeout == OAL_TRUE) {
        hmac_roam_main_clear(&g_roam_info_context.roam_info);
        hmac_roam_info_context_clear();
        return OAL_FAIL;
    }
    ret = hmac_roam_main_fsm_action(&g_roam_info_context.roam_info, ROAM_MAIN_FSM_EVENT_START_CONNECT,
                                    (void *)(&g_roam_info_context.bss_dscr));
    if (ret != OAL_SUCC) {
        hmac_roam_main_clear(&g_roam_info_context.roam_info);
        hmac_roam_info_context_clear();
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_task_timeout_indicate(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    // 置位超时标记
    g_roam_info_context.scan_timeout = OAL_TRUE;
    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_task_timeout_indicate::roam task timeout!}");

    return OAL_SUCC;
}


static void hmac_roam_task_timeout(hmac_roam_info_stru *roam_info, uint32_t timeout)
{
    frw_timeout_stru *timer = &(roam_info->st_timer);

    /* 启动roam task超时定时器 */
    frw_timer_create_timer_m(timer,
                             hmac_roam_task_timeout_indicate,
                             timeout,
                             roam_info,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             roam_info->pst_hmac_vap->st_vap_base_info.ul_core_id);
}


void hmac_roam_handle_scan_result_windows(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    PADAPTER adapter =
        ((PADAPTER)(roam_info->pst_hmac_vap->pst_net_device->pAdapter))->pPortCommonInfo->pDefaultAdapter;
    if (adapter == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result_windows: null ptr!}");
        return;
    }

    PMGNT_INFO mgntInfo = &(adapter->MgntInfo);
    memcpy_s(mgntInfo->roamBssid, WLAN_MAC_ADDR_LEN, bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN);
    DrvIFIndicateRoamingNeeded(adapter, RT_PREPARE_ROAM_NORMAL_ROAM_BETTER_AP);

    oam_warning_log0(0, OAM_SF_ROAM, "{Enter hmac_roam_handle_scan_result_windows!}");
    hmac_roam_debug_bss_info(bss_dscr);
    hmac_roam_info_save_context(roam_info, bss_dscr);
    // 启动ROAM_TASK超时定时器
    hmac_roam_task_timeout(roam_info, ROAM_TASK_INDICATE_TIME_MAX);
    return;
}
#endif


uint32_t hmac_roam_handle_scan_result(hmac_roam_info_stru *roam_info, void *param)
{
    mac_bss_dscr_stru *bss_dscr = NULL;
    hmac_scan_rsp_stru scan_rsp;
    mac_device_stru *mac_dev = NULL;

    if (hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::check_state fail!}");
        return OAL_FAIL;
    }

    mac_dev = mac_res_get_dev(roam_info->pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::mac_dev null.}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    bss_dscr = hmac_roam_check_scan_result(roam_info, param);
    if (bss_dscr != NULL) {
        roam_info->uc_invalid_scan_cnt = 0;
        roam_info->st_static.scan_end_timetamp = (uint32_t)oal_time_get_stamp_ms();
#ifdef _PRE_WINDOWS_SUPPORT
        hmac_roam_handle_scan_result_windows(roam_info, bss_dscr);
        return OAL_SUCC;
#else
        /* 扫描结果发给sme */
        memset_s(&scan_rsp, sizeof(scan_rsp), 0, sizeof(scan_rsp));
        scan_rsp.uc_result_code = MAC_SCAN_SUCCESS;
        hmac_send_rsp_to_sme_sta(roam_info->pst_hmac_vap, HMAC_SME_SCAN_RSP, (uint8_t *)&scan_rsp);
        memcpy_s(&roam_info->save_bss_dscr_info, sizeof(mac_bss_dscr_stru), bss_dscr, sizeof(mac_bss_dscr_stru));
        /* 扫描到可用的bss，开始connect */
        return hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START_CONNECT,
            (void *)&roam_info->save_bss_dscr_info);
#endif
    }

    /* 如果是亮屏的，不暂停漫游 */
    roam_info->uc_invalid_scan_cnt = (mac_dev->uc_in_suspend == OAL_FALSE) ? 0 : roam_info->uc_invalid_scan_cnt + 1;

    /* 多次无效扫描暂停漫游，防止在某些场景下一直唤醒HOST */
    if (roam_info->uc_invalid_scan_cnt >= ROAM_INVALID_SCAN_MAX) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::ignore_rssi_trigger after %d invalid_scan.}",
                         roam_info->uc_invalid_scan_cnt);
        roam_info->uc_invalid_scan_cnt = 0;
        hmac_roam_rssi_trigger_type(roam_info->pst_hmac_vap, ROAM_ENV_LINKLOSS);
    }
    /* 删除定时器 */
    hmac_roam_main_del_timer(roam_info);
    hmac_roam_main_clear(roam_info);
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    /* 重新出发漫游检查 */
    if (roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        hmac_11v_roam_scan_check(roam_info->pst_hmac_vap);
    }
#endif
    return OAL_SUCC;
}


uint32_t hmac_roam_main_check_state(hmac_roam_info_stru *roam_info, mac_vap_state_enum_uint8 vap_state,
    roam_main_state_enum main_state)
{
    if (roam_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->pst_hmac_user == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    if (roam_info->uc_enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if ((roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state != vap_state) ||
        (roam_info->en_main_state != main_state)) {
        oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_main_check_state::unexpect vap State[%d] main_state[%d]!}",
                         roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state, roam_info->en_main_state);
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    return OAL_SUCC;
}

void hmac_roam_main_clear(hmac_roam_info_stru *roam_info)
{
    /* 清理状态 */
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_INIT);

    hmac_roam_connect_stop(roam_info->pst_hmac_vap);
}


uint32_t hmac_roam_resume_pm(hmac_roam_info_stru *roam_info)
{
    uint32_t ret;

    ret = hmac_config_set_pm_by_module(&roam_info->pst_hmac_vap->st_vap_base_info,
                                       MAC_STA_PM_CTRL_TYPE_ROAM,
                                       MAC_STA_PM_SWITCH_ON);
    if (ret != OAL_SUCC) {
        oam_warning_log1(roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_roam_resume_pm::hmac_config_set_pm_by_module failed[%d].}", ret);
    }

    return ret;
}


void hmac_roam_resume_security_port(hmac_roam_info_stru *roam_info)
{
    uint32_t ret;
    mac_h2d_roam_sync_stru h2d_sync = { 0 };

    if (roam_info->pst_hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::vap null!}");
        return;
    }

    if (roam_info->pst_hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::user null!}");
        return;
    }

    /* 设置用户8021x端口合法性的状态为合法 */
    mac_user_set_port(&roam_info->pst_hmac_user->st_user_base_info, OAL_TRUE);

    // 填充同步信息
    h2d_sync.back_to_old = OAL_FALSE;

    // 发送同步信息
    ret = hmac_config_send_event(&roam_info->pst_hmac_vap->st_vap_base_info,
                                 WLAN_CFGID_ROAM_HMAC_SYNC_DMAC,
                                 sizeof(mac_h2d_roam_sync_stru),
                                 (uint8_t *)&h2d_sync);
    if (ret != OAL_SUCC) {
        oam_error_log1(roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_resume_security_port::send event[WLAN_CFGID_ROAM_HMAC_SYNC_DMAC] failed[%d].}", ret);
    }

    return;
}


uint32_t hmac_roam_scan_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    ret = hmac_roam_scan_complete(roam_info->pst_hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::scan_complete fail[%d]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}
