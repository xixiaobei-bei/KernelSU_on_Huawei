

#include "oal_cfg80211.h"
#include "mac_regdomain.h"
#include "hmac_main.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_p2p.h"
#include "hmac_dfx.h"
#include "mac_mib.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#include "hisi_customize_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SME_STA_C

/*****************************************************************************
    g_handle_rsp_func_sta: 处理发给sme的消息
*****************************************************************************/
OAL_STATIC hmac_sme_handle_rsp_func g_handle_rsp_func_sta[HMAC_SME_RSP_BUTT] = {
    hmac_handle_scan_rsp_sta,
    hmac_handle_join_rsp_sta,
    hmac_handle_auth_rsp_sta,
    hmac_handle_asoc_rsp_sta,
};

OAL_STATIC void hmac_cfg80211_scan_comp_cb(void *scan_record);
static void hmac_send_connect_result_to_dmac_sta(hmac_vap_stru *hmac_vap, uint32_t result);

static uint8_t hmac_prepare_scan_channel(mac_scan_req_stru *scan_params, uint8_t max_channel,
    uint8_t band, uint8_t offset)
{
    uint8_t chan_idx, chan_number;
    uint32_t ret;
    uint8_t chan_num = 0;
    for (chan_idx = 0; chan_idx < max_channel; chan_idx++) {
        /* 判断信道是不是在管制域内, 初始扫描由于FPGA还不支持6G扫描，目前只扫2G，5G */
        ret = mac_is_channel_idx_valid(band, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(band, chan_idx, OAL_FALSE, &chan_number);

            scan_params->ast_channel_list[chan_num + offset].uc_chan_number = chan_number;
            scan_params->ast_channel_list[chan_num + offset].en_band = band;
            scan_params->ast_channel_list[chan_num + offset].uc_chan_idx = chan_idx;
            scan_params->ast_channel_list[chan_num + offset].ext6g_band = OAL_FALSE;
            scan_params->uc_channel_nums++;
            chan_num++;
        }
    }
    return chan_num;
}


OAL_STATIC void hmac_prepare_scan_req(mac_scan_req_stru *scan_params, int8_t *desired_ssid)
{
    uint8_t chan_num_2g, chan_num_5g;

    /* 设置初始扫描请求的参数 */
    scan_params->en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->en_scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_params->us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    scan_params->uc_probe_delay = 0;
    scan_params->uc_scan_func = MAC_SCAN_FUNC_BSS;     /* 默认扫描bss */
    scan_params->p_fn_cb = hmac_cfg80211_scan_comp_cb; /* 使用cfg80211接口回调，因为ut/dmt都跟这个接口耦合 */
    scan_params->uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;

    /* 设置初始扫描请求的ssid */
    /* 初始扫描请求只指定1个ssid, 要么是通配, 要么是mib值 */
    if (desired_ssid[0] == '\0') {
        scan_params->ast_mac_ssid_set[0].auc_ssid[0] = '\0'; /* 通配ssid */
    } else {
        /* copy包括字符串结尾的0 */
        if (memcpy_s(scan_params->ast_mac_ssid_set[0].auc_ssid, WLAN_SSID_MAX_LEN,
                     desired_ssid, OAL_STRLEN(desired_ssid) + 1) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "hmac_prepare_scan_req::memcpy fail!");
            return;
        }
    }

    scan_params->uc_ssid_num = 1;

    /* 设置初始扫描请求的bssid */
    /* 初始扫描请求只指定1个bssid，为广播地址 */
    oal_set_mac_addr(scan_params->auc_bssid[0], BROADCAST_MACADDR);
    scan_params->uc_bssid_num = 1;

    /* 2G初始扫描信道, 全信道扫描 */
    chan_num_2g = hmac_prepare_scan_channel(scan_params, MAC_CHANNEL_FREQ_2_BUTT, WLAN_BAND_2G, 0);
    oam_info_log1(0, OAM_SF_SCAN, "{hmac_prepare_scan_req::after regdomain filter, the 2g total channel num is %d",
                  chan_num_2g);

    /* 5G初始扫描信道, 全信道扫描 */
    chan_num_5g = hmac_prepare_scan_channel(scan_params, MAC_CHANNEL_FREQ_5_BUTT, WLAN_BAND_5G, chan_num_2g);
    oam_info_log1(0, OAM_SF_SCAN, "{hmac_prepare_scan_req::after regdomain filter, the 5g total channel num is %d",
                  chan_num_5g);
}


int32_t hmac_cfg80211_dump_survey(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
                                  int32_t idx, oal_survey_info_stru *info)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_device_stru *hmac_dev = NULL;
    wlan_scan_chan_stats_stru *record = NULL;

    if (oal_any_null_ptr3(wiphy, netdev, info) || idx < 0) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{hmac_cfg80211_dump_survey::wiphy or netdev or info is null or l_idx[%d]<0!}", idx);
        return -OAL_EINVAL;
    }

    memset_s(info, sizeof(oal_survey_info_stru), 0, sizeof(oal_survey_info_stru));

    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey::mac_vap = NULL}");
        return -OAL_EINVAL;
    }

    hmac_dev = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_dev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey::hmac_dev = NULL}");
        return -OAL_EINVAL;
    }

    if (hmac_dev->st_scan_mgmt.en_is_scanning) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey::drop request while scan running}");
        return -OAL_EINVAL;
    }

    record = hmac_dev->st_scan_mgmt.st_scan_record_mgmt.ast_chan_results;

    if (idx >= hmac_dev->st_scan_mgmt.st_scan_record_mgmt.uc_chan_numbers) {
        return -OAL_ENOENT;
    }
    info->time = record[idx].total_stats_time_us / HMAC_S_TO_MS;
    info->time_busy = (record[idx].total_stats_time_us -
                       record[idx].total_free_time_20M_us) / HMAC_S_TO_MS;
    info->time_ext_busy = 0;
    info->time_rx = record[idx].total_recv_time_us / HMAC_S_TO_MS;
    info->time_tx = record[idx].total_send_time_us / HMAC_S_TO_MS;

    info->filled = SURVEY_INFO_TIME | SURVEY_INFO_TIME_BUSY |
                   SURVEY_INFO_TIME_RX | SURVEY_INFO_TIME_TX;

    if (record[idx].uc_free_power_cnt && record[idx].s_free_power_stats_20M < 0) {
        info->noise = record[idx].s_free_power_stats_20M / record[idx].uc_free_power_cnt;
        info->filled |= SURVEY_INFO_NOISE_DBM;
    }

    info->channel = oal_ieee80211_get_channel(wiphy,
        oal_ieee80211_channel_to_frequency(record[idx].uc_channel_number,
            mac_get_band_by_channel_num(record[idx].uc_channel_number)));
    return 0;
}


