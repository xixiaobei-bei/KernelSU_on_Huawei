

#ifndef __HMAC_DFS_H__
#define __HMAC_DFS_H__

#ifdef _PRE_WLAN_FEATURE_DFS

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "hmac_vap.h"
#include "hmac_scan.h"

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFS_H

/* 2 宏定义 */
#define HMAC_DFS_ONE_SEC_IN_MS 1000
#define HMAC_DFS_ONE_MIN_IN_MS (60 * HMAC_DFS_ONE_SEC_IN_MS)
#define HMAC_DFS_SIXTY_TWO_SEC_IN_MS (62 * HMAC_DFS_ONE_SEC_IN_MS)

/* CAC检测时长，5600MHz ~ 5650MHz频段外，默认60秒 */
#define HMAC_DFS_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS HMAC_DFS_SIXTY_TWO_SEC_IN_MS

/* CAC检测时长，5600MHz ~ 5650MHz频段内，默认10分钟 */
#define HMAC_DFS_CAC_IN_5600_TO_5650_MHZ_TIME_MS (10 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段外，默认6分钟 */
#define HMAC_DFS_OFF_CH_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS (6 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段内，默认60分钟 */
#define HMAC_DFS_OFF_CH_CAC_IN_5600_TO_5650_MHZ_TIME_MS (60 * HMAC_DFS_ONE_MIN_IN_MS)

/* Non-Occupancy Period时长，默认30分钟 */
#define HMAC_DFS_NON_OCCUPANCY_PERIOD_TIME_MS (30 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC在工作信道上的驻留时长 */
#define HMAC_DFS_OFF_CHAN_CAC_PERIOD_TIME_MS 15

/* Off-channel CAC在Off-channel信道上的驻留时长 */
#define HMAC_DFS_OFF_CHAN_CAC_DWELL_TIME_MS 30

#define HMAC_DFS_IS_INVALID_RADAR_CHAN(_pst_mac_device, _uc_ch) \
    ((_pst_mac_device->st_ap_channel_list[st_chan_info.ast_channels[_uc_ch].uc_idx].en_ch_status != \
     MAC_CHAN_DFS_REQUIRED) && \
     (_pst_mac_device->st_ap_channel_list[st_chan_info.ast_channels[_uc_ch].uc_idx].en_ch_status != \
     MAC_CHAN_BLOCK_DUE_TO_RADAR))

extern uint8_t g_go_cac;

typedef struct {
    uint8_t uc_chan_idx;  /* 信道索引 */
    uint8_t uc_device_id; /* device id */
    uint8_t auc_resv[NUM_2_BYTES];
    frw_timeout_stru st_dfs_nol_timer; /* NOL节点定时器 */
    oal_dlist_head_stru st_entry;      /* NOL链表 */
} mac_dfs_nol_node_stru;

typedef enum _hmac_dfs_status_ {
    HMAC_CAC_START = 1,
    HMAC_INS_START,
    HMAC_CAC_DETECT,
    HMAC_INS_DETECT,
    HMAC_CAC_STOP,
    HMAC_INS_STOP,
    HMAC_BACK_80M,

    HMAC_DFS_STUS_BUTT
} hmac_dfs_status_enum;

typedef struct {
    struct timeval st_time;
    uint32_t en_dfs_status; // 枚举定义hmac_dfs_status_enum
    uint8_t auc_name[OAL_IF_NAME_SIZE];
    uint16_t us_freq;
    uint8_t uc_bw;
    uint8_t bit_go_cac_forbit_scan : 1,
              bit_rsv : 7;
} hmac_dfs_radar_result_stru;

extern hmac_dfs_radar_result_stru g_st_dfs_result;

void hmac_dfs_init(mac_device_stru *mac_device);
void hmac_dfs_channel_list_init(mac_device_stru *mac_device);
uint32_t hmac_dfs_recalculate_channel(mac_device_stru *mac_device, uint8_t *freq,
    wlan_channel_bandwidth_enum_uint8 *bandwidth);
