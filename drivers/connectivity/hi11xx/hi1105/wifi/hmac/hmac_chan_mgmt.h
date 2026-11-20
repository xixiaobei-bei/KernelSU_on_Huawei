

#ifndef __HMAC_CHAN_MGMT_H__
#define __HMAC_CHAN_MGMT_H__

/* 1 其他头文件包含 */
#include "hmac_vap.h"
#include "mac_regdomain.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_H
/* 2 宏定义 */
#define HMAC_CHANNEL_SWITCH_COUNT 5

/* 3 枚举定义 */
typedef enum {
    HMAC_OP_ALLOWED = BIT0,
    HMAC_SCA_ALLOWED = BIT1,
    HMAC_SCB_ALLOWED = BIT2,
} hmac_chan_op_enum;

typedef enum {
    HMAC_NETWORK_SCA = 0,
    HMAC_NETWORK_SCB = 1,

    HMAC_NETWORK_BUTT,
} hmac_network_type_enum;

typedef enum {
    MAC_CHNL_AV_CHK_NOT_REQ = 0,  /* 不需要进行信道扫描 */
    MAC_CHNL_AV_CHK_IN_PROG = 1,  /* 正在进行信道扫描 */
    MAC_CHNL_AV_CHK_COMPLETE = 2, /* 信道扫描已完成 */

    MAC_CHNL_AV_CHK_BUTT,
} mac_chnl_av_chk_enum;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint16_t aus_num_networks[HMAC_NETWORK_BUTT];
    uint8_t en_chan_op;
    uint8_t auc_resv[3];  // 保留3字节
} hmac_eval_scan_report_stru;

typedef struct {
    uint8_t uc_idx;   /* 信道索引号 */
    uint16_t us_freq; /* 信道频点 */
    uint8_t auc_resv;
} hmac_dfs_channel_info_stru;

/* 频率信道索引映射关系 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, uint32_t change);


void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap,
                                        uint8_t uc_channel,
                                        wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap);
void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap);
uint32_t hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem);
void hmac_chan_sync(mac_vap_stru *pst_mac_vap,
                    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                    oal_bool_enum_uint8 en_switch_immediately);
void hmac_chan_update_40m_intol_user(mac_vap_stru *pst_mac_vap);

void hmac_40m_intol_sync_data(mac_vap_stru *pst_mac_vap,
                              wlan_channel_bandwidth_enum_uint8 en_40M_bandwidth,
                              oal_bool_enum_uint8 en_40M_intol_user);
void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
                                              uint8_t uc_channel,
                                              wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_check_ap_channel_follow_sta(mac_vap_stru *pst_check_mac_vap,
                                          const mac_channel_stru *pst_set_mac_channel,
                                          uint8_t *puc_ap_follow_channel);
uint16_t mac_get_center_freq1_from_freq_and_bandwidth(uint16_t freq,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);

/* 11 inline函数定义 */

OAL_STATIC OAL_INLINE void hmac_chan_initiate_switch_to_20mhz_ap(mac_vap_stru *pst_mac_vap)
{
    /* 设置VAP带宽模式为20MHz */
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = WLAN_BW_SWITCH_40_TO_20;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_scan_availability(mac_device_stru *pst_mac_device,
                                                                      mac_ap_ch_info_stru *pst_channel_info)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    if (OAL_FALSE == mac_dfs_get_dfs_enable(pst_mac_device)) {
        return OAL_TRUE;
    }

    return ((pst_channel_info->en_ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (pst_channel_info->en_ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR));
#else
    return OAL_TRUE;
#endif
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_is_ch_op_allowed(
    hmac_eval_scan_report_stru *pst_chan_scan_report, uint8_t uc_chan_idx)
{
    return (pst_chan_scan_report[uc_chan_idx].en_chan_op & HMAC_OP_ALLOWED);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_chan_mgmt.h */