OAL_STATIC void hmac_cfg80211_scan_comp_cb(void *record)
{
    hmac_scan_record_stru *scan_record = (hmac_scan_record_stru *)record;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_scan_rsp_stru scan_rsp = {0};

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_scan_comp_cb::hmac_vap is null.");
        return;
    }

    /* 上报扫描结果结构体初始化 */
    memset_s(&scan_rsp, sizeof(scan_rsp), 0, sizeof(scan_rsp));
    scan_rsp.uc_result_code = scan_record->en_scan_rsp_status;

    /* 扫描结果发给sme */
    hmac_send_rsp_to_sme_sta(hmac_vap, HMAC_SME_SCAN_RSP, (uint8_t *)&scan_rsp);

    return;
}

OAL_STATIC void hmac_prepare_scan_req_sta_set_scan_cnt(mac_vap_stru *mac_vap,
    mac_scan_req_stru *scan_params, mac_cfg80211_scan_param_stru *cfg80211_scan_param,
    oal_bool_enum_uint8 scan_when_go_up)
{
    if ((cfg80211_scan_param->l_ssid_num <= 1) && (scan_when_go_up == OAL_TRUE ||
         mac_vap->en_vap_state == MAC_VAP_STATE_UP || mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE ||
         (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && mac_vap->us_user_nums > 0))) {
        scan_params->uc_max_scan_count_per_channel = 1;
    }
}

static void hmac_cfg80211_prepare_scan_params(mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    scan_params->en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->en_scan_type = cfg80211_scan_param->en_scan_type;

    /* 设置扫描时间 */
    scan_params->us_scan_time = (scan_params->en_scan_type == WLAN_SCAN_TYPE_ACTIVE) ?
        WLAN_DEFAULT_ACTIVE_SCAN_TIME : WLAN_DEFAULT_PASSIVE_SCAN_TIME;

    scan_params->uc_probe_delay = 0;
    scan_params->uc_scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
    scan_params->uc_scan_func |= MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
#endif
    scan_params->p_fn_cb = hmac_cfg80211_scan_comp_cb; /* 扫描完成回调函数 */
    scan_params->uc_max_scan_count_per_channel = WLAN_MAX_SCAN_COUNT_PER_CHANNEL;
}
#define MAC_MAX_CHANNEL_NUM_6G_E 95 /* 5.925G~6.425G最大的信道号 */
static void hmac_cfg80211_prepare_scan_channel(mac_vap_stru *mac_vap, mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param, uint8_t band, uint8_t is_6ghz)
{
    uint8_t chan_num = 0;    /* 信道数量 */
    uint8_t loop;
    uint8_t max_num_channel = 0;
    uint8_t channel_number;  /* 信道号 */
    uint8_t channel_idx = 0; /* 信道号索引 */
    uint32_t *channels = NULL;
    uint8_t offset = scan_params->uc_channel_nums;

    if (band == WLAN_BAND_2G) {
        max_num_channel = cfg80211_scan_param->num_channels_2g;
        channels = cfg80211_scan_param->channels_2g;
    } else {
        if (is_6ghz == OAL_FALSE) {
            max_num_channel = cfg80211_scan_param->num_channels_5g;
            channels = cfg80211_scan_param->channels_5g;
        } else {
            if ((mac_device_band_is_support(mac_vap->uc_device_id, MAC_DEVICE_CAP_6G_LOW_BAND) != OAL_TRUE) &&
                (mac_device_band_is_support(mac_vap->uc_device_id, MAC_DEVICE_CAP_6G_ALL_BAND) != OAL_TRUE)) {
                return;
            }
            max_num_channel = cfg80211_scan_param->num_channels_6g;
            channels = cfg80211_scan_param->channels_6g;
        }
    }
    for (loop = 0; loop < max_num_channel; loop++) {
        channel_number = (uint8_t)channels[loop];
        /* 如果当前板级只支持6.4G频段则不再添加后续信道信息 */
        if ((mac_device_band_is_support(mac_vap->uc_device_id, MAC_DEVICE_CAP_6G_LOW_BAND) == OAL_TRUE) &&
            (channel_number > MAC_MAX_CHANNEL_NUM_6G_E)) {
            return;
        }
        /* 判断信道是不是在管制域内 */
        if (mac_is_channel_num_valid(band, channel_number, is_6ghz) != OAL_SUCC) {
            continue;
        }
        if (mac_get_channel_idx_from_num(band, channel_number, is_6ghz, &channel_idx) != OAL_SUCC) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_cfg80211_prepare_scan_channel:: \
                get channel idx fail, band:%d, channel:%d}", band, channel_number);
        }

        scan_params->ast_channel_list[chan_num + offset].en_band = band;
        scan_params->ast_channel_list[chan_num + offset].uc_chan_number = channel_number;
        scan_params->ast_channel_list[chan_num + offset].uc_chan_idx = channel_idx;
        scan_params->ast_channel_list[chan_num + offset].ext6g_band = (is_6ghz == OAL_TRUE ? OAL_TRUE : OAL_FALSE);
        scan_params->uc_channel_nums++;
        chan_num++;
    }
}

