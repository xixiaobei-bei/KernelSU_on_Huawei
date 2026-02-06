

#ifdef _PRE_WLAN_FEATURE_HID2D
/* 1 头文件包含 */
#include "mac_ie.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "wlan_types.h"
#include "hmac_chan_mgmt.h"
#include "mac_device.h"
#include "hmac_hid2d.h"
#include "hmac_scan.h"
#include "oal_cfg80211.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HID2D_C

#define DECIMAL_TO_INTEGER 100 /* 用于将小数转换成整数，便于计算 */
/* 2 全局变量定义 */
/* 全局待选信道覆盖所有信道，实际运行中可选择的待选信道根据GO与GC所支持的信道列表求交集所得，
   用全局待选信道的序号表示 */
/* 2.4G全频段待选信道 */
hmac_hid2d_chan_stru g_aus_channel_candidate_list_2g[HMAC_HID2D_CHANNEL_NUM_2G] = {
    // idx  chan_idx  chan_num   bandwidth
    {0,     0,      1,    WLAN_BAND_WIDTH_20M},
    {1,     1,      2,    WLAN_BAND_WIDTH_20M},
    {2,     2,      3,    WLAN_BAND_WIDTH_20M},
    {3,     3,      4,    WLAN_BAND_WIDTH_20M},
    {4,     4,      5,    WLAN_BAND_WIDTH_20M},
    {5,     5,      6,    WLAN_BAND_WIDTH_20M},
    {6,     6,      7,    WLAN_BAND_WIDTH_20M},
    {7,     7,      8,    WLAN_BAND_WIDTH_20M},
    {8,     8,      9,    WLAN_BAND_WIDTH_20M},
    {9,     9,      10,   WLAN_BAND_WIDTH_20M},
    {10,    10,     11,   WLAN_BAND_WIDTH_20M},
    {11,    11,     12,   WLAN_BAND_WIDTH_20M},
    {12,    12,     13,   WLAN_BAND_WIDTH_20M},
    {13,    13,     14,   WLAN_BAND_WIDTH_20M},
    {14,    0,      1,    WLAN_BAND_WIDTH_40PLUS},
    {15,    4,      5,    WLAN_BAND_WIDTH_40PLUS},
    {16,    6,      7,    WLAN_BAND_WIDTH_40PLUS},
    {17,    8,      9,    WLAN_BAND_WIDTH_40PLUS},
    {18,    1,      2,    WLAN_BAND_WIDTH_40PLUS},
    {19,    5,      6,    WLAN_BAND_WIDTH_40PLUS},
    {20,    7,      8,    WLAN_BAND_WIDTH_40PLUS},
    {21,    12,     13,   WLAN_BAND_WIDTH_40MINUS},
    {22,    4,      5,    WLAN_BAND_WIDTH_40MINUS},
    {23,    6,      7,    WLAN_BAND_WIDTH_40MINUS},
    {24,    10,     11,   WLAN_BAND_WIDTH_40MINUS},
    {25,    11,     12,   WLAN_BAND_WIDTH_40MINUS},
    {26,    9,      10,   WLAN_BAND_WIDTH_40MINUS},
    {27,    2,      3,    WLAN_BAND_WIDTH_40PLUS},
    {28,    3,      4,    WLAN_BAND_WIDTH_40PLUS},
    {29,    5,      6,    WLAN_BAND_WIDTH_40MINUS},
    {30,    7,      8,    WLAN_BAND_WIDTH_40MINUS},
    {31,    8,      9,    WLAN_BAND_WIDTH_40MINUS},
};
/* 5G全频段待选信道 */
hmac_hid2d_chan_stru g_aus_channel_candidate_list_5g[HMAC_HID2D_CHANNEL_NUM_5G] = {
    // idx  chan_idx  chan_num   bandwidth
    {0,     0,      36,    WLAN_BAND_WIDTH_20M},
    {1,     1,      40,    WLAN_BAND_WIDTH_20M},
    {2,     2,      44,    WLAN_BAND_WIDTH_20M},
    {3,     3,      48,    WLAN_BAND_WIDTH_20M},
    {4,     0,      36,    WLAN_BAND_WIDTH_40PLUS},
    {5,     1,      40,    WLAN_BAND_WIDTH_40MINUS},
    {6,     2,      44,    WLAN_BAND_WIDTH_40PLUS},
    {7,     3,      48,    WLAN_BAND_WIDTH_40MINUS},
    {8,     0,      36,    WLAN_BAND_WIDTH_80PLUSPLUS},
    {9,     1,      40,    WLAN_BAND_WIDTH_80MINUSPLUS},
    {10,    2,      44,    WLAN_BAND_WIDTH_80PLUSMINUS},
    {11,    3,      48,    WLAN_BAND_WIDTH_80MINUSMINUS},
    {12,    4,      52,    WLAN_BAND_WIDTH_20M},
    {13,    5,      56,    WLAN_BAND_WIDTH_20M},
    {14,    6,      60,    WLAN_BAND_WIDTH_20M},
    {15,    7,      64,    WLAN_BAND_WIDTH_20M},
    {16,    4,      52,    WLAN_BAND_WIDTH_40PLUS},
    {17,    5,      56,    WLAN_BAND_WIDTH_40MINUS},
    {18,    6,      60,    WLAN_BAND_WIDTH_40PLUS},
    {19,    7,      64,    WLAN_BAND_WIDTH_40MINUS},
    {20,    4,      52,    WLAN_BAND_WIDTH_80PLUSPLUS},
    {21,    5,      56,    WLAN_BAND_WIDTH_80MINUSPLUS},
    {22,    6,      60,    WLAN_BAND_WIDTH_80PLUSMINUS},
    {23,    7,      64,    WLAN_BAND_WIDTH_80MINUSMINUS},
    {24,    8,      100,   WLAN_BAND_WIDTH_20M},
    {25,    9,      104,   WLAN_BAND_WIDTH_20M},
    {26,    10,     108,   WLAN_BAND_WIDTH_20M},
    {27,    11,     112,   WLAN_BAND_WIDTH_20M},
    {28,    8,      100,   WLAN_BAND_WIDTH_40PLUS},
    {29,    9,      104,   WLAN_BAND_WIDTH_40MINUS},
    {30,    10,     108,   WLAN_BAND_WIDTH_40PLUS},
    {31,    11,     112,   WLAN_BAND_WIDTH_40MINUS},
    {32,    8,      100,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {33,    9,      104,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {34,    10,     108,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {35,    11,     112,   WLAN_BAND_WIDTH_80MINUSMINUS},
    {36,    12,     116,   WLAN_BAND_WIDTH_20M},
    {37,    13,     120,   WLAN_BAND_WIDTH_20M},
    {38,    14,     124,   WLAN_BAND_WIDTH_20M},
    {39,    15,     128,   WLAN_BAND_WIDTH_20M},
    {40,    12,     116,   WLAN_BAND_WIDTH_40PLUS},
    {41,    13,     120,   WLAN_BAND_WIDTH_40MINUS},
    {42,    14,     124,   WLAN_BAND_WIDTH_40PLUS},
    {43,    15,     128,   WLAN_BAND_WIDTH_40MINUS},
    {44,    12,     116,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {45,    13,     120,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {46,    14,     124,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {47,    15,     128,   WLAN_BAND_WIDTH_80MINUSMINUS},
    {48,    16,     132,   WLAN_BAND_WIDTH_20M},
    {49,    17,     136,   WLAN_BAND_WIDTH_20M},
    {50,    18,     140,   WLAN_BAND_WIDTH_20M},
    {51,    19,     144,   WLAN_BAND_WIDTH_20M},
    {52,    16,     132,   WLAN_BAND_WIDTH_40PLUS},
    {53,    17,     136,   WLAN_BAND_WIDTH_40MINUS},
    {54,    18,     140,   WLAN_BAND_WIDTH_40PLUS},
    {55,    19,     144,   WLAN_BAND_WIDTH_40MINUS},
    {56,    16,     132,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {57,    17,     136,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {58,    18,     140,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {59,    19,     144,   WLAN_BAND_WIDTH_80MINUSMINUS},
    {60,    20,     149,   WLAN_BAND_WIDTH_20M},
    {61,    21,     153,   WLAN_BAND_WIDTH_20M},
    {62,    22,     157,   WLAN_BAND_WIDTH_20M},
    {63,    23,     161,   WLAN_BAND_WIDTH_20M},
    {64,    20,     149,   WLAN_BAND_WIDTH_40PLUS},
    {65,    21,     153,   WLAN_BAND_WIDTH_40MINUS},
    {66,    22,     157,   WLAN_BAND_WIDTH_40PLUS},
    {67,    23,     161,   WLAN_BAND_WIDTH_40MINUS},
    {68,    20,     149,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {69,    21,     153,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {70,    22,     157,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {71,    23,     161,   WLAN_BAND_WIDTH_80MINUSMINUS},
    {72,    24,     165,   WLAN_BAND_WIDTH_20M}
};
/* 扫描顺序数组: 初始化为发布会场景的扫描顺序，正常业务场景需要在开始扫描时根据实际待选信道列表确定并赋值 */
uint8_t g_scan_chan_list[HMAC_HID2D_MAX_SCAN_CHAN_NUM] = { 8, 20, 32, 44, 56, 68 };
/* 一轮扫描的数组值，确定g_scan_chan_list的边界，初始化值为发布会场景 */
uint8_t g_scan_chan_num_max = HMAC_HID2D_SCAN_CHAN_NUM_FOR_APK;
/* 实际可支持的信道列表数组 */
uint8_t g_supp_chan_list[HMAC_HID2D_CHANNEL_NUM_5G] = { 0 };
/* 实际可支持的信道个数，确定g_supp_chan_list的边界 */
uint8_t g_supp_chan_num_max = 0;
/* 扫描信息 */
hmac_hid2d_auto_channel_switch_stru g_st_hmac_hid2d_acs_info = {
    .uc_acs_mode = OAL_FALSE,
};

OAL_STATIC const wal_hid2d_cmd_entry_stru g_hid2d_vendor_cmd[] = {
    { WAL_HID2D_INIT_CMD,          wal_hipriv_hid2d_presentation_init },
    { WAL_HID2D_SCAN_START_CMD,    wal_hipriv_hid2d_scan_channel },
    { WAL_HID2D_CHAN_SWITCH_CMD,   wal_hipriv_hid2d_swchan },
    { WAL_HID2D_EDCA_CMD,          wal_hipriv_hid2d_edca_cfg },
    { WAL_HID2D_ACS_CMD,           wal_ioctl_set_hid2d_acs_mode },
    { WAL_HID2D_ACS_STATE_CMD,     wal_ioctl_update_acs_state },
    { WAL_HID2D_LINK_MEAS_CMD,     wal_ioctl_hid2d_link_meas },
};

/* 3 函数声明 */
uint32_t hmac_hid2d_scan_chan_once(mac_vap_stru *pst_mac_vap, uint8_t chan_index, uint8_t is_apk_scan);
uint32_t hmac_hid2d_timeout_handle(void *p_arg);
uint8_t hmac_hid2d_get_best_chan(mac_channel_stru *pst_scan_chan);
uint32_t hmac_hid2d_apk_scan_handler(hmac_scan_record_stru *pst_scan_record, uint8_t uc_scan_idx,
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info);

uint32_t hmac_hid2d_acs_scan_handler(hmac_scan_record_stru *scan_record, uint8_t scan_idx,
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info);
uint32_t hmac_hid2d_acs_timeout_handle(void *arg);
uint32_t hmac_hid2d_chan_stat_report_timeout_handle(void *arg);
uint32_t hmac_hid2d_acs_get_best_chan(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info,
    hmac_hid2d_chan_info_stru *chan_info, hmac_hid2d_chan_stru *candidate_chan_list);

/* 4 函数实现 */

uint32_t hmac_hid2d_find_p2p_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t uc_vap_index;
    uint8_t uc_no_p2p_vap_flag = OAL_TRUE;
    mac_vap_stru *pst_p2p_mac_vap = NULL;

    /* 找到挂接在该device上的p2p vap */
    for (uc_vap_index = 0; uc_vap_index < pst_mac_device->uc_vap_num; uc_vap_index++) {
        pst_p2p_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_index]);
        if (pst_p2p_mac_vap == NULL) {
            continue;
        }
        if (pst_p2p_mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
            uc_no_p2p_vap_flag = OAL_FALSE;
            break;
        }
    }
    if (uc_no_p2p_vap_flag == OAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_hid2d_find_p2p_vap_ext::no p2p vap error!}");
        return OAL_FAIL;
    }
    *ppst_mac_vap = pst_p2p_mac_vap;
    return OAL_SUCC;
}
uint32_t hmac_hid2d_find_up_p2p_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t uc_vap_index;
    uint8_t uc_no_p2p_vap_flag = OAL_TRUE;
    mac_vap_stru *pst_p2p_mac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;

    /* 找到挂接在该device上的p2p vap */
    for (uc_vap_index = 0; uc_vap_index < pst_mac_device->uc_vap_num; uc_vap_index++) {
        pst_p2p_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_index]);
        if (pst_p2p_mac_vap == NULL) {
            continue;
        }
        mac_vap = pst_p2p_mac_vap;
        if (pst_p2p_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE || pst_p2p_mac_vap->en_p2p_mode == WLAN_P2P_CL_MODE) {
            uc_no_p2p_vap_flag = OAL_FALSE;
            break;
        }
    }
    if (uc_no_p2p_vap_flag == OAL_TRUE) {
        oam_error_log0(0, 0, "{hmac_hid2d_find_p2p_vap::no up p2p vap error!}");
        *ppst_mac_vap = mac_vap; /* pst_mac_vap是非p2p GO/CL vap，用于p2p断链后的方案善后处理 */
        return OAL_FAIL;
    }
    *ppst_mac_vap = pst_p2p_mac_vap;
    return OAL_SUCC;
}

uint32_t hmac_hid2d_scan_chan_start(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_device)
{
    hmac_hid2d_chan_info_stru *pst_chan_info = NULL;
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info = NULL;
    uint8_t uc_scan_chan_idx;
    uint32_t ret;

    pst_hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;

    /* 如果auto channel switch算法使能，则直接退出 */
    if (pst_hmac_hid2d_acs_info->uc_acs_mode == OAL_TRUE) {
        return OAL_SUCC;
    }
    pst_hmac_hid2d_acs_info->uc_apk_scan = OAL_TRUE;
    /* 初始化信道信息 */
    pst_chan_info = pst_hmac_hid2d_acs_info->chan_info_5g;
    memset_s(pst_chan_info, sizeof(hmac_hid2d_chan_info_stru) * HMAC_HID2D_CHANNEL_NUM_5G,
        0, sizeof(hmac_hid2d_chan_info_stru) * HMAC_HID2D_CHANNEL_NUM_5G);
    /* 每个信道扫描10次 */
    pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx = 0;
    pst_hmac_hid2d_acs_info->update_num = 1;
    /* max_update_num用于滑动平均更新中的老化系数修正 */
    pst_hmac_hid2d_acs_info->max_update_num =
        (HMAC_HID2D_DECAY * 10 - 100) / (100 - HMAC_HID2D_DECAY); // 公式: (10x - 100) / (100 - x)
    /* 开启一次扫描 */
    uc_scan_chan_idx = g_scan_chan_list[pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx];

    ret = hmac_hid2d_scan_chan_once(pst_mac_vap, uc_scan_chan_idx, OAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, 0, "hmac_hid2d_scan_chan_start:hmac_hid2d_scan_chan_once failed");
    }

    /* 开启定时器 */
    frw_timer_create_timer_m(&pst_hmac_hid2d_acs_info->st_scan_chan_timer,
        hmac_hid2d_timeout_handle,
        HMAC_HID2D_SCAN_TIMER_CYCLE_MS,
        pst_mac_device,
        OAL_FALSE,
        OAM_MODULE_ID_HMAC,
        pst_mac_device->core_id);

    return OAL_SUCC;
}

uint32_t hmac_hid2d_scan_chan_once(mac_vap_stru *pst_mac_vap, uint8_t chan_index, uint8_t is_apk_scan)
{
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info = NULL;
    hmac_hid2d_chan_stru *pst_candidate_list = NULL;
    mac_channel_stru scan_channel = {0};
    mac_scan_req_h2d_stru st_h2d_scan_params;
    hmac_vap_stru *pst_hmac_vap = NULL;
    uint32_t ret;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, 0, "{hmac_hid2d_scan_chan_once::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    /* 如果是apk触发的扫描 */
    if (is_apk_scan == OAL_TRUE) {
        scan_channel.en_band = WLAN_BAND_5G;
        pst_candidate_list = g_aus_channel_candidate_list_5g;
    } else {
        scan_channel.en_band = pst_hmac_hid2d_acs_info->uc_cur_band;
        pst_candidate_list = (scan_channel.en_band == WLAN_BAND_5G) ? g_aus_channel_candidate_list_5g :
            g_aus_channel_candidate_list_2g;
    }
    scan_channel.uc_chan_number = pst_candidate_list[chan_index].uc_chan_number;
    scan_channel.en_bandwidth = pst_candidate_list[chan_index].en_bandwidth;
    scan_channel.uc_chan_idx = pst_candidate_list[chan_index].uc_chan_idx;

    /* 设置扫描参数 */
    memset_s(&st_h2d_scan_params, sizeof(mac_scan_req_h2d_stru), 0, sizeof(mac_scan_req_h2d_stru));
    st_h2d_scan_params.st_scan_params.uc_vap_id = pst_mac_vap->uc_vap_id;
    st_h2d_scan_params.st_scan_params.en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_h2d_scan_params.st_scan_params.en_scan_type = WLAN_SCAN_TYPE_PASSIVE;
    st_h2d_scan_params.st_scan_params.uc_probe_delay = 0;
    st_h2d_scan_params.st_scan_params.uc_scan_func = MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
    st_h2d_scan_params.st_scan_params.uc_max_scan_count_per_channel = 1;
    st_h2d_scan_params.st_scan_params.us_scan_time = (is_apk_scan == OAL_TRUE) ?
        HMAC_HID2D_SCAN_TIME_PER_CHAN_APK : pst_hmac_hid2d_acs_info->scan_time;
    st_h2d_scan_params.st_scan_params.en_scan_mode = WLAN_SCAN_MODE_HID2D_SCAN;
    st_h2d_scan_params.st_scan_params.uc_channel_nums = 1;
    st_h2d_scan_params.st_scan_params.ast_channel_list[0] = scan_channel;

    ret = hmac_scan_proc_scan_req_event(pst_hmac_vap, &st_h2d_scan_params);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, 0, "hmac_hid2d_scan_chan_once:hmac scan req failed");
    }

    return ret;
}

uint8_t hmac_hid2d_get_best_chan(mac_channel_stru *pst_scan_chan)
{
    hmac_hid2d_chan_info_stru *pst_chan_info = NULL;
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info = NULL;
    uint8_t uc_index;
    uint8_t uc_chan_idx;
    int16_t highest_chan_load = 0;
    int16_t lowest_noise_rssi = 0;
    uint8_t best_chan_of_chan_load = 0;
    uint8_t best_chan_of_noise_rssi = 0;

    pst_hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    pst_chan_info = pst_hmac_hid2d_acs_info->chan_info_5g;

    for (uc_index = 0; uc_index < g_scan_chan_num_max; uc_index++) {
        uc_chan_idx = g_scan_chan_list[uc_index];
        oam_warning_log3(0, OAM_SF_SCAN, "{HiD2D results: channel[%d], avg chan_load is %d, avg noise rssi is %d}",
            g_aus_channel_candidate_list_5g[uc_chan_idx].uc_chan_number,
            pst_chan_info[uc_chan_idx].chan_load, pst_chan_info[uc_chan_idx].chan_noise_rssi);
        if (pst_chan_info[uc_chan_idx].chan_load > highest_chan_load) {
            highest_chan_load = pst_chan_info[uc_chan_idx].chan_load;
            best_chan_of_chan_load = uc_chan_idx;
        }
        if (pst_chan_info[uc_chan_idx].chan_noise_rssi < lowest_noise_rssi) {
            lowest_noise_rssi = pst_chan_info[uc_chan_idx].chan_noise_rssi;
            best_chan_of_noise_rssi = uc_chan_idx;
        }
    }
    /* 分别按照占空比最高和底噪最小选择了信道A和信道B */
    /* 如果信道A和信道B是一个信道，则就report该信道，否则优先选择占空比最高的信道A，除非信道B的占空比略小于A，
        但底噪比A低5dB以上 */
    if (best_chan_of_chan_load == best_chan_of_noise_rssi) {
        uc_chan_idx = best_chan_of_chan_load;
    } else if ((highest_chan_load - pst_chan_info[best_chan_of_noise_rssi].chan_load < HMAC_HID2D_CHAN_LOAD_DIFF)
        && (pst_chan_info[best_chan_of_chan_load].chan_noise_rssi - lowest_noise_rssi > HMAC_HID2D_NOISE_DIFF)) {
        uc_chan_idx = best_chan_of_noise_rssi;
    } else {
        uc_chan_idx = best_chan_of_chan_load;
    }
    pst_scan_chan->en_band = WLAN_BAND_5G;
    pst_scan_chan->uc_chan_number = g_aus_channel_candidate_list_5g[uc_chan_idx].uc_chan_number;
    pst_scan_chan->en_bandwidth = g_aus_channel_candidate_list_5g[uc_chan_idx].en_bandwidth;
    pst_scan_chan->uc_chan_idx = g_aus_channel_candidate_list_5g[uc_chan_idx].uc_chan_idx;
    oam_warning_log2(0, OAM_SF_SCAN, "{HiD2D results: best channel num is %d, bw is %d}",
        pst_scan_chan->uc_chan_number, pst_scan_chan->en_bandwidth);

    return OAL_SUCC;
}


uint32_t hmac_hid2d_timeout_handle(void *p_arg)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_p2p_vap = NULL;
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info = NULL;
    mac_channel_stru st_scan_chan = {0};
    mac_channel_stru *pst_best_chan = NULL;
    uint8_t uc_scan_chan_idx;
    uint32_t ret;
    int32_t l_ret;

    pst_mac_device = (mac_device_stru *)p_arg;
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_hid2d_find_p2p_vap(pst_mac_device, (mac_vap_stru **)&pst_p2p_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pst_hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;

    /* HiD2D发布会场景: 判断是否全部扫描结束，如果结束则评估最优信道并上报到hmac层，不结束则触发下一次扫描 */
    if ((pst_hmac_hid2d_acs_info->update_num < HMAC_HID2D_SCAN_TIMES_PER_CHAN_FOR_APK) ||
        (pst_hmac_hid2d_acs_info->update_num == HMAC_HID2D_SCAN_TIMES_PER_CHAN_FOR_APK &&
        pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx < HMAC_HID2D_SCAN_CHAN_NUM_FOR_APK - 1)) {
        pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx = (pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx + 1)
            % HMAC_HID2D_SCAN_CHAN_NUM_FOR_APK;
        uc_scan_chan_idx = g_scan_chan_list[pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx];
        if (pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx == 0) {
            pst_hmac_hid2d_acs_info->update_num++;
        }
        /* 再下发一次扫描，并开启定时器 */
        hmac_hid2d_scan_chan_once(pst_p2p_vap, uc_scan_chan_idx, OAL_TRUE);
        frw_timer_create_timer_m(&pst_hmac_hid2d_acs_info->st_scan_chan_timer,
            hmac_hid2d_timeout_handle,
            HMAC_HID2D_SCAN_TIMER_CYCLE_MS,
            pst_mac_device,
            OAL_FALSE,
            OAM_MODULE_ID_HMAC,
            pst_mac_device->core_id);
    } else {
        /* 获取最空闲的信道 */
        pst_hmac_hid2d_acs_info->uc_apk_scan = OAL_FALSE;
        hmac_hid2d_get_best_chan(&st_scan_chan);
        /* 将扫描结果存储在mac_device_stru中 */
        pst_mac_device->is_ready_to_get_scan_result = OAL_TRUE;
        pst_best_chan = &(pst_mac_device->st_best_chan_for_hid2d);
        l_ret = memcpy_s(pst_best_chan, sizeof(mac_channel_stru), &st_scan_chan, sizeof(mac_channel_stru));
        if (l_ret != EOK) {
            oam_error_log1(0, OAM_SF_SCAN, "hmac_hid2d_best_chan_report::memcpy fail! l_ret[%d]", l_ret);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

uint32_t hmac_hid2d_apk_scan_handler(hmac_scan_record_stru *pst_scan_record, uint8_t uc_scan_idx,
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info)
{
    hmac_hid2d_chan_info_stru *pst_chan_info = NULL;
    wlan_scan_chan_stats_stru *pst_chan_result = NULL;
    uint8_t uc_channel_number;
    uint8_t uc_scan_chan_idx;
    int16_t chan_load; /* 信道占空比 (千分之X) */
    int16_t noise_rssi; /* 信道的底噪 */
    int16_t decay; /* 滑动平均计算的老化系数 */
    int16_t avg_data;

    /* 获得当前扫描的信道 */
    uc_scan_chan_idx = g_scan_chan_list[pst_hmac_hid2d_acs_info->uc_cur_scan_chan_idx];
    pst_chan_info = pst_hmac_hid2d_acs_info->chan_info_5g;
    /* 获取上报的信道信息 */
    pst_chan_result = &(pst_scan_record->ast_chan_results[uc_scan_idx]);
    uc_channel_number = pst_chan_result->uc_channel_number;
    /* 如果上报的信道号和带宽与下发的信道号和带宽不匹配，则直接退出 */
    if (uc_channel_number != g_aus_channel_candidate_list_5g[uc_scan_chan_idx].uc_chan_number) {
        oam_warning_log2(0, OAM_SF_SCAN, "channel not match, scan chan is [%d], report chan is [%d]!",
            g_aus_channel_candidate_list_5g[uc_scan_chan_idx].uc_chan_number, uc_channel_number);
        return OAL_FAIL;
    }

    /* 获取扫描结果，统计占空比和底噪，并累加到chan_info中 */
    decay = HMAC_HID2D_DECAY;
    if (pst_hmac_hid2d_acs_info->update_num < pst_hmac_hid2d_acs_info->max_update_num) {
        decay = (1 + pst_hmac_hid2d_acs_info->update_num) * DECIMAL_TO_INTEGER
            / (10 + pst_hmac_hid2d_acs_info->update_num); /* 10为滑动平均公式里面因数 */
    }
    chan_load = (pst_chan_result->total_free_time_80M_us << HMAC_HID2D_CHAN_LOAD_SHIFT)
        / pst_chan_result->total_stats_time_us;
    avg_data = pst_chan_info[uc_scan_chan_idx].chan_load;
    pst_chan_info[uc_scan_chan_idx].chan_load = (decay * avg_data + (DECIMAL_TO_INTEGER - decay) * chan_load)
        / DECIMAL_TO_INTEGER;
    if (pst_chan_result->uc_free_power_cnt == 0) {
        noise_rssi = 0;
    } else {
        noise_rssi = (int8_t)(pst_chan_result->s_free_power_stats_20M
            / (int16_t)pst_chan_result->uc_free_power_cnt);
    }
    avg_data = pst_chan_info[uc_scan_chan_idx].chan_noise_rssi;
    pst_chan_info[uc_scan_chan_idx].chan_noise_rssi = (decay * avg_data + (DECIMAL_TO_INTEGER - decay) * noise_rssi)
        / DECIMAL_TO_INTEGER;
    return OAL_SUCC;
}

uint32_t hmac_hid2d_scan_complete_handler(hmac_scan_record_stru *pst_scan_record, uint8_t uc_scan_idx)
{
    hmac_hid2d_auto_channel_switch_stru *pst_hmac_hid2d_acs_info = NULL;
    uint32_t ret = OAL_SUCC;

    if (pst_scan_record == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    if (pst_hmac_hid2d_acs_info->uc_apk_scan) {
        ret = hmac_hid2d_apk_scan_handler(pst_scan_record, uc_scan_idx, pst_hmac_hid2d_acs_info);
    } else if (pst_hmac_hid2d_acs_info->uc_acs_mode) {
        ret = hmac_hid2d_acs_scan_handler(pst_scan_record, uc_scan_idx, pst_hmac_hid2d_acs_info);
    }

    return ret;
}


uint16_t hmac_hid2d_get_idx_from_chan_num(uint8_t band, uint8_t channel_num, uint8_t bw)
{
    uint8_t channel_idx = 0;
    uint8_t idx;
    hmac_hid2d_chan_stru *candidate_list = NULL;
    uint8_t total_num;

    if (band == WLAN_BAND_2G) {
        candidate_list = g_aus_channel_candidate_list_2g;
        total_num = HMAC_HID2D_CHANNEL_NUM_2G;
    } else {
        candidate_list = g_aus_channel_candidate_list_5g;
        total_num = HMAC_HID2D_CHANNEL_NUM_5G;
    }

    for (idx = 0; idx < total_num; idx++) {
        if (candidate_list[idx].uc_chan_number == channel_num &&
            candidate_list[idx].en_bandwidth == bw) {
            channel_idx = idx;
        }
    }

    return channel_idx;
}


uint32_t hmac_hid2d_get_valid_2g_channel_list(void)
{
    uint8_t *scan_chan_list = g_scan_chan_list;
    uint8_t *supp_chan_list = g_supp_chan_list;
    uint8_t max_scan_chan_num = 0;
    uint8_t max_supp_chan_num = 0;
    uint8_t idx;
    uint8_t chan_idx;
    uint8_t valid_flag1 = 0;
    uint8_t valid_flag2;

    for (idx = 0; idx < 14; idx++) { /* 2.4G上20M信道个数为14个 */
        chan_idx = g_aus_channel_candidate_list_2g[idx].uc_chan_idx;
        if (g_aus_channel_candidate_list_2g[idx].en_bandwidth == WLAN_BAND_WIDTH_20M) {
            valid_flag1 = (mac_is_channel_idx_valid(WLAN_BAND_2G, chan_idx, OAL_FALSE) == OAL_SUCC) ? 1 : 0;
        }
        if (valid_flag1) {
            supp_chan_list[max_supp_chan_num] = idx;
            max_supp_chan_num++;
        }
    }
    for (idx = 14; idx < HMAC_HID2D_CHANNEL_NUM_2G; idx++) { /* 2.4G上20M信道个数为14个 */
        chan_idx = g_aus_channel_candidate_list_2g[idx].uc_chan_idx;
        if (g_aus_channel_candidate_list_2g[idx].en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
            valid_flag1 = (mac_is_channel_idx_valid(WLAN_BAND_2G, chan_idx, OAL_FALSE) == OAL_SUCC) ? 1 : 0;
            valid_flag2 = (mac_is_channel_idx_valid(WLAN_BAND_2G,
                chan_idx + 4, OAL_FALSE) == OAL_SUCC) ? 1 : 0; /* idx+4信道号 */
        } else {
            valid_flag1 = (mac_is_channel_idx_valid(WLAN_BAND_2G, chan_idx, OAL_FALSE) == OAL_SUCC) ? 1 : 0;
            valid_flag2 = (mac_is_channel_idx_valid(WLAN_BAND_2G,
                chan_idx - 4, OAL_FALSE) == OAL_SUCC) ? 1 : 0; /* idx-4信道号 */
        }
        if (valid_flag1 && valid_flag2) {
            supp_chan_list[max_supp_chan_num] = idx;
            max_supp_chan_num++;
            scan_chan_list[max_scan_chan_num] = idx;
            max_scan_chan_num++;
        }
    }
    g_supp_chan_num_max = max_supp_chan_num;
    g_scan_chan_num_max = max_scan_chan_num;
    return OAL_SUCC;
}


uint8_t hmac_hid2d_is_valid_5g_channel(uint8_t idx)
{
    uint8_t chan_idx;
    uint8_t valid, dfs;
    uint8_t valid_flag;

    chan_idx = g_aus_channel_candidate_list_5g[idx].uc_chan_idx;
    valid = (uint8_t)mac_is_channel_idx_valid(WLAN_BAND_5G, chan_idx, OAL_FALSE);
    dfs = mac_is_dfs_channel(WLAN_BAND_5G, g_aus_channel_candidate_list_5g[idx].uc_chan_number);
    valid_flag = (valid == OAL_SUCC && dfs == OAL_FALSE) ? 1 : 0;
    return valid_flag;
}


uint32_t hmac_hid2d_get_valid_5g_channel_list(void)
{
    uint8_t *scan_chan_list = g_scan_chan_list;
    uint8_t *supp_chan_list = g_supp_chan_list;
    uint8_t max_scan_chan_num = 0;
    uint8_t max_supp_chan_num = 0;
    uint8_t idx, k;
    uint8_t valid_flag[HMAC_HID2D_MAX_CHANNELS];
    uint8_t cnt;

    idx = 0;
    while (idx < HMAC_HID2D_CHANNEL_NUM_5G - 1) {
        for (k = 0; k < HMAC_HID2D_MAX_CHANNELS; k++) {
            valid_flag[k] = hmac_hid2d_is_valid_5g_channel(idx + k);
            if (valid_flag[k] == OAL_TRUE) {
                supp_chan_list[max_supp_chan_num++] = idx + k;
            }
        }
        idx = idx + HMAC_HID2D_MAX_CHANNELS;
        if (valid_flag[0] && valid_flag[1]) {
            supp_chan_list[max_supp_chan_num++] = idx++;
            supp_chan_list[max_supp_chan_num++] = idx++;
        }
        if (valid_flag[2] && valid_flag[3]) { /* 查看80M信道中后40M信道(序号2、3)是否可用 */
            supp_chan_list[max_supp_chan_num++] = idx++;
            supp_chan_list[max_supp_chan_num++] = idx++;
        }
        if (valid_flag[0] && valid_flag[1] && valid_flag[2] && valid_flag[3]) { /* 查看各20M信道(0, 1, 2, 3)是否可用 */
            scan_chan_list[max_scan_chan_num++] = idx;
            supp_chan_list[max_supp_chan_num++] = idx++;
            supp_chan_list[max_supp_chan_num++] = idx++;
            supp_chan_list[max_supp_chan_num++] = idx++;
            supp_chan_list[max_supp_chan_num++] = idx++;
        }
    }
    cnt = max_scan_chan_num;
    for (k = 0; k < cnt; k++) {
        scan_chan_list[cnt + k] = scan_chan_list[k] + 1; /* 添加次20M为主信道的80M信道 */
        scan_chan_list[cnt * 2 + k] = scan_chan_list[k] + 2; /* 添加以第3个20M(序号2)为主信道的80M信道 */
        scan_chan_list[cnt * 3 + k] = scan_chan_list[k] + 3; /* 添加以第4个20M(序号3)为主信道的80M信道 */
        max_scan_chan_num = max_scan_chan_num + 3; /* 本轮添加了3个扫描信道 */
    }
    /* 对于165信道的特殊处理 */
    valid_flag[0] = hmac_hid2d_is_valid_5g_channel(idx);
    if (valid_flag[0] == OAL_TRUE) {
        supp_chan_list[max_supp_chan_num++] = idx;
        scan_chan_list[max_scan_chan_num++] = idx;
    }
    g_supp_chan_num_max = max_supp_chan_num;
    g_scan_chan_num_max = max_scan_chan_num;
    return OAL_SUCC;
}


hmac_hid2d_net_mode_enum hmac_hid2d_check_vap_state(mac_vap_stru *mac_vap, mac_device_stru *mac_device)
{
    hmac_hid2d_net_mode_enum state;
    uint8_t is_dbdc;
    uint8_t is_only_p2p;
    mac_vap_stru *another_vap = NULL;

    /* 判断当前vap是否是GO，如果不是则直接退出 */
    if (mac_vap->en_p2p_mode != WLAN_P2P_GO_MODE) {
        return HMAC_HID2D_NET_MODE_BUTT;
    }

    /* 判断是否存在另一个up的vap */
    another_vap = mac_vap_find_another_up_vap_by_mac_vap(mac_vap);
    /* 如果能找到除了p2p vap之外的up的vap，说明不是单p2p */
    is_only_p2p = (another_vap == NULL) ? OAL_TRUE : OAL_FALSE;
    is_dbdc = mac_is_dbdc_running(mac_device);
    if (is_only_p2p) {
        state = HMAC_HID2D_P2P_ONLY;
    } else if (is_dbdc) {
        state = HMAC_HID2D_DBDC;
    } else {
        state = HMAC_HID2D_SAME_BAND;
    }
    return state;
}


uint8_t hmac_hid2d_get_next_scan_channel(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info)
{
    uint8_t next_chan_idx;

    /* 第一次触发扫描，直接返回0 */
    if (hmac_hid2d_acs_info->uc_cur_scan_chan_idx == g_scan_chan_num_max) {
        hmac_hid2d_acs_info->uc_cur_scan_chan_idx = 0;
        return 0;
    }
    /* 先获取上次扫描的信道在g_scan_chan_list中的idx，然后加1获取本次应该扫描的信道idx */
    next_chan_idx = (hmac_hid2d_acs_info->uc_cur_scan_chan_idx + 1) % g_scan_chan_num_max;
    /* 判断是否是当前信道，如果是当前信道则直接跳过 */
    /* 当前先判断当前信道是否与扫描信道严格一致
       如果当前信道为20M/40M，芯片上报的40M/80M信息有效将判断条件改为扫描信道是否包含当前信道 */
    if (hmac_hid2d_acs_info->uc_cur_chan_idx == g_scan_chan_list[next_chan_idx]) {
        next_chan_idx = (next_chan_idx + 1) % g_scan_chan_num_max;
    }
    hmac_hid2d_acs_info->uc_cur_scan_chan_idx = next_chan_idx;
    return next_chan_idx;
}


void hmac_hid2d_acs_destroy_timer(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info)
{
    if (hmac_hid2d_acs_info->st_chan_stat_report_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(hmac_hid2d_acs_info->st_chan_stat_report_timer));
    }
    if (hmac_hid2d_acs_info->st_scan_chan_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(hmac_hid2d_acs_info->st_scan_chan_timer));
    }
}


void hmac_hid2d_acs_init_params(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info, mac_vap_stru *p2p_vap)
{
    hmac_hid2d_acs_destroy_timer(hmac_hid2d_acs_info);
    memset_s(hmac_hid2d_acs_info, sizeof(hmac_hid2d_auto_channel_switch_stru),
        0, sizeof(hmac_hid2d_auto_channel_switch_stru));
    hmac_hid2d_acs_info->uc_acs_mode = OAL_TRUE;
    hmac_hid2d_acs_info->uc_vap_id = p2p_vap->uc_vap_id;
    hmac_hid2d_acs_info->uc_cur_band = p2p_vap->st_channel.en_band;
    if (hmac_hid2d_acs_info->uc_cur_band == WLAN_BAND_2G) {
        hmac_hid2d_acs_info->candidate_list = g_aus_channel_candidate_list_2g;
        hmac_hid2d_get_valid_2g_channel_list();
    } else {
        hmac_hid2d_acs_info->candidate_list = g_aus_channel_candidate_list_5g;
        hmac_hid2d_get_valid_5g_channel_list();
    }
    hmac_hid2d_acs_info->uc_cur_chan_idx = hmac_hid2d_get_idx_from_chan_num(p2p_vap->st_channel.en_band,
        p2p_vap->st_channel.uc_chan_number, p2p_vap->st_channel.en_bandwidth);
    hmac_hid2d_acs_info->uc_cur_scan_chan_idx = g_scan_chan_num_max;
    hmac_hid2d_acs_info->scan_chan_idx = HMAC_HID2D_CHANNEL_NUM_5G;
    hmac_hid2d_acs_info->scan_time = HMAC_HID2D_SCAN_TIME_PER_CHAN_ACS;
}


void hmac_hid2d_acs_exit(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info, mac_vap_stru *p2p_vap,
    uint8_t exit_type)
{
    uint32_t ret;
    uint8_t acs_mode;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_hid2d_acs_init_report_stru acs_init_report;
    hmac_hid2d_acs_exit_report_stru acs_exit_report;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(p2p_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }
    if (exit_type == HMAC_HID2D_NON_GO) {
        acs_init_report.acs_info_type = HMAC_HID2D_INIT_REPORT;
        acs_init_report.acs_exit_type = HMAC_HID2D_NON_GO;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL, (uint8_t *)(&acs_init_report),
            sizeof(acs_init_report));
#endif
        return;
    } else if (exit_type != HMAC_HID2D_FWK_DISABLE) {
        /* 上报内核方案退出的原因 */
        acs_exit_report.acs_info_type = HMAC_HID2D_EXTI_REPORT;
        acs_exit_report.acs_exit_type = exit_type;
        oam_warning_log1(0, OAM_SF_ANY, "HiD2D ACS: exit and report to hal, exit type[%d].", exit_type);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL,
            (uint8_t *)(&acs_exit_report), sizeof(acs_exit_report));
#endif
    }

    /* 注销定时器 */
    hmac_hid2d_acs_destroy_timer(hmac_hid2d_acs_info);
    /* 将状态同步给dmac */
    acs_mode = OAL_FALSE;
    hmac_hid2d_acs_info->uc_acs_mode = acs_mode;
    hmac_hid2d_acs_info->acs_state = HMAC_HID2D_ACS_DISABLE;
    ret = hmac_config_send_event(p2p_vap, WLAN_CFGID_HID2D_ACS_MODE, sizeof(uint8_t), &acs_mode);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return;
    }
}


void hmac_hid2d_init_report(hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info, mac_vap_stru *p2p_vap,
    mac_device_stru *mac_device)
{
    hmac_hid2d_acs_init_report_stru acs_init_report;
    hmac_vap_stru *hmac_vap = NULL;
    int32_t ret;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(p2p_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }
    acs_init_report.acs_info_type = HMAC_HID2D_INIT_REPORT;
    acs_init_report.cur_band = hmac_hid2d_acs_info->uc_cur_band;
    acs_init_report.chan_number = p2p_vap->st_channel.uc_chan_number;
    acs_init_report.bandwidth = p2p_vap->st_channel.en_bandwidth;
    acs_init_report.go_mode = hmac_hid2d_check_vap_state(p2p_vap, mac_device);
    ret = memcpy_s(acs_init_report.supp_chan_list, HMAC_HID2D_CHANNEL_NUM_5G,
        g_supp_chan_list, sizeof(uint8_t) * g_supp_chan_num_max);
    acs_init_report.supp_chan_num_max = g_supp_chan_num_max;
    ret += memcpy_s(acs_init_report.scan_chan_list, HMAC_HID2D_CHANNEL_NUM_5G,
        g_supp_chan_list, sizeof(uint8_t) * g_scan_chan_num_max);
    if (ret != EOK) {
        oam_warning_log1(0, 0, "{hmac_hid2d_init_report::memcpy fail, ret[%d].}", ret);
    }
    acs_init_report.scan_chan_num_max = g_scan_chan_num_max;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 上报内核 */
    oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL,
        (uint8_t *)(&acs_init_report), sizeof(acs_init_report));
#endif
}


