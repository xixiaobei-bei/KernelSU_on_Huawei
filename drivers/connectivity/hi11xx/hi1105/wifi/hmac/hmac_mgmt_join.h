

#ifndef __HMAC_MGMT_JOIN_H__
#define __HMAC_MGMT_JOIN_H__

#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_fsm.h"

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_JOIN_H
/* 加入请求参数 */
typedef struct {
    mac_bss_dscr_stru st_bss_dscr; /* 要加入的bss网络 */
} hmac_join_req_stru;

/* 认证请求参数 */
typedef struct {
    uint16_t us_timeout;
    uint8_t auc_resv[BYTE_OFFSET_2];
} hmac_auth_req_stru;

/* 关联请求参数 */
typedef struct {
    uint16_t us_assoc_timeout;
    uint8_t auc_resv[BYTE_OFFSET_2];
} hmac_asoc_req_stru;

/* 加入结果 */
typedef struct {
    hmac_mgmt_status_enum_uint8 en_result_code;
    uint8_t auc_resv[BYTE_OFFSET_3];
} hmac_join_rsp_stru;

/* 认证结果 */
typedef struct {
    uint8_t auc_peer_sta_addr[WLAN_MAC_ADDR_LEN]; /* mesh下peer station的地址 */
    uint16_t us_status_code;                      /* 认证结果 */
} hmac_auth_rsp_stru;

typedef struct {
    uint64_t ull_cookie;
    uint32_t listen_duration;                            /* 监听时间   */
    uint8_t uc_listen_channel;                              /* 监听的信道 */
    wlan_channel_bandwidth_enum_uint8 en_listen_channel_type; /* 监听信道类型 */
    uint8_t uc_home_channel;                                /* 监听结束返回的信道 */
    wlan_channel_bandwidth_enum_uint8 en_home_channel_type;   /* 监听结束，返回主信道类型 */
    /* P2P0和P2P_CL 公用VAP 结构，保存进入监听前VAP 的状态，便于监听结束时恢复该状态 */
    mac_vap_state_enum_uint8 en_last_vap_state;
    wlan_channel_band_enum_uint8 en_band;
    wlan_ieee80211_roc_type_uint8 en_roc_type; /* roc类型 */
    uint8_t uc_rev;
    oal_ieee80211_channel_stru st_listen_channel;
} hmac_remain_on_channel_param_stru; // only host

uint32_t hmac_mgmt_timeout_sta(void *arg);
void hmac_update_join_req_params_2040(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr);
void hmac_update_join_req_params_prot_sta(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req);
void hmac_sta_bandwidth_down_by_channel(mac_vap_stru *mac_vap);
oal_bool_enum_uint8 hmac_is_rate_support(uint8_t *rates, uint8_t rate_num, uint8_t rate);
oal_bool_enum_uint8 hmac_is_support_11grate(uint8_t *rates, uint8_t rate_num);
oal_bool_enum_uint8 hmac_is_support_11brate(uint8_t *rates, uint8_t rate_num);
void hmac_sta_get_user_protocol_by_sup_rate(mac_bss_dscr_stru *bss_dscr,
    wlan_protocol_enum_uint8 *protocol_mode);
void hmac_sta_get_user_protocol(mac_bss_dscr_stru *bss_dscr,
    wlan_protocol_enum_uint8 *protocol_mode);
uint32_t hmac_sta_get_user_protocol_etc(mac_bss_dscr_stru *bss_dscr,
    wlan_protocol_enum_uint8 *protocol_mode);
uint32_t hmac_sta_wait_join(hmac_vap_stru *pst_sta, void *pst_msg);
uint32_t hmac_sta_wait_join_rx(hmac_vap_stru *hmac_sta, void *param);
uint32_t hmac_sta_wait_join_timeout(hmac_vap_stru *pst_hmac_sta, void *pst_msg);
uint32_t hmac_sta_wait_join_misc(hmac_vap_stru *pst_hmac_sta, void *pst_msg);
uint32_t hmac_sta_wait_auth(hmac_vap_stru *pst_sta, void *pst_msg);
uint32_t hmac_sta_wait_auth_seq2_rx(hmac_vap_stru *pst_sta, void *pst_msg);
uint32_t hmac_sta_wait_auth_seq4_rx(hmac_vap_stru *pst_sta, void *p_msg);
uint32_t hmac_sta_wait_asoc(hmac_vap_stru *pst_sta, void *pst_msg);
uint32_t hmac_p2p_listen_timeout(hmac_vap_stru *pst_hmac_vap_sta, void *p_param);
uint32_t hmac_p2p_remain_on_channel(hmac_vap_stru *pst_hmac_vap_sta, void *p_param);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of hmac_nrcoex.h */