static void hmac_cfg80211_set_scan_params_part(mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    scan_params->uc_max_send_probe_req_count_per_channel = (uint8_t)cfg80211_scan_param->l_ssid_num;

    if (scan_params->uc_max_send_probe_req_count_per_channel == 0) {
        scan_params->ast_mac_ssid_set[0].auc_ssid[0] = '\0'; /* 通配ssid */
        scan_params->uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    }

    if ((scan_params->uc_max_send_probe_req_count_per_channel > 3) && /* 每次信道发送扫描请求帧的个数大于3个 */
        (scan_params->en_scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
        /* 如果指定SSID个数大于3个,则调整发送超时时间为40ms,默认发送等待超时20ms */
        scan_params->us_scan_time = WLAN_LONG_ACTIVE_SCAN_TIME;
    }

    scan_params->uc_ssid_num = (uint8_t)cfg80211_scan_param->l_ssid_num;
}


OAL_STATIC void hmac_cfg80211_prepare_scan_req_sta(mac_vap_stru *mac_vap,
    mac_scan_req_stru *scan_params, mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    uint8_t loop;
    uint8_t ssid_len = 0;
    mac_device_stru *mac_device = NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_vap_stru *mac_vap_temp = NULL;
#endif
    uint32_t ret;
    oal_bool_enum_uint8 scan_when_go_up = OAL_FALSE;

    hmac_cfg80211_prepare_scan_params(scan_params, cfg80211_scan_param);

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* p2p Go发起扫描时，会使用p2p device设备进行 */
    if (mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        ret = mac_device_find_up_p2p_go(mac_device, &mac_vap_temp);
        if ((ret == OAL_SUCC) && (mac_vap_temp != NULL)) {
            scan_when_go_up = OAL_TRUE;
        }
    }
#endif /* _PRE_WLAN_FEATURE_P2P */
    // 降低圈复杂度封装
    hmac_prepare_scan_req_sta_set_scan_cnt(mac_vap, scan_params, cfg80211_scan_param, scan_when_go_up);

    for (loop = 0; loop < cfg80211_scan_param->l_ssid_num; loop++) {
        if (memcpy_s(scan_params->ast_mac_ssid_set[loop].auc_ssid, WLAN_SSID_MAX_LEN,
            cfg80211_scan_param->st_ssids[loop].auc_ssid, cfg80211_scan_param->st_ssids[loop].uc_ssid_len) != EOK) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_cfg80211_prepare_scan_req_sta::memcpy fail}");
            return;
        }
        /* ssid末尾置'\0' */
        ssid_len = cfg80211_scan_param->st_ssids[loop].uc_ssid_len;
        scan_params->ast_mac_ssid_set[loop].auc_ssid[ssid_len] = '\0';
    }

    /* 如果上层下发了指定ssid，则每次扫描发送的probe req帧的个数为下发的ssid个数 */
    hmac_cfg80211_set_scan_params_part(scan_params, cfg80211_scan_param);

    oal_set_mac_addr(scan_params->auc_bssid[0], BROADCAST_MACADDR);
    scan_params->uc_bssid_num = 1; /* 初始扫描请求指定1个bssid，为广播地址 */

    /* 2G初始扫描信道 */
    hmac_cfg80211_prepare_scan_channel(mac_vap, scan_params, cfg80211_scan_param, WLAN_BAND_2G, OAL_FALSE);
    /* 5G初始扫描信道 */
    hmac_cfg80211_prepare_scan_channel(mac_vap, scan_params, cfg80211_scan_param, WLAN_BAND_5G, OAL_FALSE);
    /* 6G初始扫描信道 */
    hmac_cfg80211_prepare_scan_channel(mac_vap, scan_params, cfg80211_scan_param, WLAN_BAND_5G, OAL_TRUE);
    /* WLAN/P2P 共存时，判断是否p2p0 发起扫描 */
    scan_params->bit_is_p2p0_scan = cfg80211_scan_param->bit_is_p2p0_scan;
    if (cfg80211_scan_param->bit_is_p2p0_scan) {
        scan_params->en_bss_type = 0;
    }
}


void hmac_prepare_join_req(hmac_join_req_stru *join_req, mac_bss_dscr_stru *bss_dscr)
{
    memset_s(join_req, sizeof(hmac_join_req_stru), 0, sizeof(hmac_join_req_stru));
    memcpy_s(&(join_req->st_bss_dscr), sizeof(mac_bss_dscr_stru), bss_dscr, sizeof(mac_bss_dscr_stru));
}


OAL_STATIC void hmac_prepare_auth_req(hmac_vap_stru *hmac_vap, hmac_auth_req_stru *auth_req)
{
    memset_s(auth_req, sizeof(hmac_auth_req_stru), 0, sizeof(hmac_auth_req_stru));
    if ((mac_mib_get_AuthenticationMode(&(hmac_vap->st_vap_base_info)) == WLAN_WITP_AUTH_SAE ||
        mac_mib_get_AuthenticationMode(&(hmac_vap->st_vap_base_info)) == WLAN_WITP_AUTH_TBPEKE) &&
        (hmac_vap->bit_sae_connect_with_pmkid == OAL_FALSE)) {
        /*
         * SAE auth timeout time increase from 150ms to 300ms/600ms for compatibility, such as 1151 AP
         * The processing capability of some IoT devices is poor. Therefore, the auth timeout period needs to be
         * changed to 1.2s.
         */
        auth_req->us_timeout =
            (uint16_t)mac_mib_get_AuthenticationResponseTimeOut(&(hmac_vap->st_vap_base_info)) * 4; /* 4*300ms等1.2s */
    } else {
        /* 增加AUTH报文次数后，将超时时间减半 */
        auth_req->us_timeout =
            (uint16_t)mac_mib_get_AuthenticationResponseTimeOut(&(hmac_vap->st_vap_base_info)) >> 1;
    }
}


OAL_STATIC void hmac_prepare_asoc_req(mac_vap_stru *mac_vap, hmac_asoc_req_stru *asoc_req)
{
    memset_s(asoc_req, sizeof(hmac_asoc_req_stru), 0, sizeof(hmac_asoc_req_stru));
    /* 增加ASOC报文次数后，将超时时间减半 */
    asoc_req->us_assoc_timeout = (uint16_t)mac_mib_get_dot11AssociationResponseTimeOut(mac_vap) >> 1;
}