uint32_t hmac_hid2d_set_acs_mode(mac_vap_stru *mac_vap, uint8_t acs_mode)
{
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *p2p_vap = NULL;
    uint32_t ret;

    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;

    /* 找到对应的P2P vap并判断是否是GO，不是则直接返回 */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_hid2d_find_up_p2p_vap(mac_device, (mac_vap_stru **)&p2p_vap);
    if (ret != OAL_SUCC) {
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_P2P_LOST);
        return ret;
    }
    if (!IS_P2P_GO(p2p_vap)) {
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_NON_GO);
        return OAL_SUCC;
    }

    if (acs_mode == OAL_FALSE) {
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_FWK_DISABLE);
        return OAL_SUCC;
    }

    /* 初始化 */
    hmac_hid2d_acs_init_params(hmac_hid2d_acs_info, p2p_vap);
    /* 上传信息到hal层 */
    hmac_hid2d_init_report(hmac_hid2d_acs_info, p2p_vap, mac_device);

    /* 同步该状态到dmac层 */
    ret = hmac_config_send_event(p2p_vap, WLAN_CFGID_HID2D_ACS_MODE, sizeof(uint8_t), &acs_mode);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(p2p_vap->uc_vap_id, 0, "HiD2D ACS: send hid2d acs mode to dmac failed[%d].", ret);
        return ret;
    }
    /* 开启dmac上报超时定时器(60s) */
    frw_timer_create_timer_m(&hmac_hid2d_acs_info->st_chan_stat_report_timer,
        hmac_hid2d_chan_stat_report_timeout_handle, HMAC_HID2D_REPORT_TIMEOUT_MS,
        mac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);

    return OAL_SUCC;
}