void hmac_dfs_cac_start(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap);
void hmac_dfs_cac_stop(mac_device_stru *mac_device, mac_vap_stru *mac_vap);
void hmac_dfs_off_cac_stop(mac_device_stru *mac_device, mac_vap_stru *mac_vap);
uint32_t hmac_switch_to_new_chan_complete(frw_event_mem_stru *event_mem);
uint32_t hmac_dfs_radar_detect_event(frw_event_mem_stru *event_mem);
uint32_t hmac_dfs_radar_detect_event_test(uint8_t vap_id);
uint32_t hmac_dfs_ap_wait_start_radar_handler(hmac_vap_stru *hmac_vap);
uint32_t hmac_dfs_ap_up_radar_handler(hmac_vap_stru *hmac_vap);
void hmac_dfs_radar_wait(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap);
uint32_t hmac_dfs_start_bss(hmac_vap_stru *hmac_vap);
void hmac_dfs_off_chan_cac_start(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap);
oal_bool_enum_uint8 hmac_dfs_try_cac(hmac_device_stru *hmac_device, mac_vap_stru *mac_vap);
uint32_t hmac_dfs_init_scan_hook(hmac_scan_record_stru *scan_record, hmac_device_stru *dev);
uint32_t hmac_dfs_go_cac_check(mac_vap_stru *mac_vap);
void hmac_dfs_status_set(uint32_t dfs_status);
void hmac_cac_chan_ctrl_machw_tx(mac_vap_stru *pst_mac_vap, uint8_t uc_cac_machw_en);
void hmac_chan_multi_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
                                           uint8_t uc_channel,
                                           wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_cac_abort_scan_check(hmac_device_stru *pst_hmac_device);
void mac_dfs_init(mac_device_stru *pst_mac_device);
void mac_dfs_set_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
void mac_dfs_set_offchan_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
oal_bool_enum_uint8 mac_dfs_get_offchan_cac_enable(mac_device_stru *pst_mac_device);
oal_bool_enum_uint8 mac_dfs_get_cac_enable(mac_device_stru *pst_mac_device);
void mac_dfs_set_dfs_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
oal_bool_enum_uint8 mac_dfs_get_dfs_enable(mac_device_stru *pst_mac_device);
void mac_dfs_set_debug_level(mac_device_stru *pst_mac_device, uint8_t uc_debug_lev);

oal_bool_enum_uint8 mac_vap_get_dfs_enable(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_dfs_another_dev_cac_start(mac_vap_stru *pst_mac_vap, mac_cfg_cac_stru *cac_param);
void hmac_config_start_zero_wait_dfs_handle(mac_vap_stru *mac_vap, mac_cfg_channel_param_stru *channel_info);
uint32_t hmac_dfs_zero_wait_report_event(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t hmac_sta_sync_vap(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel,
    wlan_protocol_enum_uint8 en_protocol);
uint32_t hmac_chan_start_bss(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel,
                             wlan_protocol_enum_uint8 en_protocol);

/* 11 内联函数定义 */

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_dfs_need_for_cac(mac_device_stru *mac_device,
    mac_vap_stru *mac_vap)
{
    uint8_t idx;
    uint8_t ch;
    uint32_t ret;
    mac_channel_list_stru st_chan_info;
    uint8_t dfs_ch_cnt = 0;
    mac_channel_stru *channel = NULL;

    /* dfs使能位 */
    if (OAL_FALSE == mac_vap_get_dfs_enable(mac_vap)) {
        oam_warning_log0(0, OAM_SF_DFS, "dfs not enable\n");
        return OAL_FALSE;
    }

    /* 定时器使能位 */
    if (mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_DFS, "dfs tiemer enabled\n");
        return OAL_FALSE;
    }

    channel = &mac_vap->st_channel;

    if (channel->en_band != WLAN_BAND_5G) {
        oam_warning_log0(0, OAM_SF_DFS, "dfs no need on 2G channel\n");
        return OAL_FALSE;
    }

    /* 获取信道索引 */
    ret = mac_get_channel_idx_from_num(MAC_RC_START_FREQ_5, channel->uc_chan_number, OAL_FALSE, &idx);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_DFS, "get ch fail, band=%d ch=%d\n", MAC_RC_START_FREQ_5, channel->uc_chan_number);
        return OAL_FALSE;
    }

    mac_get_ext_chan_info(idx, channel->en_bandwidth, &st_chan_info);

    for (ch = 0; ch < st_chan_info.channels; ch++) {
        /* 信道状态 */
        if (HMAC_DFS_IS_INVALID_RADAR_CHAN(mac_device, ch)) {
            dfs_ch_cnt++;
        }
    }

    if (dfs_ch_cnt == ch) {
        oam_warning_log2(0, OAM_SF_DFS, "all of the ch(pri_ch=%d,bw=%d) are not dfs channel,not need cac\n",
            channel->uc_chan_number, channel->en_bandwidth);
        return OAL_FALSE;
    }

    /* CAC使能位 */
    if (OAL_FALSE == mac_dfs_get_cac_enable(mac_device)) {
        oam_warning_log0(0, OAM_SF_DFS, "cac not enabled");
        return OAL_FALSE;
    }

    if (OAL_FAIL == hmac_dfs_go_cac_check(mac_vap)) {
        oam_warning_log0(0, OAM_SF_DFS, "This time GO do not CAC,recover nexttime.");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE hmac_dfs_radar_result_stru *hmac_dfs_result_info_get(void)
{
    return &g_st_dfs_result;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of _PRE_WLAN_FEATURE_DFS */

#endif /* end of hmac_dfs.h */