uint32_t hmac_sta_initiate_scan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_scan_req_stru *scan_params = NULL;
    mac_scan_req_h2d_stru scan_h2d_params;
    hmac_vap_stru *hmac_vap = NULL;
    int8_t *desired_ssid = NULL;

    mac_device_stru *mac_device = NULL;
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_initial_scan: mac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_sta_initiate_scan::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_sta_initiate_scan::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    desired_ssid = (int8_t *)mac_mib_get_DesiredSSID(mac_vap);
    memset_s(&scan_h2d_params, sizeof(mac_scan_req_h2d_stru), 0, sizeof(mac_scan_req_h2d_stru));
    scan_params = &(scan_h2d_params.st_scan_params);
    hmac_prepare_scan_req(scan_params, desired_ssid);
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_sta_initial_scan: mac_device is null device id[%d].}",
                       mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    scan_params->bit_is_p2p0_scan = param[0];
    /* 记录扫描参数信息到device结构体 */
    memcpy_s(&(mac_device->st_scan_params), sizeof(mac_scan_req_stru),
             scan_params, sizeof(mac_scan_req_stru));
    /* 状态机调用: hmac_scan_proc_scan_req_event */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_SCAN_REQ, (void *)(&scan_h2d_params));
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_sta_initiate_scan::hmac_fsm_call_func_sta fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_cfg80211_check_can_start_sched_scan(mac_vap_stru *mac_vap)
{
    hmac_device_stru *hmac_device = NULL;
    mac_vap_stru *mac_vap_tmp = NULL;
    uint32_t ret;

    /* 获取mac device */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_check_can_start_sched_scan::hmac_device[%d] null.}",
                         mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果vap的模式不是STA，则返回，不支持其它模式的vap的调度扫描 */
    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_check_can_start_sched_scan::vap mode[%d] don't support sched scan.}",
                         mac_vap->en_vap_mode);
        hmac_device->st_scan_mgmt.pst_sched_scan_req = NULL;
        hmac_device->st_scan_mgmt.en_sched_scan_complete = OAL_TRUE;
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 如果存在当前device存在up的vap，则不启动调度扫描 */
    ret = mac_device_find_up_vap(hmac_device->pst_device_base_info, &mac_vap_tmp);
    if (ret == OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_check_can_start_sched_scan::exist up vap, don't start sched scan.}");
        hmac_device->st_scan_mgmt.pst_sched_scan_req = NULL;
        hmac_device->st_scan_mgmt.en_sched_scan_complete = OAL_TRUE;
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    return OAL_SUCC;
}

static uint32_t hmac_cfg80211_start_scan(mac_vap_stru *mac_vap, mac_pno_scan_stru pno_scan_params,
                                         hmac_vap_stru *hmac_vap)
{
    oal_bool_enum_uint8 is_random_mac_addr_scan;
    uint32_t ret;
    hmac_device_stru *hmac_device = NULL;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_start_scan::device id[%d],hmac_device null.}",
                         hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 设置PNO调度扫描结束时，如果有结果上报，则上报扫描结果的回调函数 */
    pno_scan_params.p_fn_cb = hmac_cfg80211_scan_comp_cb;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    is_random_mac_addr_scan = g_wlan_cust.uc_random_mac_addr_scan;
#else
    is_random_mac_addr_scan = hmac_device->st_scan_mgmt.en_is_random_mac_addr_scan;
#endif

    /* 设置发送的probe req帧中源mac addr */
    pno_scan_params.en_is_random_mac_addr_scan = is_random_mac_addr_scan;
    hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap, pno_scan_params.auc_sour_mac_addr,
                                             is_random_mac_addr_scan, OAL_FALSE);
    /* 状态机调用: hmac_scan_proc_scan_req_event */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_SCHED_SCAN_REQ, (void *)(&pno_scan_params));
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_start_scan::hmac_fsm_call_func_sta fail[%d].}", ret);
        hmac_device->st_scan_mgmt.pst_sched_scan_req = NULL;
        hmac_device->st_scan_mgmt.en_sched_scan_complete = OAL_TRUE;
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_cfg80211_start_sched_scan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_pno_scan_stru *cfg80211_pno_scan_params = NULL;
    mac_pno_scan_stru pno_scan_params;
    uint32_t ret;

    /* 参数合法性检查 */
    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_start_sched_scan::input null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上层传的是地址，因此需要取值获取到真正的pno扫描参数所在的地址 */
    cfg80211_pno_scan_params = (mac_pno_scan_stru *)(uintptr_t)(*(unsigned long *)param);

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_start_sched_scan::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 拷贝上层下发的pno扫描参数 */
    memcpy_s(&pno_scan_params, sizeof(mac_pno_scan_stru),
             cfg80211_pno_scan_params, sizeof(mac_pno_scan_stru));

    /* 检测当前device是否可以启动调度扫描 */
    ret = hmac_cfg80211_check_can_start_sched_scan(mac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 启动扫描 */
    ret = hmac_cfg80211_start_scan(mac_vap, pno_scan_params, hmac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_cfg80211_stop_sched_scan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_STOP_SCHED_SCAN, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_cfg80211_stop_sched_scan::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC void hmac_free_scan_param_resource(mac_cfg80211_scan_param_stru *scan_param)
{
    if (scan_param->channels_2g != NULL) {
        oal_free(scan_param->channels_2g);
        scan_param->channels_2g = NULL;
    }
    if (scan_param->channels_5g != NULL) {
        oal_free(scan_param->channels_5g);
        scan_param->channels_5g = NULL;
    }
    if (scan_param->puc_ie != NULL) {
        oal_free(scan_param->puc_ie);
        scan_param->puc_ie = NULL;
    }
}

OAL_STATIC void hmac_cfg80211_free_scan_param_resource(mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    /* 因为扫描命令下发修改为非阻塞方式，因此，信道申请的内存需要再此处释放 */
    hmac_free_scan_param_resource(cfg80211_scan_param);
}

OAL_STATIC uint32_t hmac_cfg80211_start_scan_fail(mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    hmac_cfg80211_free_scan_param_resource(cfg80211_scan_param);
    return OAL_ERR_CODE_PTR_NULL;
}

OAL_STATIC void hmac_cfg80211_pre_start_scan(mac_scan_req_stru *scan_params, mac_vap_stru *mac_vap,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    hmac_cfg80211_prepare_scan_req_sta(mac_vap, scan_params, cfg80211_scan_param);
    /* 设置P2P/WPS IE 信息到 vap 结构体中 */
    if (IS_LEGACY_VAP(mac_vap)) {
        hmac_config_del_p2p_ie((uint8_t *)(cfg80211_scan_param->puc_ie),
                               &(cfg80211_scan_param->ie_len));
    }
    /* P2P0 扫描时记录P2P listen channel */
    if (cfg80211_scan_param->bit_is_p2p0_scan == OAL_TRUE) {
        hmac_find_p2p_listen_channel(mac_vap, (uint16_t)(cfg80211_scan_param->ie_len),
                                     (uint8_t *)(cfg80211_scan_param->puc_ie));
        scan_params->uc_p2p0_listen_channel = mac_vap->uc_p2p_listen_channel;
    }
}

OAL_STATIC mac_cfg80211_scan_param_stru* hmac_cfg80211_get_scan_param(mac_vap_stru *mac_vap, uint8_t *param)
{
    mac_cfg80211_scan_param_stru *cfg80211_scan_param = NULL;
    if (param == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_get_scan_param::scan failed, param is null.}");
        return NULL;
    }

    cfg80211_scan_param = (mac_cfg80211_scan_param_stru *)param;
    if (cfg80211_scan_param->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_get_scan_param::cfg80211 scan param ie_len[%d] error.}",
                         cfg80211_scan_param->ie_len);
        hmac_cfg80211_free_scan_param_resource(cfg80211_scan_param);
        return NULL;
    }
    return cfg80211_scan_param;
}