uint32_t hmac_hid2d_update_acs_state(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info = NULL;

    oam_warning_log1(0, OAM_SF_ANY, "HiD2D ACS: acs state updated to [%d].", *param);
    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    hmac_hid2d_acs_info->acs_state = *param;
    return OAL_SUCC;
}

uint32_t hmac_hid2d_link_meas(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_device_stru *mac_device = NULL;
    mac_hid2d_link_meas_stru *hid2d_meas_cmd = NULL;
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info = NULL;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_SUCC;
    }
    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    hid2d_meas_cmd = (mac_hid2d_link_meas_stru *)param;
    if (hmac_hid2d_acs_info->uc_acs_mode == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "HiD2D ACS: acs disabled, not support link measurement.");
        return OAL_SUCC;
    }
    if (hid2d_meas_cmd->link_meas_cmd_type == HMAC_HID2D_LINK_MEAS_START_BY_CHAN_LIST) {
        hmac_hid2d_acs_info->scan_time = hid2d_meas_cmd->scan_time;
        hmac_hid2d_acs_info->scan_interval = hid2d_meas_cmd->scan_interval;
        oam_warning_log2(0, OAM_SF_ANY, "HiD2D ACS: start link measurement by default scan list, scan time[%u]ms, \
            scan interval[%u]s.", hmac_hid2d_acs_info->scan_time, hmac_hid2d_acs_info->scan_interval);
        frw_timer_create_timer_m(&hmac_hid2d_acs_info->st_scan_chan_timer,
            hmac_hid2d_acs_timeout_handle,
            HMAC_HID2D_SCAN_TIMER_CYCLE_MS * hmac_hid2d_acs_info->scan_interval,
            mac_device,
            OAL_FALSE,
            OAM_MODULE_ID_HMAC,
            mac_device->core_id);
    } else if (hid2d_meas_cmd->link_meas_cmd_type == HMAC_HID2D_LINK_MEAS_UPDATE_SCAN_INTERVAL) {
        hmac_hid2d_acs_info->scan_interval = hid2d_meas_cmd->scan_interval;
        oam_warning_log1(0, OAM_SF_ANY, "HiD2D ACS: update scan interval[%u]s.", hmac_hid2d_acs_info->scan_interval);
        if (hmac_hid2d_acs_info->st_scan_chan_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(hmac_hid2d_acs_info->st_scan_chan_timer));
        }
        frw_timer_create_timer_m(&hmac_hid2d_acs_info->st_scan_chan_timer, hmac_hid2d_acs_timeout_handle,
            HMAC_HID2D_SCAN_TIMER_CYCLE_MS * hmac_hid2d_acs_info->scan_interval,
            mac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
    } else if (hid2d_meas_cmd->link_meas_cmd_type == HMAC_HID2D_LINK_MEAS_UPDATE_SCAN_TIME) {
        hmac_hid2d_acs_info->scan_time = hid2d_meas_cmd->scan_time;
        oam_warning_log1(0, OAM_SF_ANY, "HiD2D ACS: update scan time[%u]ms.", hmac_hid2d_acs_info->scan_time);
    } else if (hid2d_meas_cmd->link_meas_cmd_type == HMAC_HID2D_LINK_MEAS_START_BY_CHAN) {
        hmac_hid2d_acs_info->scan_chan_idx = hid2d_meas_cmd->scan_chan;
        hmac_hid2d_acs_info->scan_time = hid2d_meas_cmd->scan_time;
        oam_warning_log2(0, OAM_SF_ANY, "HiD2D ACS: scan channel idx[%d] once for [%u]ms.",
            hmac_hid2d_acs_info->scan_chan_idx, hmac_hid2d_acs_info->scan_time);
        hmac_hid2d_scan_chan_once(mac_vap, hmac_hid2d_acs_info->scan_chan_idx, OAL_FALSE);
    }

    return OAL_SUCC;
}


