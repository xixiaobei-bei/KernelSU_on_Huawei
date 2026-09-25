

#ifndef __HMAC_DFS_H__
#define __HMAC_DFS_H__

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif


#ifdef _PRE_WLAN_FEATURE_DFS

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "hmac_vap.h"
#include "hmac_scan.h"

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

extern oal_uint8 g_go_cac;

typedef enum _hmac_dfs_status_ {
    HMAC_CAC_START = 1,
    HMAC_INS_START,
    HMAC_CAC_DETECT,
    HMAC_INS_DETECT,
    HMAC_CAC_STOP,
    HMAC_INS_STOP,
    HMAC_BACK_80M,

    HMAC_DFS_STUS_BUTT
} hmac_dfs_status;
typedef oal_uint32 hmac_dfs_status_enum_uint32;

typedef struct {
    struct timeval st_time;
    hmac_dfs_status_enum_uint32 en_dfs_status;
    oal_uint8 auc_name[OAL_IF_NAME_SIZE];
    oal_uint16 us_freq;
    oal_uint8 uc_bw;
    oal_uint8 bit_go_cac_forbit_scan : 1,
              bit_rsv : 7;
} hmac_dfs_radar_result_stru;

extern hmac_dfs_radar_result_stru g_st_dfs_result;

extern oal_void hmac_dfs_init_etc(mac_device_stru *pst_mac_device);
extern oal_void hmac_dfs_channel_list_init_etc(mac_device_stru *pst_mac_device);
extern oal_uint32 hmac_dfs_recalculate_channel_etc(mac_device_stru *pst_mac_device,
                                                   oal_uint8 *puc_freq,
                                                   wlan_channel_bandwidth_enum_uint8 *pen_bandwidth);
extern oal_void hmac_dfs_cac_start_etc(mac_device_stru *pst_mac_device, hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_dfs_cac_stop_etc(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_void hmac_dfs_off_cac_stop_etc(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 hmac_dfs_radar_detect_event_etc(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 hmac_dfs_radar_detect_event_test(oal_uint8 uc_vap_id);
extern oal_uint32 hmac_dfs_ap_wait_start_radar_handler_etc(hmac_vap_stru *pst_hmac_vap);
extern oal_uint32 hmac_dfs_ap_up_radar_handler_etc(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_dfs_radar_wait_etc(mac_device_stru *pst_mac_device, hmac_vap_stru *pst_hmac_vap);
extern oal_void test_dfs(oal_uint8 uc_vap_id);
extern oal_void test_csa(oal_uint8 uc_vap_id, oal_uint8 uc_chan_id, oal_uint8 uc_sw_cnt);
extern oal_uint32 hmac_dfs_start_bss_etc(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_dfs_off_chan_cac_start_etc(mac_device_stru *pst_mac_device, hmac_vap_stru *pst_hmac_vap);
extern oal_bool_enum_uint8 hmac_dfs_try_cac_etc(hmac_device_stru *pst_hmac_device, mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_dfs_init_scan_hook_etc(hmac_scan_record_stru *pst_scan_record,
                                              hmac_device_stru *pst_dev);
extern oal_uint32 hmac_dfs_go_cac_check(mac_vap_stru *p_mac_vap);
extern oal_void hmac_dfs_status_set(oal_uint32 dfs_status);

/* 11 内联函数定义 */

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_dfs_need_for_cac(mac_device_stru *pst_mac_device,
                                                                mac_vap_stru *pst_mac_vap)
{
    oal_uint8 uc_idx;
    oal_uint8 uc_ch;
    oal_uint32 ul_ret;
    mac_channel_list_stru st_chan_info;
    oal_uint8 uc_dfs_ch_cnt = 0;
    mac_channel_stru *pst_channel = OAL_PTR_NULL;

    /* dfs使能位 */
    if (OAL_FALSE == mac_vap_get_dfs_enable(pst_mac_vap)) {
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "dfs not enable\n");
        return OAL_FALSE;
    }

    /* 定时器使能位 */
    if (pst_mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "dfs tiemer enabled\n");
        return OAL_FALSE;
    }

    pst_channel = &pst_mac_vap->st_channel;

    if (pst_channel->en_band != WLAN_BAND_5G) {
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "dfs no need on 2G channel\n");
        return OAL_FALSE;
    }

    /* 获取信道索引 */
    ul_ret = mac_get_channel_idx_from_num_etc(MAC_RC_START_FREQ_5, pst_channel->uc_chan_number, &uc_idx);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG2(0, OAM_SF_DFS, "get ch fail, band=%d ch=%d\n", MAC_RC_START_FREQ_5, pst_channel->uc_chan_number);
        return OAL_FALSE;
    }

    mac_get_ext_chan_info(uc_idx, pst_channel->en_bandwidth, &st_chan_info);

    for (uc_ch = 0; uc_ch < st_chan_info.ul_channels; uc_ch++) {
        /* 信道状态 */
        if ((pst_mac_device->st_ap_channel_list[st_chan_info.ast_channels[uc_ch].uc_idx].en_ch_status !=
            MAC_CHAN_DFS_REQUIRED) &&
            (pst_mac_device->st_ap_channel_list[st_chan_info.ast_channels[uc_ch].uc_idx].en_ch_status !=
            MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            uc_dfs_ch_cnt++;
        }
    }

    if (uc_dfs_ch_cnt == uc_ch) {
        OAM_WARNING_LOG2(0, OAM_SF_DFS, "all of the ch(pri_ch=%d,bw=%d) are not dfs channel,not need cac\n",
                         pst_channel->uc_chan_number, pst_channel->en_bandwidth);
        return OAL_FALSE;
    }

    /* CAC使能位 */
    if (OAL_FALSE == mac_dfs_get_cac_enable(pst_mac_device)) {
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "cac not enabled");
        return OAL_FALSE;
    }

    if (OAL_FAIL == hmac_dfs_go_cac_check(pst_mac_vap)) {
        g_go_cac = OAL_TRUE;
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "This time GO do not CAC,recover nexttime.");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE hmac_dfs_radar_result_stru *hmac_dfs_result_info_get(oal_void)
{
    return &g_st_dfs_result;
}

#endif /* end of _PRE_WLAN_FEATURE_DFS */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* end of hmac_dfs.h */