uint32_t hmac_cfg80211_start_scan_sta(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_scan_req_stru *scan_params = NULL;
    mac_scan_req_h2d_stru scan_h2d_params;
    oal_app_ie_stru *app_ie = NULL;
    uint32_t ret;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    mac_cfg80211_scan_param_stru *cfg80211_scan_param = hmac_cfg80211_get_scan_param(mac_vap, param);
    if (cfg80211_scan_param == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_unlikely(oal_any_null_ptr2(mac_device, hmac_vap))) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_start_scan_sta::scan failed, mac_device or hmac vap is null.}");
        return hmac_cfg80211_start_scan_fail(cfg80211_scan_param);
    }

    /* 初始扫描列表清0 */
    memset_s(&scan_h2d_params, sizeof(mac_scan_req_h2d_stru), 0, sizeof(mac_scan_req_h2d_stru));

    /* 将内核下发的扫描参数更新到驱动扫描请求结构体中 */
    scan_params = &(scan_h2d_params.st_scan_params);
    hmac_cfg80211_pre_start_scan(scan_params, mac_vap, cfg80211_scan_param);

    app_ie = (oal_app_ie_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_app_ie_stru), OAL_TRUE);
    if (oal_unlikely(app_ie == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_start_scan_sta::scan failed, oal_memalloc failed.}");
        return hmac_cfg80211_start_scan_fail(cfg80211_scan_param);
    }

    app_ie->ie_len = cfg80211_scan_param->ie_len;
    if (app_ie->ie_len && (memcpy_s(app_ie->auc_ie, sizeof(app_ie->auc_ie),
        cfg80211_scan_param->puc_ie, app_ie->ie_len) != EOK)) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "hmac_cfg80211_start_scan_sta::memcpy fail! src_len[%d]", app_ie->ie_len);
    }

    app_ie->en_app_ie_type = OAL_APP_PROBE_REQ_IE;
    ret = hmac_config_set_app_ie_to_vap(mac_vap, app_ie, app_ie->en_app_ie_type);
    oal_mem_free_m(app_ie, OAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_cfg80211_start_scan_sta::hmac_config_set_app_ie_to_vap fail, err_code=%d.}", ret);
        return hmac_cfg80211_start_scan_fail(cfg80211_scan_param);
    }

    scan_h2d_params.scan_flag = cfg80211_scan_param->scan_flag;
    hmac_cfg80211_free_scan_param_resource(cfg80211_scan_param);

    /* 状态机调用: hmac_scan_proc_scan_req_event，hmac_scan_proc_scan_req_event_exception */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_SCAN_REQ, (void *)(&scan_h2d_params));
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cfg80211_start_scan_sta::hmac_fsm_call_func_sta fail[%d].}", ret);
        hmac_scan_proc_scan_req_event_exception(hmac_vap, &ret);
        return ret;
    }

    return OAL_SUCC;
}

void hmac_sta_initiate_join_log(mac_vap_stru *mac_vap)
{
    uint8_t i;
    uint8_t rate_nums;
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);

    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_sta_initiate_join_log::mac_device null.}");
        return;
    }

    // 打印11G 当前速率值
    for (i = 0; i < MAC_DATARATES_PHY_80211G_NUM; i++) {
        oam_warning_log1(0, OAM_SF_ASSOC, "{11g rate = %u.}", mac_device->st_mac_rates_11g[i].uc_mac_rate);
    }
    // 打印 当前rate_nums值
    rate_nums = mac_mib_get_SupportRateSetNums(mac_vap);
    oam_warning_log1(0, OAM_SF_ASSOC, "{rate num = %u.}", rate_nums);
}

uint32_t hmac_sta_initiate_join(mac_vap_stru *mac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_join_req_stru join_req;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint8_t rate_num;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, bss_dscr))) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_initiate_join::mac_vap or bss_dscr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_initiate_join::mac_res_get_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    rate_num = (bss_dscr->uc_num_supp_rates < WLAN_MAX_SUPP_RATES) ?
        bss_dscr->uc_num_supp_rates : WLAN_MAX_SUPP_RATES;
    if (memcpy_s(hmac_vap->auc_supp_rates, WLAN_MAX_SUPP_RATES, bss_dscr->auc_supp_rates, rate_num) != EOK) {
        oam_error_log1(0, OAM_SF_ASSOC, "hmac_sta_initiate_join::memcpy fail! bss_supp_rate_num[%d]", rate_num);
        hmac_sta_initiate_join_log(mac_vap);
        return OAL_FAIL;
    }

    mac_mib_set_SupportRateSetNums(mac_vap, bss_dscr->uc_num_supp_rates);

    hmac_prepare_join_req(&join_req, bss_dscr);
    /* 状态机调用 hmac_sta_wait_join */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_JOIN_REQ, (void *)(&join_req));
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_initiate_join::hmac_fsm_call_func_sta fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_sta_initiate_auth(mac_vap_stru *mac_vap)
{
    hmac_auth_req_stru auth_req = {0};
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;

    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_initiate_auth: mac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_initiate_auth: hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_prepare_auth_req(hmac_vap, &auth_req);
    /* 状态机调用 hmac_sta_wait_auth */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_AUTH_REQ, (void *)(&auth_req));
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_initiate_join::hmac_fsm_call_func_sta fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_sta_initiate_asoc(mac_vap_stru *mac_vap)
{
    hmac_asoc_req_stru asoc_req = {0};
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;

    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_initiate_asoc::mac_vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "hmac_sta_initiate_asoc::hmac_vap null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_prepare_asoc_req(mac_vap, &asoc_req);
    /* 状态机调用  */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_ASOC_REQ, (void *)(&asoc_req));
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "hmac_sta_initiate_asoc::hmac_fsm_call_func_sta fail[%d]", ret);
        return ret;
    }

    return OAL_SUCC;
}


