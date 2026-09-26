

#ifndef __HMAC_MGMT_JOIN_H__
#define __HMAC_MGMT_JOIN_H__

/* 1 其他头文件包含 */
#include "hmac_mgmt_sta.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_JOIN_H

/*  函数声明 */
extern oal_uint32  hmac_mgmt_timeout_sta(oal_void *p_arg);
extern oal_void hmac_update_join_req_params_prot_sta(hmac_vap_stru *pst_hmac_vap,
                                                     hmac_join_req_stru *pst_join_req);
extern oal_bool_enum_uint8  hmac_is_support_11grate(oal_uint8 *puc_rates, oal_uint8 uc_rate_num);
extern oal_bool_enum_uint8  hmac_is_support_11brate(oal_uint8 *puc_rates, oal_uint8 uc_rate_num);
extern oal_uint32 hmac_sta_get_user_protocol(
    mac_bss_dscr_stru *pst_bss_dscr, wlan_protocol_enum_uint8  *pen_protocol_mode);
extern oal_uint32  hmac_sta_wait_join(hmac_vap_stru *pst_sta, oal_void *pst_msg);
extern oal_uint32  hmac_sta_wait_join_timeout(hmac_vap_stru *pst_sta, oal_void *pst_msg);
extern oal_void hmac_set_auth_assoc_send_status_report_flag(
    hmac_vap_stru *pst_hmac_vap, oal_bool_enum_uint8 en_status);
extern oal_uint32  hmac_sta_wait_auth(hmac_vap_stru *pst_sta, oal_void *pst_msg);
extern oal_uint32  hmac_sta_wait_beacon_before_auth(hmac_vap_stru *pst_hmac_sta, oal_void *pst_msg);
extern oal_uint32  hmac_sta_wait_auth_seq2_rx(hmac_vap_stru *pst_sta, oal_void *pst_msg);
extern oal_uint32  hmac_sta_wait_auth_seq4_rx(hmac_vap_stru *pst_sta, oal_void *p_msg);
extern oal_uint32  hmac_sta_wait_asoc(hmac_vap_stru *pst_sta, oal_void *pst_msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