uint32_t hmac_hid2d_acs_timeout_handle(void *arg)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *p2p_vap = NULL;
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info = NULL;
    uint8_t next_chan_idx;
    uint8_t next_scan_chan;
    uint32_t ret;

    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    mac_device = (mac_device_stru *)arg;
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_hid2d_find_up_p2p_vap(mac_device, (mac_vap_stru **)&p2p_vap);
    if (ret != OAL_SUCC) {
        /* P2P 断链, 退出方案 */
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_P2P_LOST);
        return ret;
    }

    if (hmac_hid2d_acs_info->acs_state == HMAC_HID2D_ACS_DISABLE) {
        return OAL_SUCC;
    } else if (hmac_hid2d_acs_info->acs_state == HMAC_HID2D_ACS_WORK) {
        next_chan_idx = hmac_hid2d_get_next_scan_channel(hmac_hid2d_acs_info);
        next_scan_chan = g_scan_chan_list[next_chan_idx];
        hmac_hid2d_scan_chan_once(p2p_vap, next_scan_chan, OAL_FALSE);
    }
    /* 开启下一次扫描的定时器 */
    frw_timer_create_timer_m(&hmac_hid2d_acs_info->st_scan_chan_timer,
        hmac_hid2d_acs_timeout_handle,
        HMAC_HID2D_SCAN_TIMER_CYCLE_MS * hmac_hid2d_acs_info->scan_interval,
        mac_device,
        OAL_FALSE,
        OAM_MODULE_ID_HMAC,
        mac_device->core_id);
    return OAL_SUCC;
}