void hmac_handle_scan_rsp_sta(hmac_vap_stru *hmac_vap, uint8_t *msg)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_scan_rsp_stru *scan_rsp = NULL;

    /* 抛扫描完成事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(hmac_scan_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_handle_scan_rsp_sta::frw_event_alloc_m fail.}");
        return;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA,
                       sizeof(hmac_scan_rsp_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);
    scan_rsp = (hmac_scan_rsp_stru *)event->auc_event_data;

    if (EOK != memcpy_s(scan_rsp, sizeof(hmac_scan_rsp_stru), msg, sizeof(hmac_scan_rsp_stru))) {
        oam_error_log0(0, OAM_SF_ASSOC, "hmac_handle_scan_rsp_sta::memcpy fail!");
        frw_event_free_m(event_mem);
        return;
    }
    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
}


void hmac_handle_join_rsp_sta(hmac_vap_stru *hmac_vap, uint8_t *msg)
{
    hmac_join_rsp_stru *join_rsp = (hmac_join_rsp_stru *)msg;
    if (join_rsp->en_result_code == HMAC_MGMT_SUCCESS) {
        oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_handle_join_rsp_sta::join succ.}");
        /* 初始化AUTH次数 */
        mac_mib_set_StaAuthCount(&hmac_vap->st_vap_base_info, 1);
        hmac_sta_initiate_auth(&(hmac_vap->st_vap_base_info));
    } else {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "hmac_handle_join_rsp_sta::join fail[%d]",
                         join_rsp->en_result_code);
        hmac_chr_connect_fail_query_and_report(hmac_vap, MAC_JOIN_RSP_TIMEOUT);
        hmac_send_connect_result_to_dmac_sta(hmac_vap, OAL_FAIL);
    }
}
static uint32_t hmac_report_copy_connect_failed_result(uint8_t **mgmt_data_req, hmac_user_stru *hmac_user,
    hmac_asoc_rsp_stru *asoc_rsp)
{
    // 有数据 就申请空间保存在本地，没有就不做本地保存的操作
    if (hmac_user->assoc_req_ie_len == 0 || hmac_user->puc_assoc_req_ie_buff == NULL) {
        return OAL_SUCC;
    }
    *mgmt_data_req = (uint8_t *)oal_memalloc(hmac_user->assoc_req_ie_len);
    if (*mgmt_data_req == NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_report_connect_failed_result::mgmt_data_req req alloc null.}");
        return OAL_FAIL;
    }
    memcpy_s(*mgmt_data_req, hmac_user->assoc_req_ie_len, (uint8_t *)hmac_user->puc_assoc_req_ie_buff,
        hmac_user->assoc_req_ie_len);
    /* 扫描超时需要释放对应HMAC VAP下的关联请求buff */
    asoc_rsp->puc_asoc_req_ie_buff = *mgmt_data_req;
    return OAL_SUCC;
}

void hmac_report_connect_failed_result(hmac_vap_stru *hmac_vap, mac_status_code_enum_uint16 reason_code)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_asoc_rsp_stru asoc_rsp;
    hmac_user_stru *hmac_user = NULL;
    int32_t ret;
    uint8_t *mgmt_data_req = NULL;

    memset_s(&asoc_rsp, sizeof(hmac_asoc_rsp_stru), 0, sizeof(hmac_asoc_rsp_stru));
    ret = memcpy_s(asoc_rsp.auc_addr_ap, WLAN_MAC_ADDR_LEN,
                   hmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN);
    asoc_rsp.en_result_code = HMAC_MGMT_TIMEOUT;
    asoc_rsp.en_status_code = reason_code;

    /* 抛加入完成事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(hmac_asoc_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_report_connect_failed_result::frw_event_alloc_m fail!}");
        return;
    }

    /* 获取用户指针 */
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user != NULL) {
        // 本地申请内存保存数据，防止hmac释放数据导致引用已释放的内存
        ret = hmac_report_copy_connect_failed_result(&mgmt_data_req, hmac_user, &asoc_rsp);
        if (ret != OAL_SUCC) {
            frw_event_free_m(event_mem);
            return;
        }
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA,
        sizeof(hmac_asoc_rsp_stru), FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);
    ret += memcpy_s(frw_get_event_payload(event_mem), sizeof(hmac_asoc_rsp_stru),
                    &asoc_rsp, sizeof(hmac_asoc_rsp_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_report_connect_failed_result::memcpy fail!");
        frw_event_free_m(event_mem);
        return;
    }
    /* 分发事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC && mgmt_data_req != NULL) {
        oal_free(mgmt_data_req);
        mgmt_data_req = NULL;
    }
    frw_event_free_m(event_mem);
}

#ifdef _PRE_WLAN_FEATURE_SAE
static uint32_t hmac_prepare_external_auth_req(hmac_vap_stru *hmac_vap,
    oal_nl80211_external_auth_action action, hmac_external_auth_req_stru *ext_auth_req)
{
    uint32_t aul_akm[WLAN_AUTHENTICATION_SUITES] = { 0 };
    uint8_t akm_suites_num;
    int32_t ret;

    memset_s(ext_auth_req, sizeof(hmac_external_auth_req_stru), 0, sizeof(hmac_external_auth_req_stru));

    ext_auth_req->en_action = action;
    ext_auth_req->us_status = MAC_SUCCESSFUL_STATUSCODE;

    memcpy_s(ext_auth_req->auc_bssid, WLAN_MAC_ADDR_LEN,
             hmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN);
    akm_suites_num = mac_mib_get_rsn_akm_suites_s(&hmac_vap->st_vap_base_info, aul_akm, sizeof(aul_akm));
    if (akm_suites_num != 1) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                       "{hmac_prepare_external_auth_req::get AKM suite failed! akm_suite_num [%d]}",
                       akm_suites_num);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ext_auth_req->key_mgmt_suite = aul_akm[0];

    ext_auth_req->st_ssid.uc_ssid_len = oal_min(sizeof(ext_auth_req->st_ssid.auc_ssid),
        OAL_STRLEN(mac_mib_get_DesiredSSID(&(hmac_vap->st_vap_base_info))));
    ret = memcpy_s(ext_auth_req->st_ssid.auc_ssid,
                   OAL_IEEE80211_MAX_SSID_LEN,
                   mac_mib_get_DesiredSSID(&(hmac_vap->st_vap_base_info)),
                   ext_auth_req->st_ssid.uc_ssid_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_SAE, "hmac_prepare_external_auth_req::memcpy fail!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


uint32_t hmac_report_external_auth_req(hmac_vap_stru *hmac_vap, oal_nl80211_external_auth_action action)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_external_auth_req_stru ext_auth_req;
    uint32_t ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_SAE, "{hmac_report_external_auth_req:: hmac_vap is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_prepare_external_auth_req(hmac_vap, action, &ext_auth_req);
    if (ret != OAL_SUCC) {
        return ret;
    }
    event_mem = frw_event_alloc_m(sizeof(ext_auth_req));
    if (event_mem == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "{hmac_report_connect_failed_result::frw_event_alloc_m fail! size[%d]}",
                         sizeof(ext_auth_req));
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_EXT_AUTH_REQ,
                       sizeof(ext_auth_req),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);
    if (memcpy_s(frw_get_event_payload(event_mem), sizeof(ext_auth_req),
        &ext_auth_req, sizeof(ext_auth_req)) != EOK) {
        oam_error_log0(0, OAM_SF_SAE, "{hmac_report_external_auth_req:: memcpy fail}");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_SAE */


void hmac_handle_connect_failed_result(hmac_vap_stru *hmac_vap, uint16_t status)
{
    hmac_user_stru *hmac_user = NULL;

    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                     "{hmac_handle_connect_failed_result::connect fail[%d]}", status);
    hmac_chr_connect_fail_query_and_report(hmac_vap, status);
    /* 上报关联失败到wpa_supplicant */
    hmac_report_connect_failed_result(hmac_vap, status);

    /* 获取用户指针 */
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user != NULL) {
        /* 发送去认证帧到AP */
        hmac_mgmt_send_deauth_frame(&hmac_vap->st_vap_base_info,
            hmac_user->st_user_base_info.auc_user_mac_addr, status, OAL_FALSE);
        /* 删除对应用户 */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    } else {
        /* 设置状态为FAKE UP */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH, "asoc:hmac_user[%d] NULL.",
                         hmac_vap->st_vap_base_info.us_assoc_vap_id);
    }

    /* 同步DMAC状态 */
    hmac_send_connect_result_to_dmac_sta(hmac_vap, OAL_FAIL);
}


