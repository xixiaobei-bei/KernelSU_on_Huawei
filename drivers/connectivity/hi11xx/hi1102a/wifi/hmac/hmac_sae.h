

#ifndef __HMAC_SAE_H__
#define __HMAC_SAE_H__

/* 1 其他头文件包含 */
#include "hmac_chan_mgmt.h"
#include "hmac_roam_connect.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SAE_H

/*  函数声明 */
#ifdef _PRE_WLAN_FEATURE_SAE
extern oal_uint32 hmac_config_external_auth(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_void hmac_update_sae_connect_param(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_ie, oal_uint32 ul_ie_len);
extern oal_void hmac_ap_up_rx_auth_req_to_host(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_auth_req);
extern oal_uint32 hmac_sta_process_sae_commit(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf);
extern oal_uint32 hmac_sta_process_sae_confirm(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf);
extern oal_bool_enum_uint8 hmac_check_illegal_sae_roam(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_bss_rsn_akm);
extern oal_uint32 hmac_roam_rx_auth_check_sae(hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *crx_event,
    oal_uint16 auth_status, oal_uint16 auth_seq_num);
extern oal_uint32 hmac_report_external_auth_req(hmac_vap_stru *pst_hmac_vap,
                                                oal_nl80211_external_auth_action en_action);
extern oal_void hmac_report_ext_auth_worker(oal_work_stru *pst_sae_report_ext_auth_worker);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