uint32_t hmac_hid2d_acs_scan_handler(hmac_scan_record_stru *scan_record, uint8_t scan_idx,
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info)
{
    hmac_hid2d_chan_stru *candidate_chan_list = NULL;
    wlan_scan_chan_stats_stru *chan_result = NULL;
    hmac_hid2d_chan_stat_report_stru chan_stats_report;
    uint8_t channel_number;
    uint8_t channel_idx;
    hmac_vap_stru *hmac_vap = NULL;

    /* 如果不是acs下发的扫描，扫描结果不处理 */
    if (scan_record->en_scan_mode != WLAN_SCAN_MODE_HID2D_SCAN) {
        return OAL_FAIL;
    }

    hmac_vap = mac_res_get_hmac_vap(scan_record->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }
    /* 获取当前上报的信息信息 */
    chan_result = &(scan_record->ast_chan_results[scan_idx]);
    channel_number = chan_result->uc_channel_number;
    candidate_chan_list = hmac_hid2d_acs_info->candidate_list;
    if (hmac_hid2d_acs_info->scan_chan_idx < HMAC_HID2D_CHANNEL_NUM_5G) {
        channel_idx = hmac_hid2d_acs_info->scan_chan_idx;
    } else {
        channel_idx = g_scan_chan_list[hmac_hid2d_acs_info->uc_cur_scan_chan_idx];
    }
    /* 判断上报的信道与下发扫描的信道是否一致 */
    if (channel_number != candidate_chan_list[channel_idx].uc_chan_number) {
        oam_warning_log2(0, 0, "HiD2D ACS: channel not match, scan chan is [%d], report chan is [%d]!",
            candidate_chan_list[channel_idx].uc_chan_number, channel_number);
        return OAL_FAIL;
    }
    /* 上报信息 */
    chan_stats_report.acs_info_type = HMAC_HID2D_CHAN_STAT_REPORT;
    chan_stats_report.scan_chan_sbs = hmac_hid2d_acs_info->uc_cur_scan_chan_idx;
    chan_stats_report.scan_chan_idx = channel_idx;
    chan_stats_report.total_free_time_20m_us = chan_result->total_free_time_20M_us;
    chan_stats_report.total_free_time_40m_us = chan_result->total_free_time_40M_us;
    chan_stats_report.total_free_time_80m_us = chan_result->total_free_time_80M_us;
    chan_stats_report.total_send_time_us = chan_result->total_send_time_us;
    chan_stats_report.total_recv_time_us = chan_result->total_recv_time_us;
    chan_stats_report.total_stats_time_us = chan_result->total_stats_time_us;
    chan_stats_report.free_power_cnt = chan_result->uc_free_power_cnt;
    chan_stats_report.free_power_stats_20m = chan_result->s_free_power_stats_20M;
    chan_stats_report.free_power_stats_40m = chan_result->s_free_power_stats_40M;
    chan_stats_report.free_power_stats_80m = chan_result->s_free_power_stats_80M;
    chan_stats_report.free_power_stats_160m = chan_result->s_free_power_stats_160M;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL,
        (uint8_t *)(&chan_stats_report), sizeof(chan_stats_report));