void hmac_handle_auth_rsp_sta(hmac_vap_stru *hmac_vap, uint8_t *msg)
{
    hmac_auth_rsp_stru *auth_rsp = (hmac_auth_rsp_stru *)msg;
    hmac_user_stru *hmac_user = NULL;
#ifdef PLATFORM_DEBUG_ENABLE
    uint8_t auc_param[] = "all";
#endif
    if (auth_rsp->us_status_code == HMAC_MGMT_SUCCESS) {
        /* 初始化AOSC次数 */
        mac_mib_set_StaAssocCount(&hmac_vap->st_vap_base_info, 1);
        hmac_sta_initiate_asoc(&hmac_vap->st_vap_base_info);
        return;
    }

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, 0, "hmac_handle_auth_rsp_sta::auth fail[%d],cnt[%d]",
        auth_rsp->us_status_code, mac_mib_get_StaAuthCount(&hmac_vap->st_vap_base_info));
    if ((MAC_UNSUPT_ALG == auth_rsp->us_status_code ||
         MAX_AUTH_CNT <= mac_mib_get_StaAuthCount(&hmac_vap->st_vap_base_info)) &&
        (WLAN_WITP_AUTH_AUTOMATIC == mac_mib_get_AuthenticationMode(&hmac_vap->st_vap_base_info))) {
        mac_mib_set_AuthenticationMode(&hmac_vap->st_vap_base_info, WLAN_WITP_AUTH_SHARED_KEY);
        /* 需要将状态机设置为 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);
        /* 更新AUTH的次数 */
        mac_mib_set_StaAuthCount(&hmac_vap->st_vap_base_info, 0);
        /* 重新发起关联动作 */
        hmac_sta_initiate_auth(&(hmac_vap->st_vap_base_info));

        return;
    }

    if (mac_mib_get_StaAuthCount(&hmac_vap->st_vap_base_info) < MAX_AUTH_CNT) {
        /* 需要将状态机设置为 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);
        /* 更新AUTH的次数 */
        mac_mib_incr_StaAuthCount(&hmac_vap->st_vap_base_info);
        /* 重新发起关联动作 */
        hmac_sta_initiate_auth(&(hmac_vap->st_vap_base_info));

        return;
    } else {
#ifdef PLATFORM_DEBUG_ENABLE
        hmac_config_reg_info(&(hmac_vap->st_vap_base_info), sizeof(auc_param), auc_param);
#endif
    }
    hmac_chr_connect_fail_query_and_report(hmac_vap, auth_rsp->us_status_code);

#ifdef _PRE_WLAN_FEATURE_SAE
    if ((mac_mib_get_AuthenticationMode(&hmac_vap->st_vap_base_info) == WLAN_WITP_AUTH_SAE ||
        (mac_mib_get_AuthenticationMode(&hmac_vap->st_vap_base_info) == WLAN_WITP_AUTH_TBPEKE)) &&
        (auth_rsp->us_status_code != MAC_AP_FULL)) {
        /* SAE关联失败，上报停止external auth到wpa_s */
        hmac_report_external_auth_req(hmac_vap, NL80211_EXTERNAL_AUTH_ABORT);
    }
#endif
    /* 上报关联失败到wpa_supplicant */
    hmac_report_connect_failed_result(hmac_vap, auth_rsp->us_status_code);
    /* 获取用户指针 */
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user != NULL) {
        /* 删除对应用户 */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    } else {
        /* The MAC state is changed to fake up state. Further MLME     */
        /* requests are processed in this state.                       */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        oam_warning_log1(0, 0, "auth hmac_user[%d] NULL.", hmac_vap->st_vap_base_info.us_assoc_vap_id);
    }
    hmac_send_connect_result_to_dmac_sta(hmac_vap, OAL_FAIL);
}

static uint32_t hmac_handle_copy_asoc_rsp_ie_buff(uint8_t **mgmt_data, uint8_t **mgmt_data_req,
    hmac_asoc_rsp_stru *asoc_rsp, hmac_vap_stru *hmac_vap)
{
    *mgmt_data = (uint8_t *)oal_memalloc(asoc_rsp->asoc_rsp_ie_len);
    if (*mgmt_data == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
            "{hmac_handle_asoc_rsp_succ_sta::mgmt_data alloc null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    memcpy_s(*mgmt_data, asoc_rsp->asoc_rsp_ie_len, (uint8_t *)asoc_rsp->puc_asoc_rsp_ie_buff,
        asoc_rsp->asoc_rsp_ie_len);
    asoc_rsp->puc_asoc_rsp_ie_buff = *mgmt_data;

    if (asoc_rsp->asoc_req_ie_len != 0 && asoc_rsp->puc_asoc_req_ie_buff != NULL) {
        *mgmt_data_req = (uint8_t *)oal_memalloc(asoc_rsp->asoc_req_ie_len);
        if (*mgmt_data_req == NULL) {
            oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                "{hmac_handle_asoc_rsp_succ_sta::mgmt_data_req alloc null.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        memcpy_s(*mgmt_data_req, asoc_rsp->asoc_req_ie_len, (uint8_t *)asoc_rsp->puc_asoc_req_ie_buff,
            asoc_rsp->asoc_req_ie_len);
        asoc_rsp->puc_asoc_req_ie_buff = *mgmt_data_req;
    }
    return OAL_SUCC;
}

