

#ifndef __HMAC_MGMT_AP_H__
#define __HMAC_MGMT_AP_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_AP_H
/* 2 宏定义 */
#define is_wep_cipher(uc_cipher) \
    (((uc_cipher) == WLAN_80211_CIPHER_SUITE_WEP_40) || ((uc_cipher) == WLAN_80211_CIPHER_SUITE_WEP_104))
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_ap_up_rx_mgmt(hmac_vap_stru *pst_hmac_vap, void *p_param);
uint32_t hmac_ap_up_misc(hmac_vap_stru *pst_hmac_vap, void *p_param);
uint32_t hmac_ap_wait_start_rx_mgmt(hmac_vap_stru *pst_hmac_vap, void *p_param);
uint32_t hmac_ap_wait_start_misc(hmac_vap_stru *pst_hmac_vap, void *p_param);
uint32_t hmac_mgmt_timeout_ap(void *p_param);
void hmac_handle_disconnect_rsp_ap(hmac_vap_stru *pst_hmac_vap,
                                   hmac_user_stru *pst_hmac_user);
oal_bool_enum hmac_go_is_auth(mac_vap_stru *pst_mac_vap);
uint32_t hmac_ap_clean_bss(hmac_vap_stru *pst_hmac_vap);
uint32_t hmac_ap_send_assoc_rsp(hmac_vap_stru *pst_hmac_vap,
                                hmac_user_stru *pst_hmac_user,
                                const unsigned char *puc_sta_addr,
                                uint8_t uc_mgmt_frm_type,
                                mac_status_code_enum_uint16 *pen_status_code);
uint32_t hmac_ap_save_user_assoc_req(hmac_user_stru *pst_hmac_user, uint8_t *puc_payload,
                                     uint32_t payload_len, uint8_t uc_mgmt_frm_type);
void hmac_handle_connect_rsp_ap(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user);
void hmac_mgmt_update_auth_mib(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_auth_rsp);
void hmac_mgmt_reset_psm(mac_vap_stru *pst_vap, uint16_t us_user_id);

#endif /* end of hmac_mgmt_ap.h */