#endif
    return OAL_SUCC;
}


void hmac_hid2d_acs_switch_completed(mac_vap_stru *mac_vap)
{
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info = NULL;
    hmac_hid2d_switch_report_stru switch_info;
    hmac_vap_stru *hmac_vap = NULL;

    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;

    if (mac_vap->en_p2p_mode != WLAN_P2P_GO_MODE || hmac_hid2d_acs_info->uc_acs_mode != OAL_TRUE) {
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    /* 将切换后的信道和带宽信息上报内核 */
    oam_warning_log0(0, OAM_SF_FRW, "HiD2D ACS: switch completed, report to hal.\n");
    switch_info.acs_info_type = HMAC_HID2D_SWITCH_SUCC_REPORT;
    switch_info.chan_number = mac_vap->st_channel.uc_chan_number;
    switch_info.bandwidth = mac_vap->st_channel.en_bandwidth;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL,
        (uint8_t *)(&switch_info), sizeof(switch_info));
#endif

    return;
}


uint32_t hmac_hid2d_cur_chan_stat_handler(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_hid2d_chan_stat_stru *chan_stats = NULL;
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info = NULL;

    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_hid2d_link_stat_report_stru link_stats;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    chan_stats = (hmac_hid2d_chan_stat_stru *)param;
    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 先清除超时上报定时器 */
    if (hmac_hid2d_acs_info->st_chan_stat_report_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(hmac_hid2d_acs_info->st_chan_stat_report_timer));
    }

    /* 上报内核 */
    link_stats.acs_info_type = HMAC_HID2D_LINK_INFO_REPORT;
    link_stats.chan_number = mac_vap->st_channel.uc_chan_number;
    link_stats.bandwidth = mac_vap->st_channel.en_bandwidth;
    link_stats.go_mode = hmac_hid2d_check_vap_state(mac_vap, mac_device);
    if (link_stats.go_mode == HMAC_HID2D_NET_MODE_BUTT) {
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, mac_vap, HMAC_HID2D_P2P_LOST);
        return OAL_SUCC;
    }
    memcpy_s(&(link_stats.link_stat), sizeof(hmac_hid2d_chan_stat_stru),
        chan_stats, sizeof(hmac_hid2d_chan_stat_stru));
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_hid2d_acs_report(hmac_vap->pst_net_device, GFP_KERNEL,
        (uint8_t *)(&link_stats), sizeof(link_stats));
#endif
    /* 开启dmac上报超时定时器 */
    frw_timer_create_timer_m(&hmac_hid2d_acs_info->st_chan_stat_report_timer,
        hmac_hid2d_chan_stat_report_timeout_handle, HMAC_HID2D_REPORT_TIMEOUT_MS,
        mac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
    return OAL_SUCC;
}


uint32_t hmac_hid2d_chan_stat_report_timeout_handle(void *arg)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *p2p_vap = NULL;
    hmac_hid2d_auto_channel_switch_stru *hmac_hid2d_acs_info;
    uint32_t ret;

    hmac_hid2d_acs_info = &g_st_hmac_hid2d_acs_info;
    mac_device = (mac_device_stru *)arg;
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_hid2d_find_up_p2p_vap(mac_device, (mac_vap_stru **)&p2p_vap);
    if (ret != OAL_SUCC) {
        /* P2P 断链, 退出方案 */
        hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_P2P_LOST);
        return ret;
    }
    oam_warning_log0(p2p_vap->uc_vap_id, 0,
        "{HiD2D ACS: Too long to obtain the channel stats report from dmac. Disable ACS.");
    hmac_hid2d_acs_exit(hmac_hid2d_acs_info, p2p_vap, HMAC_HID2D_DMAC_REPORT_TIMEOUT);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION

uint32_t wal_hipriv_hid2d_get_best_chan(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *p_ifr)
{
    mac_device_stru *pst_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_channel_stru *pst_best_chan = NULL;
    uint32_t result;
    uint8_t prim_chan, bw;

    pst_mac_vap = (mac_vap_stru *)oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::OAL_NET_DEV_PRIV(pst_net_dev) null.}");
        return OAL_FAIL;
    }
    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
        if (pst_device->is_ready_to_get_scan_result == OAL_FALSE) {
            oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::scan result is not ready.}");
            result = 0;
        } else {
            pst_best_chan = &(pst_device->st_best_chan_for_hid2d);
            oam_warning_log2(0, OAM_SF_ANY, "HiD2D Presentation::Best channel number %d, bandwidth mode %d",
                pst_best_chan->uc_chan_number, pst_best_chan->en_bandwidth);
            prim_chan = pst_best_chan->uc_chan_number;
            bw = (pst_best_chan->en_bandwidth == WLAN_BAND_WIDTH_20M) ? 20 : 0; /* 将带宽类型转化为具体带宽值20M */
            bw = (pst_best_chan->en_bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS && bw == 0) ? 40 : 80; /* 40M或80M */
            // 拼成一个32位的int值传递给上层: 0-7Bit为带宽，8-15bit为主信道
            result = (uint32_t)((prim_chan << 8) | bw);
        }
        // 传给上层
        oal_copy_to_user(p_ifr->ifr_data + BYTE_OFFSET_8, &result, sizeof(uint32_t));
    }
    return OAL_SUCC;
}


