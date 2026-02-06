

#ifndef __HMAC_SUPPORT_PMF_H__
#define __HMAC_SUPPORT_PMF_H__

/* 1 其他头文件包含 */
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SUPPORT_PMF_H

/*  函数声明 */
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
extern oal_uint32 hmac_sa_query_del_user(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user);
extern oal_uint32 hmac_send_sa_query_req(mac_vap_stru *pst_mac_vap,
                                         hmac_user_stru *pst_hmac_user,
                                         oal_bool_enum_uint8 en_is_protected,
                                         oal_uint16 us_init_trans_id);
extern oal_uint32 hmac_sa_query_interval_timeout(oal_void *p_arg);
extern oal_void hmac_send_sa_query_rsp(mac_vap_stru *pst_mac_vap,
                                       oal_uint8 *pst_hdr,
                                       oal_bool_enum_uint8 en_is_protected);
extern oal_uint32 hmac_start_sa_query(mac_vap_stru *pst_mac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      oal_bool_enum_uint8 en_is_protected);
extern oal_uint32 hmac_pmf_check_err_code(mac_user_stru *pst_user_base_info,
                                          oal_bool_enum_uint8 en_is_protected,
                                          oal_uint8 *puc_mac_hdr);
extern oal_void hmac_rx_sa_query_req(hmac_vap_stru *pst_hmac_vap,
                                     oal_netbuf_stru *pst_netbuf,
                                     oal_bool_enum_uint8 en_is_protected);
extern oal_void hmac_rx_sa_query_rsp(hmac_vap_stru *pst_hmac_vap,
                                     oal_netbuf_stru *pst_netbuf,
                                     oal_bool_enum_uint8 en_is_protected);
extern oal_void hmac_sta_roam_update_pmf(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr);
extern oal_void hmac_sta_up_category_sa_query(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 en_is_protected, oal_uint8 *puc_data);
extern oal_void hmac_stop_sa_query_timer(hmac_user_stru *pst_hmac_user);
extern oal_uint32 hmac_check_sa_query_trigger(
    hmac_user_stru *pst_hmac_user, hmac_vap_stru *pst_hmac_vap, mac_status_code_enum_uint16 *pen_status_code);
extern oal_void hmac_ap_up_category_sa_query(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf, oal_bool_enum_uint8 en_is_protected);
extern oal_uint32 hmac_get_assoc_comeback_time(mac_vap_stru *pst_mac_vap,
                                               hmac_user_stru *pst_hmac_user);
extern oal_bool_enum_uint8 hmac_device_pmf_find_black_list(hmac_device_stru *pst_hmac_dev, oal_uint8 *puc_mac_addr);
extern oal_void hmac_device_pmf_add_black_list(hmac_device_stru *pst_hmac_dev, oal_uint8 *puc_mac_addr);
extern oal_void hmac_pmf_update_pmf_black_list(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr,
                                               mac_nl80211_mfp_enum_uint8 en_conn_pmf_enable);
extern oal_uint32 hmac_config_set_pmf_cap(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pmf_capable,
                                          oal_bool_enum_uint8 en_pmf_required);
extern oal_uint32 hmac_enable_pmf(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_param);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif