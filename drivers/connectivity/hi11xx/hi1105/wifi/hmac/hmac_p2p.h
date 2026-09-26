

#ifndef __HMAC_P2P_H__
#define __HMAC_P2P_H__

#include "hmac_fsm.h"
#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_P2P_H

/* p2p 状态码 */
typedef enum {
    P2P_STATUS_DISCOVERY_ON = 0,
    P2P_STATUS_SEARCH_ENABLED,
    P2P_STATUS_IF_ADD,
    P2P_STATUS_IF_DEL,
    P2P_STATUS_IF_DELETING,
    P2P_STATUS_IF_CHANGING,
    P2P_STATUS_IF_CHANGED,
    P2P_STATUS_LISTEN_EXPIRED,
    P2P_STATUS_ACTION_TX_COMPLETED,
    P2P_STATUS_ACTION_TX_NOACK,
    P2P_STATUS_SCANNING,
    P2P_STATUS_GO_NEG_PHASE,
    P2P_STATUS_DISC_IN_PROGRESS
} hmac_cfgp2p_status_enum;
typedef uint32_t hmac_cfgp2p_status_enum_uint32;

/* P2P OPS 节能配置参数 */
/* 该结构体host与device共用,需保证结构体一致 */
typedef struct {
    uint8_t en_ops_ctrl;
    uint8_t uc_ct_window;
    uint8_t en_pause_ops;
    uint8_t auc_rsv[1];
} mac_cfg_p2p_ops_param_stru; /* hd_event */

uint32_t hmac_check_p2p_vap_num(mac_device_stru *mac_device, wlan_p2p_mode_enum_uint8 p2p_mode);
uint32_t hmac_p2p_send_listen_expired_to_host(hmac_vap_stru *hmac_vap);
uint32_t hmac_p2p_send_listen_expired_to_device(hmac_vap_stru *hmac_vap);
uint32_t hmac_add_p2p_cl_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_del_p2p_cl_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_p2p_check_can_enter_state(mac_vap_stru *mac_vap, hmac_fsm_input_type_enum input_req);
void hmac_disable_p2p_pm(hmac_vap_stru *hmac_vap);
oal_bool_enum_uint8 hmac_is_p2p_go_neg_req_frame(const uint8_t *data);
uint8_t hmac_get_p2p_status(uint32_t p2p_status, hmac_cfgp2p_status_enum_uint32 status);
void hmac_set_p2p_status(uint32_t *p2p_status, hmac_cfgp2p_status_enum_uint32 status);
void hmac_clr_p2p_status(uint32_t *p2p_status, hmac_cfgp2p_status_enum_uint32 status);
void hmac_p2p_sta_join_go_back_to_80m_handle(mac_vap_stru *sta_mac_vap);
mac_p2p_scenes_enum_uint8 hmac_p2p_get_scenes(mac_vap_stru *mac_vap);
uint32_t hmac_p2p_change_go_channel(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
void hmac_vap_join_avoid_dbac_back_to_11n_handle(mac_vap_stru *mac_vap, mac_bss_dscr_stru *bss_dscr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of hmac_p2p.h */