uint32_t wal_hipriv_hid2d_presentation_init(oal_net_device_stru *net_dev, int8_t *cmd)
{
    oal_net_device_stru *hisilicon_net_dev = NULL;
    int8_t setcountry[] = {" 99"};
    int8_t fixcountry[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {"hid2d_presentation"};
    uint32_t ret_type;
    int32_t ret;

    hisilicon_net_dev = wal_config_get_netdev("Hisilicon0", OAL_STRLEN("Hisilicon0"));
    if (hisilicon_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::wal_config_get_netdev return null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(hisilicon_net_dev);
    /* 设置国家码需要使用"Hisilicon0"下命令 */
    ret_type = wal_hipriv_setcountry(hisilicon_net_dev, setcountry);
    if (ret_type != OAL_SUCC) {
        return ret_type;
    }
    ret = strcat_s(fixcountry, sizeof(fixcountry), cmd);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    ret_type = wal_hipriv_set_val_cmd(net_dev, fixcountry);
    if (ret_type != OAL_SUCC) {
        return ret_type;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_hid2d_opt_cfg(oal_net_device_stru *net_dev, int8_t *cmd, uint8_t cmd_type)
{
    int8_t alg_cfg_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t ret;
    uint8_t idx;

    hmac_ioctl_hid2d_apk_cfg_stru hid2d_alg_cfg_cmd_tbl[] = {
        {WAL_HID2D_HIGH_BW_MCS_CMD, "hid2d_disable_1024qam", {0, 0, 0}},
        {WAL_HID2D_RTS_CMD, "hid2d_always_rts", {0, 0, 0}},
        {WAL_HID2D_TPC_CMD, "hid2d_set_high_txpower", {0, 0, 0}},
        {WAL_HID2D_CCA_CMD, "hid2d_set_cca", {0, 0, 0}}
    };
    for (idx = 0; idx < oal_array_size(hid2d_alg_cfg_cmd_tbl); idx++) {
        if (cmd_type == hid2d_alg_cfg_cmd_tbl[idx].alg_cfg_type) {
            ret = strcat_s(alg_cfg_cmd, sizeof(alg_cfg_cmd), hid2d_alg_cfg_cmd_tbl[idx].name);
            if (ret != EOK) {
                return OAL_FAIL;
            }
            ret = strcat_s(alg_cfg_cmd, sizeof(alg_cfg_cmd), cmd);
            if (ret != EOK) {
                return OAL_FAIL;
            }
            return wal_hipriv_alg_cfg(net_dev, alg_cfg_cmd);
        }
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_hid2d_edca_cfg(oal_net_device_stru *net_dev, int8_t *cmd)
{
    oal_net_device_stru *p2p_net_dev = NULL;
    int8_t edca_cmd[] = {"edca 0 aifsn 1 cwmin 0 cwmax 0"};
    int8_t value_cmd[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    int8_t alg_edca_enable[] = {"edca_opt_en_sta 1"};
    int8_t alg_edca_disable[] = {"edca_opt_en_sta 0"};
    uint8_t tid_no_idx = 5; /* edca_cmd中第5个字符为tid_no */
    uint8_t value, idx, replace_idx, tid_idx;
    uint32_t ret, offset;

    /* wlan0 phy_debug [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] */
    /* 不同档位对应的edca参数: aifsn cwmin cwmax */
    int8_t tid_no[4] = { '0', '1', '2', '3' }; /* 4个队列 */
    int8_t edca_cfg[5][3] = { /* 5个设置档位，每个档位配置3个参数 */
        { '1', '0', '0' }, { '1', '1', '1' }, { '1', '1', '2' }, { '2', '1', '2' }, { '2', '2', '3' }
    };
    /* 需要替换的字符(value)在edca_cmd中的下标 */
    uint8_t params_idx[] = { 13, 21, 29 };

    ret = wal_get_cmd_one_arg(cmd, value_cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    value = (uint8_t)oal_atoi(value_cmd);
    if (value > 5) { /* 共提供5个档位的edca配置 */
        oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::invalid esca params.}");
        return OAL_FAIL;
    }
    p2p_net_dev = wal_config_get_netdev("p2p0", OAL_STRLEN("p2p0"));
    if (p2p_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::wal_config_get_netdev return null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(p2p_net_dev);
    /* 恢复动态edca参数调整 */
    if (value == 0) {
        return wal_hipriv_alg_cfg(p2p_net_dev, alg_edca_enable);
    }
    /* 先关闭动态edca参数调整 */
    ret = wal_hipriv_alg_cfg(p2p_net_dev, alg_edca_disable);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 根据档位调整全部bk be vi vo队列的edca参数 */
    for (tid_idx = 0; tid_idx < 4; tid_idx++) { /* 依次对4个队列进行参数配置 */
        edca_cmd[tid_no_idx] = tid_no[tid_idx];
        for (idx = 0; idx < 3; idx++) { /* 每个队列需要配置3个edca参数 */
            replace_idx = params_idx[idx];
            edca_cmd[replace_idx] = edca_cfg[value - 1][idx];
        }
        wal_hipriv_set_phy_debug_switch(p2p_net_dev, edca_cmd);
        wal_hipriv_set_phy_debug_switch(net_dev, edca_cmd);
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_hid2d_swchan(oal_net_device_stru *net_dev, int8_t *cmd)
{
    int8_t sw_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {" "};
    int8_t value_cmd[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t ret_type, value, offset;
    int32_t ret;
    uint16_t prim_chan, bw;

    /* 转化下信道与带宽然后下发命令 */
    ret_type = wal_get_cmd_one_arg(cmd, value_cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret_type != OAL_SUCC) {
        return ret_type;
    }
    value = (uint32_t)oal_atoi(value_cmd);
    prim_chan = (uint16_t)(value >> 8); /* 前8位表示主信道号 */
    bw = (uint16_t)(value & 0xFF); /* 后8位表示带宽 */

    memset_s(value_cmd, sizeof(value_cmd), 0, sizeof(value_cmd));
    oal_itoa(prim_chan, value_cmd, sizeof(value_cmd));
    ret = strcat_s(sw_cmd, sizeof(sw_cmd), value_cmd);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    ret = strcat_s(sw_cmd, sizeof(sw_cmd), " ");
    if (ret != EOK) {
        return OAL_FAIL;
    }
    memset_s(value_cmd, sizeof(value_cmd), 0, sizeof(value_cmd));
    oal_itoa(bw, value_cmd, sizeof(value_cmd));
    ret = strcat_s(sw_cmd, sizeof(sw_cmd), value_cmd);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    ret_type = wal_hipriv_hid2d_switch_channel(net_dev, sw_cmd);
    if (ret_type != OAL_SUCC) {
        return ret_type;
    }
    return OAL_SUCC;
}


uint32_t wal_vendor_priv_cmd_hid2d_apk(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr,
    uint8_t *cmd)
{
    int8_t cmd_type_str[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    int32_t cmd_type;
    uint32_t ret, offset;
    uint8_t cmd_idx;
    uint8_t is_valid_cmd = OAL_FALSE;

    /* 判断是不是HiD2D APK相关命令, 非HiD2D APK命令，直接返回 */
    if (oal_strncasecmp(cmd, CMD_HID2D_PARAMS, CMD_HID2D_PARAMS_LEN) != 0) {
        return OAL_SUCC;
    }
    /* 获取type值 */
    cmd += CMD_HID2D_PARAMS_LEN;
    ret = wal_get_cmd_one_arg(cmd, cmd_type_str, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d_apk::invalid cmd}");
        return ret;
    }
    cmd_type = (int32_t)oal_atoi(cmd_type_str);
    cmd += offset;

    /* 解析Type值 */
    if (cmd_type == 256) { /* 256为返回最佳信道的cmd type */
        return wal_hipriv_hid2d_get_best_chan(net_dev, ifr);
    }
    if ((uint8_t)cmd_type >= WAL_HID2D_HIGH_BW_MCS_CMD && (uint8_t)cmd_type <= WAL_HID2D_CCA_CMD) {
        return wal_hipriv_hid2d_opt_cfg(net_dev, cmd, (uint8_t)cmd_type);
    }
    for (cmd_idx = 0; cmd_idx < oal_array_size(g_hid2d_vendor_cmd); cmd_idx++) {
        if ((uint8_t)cmd_type == g_hid2d_vendor_cmd[cmd_idx].hid2d_cfg_type) {
            is_valid_cmd = OAL_TRUE;
            return g_hid2d_vendor_cmd[cmd_idx].func(net_dev, cmd);
        }
    }
    if (is_valid_cmd == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::invalid cmd}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif

uint32_t wal_hipriv_hid2d_switch_channel(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_csa_debug_stru st_csa_cfg;
    uint8_t uc_channel;
    wlan_channel_bandwidth_enum_uint8 en_bw_mode;
    int32_t l_ret;
    uint32_t ret;

    memset_s(&st_csa_cfg, sizeof(st_csa_cfg), 0, sizeof(st_csa_cfg));

    /* 获取要切换的主信道信道号 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{HiD2D Presentation::get channel error, return.}");
        return ret;
    }
    pc_param += off_set;
    uc_channel = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 获取信道带宽模式 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{HiD2D Presentation::get bandwidth error, return.}");
        return ret;
    }
    ret = mac_regdomain_get_bw_mode_by_cmd(ac_name, uc_channel, &en_bw_mode);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param += off_set;
    st_csa_cfg.en_bandwidth = en_bw_mode;
    st_csa_cfg.uc_channel = uc_channel;

    oam_warning_log2(0, OAM_SF_CFG, "{HiD2D Presentation::switch to CH%d, BW is %d}", uc_channel, en_bw_mode);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_HID2D_SWITCH_CHAN, sizeof(st_csa_cfg));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        (const void *)&st_csa_cfg, sizeof(st_csa_cfg))) {
        oam_error_log0(0, OAM_SF_ANY, "HiD2D Presentation::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_csa_cfg),
        (uint8_t *)&st_write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{HiD2D Presentation::err[%d]!}", ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_hid2d_scan_channel(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t                      off_set;
    int8_t                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t                      ret;
    uint32_t                      value;
    uint32_t                      cmd_id;
    uint32_t                      offset = 0;

    /* 获取每个信道的扫描次数 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_hid2d_scan_channel::wal_get_cmd_one_arg fail}");
        return ret;
    }
    pc_param += off_set;
    value = oal_atoi(ac_name);

    ret = wal_get_cmd_id("scan_chan", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_hid2d_scan_channel:find scan_chan cmd is fail");
        return ret;
    }
    return wal_process_cmd_params(net_dev, cmd_id, &value);
}
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP

int32_t wal_ioctl_set_hid2d_state(oal_net_device_stru *pst_net_dev, uint8_t uc_param,
    wal_wifi_priv_cmd_stru *priv_cmd)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    if (uc_param == ENABLE) {
        hi110x_hcc_ip_pm_ctrl(DISABLE);
        wal_hid2d_sleep_mode(pst_net_dev, DISABLE, priv_cmd);
        wal_hid2d_napi_mode(pst_net_dev, DISABLE);
        wal_hid2d_freq_boost(pst_net_dev, ENABLE);
    } else if (uc_param == DISABLE) {
        hi110x_hcc_ip_pm_ctrl(ENABLE);
        wal_hid2d_sleep_mode(pst_net_dev, ENABLE, priv_cmd);
        wal_hid2d_napi_mode(pst_net_dev, ENABLE);
        wal_hid2d_freq_boost(pst_net_dev, DISABLE);
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_HID2D_STATE, sizeof(uint8_t));

    st_write_msg.auc_value[0] = uc_param;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_hid2d_state::return err code [%d]!}", l_ret);
        return (int32_t)l_ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_hid2d_state::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}
#endif
#endif /* end of this file */