static void hmac_handle_free_ie_buff(uint8_t **mgmt_data, uint8_t **mgmt_data_req)
{
    if (*mgmt_data != NULL) {
        oal_free(*mgmt_data);
        *mgmt_data = NULL;
    }

    if (*mgmt_data_req != NULL) {
        oal_free(*mgmt_data_req);
        *mgmt_data_req = NULL;
    }
}

static void hmac_handle_asoc_rsp_succ_sta(hmac_vap_stru *hmac_vap, uint8_t *msg, hmac_asoc_rsp_stru *asoc_rsp)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    uint8_t *mgmt_data = NULL;
    uint32_t ret;
    int32_t ret1;
    uint8_t *mgmt_data_req = NULL;

    /* 抛加入完成事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(hmac_asoc_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_handle_asoc_rsp_sta::frw_event_alloc_m fail!}");
        return;
    }
    // 本地申请内存保存数据，防止hmac释放数据导致引用已释放的内存
    ret = hmac_handle_copy_asoc_rsp_ie_buff(&mgmt_data, &mgmt_data_req, asoc_rsp, hmac_vap);
    if (ret != OAL_SUCC) {
        hmac_handle_free_ie_buff(&mgmt_data, &mgmt_data_req);
        frw_event_free_m(event_mem);
        return;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA, sizeof(hmac_asoc_rsp_stru),
                       FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);
    ret1 = memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(hmac_asoc_rsp_stru),
        (uint8_t *)msg, sizeof(hmac_asoc_rsp_stru));
    if (ret1 != EOK) {
        oam_error_log0(0, OAM_SF_AUTH, "hmac_handle_asoc_rsp_succ_sta::memcpy fail!");
        hmac_handle_free_ie_buff(&mgmt_data, &mgmt_data_req);
        frw_event_free_m(event_mem);
        return;
    }
    /* 分发事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        hmac_handle_free_ie_buff(&mgmt_data, &mgmt_data_req);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "hmac_handle_asoc_rsp_succ_sta::frw_event_dispatch_event fail[%d]", ret);
    }
    frw_event_free_m(event_mem);
}

static void hmac_handle_asoc_rsp_fail_sta(hmac_vap_stru *hmac_vap, hmac_asoc_rsp_stru *asoc_rsp)
{
    hmac_user_stru *hmac_user = NULL;

    if (mac_mib_get_StaAssocCount(&hmac_vap->st_vap_base_info) >= MAX_ASOC_CNT) {
#ifdef PLATFORM_DEBUG_ENABLE
        hmac_config_reg_info(&(hmac_vap->st_vap_base_info), 4, (uint8_t *)"all"); /* 4是"all"长度 */
#endif
#ifdef _PRE_WLAN_1103_CHR
        hmac_chr_connect_fail_query_and_report(hmac_vap, asoc_rsp->en_status_code);
#endif
    /* 上报关联失败到wpa_supplicant */
        hmac_report_connect_failed_result(hmac_vap, asoc_rsp->en_status_code);
        /* 获取用户指针 */
        hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
        if (hmac_user != NULL) {
             /* 发送去认证帧到AP */
            hmac_mgmt_send_deauth_frame(&hmac_vap->st_vap_base_info,
                                        hmac_user->st_user_base_info.auc_user_mac_addr,
                                        MAC_AUTH_NOT_VALID, OAL_FALSE);
            /* 删除对应用户 */
            hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
        } else {
             /* 设置状态为FAKE UP */
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH, "asoc:hmac_user[%d] NULL.",
                             hmac_vap->st_vap_base_info.us_assoc_vap_id);
        }
        /* 同步DMAC状态 */
        hmac_send_connect_result_to_dmac_sta(hmac_vap, OAL_FAIL);
    } else {
        /* 需要将状态机设置为MAC_VAP_STATE_STA_AUTH_COMP,重新发起关联 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);
        /* 发起ASOC的次数 */
        mac_mib_incr_StaAssocCount(&hmac_vap->st_vap_base_info);
        /* 重新发起关联动作 */
        hmac_sta_initiate_asoc(&hmac_vap->st_vap_base_info);
    }
}


void hmac_handle_asoc_rsp_sta(hmac_vap_stru *hmac_vap, uint8_t *msg)
{
    hmac_asoc_rsp_stru *asoc_rsp = (hmac_asoc_rsp_stru *)msg;

    if (asoc_rsp->en_result_code == HMAC_MGMT_SUCCESS) {
        oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id,
                      OAM_SF_ASSOC,
                      "{hmac_handle_asoc_rsp_sta::asoc succ.}");
        /* 关联成功处理 */
        hmac_handle_asoc_rsp_succ_sta(hmac_vap, msg, asoc_rsp);
    } else {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_handle_asoc_rsp_sta::asoc fail[%d], assoc_cnt[%d]}",
                         asoc_rsp->en_result_code,
                         mac_mib_get_StaAssocCount(&hmac_vap->st_vap_base_info));
        /* 关联失败处理 */
        hmac_handle_asoc_rsp_fail_sta(hmac_vap, asoc_rsp);
    }
}


void hmac_send_rsp_to_sme_sta(hmac_vap_stru *hmac_vap, hmac_sme_rsp_enum type, uint8_t *msg)
{
    g_handle_rsp_func_sta[type](hmac_vap, msg);
}


static void hmac_send_connect_result_to_dmac_sta(hmac_vap_stru *hmac_vap, uint32_t result)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;

    /* 抛事件到DMAC, 申请事件内存 */
    event_mem = frw_event_alloc_m(sizeof(uint32_t));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "pst_event_mem null.");
        return;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_CONN_RESULT,
                       sizeof(uint32_t),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);
    *((uint32_t *)(event->auc_event_data)) = result;

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return;
}

